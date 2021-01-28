#include <variables.h>

void startupSequence(); 
void data();
void printData();
void getFrequency();
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
  while (!motorMinSet && byte_read != 9)
  {
    if (Serial.available())
    {
      // Read the most recent byte from the serial monitor
      byte_read = Serial.read() - '0';

      switch (byte_read)
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
          Serial.println("\nPress 3 to set MIN flow rate before you continue...")
        break;

      default:
        Serial.println("Invalid Input!");
      }
    }
  }

  Serial.println("\n\nPress 1 to -- increase flow.\nPress 2 to -- decrease flow.\nPress 3 to -- set MAX.\nPress 9 to -- continue.");
  while (!motorMaxSet && byte_read != 9)
  {
    if (Serial.available())
    {
      byte_read = Serial.read() - '0';

      switch (byte_read)
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

void data() {

  getFrequency();
  getMass();
  getTemp();
  //----------------------------------------------------------- Print Statement -------------------------------------------------------//
  if (time > print_time + 1/data_per_second) {
    printData();
  }


}

void printData() {

  print_time = millis()/1000;

  Serial.print("\t");
  Serial.print("SetP:");      Serial.print("\t");     Serial.print(motorSetPosition);           Serial.print("\t");
  Serial.print("CurP:");      Serial.print("\t");     Serial.print(motor.currentPosition());    Serial.print("\t");
  Serial.print("PID ER:");    Serial.print("\t");     Serial.print(PID_error);                  Serial.print("\t");
  Serial.print("PID");        Serial.print("\t");     Serial.print(PID_value);                  Serial.print("\t");
  Serial.print("P:");         Serial.print("\t");     Serial.print(PID_p);                      Serial.print("\t");
  Serial.print("I:");         Serial.print("\t");     Serial.print(PID_i);                      Serial.print("\t");
  Serial.print("D:");         Serial.print("\t");     Serial.print(PID_d);                      Serial.print("\t");
  Serial.print("R°°:");       Serial.print("\t");     Serial.print(tempRoom);                   Serial.print("\t");
  Serial.print("W°:");        Serial.print("\t");     Serial.print(tempWash);                   Serial.print("\t");
  Serial.print("Out°:");      Serial.print("\t");     Serial.print(tempOutlet);                 Serial.print("\t");
  Serial.print("T°:");        Serial.print("\t");     Serial.print(tempTower);                  Serial.print("\t");
  Serial.print("M: ");        Serial.print("\t");     Serial.print(mass);                       Serial.print("\t");
  Serial.print("ΔM: ");       Serial.print("\t");     Serial.print(mass_derivative);            Serial.print("\t");
  Serial.print("F:");         Serial.print("\t");     Serial.print(frequency);                  Serial.print("\t"); //20ms sample in H
  Serial.print("ST:");        Serial.print("\t");     Serial.print(set_temperature);            Serial.print("\t");
  Serial.print("STCnt:");     Serial.print("\t");     Serial.print(set_temp_counter);           Serial.print("\t");
  Serial.print("ChkP:");      Serial.print("\t");     Serial.print(checkpoint);                 Serial.print("\t");
  Serial.print("State:");     Serial.print("\t");     Serial.print(state);                      Serial.println("\t");

}

void getFrequency() {
  if (FreqCount.available()) {
    frequency = FreqCount.read();
  }
}

float getMass() {
  // Update() should be called at least as often as HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS
  // Longer delay in sketch will reduce effective sample rate (be carefull with delay() in loop)
  LoadCell.update();
  // Get smoothed value from data set + current calibration factor
  mass = LoadCell.getData() * -1;
  t = millis();
  // Calculate the mass flow rate
  average_mass = calculateAverage(mass);
  elapsed_time2 = (t - time_prev2) / 1000;
  if (elapsed_time2 > 1) {
    mass_derivative = (average_mass - prev_mass) / elapsed_time2;
    time_prev2 = t;
    prev_mass = average_mass;
  }
  // Receive from serial terminal
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }
  return average_mass;
}

void getTemp(){
  // Read the value of temperature probes
  tempSensors.requestTemperatures();
  // Store the actual temperature now
  tempRoom = tempSensors.getTempC(tempR);
  tempTower =  tempSensors.getTempC(tempT);
  tempWash = tempSensors.getTempC(tempW);
  tempOutlet = tempSensors.getTempC(tempO);
}


void storeError(float input, float timeMillis) {
  // read from the sensor:
  PID_temperature_error[0] = PID_temperature_error[1];
  derivativeTime[0] = derivativeTime[1];

  PID_temperature_error[1] = PID_temperature_error[2];
  derivativeTime[1] = derivativeTime[2];

  PID_temperature_error[2] = input;
  derivativeTime[2] = timeMillis;
  }

void calculatePID() {
  // Check if we need to increment the temperature
  temperatureIncrementer();
  //Next we calculate the error between the setpoint and the real value
  PID_error = set_temperature - tempTower;
  storeError(PID_error, millis());
  //Calculate the P value
  PID_p = kp * PID_error;
  //Calculate the I value in a range on +-5
  if (-15 < PID_error && PID_error < 15){
      PID_i = PID_i + (ki * PID_error);
      if (PID_i > 300){
        PID_i = 300;}
      else if (PID_i < -50){
        PID_i = -50;}
    }

  //For derivative we need real time to calculate speed change rate
  time_prev = Time;                            // the previous time is stored before the actual time read
  Time = millis() / 1000;                            // actual time read
  elapsed_time = (Time - time_prev);
  //Now we can calculate the D value
  PID_d = kd * ( 3*PID_temperature_error[2] - 4*PID_temperature_error[1] + PID_temperature_error[0] ) / ((derivativeTime[2] - derivativeTime[0])/1000);;

  //Final total PID value is the sum of P + I + D
  PID_value = PID_p + PID_i + PID_d;

  // Re-define PID min and max
  if (PID_value < PID_min){
    PID_min = PID_value;}
  if (PID_value > PID_max){
    PID_max= PID_value;}

}

void setValvePosition() {
    // Calculate the valve position.
    motorSetPosition = map(PID_value, PID_min, PID_max, motorSetPositionMax, 0);

  while(motor.currentPosition() > motorSetPosition && motor.currentPosition() > 0) {
    motor.setSpeed(-200); // Less water
    motor.runSpeed();
  }
  while(motor.currentPosition() < motorSetPosition && motor.currentPosition() < motorSetPositionMax) {
    motor.setSpeed(200); // More Water
    motor.runSpeed();
  }
}

void temperatureIncrementer() {
  time = millis()/1000;
  // Checks for increments
      //Increment the tower temperature if the mass flow rate falls below a certain level
      if(set_temp_counter == 0 && checkpoint == checkpoint_const){
        checkpoint = time + checkpoint_increment;
        min_mass_derivative = 0.15;
        //checkpoint = time;
      }
      // Checks every second to see if the mass rate is too slow (min_mass_derivative). If mass_derivative < min_mass_derivative then it increments a counter.
      // If the counter is above 30 at our checkpoint then increment the setTemp.
      elapsed_time3 = (time - time_prev3);
      if(elapsed_time3 >= 1) {
        time_prev3 = time;
        if(mass_derivative < min_mass_derivative){
          //Only increment if distillation has started
          set_temp_counter += 1;
          //Keep the counter at the max level
          if(set_temp_counter > set_temp_counter_Max) {
            set_temp_counter = set_temp_counter_Max;
          }
        }
        else{
          if(set_temp_counter > 0){
            set_temp_counter -= 1;
          }
          else{
            set_temp_counter = 0;
          }
        }
      }

      //Check to see if the set_temp_counter has reached its max and that we have waited enough time to reach our checkpoint.
      if(set_temp_counter == set_temp_counter_Max  && time >= checkpoint){
        set_temperature += 1;
        set_temp_counter = 0;
        checkpoint = time + checkpoint_increment; //Increment for checkpoint
      }
}

float calculateAverage(float input) {
  total = total - readings[read_index]; // subtract the last reading
  readings[read_index] = input;
  total = total + readings[read_index];
  read_index = read_index + 1;

  if (read_index >= num_readings) {
    read_index = 0;
  }

  // calculate the average:
  average = total / num_readings;
  return average;
}
