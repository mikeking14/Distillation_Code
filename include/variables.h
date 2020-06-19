#include <AccelStepper.h>
#define dirPin 2  //StepperMotor Direction pin
#define stepPin 3 //StepperMotor Stepping pin
#define motorInterfaceType 1  //StepperMotor Interface Type (1 is for driver)
#include <OneWire.h>  // Temperature
#include <DallasTemperature.h>  // Temperarure
#define ONE_WIRE_BUS 12 // Temperature Data wire on pin 13
#include <FreqCount.h>  // Frequency
#include <HX711_ADC.h> // Load Cell

int state = 0;
int previousState = 1;

// Motor
int motorSetPosition = 0.0;
AccelStepper motor = AccelStepper(motorInterfaceType, stepPin, dirPin); // AccelStepper instance for cooling water flow valve
// Temperature
OneWire oneWire(ONE_WIRE_BUS); // oneWire instance for Maxim/Dallas temperature IC
DallasTemperature tempSensors(&oneWire);
// Declare the addresses of the DS18B20's
DeviceAddress tempHE = {0x28, 0x25, 0x34, 0x94, 0x97, 0x0E, 0x03, 0x5B};
DeviceAddress tempT = {0x28, 0xB8, 0x3E, 0x94, 0x97, 0x02, 0x03, 0x50};
DeviceAddress tempW = {0x28, 0xFF, 0x87, 0x19, 0xA5, 0x16, 0x03, 0x1E};
DeviceAddress tempO = {0x28, 0xFF, 0x2B, 0x9F, 0x83, 0x16, 0x03, 0x99};

// PID temperature control
float tempHeatExchanger; float tempTower; float tempWash; float tempOutlet;
float prevTempHeatExchanger; float prevTempTower; float prevTempWash; float prevTempOutlet;

// Store temperature
const int num_temp_readings = 3;
float PID_temperature_error[num_temp_readings];
int warmupTemp = 25;
// Derivative
float derivativeTime[num_temp_readings];

float set_temperature = 60.0; //Temperature at which the cooling motor will keep the outlet temperature
int set_temp_counter = 20;
int set_temp_counter_Max = 50;
float PID_error = 0.0;
float elapsed_time, Time, time_prev;
float elapsed_time2; float time_prev2 = 0.0;
float elapsed_time3; float time_prev3 = 0.0;
int PID_value = 0;

//PID Constants
float kp;   float ki;   float kd;
//PID Variables
float PID_p = 0.0;    float PID_i = 0.0;    float PID_d = 0.0;
int PID_max = 1000;    int PID_min = 0;      float PID_Percent = 0.0;

// Frequency
unsigned long frequency;

// Load Cell
HX711_ADC LoadCell(8, 9); //HX711 constructor (dout pin, sck pin)
long t;
float mass = 0.0;
float average_mass = 0.0;
float derivative_mass = 0.0;
float prev_mass = 0.0;
float min_mass_derivative = 0.1;
int checkpoint_const = 10000;
int checkpoint = checkpoint_const;
int checkpoint_increment = 100;
long stabilising_time = 5000; // tare preciscion can be improved by adding a few seconds of stabilising time

// Other
const int room_temperature_constant = 22;
float epsilon = 0.0;
float constant_F = 4500000.0;
float constant_T = 0.3;
unsigned long time = 0;
float print_time = 0;
int startup = 1;
byte byte_read;

//Averaging Function Variables
const int num_readings = 10;
float readings[num_readings]; // the readings from the analog input
int read_index = 0; // the index of the current reading
float total = 0; // the running total
float average = 0; // the average
