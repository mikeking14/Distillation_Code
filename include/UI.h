#include <SPI.h>
#include <Wire.h>
#include <ILI9341_t3.h>
#include <XPT2046_Touchscreen.h>
#include <cmath>
#include <SensorControl.h>
#include <IniControl.h>

using std::abs;

// Touch calibrations
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000
#define MINPRESSURE 10
#define MAXPRESSURE 1000

// Buttone dimensions
#define FRAME_X 250
#define FRAME_Y 40
#define FRAME_W 100
#define FRAME_H 50
#define BUTTON_TEXT_SZ 2
#define LABEL_TEXT_SZ 3

// Pin declarations
#define CS_PIN 8
#define TIRQ_PIN 2 // TIRQ interrupt for touch detect (faster than checking every loop)
#define TFT_CS 10
#define TFT_DC 9
XPT2046_Touchscreen touch(CS_PIN, TIRQ_PIN);
ILI9341_t3 display = ILI9341_t3(TFT_CS, TFT_DC);

// Main Menu Buttons
Adafruit_GFX_Button backB;
Adafruit_GFX_Button dataB;
Adafruit_GFX_Button settingsB;
Adafruit_GFX_Button startRunB;
// Data Page Buttons
// Settings Page Buttons
Adafruit_GFX_Button runIncrementB;
Adafruit_GFX_Button runDecrementB;
Adafruit_GFX_Button runB;
Adafruit_GFX_Button dayB;
Adafruit_GFX_Button monthB;
Adafruit_GFX_Button yearB;
Adafruit_GFX_Button dateIncrementB;
Adafruit_GFX_Button dateDecrementB;
Adafruit_GFX_Button hourB;
Adafruit_GFX_Button minuteB;
Adafruit_GFX_Button secondB;
Adafruit_GFX_Button timeIncrementB;
Adafruit_GFX_Button timeDecrementB;
// Run In Progress Page Buttons
Adafruit_GFX_Button pauseRunB;
Adafruit_GFX_Button resumeRunB;
Adafruit_GFX_Button stopRunB;
Adafruit_GFX_Button viewDataB;

int day = 31, month = 12, year = 21;
int hour = 23, minute = 59, second = 59;
int sCount = 0, ssCount = 0, settingsRunNumber = 1;
bool runStarted = false, runPaused = false, runStopped = true;
bool methanolPresent = true;
char tempString[10];
elapsedMillis displayTimeout;
elapsedMillis refreshTimeout;
elapsedMillis clockTick;

enum MenuPage
{
    mainP,
    dataP,
    settingsP,
    screenSaverP,
    runP
};
MenuPage currentPage = mainP; // Set default page as mainP

enum SettingsButtonFocus
{
    runFocus,
    dayFocus,
    monthFocus,
    yearFocus,
    hourFocus,
    minuteFocus,
    secondFocus,
    noFocus
};
SettingsButtonFocus focusedButton = noFocus;

void mainPage();
void dataPage();
void settingsPage();
void settingsButtonHelper(int x, int y, int crement);
void focusButton(Adafruit_GFX_Button *button, SettingsButtonFocus focus, int labelVal);
void screensaverPage();
void runPage();
void updateTime();

