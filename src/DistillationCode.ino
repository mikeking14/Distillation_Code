//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-Libraries and Variables-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////

//-----------------------------------------------------------Temperature---------------------------------------------------------////
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 13 on the Arduino
#define ONE_WIRE_BUS 13

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature tempSensors(&oneWire);
// Declare the addresses of the DS18B20's
DeviceAddress tempOnPin13 = {0x28, 0x25, 0x34, 0x94, 0x97, 0x0E, 0x03, 0x5B};
DeviceAddress tempOnPin12 = {0x28, 0xB8, 0x3E, 0x94, 0x97, 0x02, 0x03, 0x50};
DeviceAddress tempOnPin11 = {0x28, 0x9B, 0x32, 0x94, 0x97, 0x08, 0x03, 0x79};
DeviceAddress tempOnPin10 = {0x28, 0xFF, 0x2B, 0x9F, 0x83, 0x16, 0x03, 0x99};

//Variables for PID temperature control
float towerTemp = 0.0; float washTemp = 0.0; float someTemp1 = 0.0; float someTemp2 = 0.0;

float set_temperature = 60.0; //Temperature at which the cooling motor will keep the outlet temperature
int temperatureResolution = 12; // set the DS18B20 resolution to 9,10,11,12 (# of bits)
float PID_error = 0.0;
float previous_error = 0.0;
float elapsedTime, Time, timePrev, timeLeft;
float elapsedTime2, timePrev2;
int PID_value = 0;
int PWM_pin = 3; //PWM for speed control of the water pump

//PID Constants
float kp = 8;   float ki = 0.90;   float kd = 15;
//PID Variables
float PID_p = 0.0;    float PID_i = 0.0;    float PID_d = 0.0;

//-----------------------------------------------------------Load Cell-----------------------------------------------------------////
#include <HX711_ADC.h>
HX711_ADC LoadCell(8, 9); //HX711 constructor (dout pin, sck pin)
long t;

// Variables to keep track of the total seconds
unsigned long time;
unsigned int dwell = 2000; // dwell in microseconds for counter
unsigned long final_counts;
unsigned long start_time;
unsigned long measured_time;

//Load Cell
float mass = 0.0;
float averageMass = 0.0;
float massRate = 0.0;
float prevMass = 0.0;
float minMassRate = 0.1;
int checkpointConst = 10000;
int checkpoint = checkpointConst;
int checkpointIncrement = 100;

//-----------------------------------------------------------Other----------------------------------------------------------------////
//Epsilon Constants
const int room_temperature_constant = 22;
float epsilon = 0.0;
float constant_F = 4500000.0;
float constant_T = 0.3;
int tempAnomolyCounter = 0;
int state = 0;

//-----------------------------------------------------------Function Variable----------------------------------------------------------------////
//Averaging Function Variables
const int numReadings = 10;
float readings[numReadings]; // the readings from the analog input
int readIndex = 0; // the index of the current reading
float total = 0; // the running total
float average = 0; // the average


//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-Setup-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////
void setup() {

  Serial.begin(115200);
  //-----------------------------------------------------------Temperature---------------------------------------------------------////
  // Start up the temperature library
  tempSensors.begin();

  //PID temperature control
  pinMode(PWM_pin, OUTPUT);
  TCCR2B = TCCR2B & B11111000 | 0x03;    // pin 3 and 11 PWM frequency of 980.39 Hz
  Time = millis();

  //-----------------------------------------------------------Load Cell-----------------------------------------------------------////
  // Start communication to the loadcell library
  LoadCell.begin();
  long stabilisingtime = 8000; // tare preciscion can be improved by adding a few seconds of stabilising time
  LoadCell.start(stabilisingtime);
  LoadCell.setCalFactor(416.0); // user set calibration factor (float)

  //-----------------------------------------------------------Frequency-----------------------------------------------------------////
  TCCR1A = 0; //initialize Timer1
  TCCR1B = 0;
  TCNT1 = 0;
  //555 output pin
  pinMode(5, INPUT_PULLUP); //external source pin for timer1
  }



//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-Loop-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////

