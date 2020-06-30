//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_- Libraries and Variables -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////

#include <variables.h>

//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_- Setup -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////

void setup() {

  Serial.begin(115200);
  tempSensors.begin();
  FreqCount.begin(1000);
  LoadCell.begin();

  motor.setMaxSpeed(500);
  motor.setCurrentPosition(0);

  Time = millis() / 1000;

  LoadCell.setCalFactor(416.0); // user set calibration factor for HX711 load cell
  LoadCell.start(stabilising_time);

  startupSequence(); // used to "zero" the cooling water valve

}

//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_- Loop -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////

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
      kp = 15;  ki = 1.0;  kd = 10.0;
      PID_max = 300;

      calculatePID();
      setValvePosition();
      data();

    }
    // Warmup - Distilation
    else if (tempTower >= (set_temperature - 10.0) && checkpoint != checkpoint_const) {
      state = 2;
      kp = 10.0;   ki = 2.0;   kd = 10.0;

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


//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_- Functions -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////

void startupSequence() {
  // Startup Sequence to Calibrate water flow to near Zero flow:
      // You will need some flow as the temperature of this water used
      // to measure the temperature for the control system


  // Set min flow rate1
  Serial.println(); Serial.println();
  Serial.println("Set Min flow rate -- 1 for More -- 2 for Less -- 3 to set -- 9 to continue: ");
  while(startup == 1){
    if (Serial.available()) {
      // Read the most recent byte from the serial monitor
      byte_read = Serial.read()- '0';

      // Open the valve by pressing 1
      if (byte_read == 1) {
        Serial.print(" More Water ");
        // Run the motor forward at 200 steps/second until the motor reaches 200 steps (0.05 revolutions):
        while(motor.currentPosition() != motorStepDistance) {
          motor.setSpeed(200);
          motor.runSpeed();
          }
        motor.setCurrentPosition(0); // Make this postion zero
      }

      // Close the valve more by pressing 2
      else if (byte_read == 2){
        Serial.print(" Less Water ");
        // Run the motor forward at -200 steps/second until the motor reaches 200 steps (0.05 revolutions):
        while(motor.currentPosition() >= - motorStepDistance) {
          motor.setSpeed(-200);
          motor.runSpeed();
          }
        motor.setCurrentPosition(0); // Make this postion zero
      }

      //Set the motor current position to zero and leave setup
      else if(byte_read == 3){
        motor.setCurrentPosition(0);
        motorMinSetBoolean = true;
        Serial.println();
        Serial.println("Press 9 to Continue");
      }

      // Continue to Max Flow rate
      else if(byte_read == 9 && motorMinSetBoolean == true){
        startup = 2;
      }

      else if(byte_read == 9 && motorMinSetBoolean == false){
        Serial.println();
        Serial.println("Press 3 to set MIN flow rate before you continue...");
      }
    }
  }

  // Set max flow rate
  Serial.println(); Serial.println();
  Serial.println("Set MAX flow rate -- 1 for More -- 2 for Less -- 3 to set -- 9 to continue: ");
  while(startup == 2){
    if (Serial.available()) {
      // Read the most recent byte from the serial monitor
      byte_read = Serial.read()- '0';

      // Open the valve by pressing 1
      if (byte_read == 1) {
        Serial.print(" More Water ");
        motor.moveTo(motor.currentPosition() + motorStepDistance);
        motor.setSpeed(200);
      }

      // Close the valve more by pressing 2
      else if (byte_read == 2){
        Serial.print(" Less Water ");
        motor.moveTo(motor.currentPosition() - motorStepDistance);
        motor.setSpeed(-200);
      }

      //Set the motor position
      else if(byte_read == 3 && motorMaxSetBoolean == false){
        motorSetPositionMax = motor.currentPosition();
        motorMaxSetBoolean = true;
        motor.moveTo(50);
        motor.setSpeed(-200);
        initalInfo();
      }

      else if(byte_read == 9 && motorMaxSetBoolean == true){
        startup = 0;
        break;
      }

      else if(byte_read == 9 && motorMaxSetBoolean == false){
        Serial.println("Press 3 to set MAX flow rate before you continue...");

      }
      // Move to target posistion
      while (motor.currentPosition() != motor.targetPosition()) {
        motor.runSpeedToPosition();
      }

    }
  }
}

void data() {

  getFrequency();
  getMass();
  getTemp();
  //----------------------------------------------------------- Print Statement -------------------------------------------------------////
  printData();

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
  Serial.print("State:");     Serial.print("\t");     Serial.print(state);                      Serial.print("\t");
  Serial.print("kP:");      Serial.print("\t");     Serial.print(kp);                      Serial.print("\t");
  Serial.print("kI");       Serial.print("\t");     Serial.print(ki);                      Serial.print("\t");
  Serial.print("kD:");      Serial.print("\t");     Serial.print(kd);                      Serial.println("\t");


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

void calculatePID() {
  // Check if we need to increment the temperature
  temperatureIncrementer();
  //Next we calculate the error between the setpoint and the real value
  PID_error = set_temperature - tempTower;
  storeError(PID_error, millis());
  //Calculate the P value
  PID_p = kp * PID_error;
  //Calculate the I value in a range on +-5
  if (-5 < PID_error && PID_error < 5){
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
      if(set_temp_counter == set_temp_counter_Max  && time > checkpoint){
        set_temperature += 1;
        set_temp_counter = 0;
        checkpoint = time + checkpoint_increment; //Increment for checkpoint
      }
}

float calculateAverage(float input) {
  // subtract the last reading:
  total = total - readings[read_index];
  // read from the sensor:
  readings[read_index] = input;
  // add the reading to the total:
  total = total + readings[read_index];
  // advance to the next position in the array:
  read_index = read_index + 1;

  // if we're at the end of the array...
  if (read_index >= num_readings) {
    // ...wrap around to the beginning:
    read_index = 0;
  }

  // calculate the average:
  average = total / num_readings;
  // send it to the computer as ASCII digits
  return average;
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

void initalInfo(){
  Serial.println(); Serial.println();
  Serial.println(); Serial.println();
  Serial.println(); Serial.println();
  Serial.println(); Serial.println();
  Serial.println("-------------------------------Initial information-------------------------------");
  Serial.println();
  Serial.println("\t"); Serial.print("Motor Set Position MAX:  ");  Serial.print(motorSetPositionMax);
  Serial.println();
  Serial.println("---------------------------------------------------------------------------------");
  Serial.println(); Serial.println();
  Serial.println(); Serial.println();
  Serial.println("Press 9 to continue to the Distilation when the motor finishes moving");
  Serial.println(); Serial.println();
}
