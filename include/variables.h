#include <AccelStepper.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <FreqMeasureMulti.h>
#include <HX711_ADC.h>
#include <SPI.h>
#include <SD.h>

#define ONE_WIRE_BUS 18 // Temperature Data wire
#define LOADCELL_DOUT_PIN 19
#define LOADCELL_SCK_PIN 20
#define capFreqPin 22
#define resFreqPin 23
#define dirPin 30             //StepperMotor Direction pin
#define stepPin 31            //StepperMotor Stepping pin
#define motorInterfaceType 32 //StepperMotor Interface Type (1 is for driver)

// Motor
int motorSetPosition = 0;
int motorStepDistance = 25;
int motorSetPositionMax;
boolean motorMaxSet = false;
boolean motorMinSet = false;

AccelStepper motor = AccelStepper(motorInterfaceType, stepPin, dirPin); // AccelStepper instance for cooling water flow valve

// Temperature
OneWire oneWire(ONE_WIRE_BUS); // oneWire instance for Maxim/Dallas temperature IC
DallasTemperature tempSensors(&oneWire);
// Declare the addresses of the DS18B20's
DeviceAddress tempR = {0x28, 0x25, 0x34, 0x94, 0x97, 0x0E, 0x03, 0x5B};
DeviceAddress tempT = {0x28, 0xB8, 0x3E, 0x94, 0x97, 0x02, 0x03, 0x50};
DeviceAddress tempW = {0x28, 0xFF, 0x87, 0x19, 0xA5, 0x16, 0x03, 0x1E};
DeviceAddress tempO = {0x28, 0x5B, 0xB3, 0x12, 0x0D, 0x00, 0x00, 0xE6};

// PID temperature control
double tempRoom;
double tempTower;
double tempWash;
double tempOutlet;
double prevtempRoom;
double prevTempTower;
double prevTempWash;
double prevTempOutlet;

// Store temperature
const int numTemperatureReadings = 3;
double PID_temperature_error[numTemperatureReadings];
int warmupTemp = 25;
// Derivative
double derivativeTime[numTemperatureReadings];

double setTemperature = 60.0; //Temperature at which the cooling motor will keep the outlet temperature
int setTemperatureCounter = 20;
int setTemepratureCounterMax = 50;
double PIDerror = 0;
double elapsedTime, elapsedTime2, elapsedTime3, currentTime, previousTime;
double previousTime2 = 0.0;
double previousTime3 = 0.0;
int PIDvalue = 0;

//PID Constants
double kp, ki, kd;
//PID Variables
double PIDp = 0.0, PIDi = 100.0, PIDd = 0.0, PIDpercent = 0.0;
int PIDmax = 500, PIDmin = 0;

// Frequencies
FreqMeasureMulti FreqMultRes;
FreqMeasureMulti FreqMultCap;
long freqRes, freqCap;
long double resSum = 0, capSum = 0;
long long int resCount = 0, capCount = 0;
elapsedMillis timeout;

// Load Cell
HX711_ADC LoadCell(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN); //HX711 constructor
long t;
double mass = 0.0;
double massAverage = 0.0;
double massDerivative = 0.0;
double massPrevious = 0.0;
double minMassDerivative = 0.025;
int checkpointConts = 10000;
int checkpoint = checkpointConts;
int checkpointIncrement = 100;
long stabilisingTime = 5000; // tare preciscion can be improved by adding a few seconds of stabilising time

// Print Variables
char pidStr[256];
char tempSetStr[256];
char tempStr[256];
char massStr[256];
char freqStr[256];
char collatedData[512];
char dataLogTXT[512];
unsigned int printPeriod = 1000; // How often to print data to serial.
int runNumber;
File dataFile;

// Other
byte userInput;
const double constantF = 4500000.0;
const double constantT = 0.3;
const int roomTemperature = 22;
char date[128];
char localTime[128];
double epsilon = 0.0; // Bad practice, consider non-zero epsilon
long tempTime = 0;

// Averaging Function Variables
const int numReadings = 10;
double readings[numReadings]; // the readings from the analog input
int readIndex = 0;            // the index of the current reading
double total = 0;             // the running total
double average = 0;           // the average

// Module in use status
bool useMassModule = true;
bool useTemperatureModule = true;