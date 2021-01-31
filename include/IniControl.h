#include <IniFile.h>

void readSettings();
void readDefault();
void getValue(const char *key);
void setValue(const char* endMarker, const size_t bufferLen, const char* value);

char defaultText[] = "#  This is the default .ini file DO NOT EDIT, changes to this file will be reverted on startup.\n#  This file is checked if the settings.ini file is corrupted or missing.\n#  Changes should be made to settings.ini\n\ndate = 2021:01:31\nlocalTime = 23:59:59\nrunNumber = 1";
char settingsText[] = "#  User settings\n\ndate = 2021:01:31\nlocalTime = 00:00:00\nrunNumber = 1";
const size_t bufferLen = 128;
char buffer[bufferLen];
IniFile settingsINI("settings.ini");
IniFile defaultINI("default.ini");

void readSettings()
{
    // Restore modifications or deletion of default.ini
    SD.remove("default.ini");
    File iniFile = SD.open("default.ini", FILE_WRITE);
    iniFile.println(defaultText);
    iniFile.close();
    // Creates directory for storing data logs if it does not exist
    if(!SD.exists("DataLogs/"))
        SD.mkdir("DataLogs/");

    // Check that settings.ini is available
    if (!settingsINI.open())
    {
        Serial.println("settings.ini failed to open\nAttempting to open default.ini");
        readDefault();
    }
    //Check settings.ini doesn't have any values too long to read
    if (!settingsINI.validate(buffer, bufferLen))
    {
        Serial.println("ERROR: Buffer overflow in settings.ini\nUsing default.ini");
        readDefault();
    }

    // Get values from settings.ini
    getValue("date");
    getValue("localTime");
    getValue("runNumber");

    // Update values in settings.ini
    char tempVal[3];
    sprintf(tempVal, "%d", (runNumber+1));
    setValue("runNumber", 4, tempVal);
}

void readDefault()
{
    defaultINI.getValue(NULL, "date", buffer, bufferLen);
    strcpy(date, buffer);

    defaultINI.getValue(NULL, "localTime", buffer, bufferLen);
    strcpy(localTime, buffer);

    defaultINI.getValue(NULL, "runNumber", buffer, bufferLen);
    runNumber = atoi(buffer);
}

void getValue(const char *key)
{
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
}

void setValue(const char* endMarker, const size_t bufferLen, const char* value)
{
    static byte ndx = 0;
    char beforeKey[128];
    char afterKey[128];
    char inputBuffer[bufferLen];
    bool keyFound = false;
    File settingsRW = SD.open("settings.ini", FILE_READ);

    while (settingsRW.available() > 0 && !keyFound)
    {
        settingsRW.read(inputBuffer, bufferLen);
        if (strcmp(endMarker, inputBuffer))
        {
            keyFound = true;
            settingsRW.readBytesUntil('\n', inputBuffer, 4);
            settingsRW.read();
        }
        else
        {
            for (size_t i = 0; i < bufferLen; i++)
            {
                beforeKey[ndx + i] = inputBuffer[i];
            }
            ndx += bufferLen;
        }
    }

    ndx = 0;
    while (settingsRW.available() > 0)
    {
        afterKey[ndx] = settingsRW.read();
    }

    settingsRW.close();
    SD.remove("settings.ini");
    settingsRW = SD.open("settings.ini", FILE_WRITE);
    settingsRW.println(beforeKey);
    settingsRW.print(endMarker);
    settingsRW.print(" = ");
    settingsRW.println(value);
    settingsRW.println(afterKey);
    settingsRW.close();
}