/*
 Library and example code for the bq76920, bq76930, and bq76940 battery management IC.
 By: Nathan Seidle
 SparkFun Electronics
 Date: September 26th, 2014
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 The bq769x0 series ICs manage large multi-cell battery packs for 12/18/24/36 and 48V systems. This code focuses on the 
 largest IC the bq76940 but most features should work on the smaller pack controllers.
 
 The bq76940 measures individual cell voltages, coulomb counter, three thermistor temperature sensors, and a bunch of
 other nice features.
 
 
 */

#include <Wire.h>

long lastTime;

//GPIO declarations
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

byte statLED = 13; //


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup()
{
  Serial.begin(9600);
  Serial.println("bq76940 example");
  
  pinMode(statLED, OUTPUT);
  digitalWrite(statLED, LOW); //Turn off the LED for now
  
  if(initBQ() == false)
    Serial.println("bq76940 failed to respond - check your wiring");


  lastTime = millis();
}

void loop()
{
  //Blink the on-board stat LED every second
  if(millis() - lastTime > 1000)
  {
    if(digitalRead(statLED) == HIGH)
      digitalWrite(statLED, LOW);
    else
      digitalWrite(statLED, HIGH);    
  }
  
  //Read the cell voltages
  
  //Read the coulomb counter
  
  //Check thermistors
  
  //Display data
}

//Initiates the first few I2C commands
//Returns true if we can verify communication
boolean initBQ(void)
{
  return true;
}
