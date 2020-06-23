//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-Notes-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-////

      Code measures the Frequency of a charge build up in a capacitor from the output of a 555 timer. (See Distillation capstone project)
      Code measures the Temperature from 2 DS18B20S
      Code measures the Weight applied to the load cell
      Combining the Frequency and Temperature circuits requires putting a diode in series with the 5V supply to the 555 timer circuit to prevent
      back inductance from the 555 timer circuit ruining the readings across the DS18B20 temperature probes.


//-------------------------------------------------------555 Circuit Timer---------------------------------------------------------////
      ** 555 timer circuit adapted from: http://www.fiz-ix.com/2012/11/measuring-signal-frequency-with-arduino/


//-------------------------------------------------------PID CONTROL--------------------------------------------------------------////
      PID CONTROL adapted from : http://electronoobs.com/eng_arduino_tut24_sch2.php


//-------------------------------------------------------Load Cell----------------------------------------------------------------////
      //-------------------------------------------------------------------------------------

      HX711_ADC.h
      Arduino master library for HX711 24-Bit Analog-to-Digital Converter for Weight Scales
      Olav Kallhovd sept2017
      Tested with      : HX711 asian module on channel A and YZC-133 3kg load cell
      Tested with MCU  : Arduino Nano

      // Changed the SAMPLES Variable in the config.h file for the HX711_ADC library to help stabilize the mass readings
      #define SAMPLES 					64 //  default of 16


      //-------------------------------------------------------------------------------------

      Settling time (number of samples) and data filtering can be adjusted in the HX711_ADC.h file

      //-------------------------------------------------------------------------------------

  //-------------------------------------------------------Things to do----------------------------------------------------------------////