void setupUI(void)
{
    Serial.begin(9600);
    display.begin();
    touch.begin();

    display.fillScreen(ILI9341_WHITE);
    // origin = left,top landscape (USB left upper)
    display.setRotation(3);
    touch.setRotation(4);

    // Main Menu Buttons
    backB.initButton(&display, 250, 40, FRAME_W, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "Back", BUTTON_TEXT_SZ); // Used globally
    dataB.initButton(&display, 250, 110, FRAME_W, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "Data", BUTTON_TEXT_SZ);
    settingsB.initButton(&display, 250, 180, FRAME_W, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "Settings", BUTTON_TEXT_SZ);
    startRunB.initButton(&display, 95, 161, 1.75 * FRAME_W, 1.75 * FRAME_H, ILI9341_BLACK, ILI9341_RED, ILI9341_BLACK, "Start Run", LABEL_TEXT_SZ);
    // Data Page Buttons
    // Settings Page Buttons
    runIncrementB.initButton(&display, 170, 100, FRAME_W / 4, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "+", BUTTON_TEXT_SZ);
    runDecrementB.initButton(&display, 30, 100, FRAME_W / 4, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "-", BUTTON_TEXT_SZ);
    sprintf(tempString, "RUN %2d", settingsRunNumber);
    runB.initButton(&display, 100, 100, FRAME_W, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, tempString, BUTTON_TEXT_SZ);
    sprintf(tempString, "%2d", day);
    dayB.initButton(&display, 65, 155, FRAME_W / 3, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, tempString, BUTTON_TEXT_SZ);
    sprintf(tempString, "%2d", month);
    monthB.initButton(&display, 100, 155, FRAME_W / 3, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, tempString, BUTTON_TEXT_SZ);
    sprintf(tempString, "%2d", year);
    yearB.initButton(&display, 135, 155, FRAME_W / 3, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, tempString, BUTTON_TEXT_SZ);
    dateIncrementB.initButton(&display, 170, 155, FRAME_W / 4, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "+", BUTTON_TEXT_SZ);
    dateDecrementB.initButton(&display, 30, 155, FRAME_W / 4, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "-", BUTTON_TEXT_SZ);
    sprintf(tempString, "%2d", hour);
    hourB.initButton(&display, 65, 210, FRAME_W / 3, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, tempString, BUTTON_TEXT_SZ);
    sprintf(tempString, "%2d", minute);
    minuteB.initButton(&display, 100, 210, FRAME_W / 3, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, tempString, BUTTON_TEXT_SZ);
    sprintf(tempString, "%2d", second);
    secondB.initButton(&display, 135, 210, FRAME_W / 3, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, tempString, BUTTON_TEXT_SZ);
    timeIncrementB.initButton(&display, 170, 210, FRAME_W / 4, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "+", BUTTON_TEXT_SZ);
    timeDecrementB.initButton(&display, 30, 210, FRAME_W / 4, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "-", BUTTON_TEXT_SZ);

    // Run In Progress Page Buttons
    pauseRunB.initButton(&display, 250, 92, FRAME_W, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "Pause", BUTTON_TEXT_SZ);
    resumeRunB.initButton(&display, 250, 144, FRAME_W, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "Resume", BUTTON_TEXT_SZ);
    stopRunB.initButton(&display, 250, 196, FRAME_W, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "Stop", BUTTON_TEXT_SZ);
    viewDataB.initButton(&display, 100, 196, 1.70 * FRAME_W, FRAME_H, ILI9341_BLACK, ILI9341_CYAN, ILI9341_BLACK, "View Data", BUTTON_TEXT_SZ);

    mainPage();
}

void mainPage()
{
    if (runStarted)
        startRunB.relabelButton("View Run", ILI9341_GREEN);
    else
        startRunB.relabelButton("Start Run", ILI9341_RED);

    display.fillScreen(ILI9341_WHITE);
    backB.drawButton();
    dataB.drawButton();
    settingsB.drawButton();
    startRunB.drawButton();

    display.setCursor(20, 40);
    display.setTextColor(ILI9341_BLACK);
    display.setTextSize(LABEL_TEXT_SZ);
    display.print("Main Menu");
    display.drawLine(20, 65, 180, 65, ILI9341_BLACK);

    currentPage = mainP;
}

