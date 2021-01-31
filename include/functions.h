#include <Variables.h>

void startupSequence();
void data();
void printData();
float getMass();
void getTemp();
void calculatePID();
void setValvePosition();
void temperatureIncrementer();
float calculateAverage(float input);

// Startup Sequence to Calibrate water flow to near Zero flow:
// You will need some flow as the temperature of this water used
// to measure the temperature for the control system
void startupSequence()
{
  Serial.println("\n\nPress 1 to -- increase flow.\nPress 2 to -- decrease flow.\nPress 3 to -- set MIN.\nPress 9 to -- continue.");
  while (!motorMinSet && userInput != 9)
  {
    if (Serial.available())
    {
      // Read the most recent byte from the serial monitor
      userInput = Serial.read() - '0';

      switch (userInput)
      {
      case 1: // Run the motor forward at 200 steps/second until the motor reaches 200 steps (0.05 revolutions):
        Serial.println("More Water");
        while (motor.currentPosition() != motorStepDistance)
        {
          motor.setSpeed(200);
          motor.runSpeed();
        }
        motor.setCurrentPosition(0); // Make this postion zero
        break;

      case 2: // Run the motor forward at -200 steps/second until the motor reaches 200 steps (0.05 revolutions):
        Serial.println("Less Water");
        while (motor.currentPosition() >= -motorStepDistance)
        {
          motor.setSpeed(-200);
          motor.runSpeed();
        }
        motor.setCurrentPosition(0); // Make this postion zero
        break;

      case 3: // Set the motor current position to zero and leave setup
        motor.setCurrentPosition(0);
        motorMinSet = true;
        Serial.println("\nPress 9 to Continue");
        break;

      case 9: // Continue to Max Flow rate
        if (!motorMinSet)
          Serial.println("\nPress 3 to set MIN flow rate before you continue...");
        break;

      default:
        Serial.println("Invalid Input!");
      }
    }
  }

  Serial.println("\n\nPress 1 to -- increase flow.\nPress 2 to -- decrease flow.\nPress 3 to -- set MAX.\nPress 9 to -- continue.");
  while (!motorMaxSet && userInput != 9)
  {
    if (Serial.available())
    {
      userInput = Serial.read() - '0';

      switch (userInput)
      {
      case 1: // Run the motor forward at 200 steps/second until the motor reaches 200 steps (0.05 revolutions):
        Serial.println(" More Water ");
        motor.moveTo(motor.currentPosition() + motorStepDistance);
        motor.setSpeed(200);
        break;

      case 2: // Run the motor forward at -200 steps/second until the motor reaches 200 steps (0.05 revolutions):
        Serial.println("Less Water");
        motor.moveTo(motor.currentPosition() - motorStepDistance);
        motor.setSpeed(-200);
        break;

      case 3: // Set the motor current position as max and leave setup
        motorSetPositionMax = motor.currentPosition();
        motorMaxSet = true;
        motor.moveTo(50);
        motor.setSpeed(-200);

        Serial.println("\n\n\n-------------------------------Initial information-------------------------------");
        Serial.print("Motor Set Position MAX:\t");
        Serial.println(motorSetPositionMax);
        Serial.println("---------------------------------------------------------------------------------\n\n\n");
        break;

      case 9:
        if (!motorMaxSet)
          Serial.println("\nPress 3 to set MAX flow rate before you continue...");
        break;

      default:
        Serial.println("Invalid Input!");
      }

      // Move to target posistion
      while (motor.currentPosition() != motor.targetPosition())
      {
        motor.runSpeedToPosition();
      }
    }
  }
}

void data()
{
  freqRes = FreqMultRes.available() ? FreqMultRes.read() : 0;
  freqCap = FreqMultCap.available() ? FreqMultCap.read() : 0;

  if (useMassModule)
    getMass();
  if (useTemperatureModule)
    getTemp();

  if (time > printTime + (1 / dataPerSecond))
    printData();
}

void printData()
{
  printTime = millis() / 1000;

  if (Serial)
  {
    sprintf(pidStr, "Motor Set: %d Motor Pos: %ld PID Err: %f PID Val %d PID P: %f PID I: %f PID D: %f", motorSetPosition, motor.currentPosition(), PIDerror, PIDvalue, PIDp, PIDi, PIDd);
    sprintf(tempSetStr, "Set Temp (C°): %3f2 Set Temp Count : %d ", setTemperature, setTemperatureCounter);
    sprintf(tempStr, "Room Temp (C°): %3f2 Wash Temp (C°): %3f2 Outlet Temp (C°): %3f2 Tower Temp (C°): %3f2", tempRoom, tempWash, tempOutlet, tempTower);
    sprintf(massStr, "M: %f ΔM: %f Checkpoint: %d", mass, massDerivative, checkpoint);
    sprintf(freqStr, "Frequency (RES): %lu Frequency (CAP): %lu\n", freqRes, freqCap);
    Serial.println(pidStr);
    Serial.println(tempSetStr);
    Serial.println(tempStr);
    Serial.println(massStr);
    Serial.println(freqStr);
  }

  sprintf(collatedData, "%d,%ld,%f,%d,%f,%f,%f,%f,%d,%f,%f,%f,%f,%f,%f,%d,%lu,%lu",
          motorSetPosition, motor.currentPosition(), PIDerror, PIDvalue, PIDp, PIDi, PIDd, setTemperature,
          setTemperatureCounter, tempRoom, tempWash, tempOutlet, tempTower, mass, massDerivative, checkpoint, freqRes, freqCap);
  dataFile = SD.open(dataLogTXT, FILE_WRITE);
  if (dataFile)
  {
    dataFile.println(collatedData);
    dataFile.close();
  }
  else
  {
    Serial.println("ERROR: failed to open data log file.");
  }
}

