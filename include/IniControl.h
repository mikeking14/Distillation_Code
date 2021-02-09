#include <IniFile.h>

void readSettings();
void readDefault();
void getValue(const char *key);
void setValue(const char *endMarker, const size_t bufferLen, const char *value);

char defaultText[] = "#  This is the default .ini file DO NOT EDIT, changes to this file will be reverted on startup.\n#  This file is checked if the settings.ini file is corrupted or missing.\n#  Changes should be made to settings.ini\n\ndate = 2021:01:31\nlocalTime = 23:59:59\nrunNumber = 1";
char settingsText[] = "#  User settings\n\ndate = 2021:01:31\nlocalTime = 00:00:00\nrunNumber = 1";
const size_t bufferLen = 128;
char buffer[bufferLen];
IniFile settingsINI("settings.ini");
IniFile defaultINI("default.ini");

void readSettings()
{
    // Restore modifications or deletion of default.ini
    if (SD.exists("default.ini"))
        SD.remove("default.ini");

    File iniFile = SD.open("default.ini", FILE_WRITE);
    iniFile.println(defaultText);
    iniFile.close();

    // Creates directory for storing data logs if it does not exist
    if (!SD.exists("DataLogs/"))
        SD.mkdir("DataLogs/");

    // Check that settings.ini is available
    if (!settingsINI.open() || !settingsINI.validate(buffer, bufferLen))
    {
        Serial.println("settings.ini failed to open\nAttempting to open default.ini");
        readDefault();
    }
    else
    {
        // Get values from settings.ini
        getValue("date");
        getValue("localTime");
        getValue("runNumber");
    }

    // Update values in settings.ini
    setValue("date", 4, date);
    setValue("localTime", 9, localTime);

    char tempVal[3];
    sprintf(tempVal, "%d", (runNumber + 1));
    setValue("runNumber", 9, tempVal);

    // Set variables based on .ini values
    sprintf(dataLogTXT, "DATALOGS/run_%d_Data.txt", runNumber);
}

void readDefault()
{
    if (!defaultINI.open())
    {
        Serial.println("default.ini failed to open\nPlease restart your device.");
        while (1)
        {
            ;
        }
    }

    defaultINI.getValue(NULL, "date", buffer, bufferLen);
    strcpy(date, buffer);
    memset(buffer, 0, bufferLen);

    defaultINI.getValue(NULL, "localTime", buffer, bufferLen);
    strcpy(localTime, buffer);
    memset(buffer, 0, bufferLen);

    defaultINI.getValue(NULL, "runNumber", buffer, bufferLen);
    runNumber = atoi(buffer);
    memset(buffer, 0, bufferLen);

    defaultINI.close();
}

void getValue(const char *key)
{
    if (!settingsINI.open())
        Serial.println("settings.ini failed to open.");

    if (settingsINI.getValue(NULL, key, buffer, bufferLen))
    {
        strcpy(date, buffer);
    }
    else
    {
        Serial.println("Key not found, using default value.");
        defaultINI.getValue(NULL, key, buffer, bufferLen);
        strcpy(date, buffer);
    }
    settingsINI.close();
}

void setValue(const char *endMarker, const size_t bufferLen, const char *value)
{
    bool keyFound = false;
    char keyBuffer[bufferLen];
    char beforeKey[128];
    char afterKey[128];
    size_t count = 0;
    File settingsRW = SD.open("settings.ini", FILE_READ);

    while (settingsRW.available() > 0 && !keyFound)
    {
        if (count < bufferLen)
        {
            keyBuffer[count] = settingsRW.read();
            beforeKey[count] = keyBuffer[count];
            count++;
        }
        else
        {
            for (size_t i = 0; i < bufferLen - 1; i++)
                {
                    keyBuffer[i] = keyBuffer[i + 1];
                }
                keyBuffer[bufferLen - 1] = settingsRW.read();
                beforeKey[count] = keyBuffer[bufferLen - 1];
                count++;

            if (strcmp(endMarker, keyBuffer))
                keyFound = true;
        }
    }

    count = 0;
    settingsRW.readBytesUntil('\n', keyBuffer, bufferLen);
    while (settingsRW.available() > 0)
    {
        afterKey[count] = settingsRW.read();
        count++;
    }
    settingsRW.close();

    if (SD.exists("settings.ini"))
        SD.remove("settings.ini");

    settingsRW = SD.open("settings.ini", FILE_WRITE);
    //settingsRW.println(beforeKey);
    settingsRW.print(beforeKey);
    //settingsRW.print(endMarker);
    settingsRW.print(" = ");
    settingsRW.println(value);
    settingsRW.print(afterKey);
    settingsRW.close();
}