void dataPage()
{

    display.fillScreen(ILI9341_WHITE);
    backB.drawButton();

#define w 50
#define h 30
    display.setTextSize(BUTTON_TEXT_SZ);

    // Time
    display.setCursor(10, 10);
    display.print("Time - ");
    sprintf(tempString, "%2d:%2d:%2d", hour, minute, second);
    display.print(tempString);

    // Mass Frequency
    display.setCursor(10, 50);
    display.print("Mass [kg]");
    display.drawRect(150 - (w / 2), 50 - (h / 2), w + 15, h, ILI9341_BLACK);
    sprintf(tempString, "%3.1f", mass);
    display.setCursor(150 + 7 - strlen(tempString) * 3 * BUTTON_TEXT_SZ, 50 - 4 * BUTTON_TEXT_SZ);
    display.print(tempString);

    // Cap Frequency
    display.setCursor(10, 90);
    display.print("Cap Freq.");
    display.drawRect(150 - (w / 2), 90 - (h / 2), w * 2, h, ILI9341_BLACK);
    sprintf(tempString, "%4.1ld", freqCap);
    display.setCursor(150 + (w / 4) - strlen(tempString) * 3 * BUTTON_TEXT_SZ, 90 - 4 * BUTTON_TEXT_SZ);
    display.print(tempString);

    // Resistance Frequency
    display.setCursor(10, 130);
    display.print("Res Freq.");
    display.drawRect(150 - (w / 2), 130 - (h / 2), w * 2, h, ILI9341_BLACK);
    sprintf(tempString, "%4.1ld", freqRes);
    display.setCursor(150 + (w / 4) - strlen(tempString) * 3 * BUTTON_TEXT_SZ, 130 - 4 * BUTTON_TEXT_SZ);
    display.print(tempString);

    // Temperature
    display.setCursor(10, 170);
    display.print("Temp [C]");
    display.drawRect(150 - (w / 2), 170 - (h / 2), w * 2, h, ILI9341_BLACK);
    sprintf(tempString, "%6.1f", tempOutlet);
    display.setCursor(150 + (w / 2) - strlen(tempString) * 3 * BUTTON_TEXT_SZ, 170 - 4 * BUTTON_TEXT_SZ);
    display.print(tempString);

    // PID Values
    display.setCursor(10, 210);
    display.print("P|I|D");

    // ----- P -----
    display.drawRect(150 - (w / 2), 210 - (h / 2), w, h, ILI9341_BLACK);
    sprintf(tempString, "%2.1f", kp);
    display.setCursor(150 - strlen(tempString) * 3 * BUTTON_TEXT_SZ, 210 - 4 * BUTTON_TEXT_SZ);
    display.print(tempString);

    // ----- I -----
    display.drawRect(200 - (w / 2), 210 - (h / 2), w, h, ILI9341_BLACK);
    sprintf(tempString, "%2.1f", ki);
    display.setCursor(200 - strlen(tempString) * 3 * BUTTON_TEXT_SZ, 210 - 4 * BUTTON_TEXT_SZ);
    display.print(tempString);

    // ----- D -----
    display.drawRect(250 - (w / 2), 210 - (h / 2), w, h, ILI9341_BLACK);
    sprintf(tempString, "%2.1f", kd);
    display.setCursor(250 - strlen(tempString) * 3 * BUTTON_TEXT_SZ, 210 - 4 * BUTTON_TEXT_SZ);
    display.print(tempString);

    currentPage = dataP;
}

void settingsPage()
{
    display.fillScreen(ILI9341_WHITE);
    display.setTextColor(ILI9341_BLACK);
    display.setTextSize(LABEL_TEXT_SZ);
    display.setCursor(20, 40);
    display.print("Settings");
    display.drawLine(20, 65, 180, 65, ILI9341_BLACK);

    backB.drawButton();

    runDecrementB.drawButton();
    runB.drawButton();
    runIncrementB.drawButton();
    display.setCursor(210, 90);
    display.print("Run No.");
    display.setCursor(210, 145);
    display.print("dd/mm/yy");
    display.setCursor(210, 195);
    display.print("hh:mm:ss");

    dateDecrementB.drawButton();
    dayB.drawButton();
    monthB.drawButton();
    yearB.drawButton();
    dateIncrementB.drawButton();

    timeDecrementB.drawButton();
    hourB.drawButton();
    minuteB.drawButton();
    secondB.drawButton();
    timeIncrementB.drawButton();

    currentPage = settingsP;
}

// Used to update a setting (e.g. increment or decrement value)
void settingsButtonHelper(int x, int y, int crement)
{
    switch (focusedButton)
    {
    case runFocus:
        settingsRunNumber = abs((settingsRunNumber + crement) % 100);
        sprintf(tempString, "RUN %2d", settingsRunNumber);
        runB.relabelButton(tempString, ILI9341_ORANGE);
        break;

    case dayFocus:
        day = abs((day + crement) % 32);
        day += day ? 0 : 1;
        sprintf(tempString, "%2d", day);
        dayB.relabelButton(tempString, ILI9341_ORANGE);
        break;

    case monthFocus:
        month = abs((month + crement) % 13);
        month += month ? 0 : 1;
        sprintf(tempString, "%2d", month);
        monthB.relabelButton(tempString, ILI9341_ORANGE);
        break;

    case yearFocus:
        year = abs((year + crement) % 100);
        sprintf(tempString, "%2d", year);
        yearB.relabelButton(tempString, ILI9341_ORANGE);
        break;

    case hourFocus:
        hour = abs((hour + crement) % 24);
        sprintf(tempString, "%2d", hour);
        hourB.relabelButton(tempString, ILI9341_ORANGE);
        break;

    case minuteFocus:
        minute = abs((minute + crement) % 60);
        sprintf(tempString, "%2d", minute);
        minuteB.relabelButton(tempString, ILI9341_ORANGE);
        break;

    case secondFocus:
        second = abs((second + crement) % 60);
        sprintf(tempString, "%2d", second);
        secondB.relabelButton(tempString, ILI9341_ORANGE);
        break;

    case noFocus:
        break;
    }
}

