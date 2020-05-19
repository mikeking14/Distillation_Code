//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-Libraries and Variables-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////
//-----------------------------------------------------------Motor---------------------------------------------------------////
#include <AccelStepper.h>
#define dirPin 2  //StepperMotor Direction pin
#define stepPin 3 //StepperMotor Stepping pin
#define motorInterfaceType 1  //StepperMotor Interface Type (1 is for driver)

int motorSetPosition = 0.0;

AccelStepper motor = AccelStepper(motorInterfaceType, stepPin, dirPin); // AccelStepper instance for cooling water flow valve

//-----------------------------------------------------------Temperature---------------------------------------------------------////
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 12 // Temperature Data wire on pin 13
OneWire oneWire(ONE_WIRE_BUS); // oneWire instance for Maxim/Dallas temperature IC

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature tempSensors(&oneWire);
// Declare the addresses of the DS18B20's
DeviceAddress tempHE = {0x28, 0x25, 0x34, 0x94, 0x97, 0x0E, 0x03, 0x5B};
DeviceAddress tempT = {0x28, 0xB8, 0x3E, 0x94, 0x97, 0x02, 0x03, 0x50};
DeviceAddress tempW = {0x28, 0xFF, 0x87, 0x19, 0xA5, 0x16, 0x03, 0x1E};
DeviceAddress tempO = {0x28, 0xFF, 0x2B, 0x9F, 0x83, 0x16, 0x03, 0x99};

// PID temperature control
float tempHeatExchanger = 0.0; float tempTower = 0.0; float tempWash = 0.0; float tempOutlet = 0.0;
float prevTempHeatExchanger = 0.0; float prevTempTower = 0.0; float prevTempWash = 0.0; float prevTempOutlet = 0.0;

// Store temperature
const int numTempReadings = 3;
float PID_temperature_error[numTempReadings];
// Derivative
float derivativeTime[numTempReadings];
float tempDerivative = 0.0;

float set_temperature = 55.0; //Temperature at which the cooling motor will keep the outlet temperature
int setTempCounter = 20;
int setTempCounterMax = 30;
float PID_error = 0.0;
float previous_error = 0.0;
float elapsedTime, Time, timePrev, timeLeft;
float elapsedTime2, timePrev2;
float elapsedTime3, timePrev3;
int PID_value = 0;

//PID Constants
float kp = 8;   float ki = 10;   float kd = 1;
//PID Variables
float PID_p = 0.0;    float PID_i = 0.0;    float PID_d = 0.0;
int PID_max = 255;    int PID_min = 0;      float PID_Percent = 0.0;

//-----------------------------------------------------------Frequency---------------------------------------------------------////
#include <FreqCount.h>
unsigned long frequency;

//-----------------------------------------------------------Load Cell-----------------------------------------------------------////
#include <HX711_ADC.h>
HX711_ADC LoadCell(8, 9); //HX711 constructor (dout pin, sck pin)
long t;

//Load Cell
float mass = 0.0;
float averageMass = 0.0;
float massRate = 0.0;
float prevMass = 0.0;
float minMassRate = 0.2;
int checkpointConst = 10000;
int checkpoint = checkpointConst;
int checkpointIncrement = 50;

//-----------------------------------------------------------Other----------------------------------------------------------------////
//Epsilon Constants
const int room_temperature_constant = 22;
float epsilon = 0.0;
float constant_F = 4500000.0;
float constant_T = 0.3;
int tempAnomolyCounter = 0;
unsigned long time = 0;
float print_time = 0;
int state = 0;
int startup = 1;
byte byteRead;

//-----------------------------------------------------------Function Variables----------------------------------------------------------------////
//Averaging Function Variables
const int numReadings = 10;
float readings[numReadings]; // the readings from the analog input
int readIndex = 0; // the index of the current reading
float total = 0; // the running total
float average = 0; // the average


//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-Setup-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////
void setup() {

  Serial.begin(115200);
  tempSensors.begin();
  FreqCount.begin(1000);
  LoadCell.begin();

  motor.setMaxSpeed(500);
  motor.setCurrentPosition(0);

  Time = millis();

  long stabilisingtime = 15000; // tare preciscion can be improved by adding a few seconds of stabilising time
  LoadCell.start(stabilisingtime);
  LoadCell.setCalFactor(416.0); // user set calibration factor (float)
  // Check if last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

  //-----------------------------------------------------------Startup Procedure---------------------------------------------------------------///
  Serial.println("Please Zero the Flow Valve by entering:");
  Serial.println("1 for Less water");
  Serial.println("2 for More water");
  Serial.println("Press 9 to resume");

  // Startup Sequence to Calibrate water flow to near Zero flow:
      // You will need some flow as the temperature of this water used
      // to measure the temperature for the control system
  while(startup == 1){
    if (Serial.available()) {
      // Read the most recent byte from the serial monitor
      byteRead = Serial.read()- '0';
      // Open the valve by pressing 2
      if (byteRead == 1) {
        Serial.println("More Water");
        // Set the current position to 0:
          motor.setCurrentPosition(0);
        // Run the motor forward at 200 steps/second until the motor reaches 200 steps (0.05 revolutions):
        while(motor.currentPosition() != 100) {
          motor.setSpeed(200);
          motor.runSpeed();
          }
      }
      // Close the valve more by pressing 2
      if (byteRead == 2){
        Serial.println("Less Water");
        // Set the current position to 0:
        motor.setCurrentPosition(0);
        // Run the motor forward at -200 steps/second until the motor reaches 200 steps (0.05 revolutions):
        while(motor.currentPosition() != -100) {
          motor.setSpeed(-200);
          motor.runSpeed();
          }
      }
      //Set the motor current position to zero and leave setup
      if(byteRead == 9){
        Serial.println("MotorPosition = 0");
        motor.setCurrentPosition(0);
        Serial.println("Startup = 0");
        startup = 0;
        Serial.println("Break");
        break;
      }
    }
  }
}