float getMass()
{
  // Update() should be called at least as often as HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS
  // Longer delay in sketch will reduce effective sample rate (be carefull with delay() in loop)
  LoadCell.update();
  // Get smoothed value from data set + current calibration factor
  mass = LoadCell.getData() * -1;
  t = millis();
  // Calculate the mass flow rate
  massAverage = calculateAverage(mass);
  elapsedTime2 = (t - previousTime2) / 1000;
  if (elapsedTime2 > 1)
  {
    massDerivative = (massAverage - massPrevious) / elapsedTime2;
    previousTime2 = t;
    massPrevious = massAverage;
  }
  // Receive from serial terminal
  if (Serial.available() > 0)
  {
    char inByte = Serial.read();
    if (inByte == 't')
      LoadCell.tareNoDelay();
  }
  return massAverage;
}

void getTemp()
{
  // Read the value of temperature probes
  tempSensors.requestTemperatures();
  // Store the actual temperature now
  tempRoom = tempSensors.getTempC(tempR);
  tempTower = tempSensors.getTempC(tempT);
  tempWash = tempSensors.getTempC(tempW);
  tempOutlet = tempSensors.getTempC(tempO);
}

void storeError(float input, float timeMillis)
{
  // Read from the sensor:
  PID_temperature_error[0] = PID_temperature_error[1];
  derivativeTime[0] = derivativeTime[1];

  PID_temperature_error[1] = PID_temperature_error[2];
  derivativeTime[1] = derivativeTime[2];

  PID_temperature_error[2] = input;
  derivativeTime[2] = timeMillis;
}

void calculatePID()
{
  // Check if we need to increment the temperature
  temperatureIncrementer();
  // Next we calculate the error between the setpoint and the real value
  PIDerror = setTemperature - tempTower;
  storeError(PIDerror, millis());
  // Calculate the P value
  PIDp = kp * PIDerror;
  // Calculate the I value in a range on +-5
  if (-15 < PIDerror && PIDerror < 15)
  {
    PIDi = PIDi + (ki * PIDerror);
    if (PIDi > 300)
    {
      PIDi = 300;
    }
    else if (PIDi < -50)
    {
      PIDi = -50;
    }
  }

  // For derivative we need real time to calculate speed change rate
  previousTime = currentTime;    // The previous time is stored before the actual time read
  currentTime = millis() / 1000; // Actual time read
  elapsedTime = (currentTime - previousTime);
  // Now we can calculate the D value
  PIDd = kd * (3 * PID_temperature_error[2] - 4 * PID_temperature_error[1] + PID_temperature_error[0]) / ((derivativeTime[2] - derivativeTime[0]) / 1000);
  ;

  // Final total PID value is the sum of P + I + D
  PIDvalue = (int)(PIDp + PIDi + PIDd);

  // Re-define PID min and max
  if (PIDvalue < PIDmin)
  {
    PIDmin = PIDvalue;
  }
  if (PIDvalue > PIDmax)
  {
    PIDmax = PIDvalue;
  }
}

void setValvePosition()
{
  // Calculate the valve position.
  motorSetPosition = map(PIDvalue, PIDmin, PIDmax, motorSetPositionMax, 0);

  while (motor.currentPosition() > motorSetPosition && motor.currentPosition() > 0)
  {
    motor.setSpeed(-200); // Less water
    motor.runSpeed();
  }
  while (motor.currentPosition() < motorSetPosition && motor.currentPosition() < motorSetPositionMax)
  {
    motor.setSpeed(200); // More Water
    motor.runSpeed();
  }
}

void temperatureIncrementer()
{
  time = millis() / 1000;
  // Checks for increments
  // Increment the tower temperature if the mass flow rate falls below a certain level
  if (setTemperatureCounter == 0 && checkpoint == checkpointConts)
  {
    checkpoint = time + checkpointIncrement;
    minMassDerivative = 0.15;
    //checkpoint = time;
  }
  // Checks every second to see if the mass rate is too slow (minMassDerivative). If mass_derivative < minMassDerivative then it increments a counter.
  // If the counter is above 30 at our checkpoint then increment the setTemp.
  elapsedTime3 = (time - previousTime3);
  if (elapsedTime3 >= 1)
  {
    previousTime3 = time;
    if (massDerivative < minMassDerivative)
    {
      // Only increment if distillation has started
      setTemperatureCounter += 1;
      // Keep the counter at the max level
      if (setTemperatureCounter > setTemepratureCounterMax)
      {
        setTemperatureCounter = setTemepratureCounterMax;
      }
    }
    else
    {
      if (setTemperatureCounter > 0)
      {
        setTemperatureCounter -= 1;
      }
      else
      {
        setTemperatureCounter = 0;
      }
    }
  }

  // Check to see if the setTemperatureCounter has reached its max and that we have waited enough time to reach our checkpoint.
  if (setTemperatureCounter == setTemepratureCounterMax && time >= checkpoint)
  {
    setTemperature += 1;
    setTemperatureCounter = 0;
    checkpoint = time + checkpointIncrement; // Increment for checkpoint
  }
}

float calculateAverage(float input)
{
  total = total - readings[readIndex]; // Subtract the last reading
  readings[readIndex] = input;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;

  if (readIndex >= numReadings)
  {
    readIndex = 0;
  }

  // Calculate the average:
  average = total / numReadings;
  return average;
}