// Used by settingsButtonHelper (or others) to focus a button so the text/colour can be updated on it
void focusButton(Adafruit_GFX_Button *button, SettingsButtonFocus focus, int labelVal)
{
    switch (focusedButton)
    {
    case runFocus:
        sprintf(tempString, "RUN %2d", settingsRunNumber);
        runB.relabelButton(tempString, ILI9341_LIGHTGREY);
        break;

    case dayFocus:
        sprintf(tempString, "%2d", day);
        dayB.relabelButton(tempString, ILI9341_LIGHTGREY);
        break;

    case monthFocus:
        sprintf(tempString, "%2d", month);
        monthB.relabelButton(tempString, ILI9341_LIGHTGREY);
        break;

    case yearFocus:
        sprintf(tempString, "%2d", year);
        yearB.relabelButton(tempString, ILI9341_LIGHTGREY);
        break;

    case hourFocus:
        sprintf(tempString, "%2d", hour);
        hourB.relabelButton(tempString, ILI9341_LIGHTGREY);
        break;

    case minuteFocus:
        sprintf(tempString, "%2d", minute);
        minuteB.relabelButton(tempString, ILI9341_LIGHTGREY);
        break;

    case secondFocus:
        sprintf(tempString, "%2d", second);
        secondB.relabelButton(tempString, ILI9341_LIGHTGREY);
        break;

    case noFocus:
        break;
    }

    focusedButton = (focusedButton == focus) ? noFocus : focus;
    if (focusedButton == runFocus)
        sprintf(tempString, "RUN %2d", labelVal);
    else
        sprintf(tempString, "%2d", labelVal);

    if (focusedButton != noFocus)
        button->relabelButton(tempString, ILI9341_ORANGE);
}

void screensaverPage()
{
    switch (ssCount++)
    {
    case 0:
        display.fillScreen(ILI9341_BLACK);
        break;

    case 1:
        display.fillScreen(ILI9341_RED);
        break;

    case 2:
        display.fillScreen(ILI9341_GREEN);
        break;

    case 3:
        display.fillScreen(ILI9341_BLUE);
        break;

    case 4:
        display.fillScreen(ILI9341_WHITE);
        ssCount = 0;
        break;
    }

    currentPage = screenSaverP;
    displayTimeout = 0;
}

void runPage()
{
    display.fillScreen(ILI9341_WHITE);

    if (runPaused)
        pauseRunB.relabelButton("Pause", ILI9341_ORANGE);
    else
        pauseRunB.relabelButton("Pause", ILI9341_LIGHTGREY);

    if (runStopped)
        stopRunB.relabelButton("Stop", ILI9341_RED);
    else
        stopRunB.relabelButton("Stop", ILI9341_LIGHTGREY);

    viewDataB.drawButton();
    pauseRunB.drawButton();
    resumeRunB.drawButton();
    stopRunB.drawButton();
    backB.drawButton();

    display.setCursor(10, 43);
    display.print("Cap Freq:");
    display.drawRect(150 - (w / 2), 48 - (h / 2), w, h, ILI9341_BLACK);
    sprintf(tempString, "%2.1ld", freqCap);
    display.setCursor(150 - strlen(tempString) * 3 * BUTTON_TEXT_SZ, 48 - 4 * BUTTON_TEXT_SZ);
    display.print(tempString);

    display.setCursor(10, 75);
    display.print("Res Freq:");
    display.drawRect(150 - (w / 2), 80 - (h / 2), w, h, ILI9341_BLACK);
    sprintf(tempString, "%2.1ld", freqRes);
    display.setCursor(150 - strlen(tempString) * 3 * BUTTON_TEXT_SZ, 80 - 4 * BUTTON_TEXT_SZ);
    display.print(tempString);

    display.setCursor(10, 108);
    display.print("Temp:");
    display.drawRect(100 - (w / 2), 113 - (h / 2), w * 2, h, ILI9341_BLACK);
    sprintf(tempString, "%2.1f", tempOutlet);
    display.setCursor(100 + (w / 2) - strlen(tempString) * 3 * BUTTON_TEXT_SZ, 113 - 4 * BUTTON_TEXT_SZ);
    display.print(tempString);

#define r 10
    display.setCursor(10, 140);
    display.print("Meth Present:");
    if (!methanolPresent)
    {
        display.fillCircle(165, 145, r, ILI9341_GREEN);
        display.drawCircle(165, 145, r, ILI9341_BLACK);
    }
    else if (methanolPresent)
    {
        display.fillCircle(165, 145, r, ILI9341_RED);
        display.drawCircle(165, 145, r, ILI9341_BLACK);
    }
    else
    {
        display.fillCircle(165, 145, r, ILI9341_ORANGE);
        display.drawCircle(165, 145, r, ILI9341_BLACK);
    }
    sprintf(tempString, "%2.1f", kp);

    display.setCursor(10, 10);
    display.print("Time - ");
    sprintf(tempString, "%2d:%2d:%2d", hour, minute, second);
    display.print(tempString);

    currentPage = runP;
}

