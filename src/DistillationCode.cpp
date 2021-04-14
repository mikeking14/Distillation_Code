#include <Arduino.h>
#include <UI.h>

void setup()
{
  Serial.begin(9600);

  if (!SD.begin(BUILTIN_SDCARD))
  {
    while (1)
    {
      Serial.println("ERROR: SD.begin() failed!\nCheck SD card and restart device.\n");
      delay(1000);
    }
  }

  FreqMultRes.begin(resFreqPin);
  FreqMultCap.begin(capFreqPin);

  if (useTemperatureModule)
  {
    tempSensors.begin();
    tempSensors.setResolution(tempO, 9);
  }

  if (useMotorModule)
  {
    motor.setMaxSpeed(500);
    motor.setCurrentPosition(0);
    startupSequence(); // Used to "zero" the cooling water valve
  }

  if (useMassModule)
  {
    LoadCell.begin();
    LoadCell.setCalFactor(416.0); // User set calibration factor for HX711 load cell
    LoadCell.start(stabilisingTime);
  }

  setupUI();

  currentTime = millis() / 1000;
}

void mkDataFile()
{
  //readSettings();

  // Add labels as first line of log file
  sprintf(dataLogTXT, "RUN%d.txt", runNumber);
  dataFile = SD.open(dataLogTXT, FILE_WRITE);
  if (dataFile)
  {
    dataFile.println("Motor Set,Motor Pos,PID Err,PID Val,PID P,PID I,PID D,Set Temp (C°),"
                     "Set Temp Count,Room Temp (C°),Wash Temp (C°),Outlet Temp (C°),"
                     "Tower Temp (C°),M(kg),ΔM (kg),Checkpoint,Frequency Res (Hz),"
                     "Frequency Cap (Hz)");
    dataFile.close();
  }
  else
  {
    Serial.println("ERROR: failed to open data log file.");
  }
}

void loop()
{
  // Initialize
  // if (useTemperatureModule)
  // {
  //   if (tempTower < warmupTemp)
  //   {
  //     // PIDvalue = PIDmax;
  //     // setValvePosition();
  //   }
  //   // Initialize - Warmup
  //   else if (warmupTemp <= tempTower && tempTower <= (setTemperature - 10.0))
  //   {
  //     kp = 8.0;
  //     ki = 1.0;
  //     kd = 10.0;
  //     PIDmax = 300;

  //     calculatePID();
  //     setValvePosition();
  //   }
  //   // Warmup - Distilation
  //   else if (tempTower >= (setTemperature - 10.0))
  //   {
  //     kp = 8.0;
  //     ki = 0.5;
  //     kd = 5.0;

  //     calculatePID();
  //     setValvePosition();
  //   }
  //   // Something isn't working
  //   else
  //   {
  //     Serial.println("Something broke!!\n");
  //     delay(5000);
  //   }
  // }

  UI();

  if (runStarted && runStopped)
  {
    runStarted = true;
    runStopped = false;
    mkDataFile();
  }

  if (runStarted && runStopped)
    runNumber = ++settingsRunNumber;
  
  if (runStarted && !runStopped && !runPaused)
    data();
}