//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-Loop-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////

void loop() {
  //-----------------------------------------------------------Time---------------------------------------------------------------///
  // Calculate and print the time
  time = (millis()) / 1000;

  //-----------------------------------------------------------Temperature & PID CONTROL------------------------------------------////
  // Store the previous temperatures incase of an anomoly in the new reading
  prevTempHeatExchanger = tempHeatExchanger;
  prevTempTower =  tempTower;
  prevTempWash =  tempWash;
  prevTempOutlet = tempOutlet;
  // Read the value of temperature probes
  tempSensors.requestTemperatures();
  // Store the actual temperature now
  tempHeatExchanger = tempSensors.getTempC(tempHE);
  tempTower =  tempSensors.getTempC(tempT);
  tempWash = tempSensors.getTempC(tempW);
  tempOutlet = tempSensors.getTempC(tempO);
        //Temperarure Anomoly Counter
        if (tempHeatExchanger < 10 || tempHeatExchanger > 130 || tempTower < 10 || tempTower > 130 ||
            tempWash < 10 || tempWash > 130 || tempOutlet< 10 || tempOutlet > 130)
           {
             tempAnomolyCounter = tempAnomolyCounter + 1;
             tempHeatExchanger = prevTempHeatExchanger;
             tempTower = prevTempTower;
             tempWash = prevTempWash;
             tempOutlet = prevTempOutlet;
           }

  //Next we calculate the error between the setpoint and the real value
  PID_error = set_temperature - tempTower;
  storeError(PID_error, millis());
  //Calculate the P value
  PID_p = kp * PID_error;
  //Calculate the I value in a range on +-10
  if (-10 < PID_error < 10){
      PID_i = PID_i + (ki * PID_error);
      if (PID_i > 300){
        PID_i = 300;}
      else if (PID_i < -25){
        PID_i = -25;}
    }

  //For derivative we need real time to calculate speed change rate
  timePrev = Time;                            // the previous time is stored before the actual time read
  Time = millis();                            // actual time read
  elapsedTime = (Time - timePrev) / 1000;
  //Now we can calculate the D value
  PID_d = kd * ( 3*PID_temperature_error[2] - 4*PID_temperature_error[1] + PID_temperature_error[0] ) / ((derivativeTime[2] - derivativeTime[0])/1000);;

  //Final total PID value is the sum of P + I + D
  PID_value = PID_p + PID_i + PID_d;

  //We define PID range between 0 and 255
  if (PID_value < PID_min){
    PID_value = PID_min ;}
  if (PID_value > PID_max){
    PID_value = PID_max;}
  // Calculate the valve position.
  motorSetPosition = -(255-PID_value);
  //Now we can set the valve position
  while((motor.currentPosition()+5) > motorSetPosition) {
    motor.setSpeed(-200); // Less water
/*    Serial.print("Motor Set Position: ");
    Serial.print(motorSetPosition);
    Serial.print("\t");
    Serial.print("Motor Current Position: ");
    Serial.print(motor.currentPosition());
    Serial.print("\t");
    Serial.println("Positive Direction (Less Water)");
    */
    motor.runSpeed();
  }
  while(motor.currentPosition()-5 < motorSetPosition) {
    motor.setSpeed(200); // More Water
/*    Serial.print("Motor Set Position: ");
    Serial.print(motorSetPosition);
    Serial.print("\t");
    Serial.print("Motor Current Position: ");
    Serial.print(motor.currentPosition());
    Serial.print("\t");
    Serial.println("Negative Direction (More Water)");
    */
    motor.runSpeed();
  }
  //Remember to store the previous error for next loop.
  previous_error = PID_error;

  //-----------------------------------------------------------Frequency-------------------------------------------------------////

  if (FreqCount.available()) {
    frequency = FreqCount.read();
  }

  //-----------------------------------------------------------Load Cell-------------------------------------------------------////

  // Update() should be called at least as often as HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS
  // Longer delay in sketch will reduce effective sample rate (be carefull with delay() in loop)
  LoadCell.update();
  // Get smoothed value from data set + current calibration factor
  float mass = LoadCell.getData() * -1;
  t = millis();
  // Calculate the mass flow rate
  averageMass = calculateAverage(mass);
  elapsedTime2 = (t - timePrev2) / 1000;
  if (elapsedTime2 > 1) {
    massRate = (averageMass - prevMass) / elapsedTime2;
    timePrev2 = t;
    prevMass = averageMass;
  }
  // Receive from serial terminal
  if (Serial.available() > 0) {
    float mass;
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  //-----------------------------------------------------------Set Temperarure Incrementer-------------------------------------------------------////

  time = millis()/1000;
  // Checks for increments
      //Increment the tower temperature if the mass flow rate falls below a certain level
      if(setTempCounter == 0 & checkpoint == checkpointConst){
        checkpoint = time + checkpointIncrement;
        minMassRate = 0.25;
        //checkpoint = time;
      }
      // Checks every second to see if the mass rate is too slow (minMassRate). If massRate < minMassRate then it increments a counter.
      // If the counter is above 30 at our checkpoint then increment the setTemp.
      elapsedTime3 = (time - timePrev3);
      if(elapsedTime3 >= 1) {
        timePrev3 = time;
        if(massRate < minMassRate){
          //Only increment if distillation has started
          setTempCounter += 1;
          //Keep the counter at the max level
          if(setTempCounter > setTempCounterMax) {setTempCounter = setTempCounterMax;}
        }
        else{
          if(setTempCounter > 0){setTempCounter -= 1;}
          else{setTempCounter = 0;}
        }
      }

      //Check to see if the setTempCounter has reached its max and that we have waited enough time to reach our checkpoint.
      if(setTempCounter == setTempCounterMax  & time > checkpoint){
        set_temperature += 1;
        setTempCounter = 0;
        checkpoint = time + checkpointIncrement; //Increment for checkpoint
      }

  //-----------------------------------------------------------Print Statement-------------------------------------------------------////

  if(time > print_time + .5){

      print_time = time;
      PID_Percent = (255 - PID_value)/(255);
      Serial.print(time);         Serial.print("\t");
      Serial.print("SetP:");      Serial.print("\t");     Serial.print(motorSetPosition);           Serial.print("\t");
      //Serial.print("CurP:");      Serial.print("\t");     Serial.print(motor.currentPosition());    Serial.print("\t");
      Serial.print("PID ER:");    Serial.print("\t");     Serial.print(PID_error);                  Serial.print("\t");
      Serial.print("PID");        Serial.print("\t");     Serial.print(PID_value);                  Serial.print("\t");
      Serial.print("P:");         Serial.print("\t");     Serial.print(PID_p);                      Serial.print("\t");
      Serial.print("I:");         Serial.print("\t");     Serial.print(PID_i);                      Serial.print("\t");
      Serial.print("D:");         Serial.print("\t");     Serial.print(PID_d);                      Serial.print("\t");
      //Serial.print("HE°:");       Serial.print("\t");     Serial.print(tempHeatExchanger);          Serial.print("\t");
      //Serial.print("W°:");        Serial.print("\t");     Serial.print(tempWash);                   Serial.print("\t");
      //Serial.print("Out°:");      Serial.print("\t");     Serial.print(tempOutlet);                 Serial.print("\t");
      Serial.print("M: ");        Serial.print("\t");     Serial.print(mass);                       Serial.print("\t");
      Serial.print("ΔM: ");       Serial.print("\t");     Serial.print(massRate);                   Serial.print("\t");
      //Serial.print("F:");         Serial.print("\t");     Serial.print(frequency);                  Serial.print("\t"); //20ms sample in H
      Serial.print("T°:");        Serial.print("\t");     Serial.print(tempTower);                  Serial.print("\t");
      Serial.print("ST:");        Serial.print(",");      Serial.print(set_temperature);            Serial.print("\t"); //20ms sample in H
      Serial.print("STCnt:");     Serial.print(",");      Serial.print(setTempCounter);             Serial.print("\t");
      Serial.print("ChkP:");      Serial.print(",");      Serial.println(checkpoint);



      /*
      if (tempAnomolyCounter > 10)
      {Serial.print("tEr:");        Serial.print("\t");   Serial.print(tempAnomolyCounter);     Serial.print("\t");}

      Serial.print("St:");       Serial.print("\t");   Serial.print(state);                  Serial.println("\t");
      */
      }

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

//Store the temperature and time in a matrix
float storeError(float input, float timeMillis) {
  // read from the sensor:
  PID_temperature_error[0] = PID_temperature_error[1];
  derivativeTime[0] = derivativeTime[1];

  PID_temperature_error[1] = PID_temperature_error[2];
  derivativeTime[1] = derivativeTime[2];

  PID_temperature_error[2] = input;
  derivativeTime[2] = timeMillis;
  }