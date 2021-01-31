#include <Arduino.h>
#include <Functions.h>
#include <IniControl.h>

void setup()
{
  Serial.begin(9600);
  Serial.println("test");
  SPI.begin();
  Serial.println("test");
  if (!SD.begin(BUILTIN_SDCARD))
  {
    while (1)
      Serial.println("ERROR: SD.begin() failed!\nCheck SD card and restart device.");
  }

  Serial.println("test");
  FreqMultRes.begin(47);
  FreqMultCap.begin(5);

  Serial.println("test");
  readSettings();

  Serial.println("test");
  if (useTemperatureModule)
  {
    tempSensors.begin();
    motor.setMaxSpeed(500);
    motor.setCurrentPosition(0);
    startupSequence(); // Used to "zero" the cooling water valve
  }
  Serial.println("test");
  if (useMassModule)
  {
    LoadCell.begin();
    LoadCell.setCalFactor(416.0); // User set calibration factor for HX711 load cell
    LoadCell.start(stabilisingTime);
  }
  Serial.println("test");
  currentTime = millis() / 1000;
}

void loop()
{
  time = millis() / 1000;

  // Initialize
  if (useTemperatureModule)
  {
    if (tempTower < warmupTemp)
    {
      // PIDvalue = PIDmax;
      // setValvePosition();
    }
    // Initialize - Warmup
    else if (warmupTemp <= tempTower && tempTower <= (setTemperature - 10.0))
    {
      kp = 8.0;
      ki = 1.0;
      kd = 10.0;
      PIDmax = 300;

      calculatePID();
      setValvePosition();
    }
    // Warmup - Distilation
    else if (tempTower >= (setTemperature - 10.0))
    {
      kp = 8.0;
      ki = 0.5;
      kd = 5.0;

      calculatePID();
      setValvePosition();
    }
    // Something isn't working
    else
    {
      Serial.println("Something broke!!\n");
      delay(5000);
    }
  }

  data();
}
