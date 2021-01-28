//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_- Libraries and Variables -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////

#include <Arduino.h>
#include <functions.h>



//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_- Setup -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////

void setup() {

  Serial.begin(115200);
  tempSensors.begin();
  resistanceFreq.begin(47);
  capacitanceFreq.begin(5);
  LoadCell.begin();

  motor.setMaxSpeed(500);
  motor.setCurrentPosition(0);

  Time = millis() / 1000;

  LoadCell.setCalFactor(416.0); // user set calibration factor for HX711 load cell
  LoadCell.start(stabilising_time);

  startupSequence(); // used to "zero" the cooling water valve

}

//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_- Loop -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////

void loop() {

  time = (millis()) / 1000;

  // State Awareness criteria
    // Initialize
    if(tempTower < warmupTemp) {
      state = 0;
      data();
      //PID_value = PID_max;
      //setValvePosition();
    }
    // Initialize - Warmup
    else if(warmupTemp <= tempTower && tempTower <= (set_temperature - 10.0)) {
      state = 1;
      kp = 8.0;  ki = 1.0;  kd = 10.0;
      PID_max = 300;

      calculatePID();
      setValvePosition();
      data();

    }
    // Warmup - Distilation
    else if (tempTower >= (set_temperature - 10.0)) {
      state = 2;
      kp = 8.0;   ki = 0.5;   kd = 5.0;

      calculatePID();
      setValvePosition();
      data();
    }
    // Something isnt working
    else {
      Serial.println("Something broke");\
      Serial.println();  Serial.println();  Serial.println();
      data();
      delay(5000);
      Serial.println();  Serial.println();  Serial.println();
    }
}