// This is naive and will lose time quickly, should update to something more stable going foreward.
void updateTime()
{
    second = (second + 1) % 60;

    if (second == 0)
        minute = (minute + 1) % 60;

    if (minute == 0 && second == 0)
        hour = (hour + 1) % 24;
}

void UI()
{
    // Cycles through screensaver and activates screensaver after 32 seconds of inactivity.
    if (currentPage == screenSaverP && displayTimeout > 2000)
        screensaverPage();
    else if (displayTimeout > 32000)
        screensaverPage();

    if (currentPage != settingsP && clockTick >= 1000)
    {
        updateTime();
        clockTick = 0;
        sCount++;
    }

    if (sCount >= 10)
    {
        if (currentPage == runP)
            runPage();
        else if (currentPage == dataP)
            dataPage();
        sCount = 0;
    }

    // Enforces at least 200 ms between touch events
    if (refreshTimeout > 200)
    {
        if (touch.tirqTouched()) // Trigger touch event on TIRQ interrupt
            if (touch.touched())
            {
                // Retrieve a point
                TS_Point point = touch.getPoint();

                // See if there's any touch dataP for us
                if (point.z > MINPRESSURE && point.z < MAXPRESSURE)
                {
                    // Scale using the calibration #'s and rotate coordinate system
                    point.x = map(point.x, TS_MINY, TS_MAXY, 0, display.height());
                    point.y = map(point.y, TS_MINX, TS_MAXX, 0, display.width());
                    int y = display.height() - point.x;
                    int x = point.y;

                    // Responds to touch event based on current page
                    switch (currentPage)
                    {
                    case mainP:
                        if (backB.contains(x, y))
                            screensaverPage();
                        else if (dataB.contains(x, y))
                            dataPage();
                        else if (settingsB.contains(x, y))
                            settingsPage();
                        else if (startRunB.contains(x, y))
                        {
                            runStarted = true;
                            runPage();
                        }
                        break;

                    case dataP:
                        if (backB.contains(x, y))
                            mainPage();
                        break;

                    case settingsP:
                        if (runIncrementB.contains(x, y))
                            settingsButtonHelper(x, y, 1);
                        else if (runDecrementB.contains(x, y))
                            settingsButtonHelper(x, y, -1);
                        else if (runB.contains(x, y))
                            focusButton(&runB, runFocus, settingsRunNumber);
                        else if (dayB.contains(x, y))
                            focusButton(&dayB, dayFocus, day);
                        else if (monthB.contains(x, y))
                            focusButton(&monthB, monthFocus, month);
                        else if (yearB.contains(x, y))
                            focusButton(&yearB, yearFocus, year);
                        else if (dateIncrementB.contains(x, y))
                            settingsButtonHelper(x, y, 1);
                        else if (dateDecrementB.contains(x, y))
                            settingsButtonHelper(x, y, -1);
                        else if (hourB.contains(x, y))
                            focusButton(&hourB, hourFocus, hour);
                        else if (minuteB.contains(x, y))
                            focusButton(&minuteB, minuteFocus, minute);
                        else if (secondB.contains(x, y))
                            focusButton(&secondB, secondFocus, second);
                        else if (timeIncrementB.contains(x, y))
                            settingsButtonHelper(x, y, 1);
                        else if (timeDecrementB.contains(x, y))
                            settingsButtonHelper(x, y, -1);
                        else if (backB.contains(x, y))
                            mainPage();
                        break;

                    case screenSaverP:
                        mainPage();
                        break;

                    case runP:
                        if (pauseRunB.contains(x, y) && !runStopped)
                            runPaused = true;
                        else if (resumeRunB.contains(x, y) && !runStopped)
                            runPaused = false;
                        else if (stopRunB.contains(x, y))
                        {
                            runPaused = false;
                            runStopped = true;
                            runStarted = false;
                        }
                        else if (viewDataB.contains(x, y))
                            dataPage();
                        else if (backB.contains(x, y))
                            mainPage();
                        break;
                    }
                }
                displayTimeout = 0;
                refreshTimeout = 0;
            }
    }
}