void loop() {

  //-----------------------------------------------------------Time---------------------------------------------------------------///
  // Calculate and print the time
  time = (millis()) / 1000;

  //-----------------------------------------------------------Temperature & PID CONTROL------------------------------------------////
  // Read the value of temperature probes
  tempSensors.requestTemperatures();

  towerTemp = tempSensors.getTempC(tempOnPin13);
  washTemp =  tempSensors.getTempC(tempOnPin12);
  someTemp1 = tempSensors.getTempC(tempOnPin11);
  someTemp2 = tempSensors.getTempC(tempOnPin10);

    if (towerTemp < 15 || towerTemp > 105 || washTemp < 15 || washTemp > 105 ||
        someTemp1 < 15 || someTemp1 > 105 || someTemp2< 15 || someTemp2 > 105)
       {
         tempAnomolyCounter = tempAnomolyCounter + 1;
         return;
       }


  //Next we calculate the error between the setpoint and the real value
  PID_error = set_temperature - towerTemp;
  //Calculate the P value
  PID_p = kp * PID_error;
  //Calculate the I value in a range on +-10
  if (-5 < PID_error < 5)
  {
    PID_i = PID_i + (ki * PID_error);
    if (PID_i > 100)
    {
      PID_i = 100;
    }
    else if (PID_i < -25)
    {
      PID_i = -5;
    }
  }

  //For derivative we need real time to calculate speed change rate
  timePrev = Time;                            // the previous time is stored before the actual time read
  Time = millis();                            // actual time read
  elapsedTime = (Time - timePrev) / 1000;
  //Now we can calculate the D value
  PID_d = kd * ((PID_error - previous_error) / elapsedTime);
  //Final total PID value is the sum of P + I + D
  PID_value = PID_p + PID_i + PID_d;

  //We define PWM range between 0 and 255
  if (PID_value < 0)
  {
    PID_value = 0 ;
  }
  if (PID_value > 107)
  {
    PID_value = 107;
  }
  //Now we can write the PWM signal to the mosfet on digital pin D3
  analogWrite(PWM_pin, 255 - PID_value);
  //analogWrite(PWM_pin,PID_value);
  previous_error = PID_error;     //Remember to store the previous error for next loop.

  //-----------------------------------------------------------Frequency-------------------------------------------------------////

  start_time = micros();
  TCNT1 = 0;//initialize counter
  // External clock source on Timer1, pin (D5). Clock on rising edge.
  // Setting bits starts timer
  TCCR1B =  bit (CS10) | bit (CS11) | bit (CS12); //external clock source pin D5 rising edge
  while (micros() - start_time < dwell) {} // do nothing but wait and count during dwell time
  TCCR1B = 0; //stop counter
  final_counts = TCNT1; //frequency limited by unsigned int TCNT1 without rollover counts
  measured_time = micros() - start_time;
  //Calculate Epsilon *****(should change [500 * final_counts] to a variable at some point)*****
  //epsilon = (constant_F / (500 * final_counts)) - ((towerTemp - 22) * constant_T);


  //-----------------------------------------------------------Load Cell-------------------------------------------------------////

  //update() should be called at least as often as HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS
  //longer delay in sketch will reduce effective sample rate (be carefull with delay() in loop)
  LoadCell.update();

  //get smoothed value from data set + current calibration factor
  float mass = LoadCell.getData() * -1;
  t = millis();

  //Calculate the mass flow rate
  averageMass = calculateAverage(mass);
  elapsedTime2 = (t - timePrev2) / 1000;
  if (elapsedTime2 > 1) {
    massRate = (averageMass - prevMass) / elapsedTime2;
    timePrev2 = t;
    prevMass = averageMass;
  }

  //receive from serial terminal
  if (Serial.available() > 0) {
    float mass;
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  //check if last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

  // Initial check(need to make sure checkpoint = checkpointConst at the start of the distillation) to start the checkpoint timer.
  // Wait for the massRate to begin (massRate > minMassRate)
  if (massRate > minMassRate & checkpoint == checkpointConst) {
    checkpoint = time + checkpointIncrement;
    state = 1;
  }
  // After the distillation has started, increment the tower temperature if the flow rate becomes too slow (ie. massRate < minMassRate)
  // and we have waited sufficiently long (ie the "time" has passed the "checkpoint" we set in the above (time > checkpoint))
  if (massRate < minMassRate & time > checkpoint) {
    set_temperature += 1;
    checkpoint = time + checkpointIncrement;
  }


  //-----------------------------------------------------------Print Statement-------------------------------------------------------////

                                                      Serial.print(time);                   Serial.print("\t");
  Serial.print("T°:");          Serial.print("\t");   Serial.print(towerTemp);              Serial.print("\t");
  Serial.print("W°:");          Serial.print("\t");   Serial.print(washTemp);               Serial.print("\t");
  Serial.print("1°:");          Serial.print("\t");   Serial.print(someTemp1);              Serial.print("\t");
  Serial.print("2°:");          Serial.print("\t");   Serial.print(someTemp2);              Serial.print("\t");

  //Uncomment if you need to see the output behind the PID Control [Format: (255-P+I+D) | P | I | D ]
  Serial.print("PID");          Serial.print("\t");   Serial.print(255 - PID_value);        Serial.print("\t");
  Serial.print("P:");           Serial.print("\t");   Serial.print(PID_p);                  Serial.print("\t");
  Serial.print("I:");           Serial.print("\t");   Serial.print(PID_i);                  Serial.print("\t");
  Serial.print("D:");           Serial.print("\t");   Serial.print(PID_d);                  Serial.print("\t");

  Serial.print("M: ");          Serial.print("\t");   Serial.print(mass);                   Serial.print("\t");
  Serial.print("ΔM: ");         Serial.print("\t");   Serial.print(massRate);               Serial.print("\t");
  Serial.print("F:");           Serial.print("\t");   Serial.print(500 * final_counts);     Serial.print("\t"); //20ms sample in H
  Serial.print("State:");       Serial.print("\t");   Serial.print(state);                  Serial.println("\t");
  if (tempAnomolyCounter > 10) {
    Serial.print("tErr:");        Serial.print("\t");   Serial.print(tempAnomolyCounter);     Serial.println("\t");
  }


  //Serial.print("Epsilon");    Serial.print("\t");   Serial.println(epsilon);                Serial.print("\t");

}

//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-Functions-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////

float calculateAverage(float input) {
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = input;
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits
  return average;
}
