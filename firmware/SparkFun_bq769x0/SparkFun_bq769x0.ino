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

 Here's how to hook up the Arduino pins to the bq76940 breakout board
 
 Arduino pin A4 (or SDA) -> bq769x0 SDA
 A5 (or SCL) -> SCL
 2 -> ALERT
 3.3V -> 3.3V
 GND -> GND
 
 Connecting the ALERT pin is recommended but is not requied. The alert pin will be raised if there is a fault condition.

 TODO:
 Get coulomb counter into global variable that increases after each IRQ
 Establish the direction of counter. Does negative mean we are charging or driving? 
 
 */

#include <Wire.h>
#include "SparkFun_BQ769x0.h"

//The bq769x0 without CRC has the 7-bit address 0x08. The bq769x0 with CRC has the address 0x18. 
//Please see the datasheet for more info
int bqI2CAddress = 0x08; //7-bit I2C address

//My pack is a 15 cell lipo that runs at 48V. Your pack may vary. Read the datasheet!
//This code is written for the bq76940. The bq76940 supports 9 to 15 cells.
#define NUMBER_OF_CELLS 15

//Max number of ms before timeout error. 100 is pretty good
#define MAX_I2C_TIME 100

volatile boolean bq769x0_IRQ_Triggered = false; //Keeps track of when the Alert pin has been raised

float gain = 0; //These are two internal factory set values.
int offset = 0; //We read them once at boot up and use them in many functions

long lastTime; //Used to blink the status LED

long totalCoulombCount = 0; //Keeps track of overall pack fuel gauage

float cellVoltage[16]; //Keeps track of the cell voltages

//GPIO declarations
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

byte irqPin = 2; //Interrupt enabled, connected to bq pin ALERT
byte statLED = 13; //Ob board status LED

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup()
{
  Serial.begin(9600);
  Serial.println("bq76940 example");
  
  Wire.begin(); //Start I2C communication

  pinMode(statLED, OUTPUT);
  digitalWrite(statLED, LOW); //Turn off the LED for now
  
  if(initBQ(2) == false) //Call init with pin 2 (IRQ0) or 3 (IRQ1)
  {
    Serial.println("bq76940 failed to respond - check your wiring");
    Serial.println("Hanging.");
    while(1);
  }
  else
  {
    Serial.println("bq76940 initialized!");
  }

  lastTime = millis();
  
  //Testing
  Serial.print("gain: ");
  Serial.print(gain);
  Serial.println("uV/lsb");

  Serial.print("offset: ");
  Serial.print(offset);
  Serial.println("mV");
  
  Serial.print("Undervoltage trip: ");
  Serial.print(readUVtrip());
  Serial.println("V");

  Serial.print("Overvoltage trip: ");
  Serial.print(readOVtrip());
  Serial.println("V");

  writeUVtrip(3.32); //Set undervoltage to 3.32V

  Serial.print("New undervoltage trip: ");
  Serial.print(readUVtrip());
  Serial.println("V"); //should print 3.32V

  writeOVtrip(4.27); //Set overvoltage to 4.27V

  Serial.print("New overvoltage trip: ");
  Serial.print(readOVtrip());
  Serial.println("V"); //should print 4.27V

  Serial.println();

  //delay(500);
  
  //while(1);

  readCellVoltage(2); //Should report a cell value around 3.7V
  readCellVoltage(15);

  while(1);

  /*
  readCoulombCounter();

  readTemp(0); //Read the die temperature. Should report something like room Temp

  delay(500);
  while(1);

  
  enableBalancing(1, true); //test
  */
}

void loop()
{
  //Each second make a reading of cell voltages
  //And blink the status LED
  if(millis() - lastTime > 1000)
  {
    //for(int i = 0 ; i < NUMBER_OF_CELLS ; i++)
    //  cellVoltage[i] = readCellVoltage(i);

    //Toggle stat LED
    if(digitalRead(statLED) == HIGH)
      digitalWrite(statLED, LOW);
    else
      digitalWrite(statLED, HIGH);    
    
    lastTime = millis();
  }
  
  //For every IRQ event read the flags and update the coulomb counter and other major events
  if(bq769x0_IRQ_Triggered == true)
  {
    //Read the status register and update if needed
    byte sysStat = registerRead(bq796x0_SYS_STAT);

    Serial.print("sysStat: 0x");
    Serial.println(sysStat, HEX);
    
    //Double check that ADC is enabled
    byte sysVal = registerRead(bq796x0_SYS_CTRL1);
    if(sysVal & bq796x0_ADC_EN)
    {
      Serial.println("ADC Enabled");
    }
  
    //We need to write 1s into all the places we want a zero, but not overwrite the 1s we want left alone
    byte sysNew = 0; 
        
    //Check for couloumb counter read
    if(sysStat & bq796x0_CC_READY)
    {
      Serial.println("CC Ready");
      totalCoulombCount += readCoulombCounter(); //Add this 250ms reading to the global fuel gauge
      sysNew |= bq796x0_CC_READY; //Clear this status bit by writing a one into this spot
    }

    if(sysStat & bq796x0_DEVICE_XREADY) //Internal fault
    {
      Serial.println("Internal fault");
      sysNew |= bq796x0_DEVICE_XREADY; //Clear this status bit by writing a one into this spot
    }

    if(sysStat & bq796x0_OVRD_ALERT) //Alert pin is being pulled high externally?
    {
      Serial.println("Override alert");
      sysNew |= bq796x0_OVRD_ALERT; //Clear this status bit by writing a one into this spot
    }

    if(sysStat & bq796x0_UV) //Under voltage
    {
      Serial.println("Under voltage alert!");
      sysNew |= bq796x0_UV; //Clear this status bit by writing a one into this spot
    }

    if(sysStat & bq796x0_OV) //Over voltage
    {
      Serial.println("Over voltage alert!");
      sysNew |= bq796x0_OV; //Clear this status bit by writing a one into this spot
    }

    if(sysStat & bq796x0_SCD) //Short circuit detect
    {
      Serial.println("Short Circuit alert!");
      //sysNew |= bq796x0_SCD; //Clear this status bit by writing a one into this spot
    }

    if(sysStat & bq796x0_OCD) //Over current detect
    {
      Serial.println("Over current alert!");
      //sysNew |= bq796x0_OCD; //Clear this status bit by writing a one into this spot
    }
    
    //Update the SYS_STAT with only the ones we want, only these bits will clear to zero
    registerWrite(bq796x0_SYS_STAT, sysNew); //address, value

    bq769x0_IRQ_Triggered = false; //Reset flag
  }
  
  if(Serial.available())
  {
    byte incoming = Serial.read();
    
    if(incoming == '1')
    {
      Serial.println("Entering ship mode");
      enterSHIPmode();      
    }
    
  }
  
  //Display cell voltages
    
  
  //Display CC fuel gauge
  //totalCoulomb = readCoulombCounter();
  
  //Display temperatures
  
  //Display any other register info
}

// this is irq handler for bq769x0 interrupts, has to return void and take no arguments
// always make code in interrupt handlers fast and short
void bq769x0IRQ()
{
  bq769x0_IRQ_Triggered = true;
}

//Initiates the first few I2C commands
//Returns true if we can verify communication
//Set CC_CFG to default 0x19
//Turn on the ADC
//Assume we are checking internal die temperatures (leave TEMP_SEL at zero)
//Configure the interrupts for Arduino Uno
//Read the Gain and Offset factory settings into global variables
boolean initBQ(byte irqPin)
{
  //Test to see if we have correct I2C communication
  byte testByte = registerRead(bq796x0_OV_TRIP); //Should be something other than zero on POR
  if(testByte == 0x00) return false; //Something is very wrong. Check wiring.
  
  //"For optimal performance, [CC_CFG] should be programmed to 0x19 upon device startup." page 40
  registerWrite(bq796x0_CC_CFG, 0x19); //address, value
  
  //Set any other settings such as OVTrip and UVTrip limits
  
  //Double check that ADC is enabled
  byte sysVal = registerRead(bq796x0_SYS_CTRL1);
  if(sysVal & bq796x0_ADC_EN)
  {
    Serial.println("ADC Already Enabled");
  }
  sysVal |= bq796x0_ADC_EN; //Set the ADC_EN bit
  registerWrite(bq796x0_SYS_CTRL1, sysVal); //address, value

  //Enable countinous reading of the Coulomb Counter
  sysVal = registerRead(bq796x0_SYS_CTRL2);
  sysVal |= bq796x0_CC_EN; //Set the CC_EN bit
  registerWrite(bq796x0_SYS_CTRL2, sysVal); //address, value
  //Serial.println("Coulomb counter enabled");
    
  //Attach interrupt
  pinMode(irqPin, INPUT); //No pull up

  if(irqPin == 2)
    //Interrupt zero on Uno is pin 2
    attachInterrupt(0, bq769x0IRQ, RISING);
  else if (irqPin == 3)
    //Interrupt one on Uno is pin 3
    attachInterrupt(1, bq769x0IRQ, RISING);
  else
    Serial.println("irqPin invalid. Alert IRQ not enabled.");

  //Gain and offset are used in multiple functions
  //Read these values into global variables
  gain = readGAIN() / (float)1000; //Gain is in uV so this converts it to mV. Example: 0.370mV/LSB
  offset = readADCoffset(); //Offset is in mV. Example: 65mV


  //Read the system status register
  byte sysStat = registerRead(bq796x0_SYS_STAT);
  if(sysStat & bq796x0_DEVICE_XREADY)
  {
    Serial.println("Device X Ready Error");
    //Try to clear it
    registerWrite(bq796x0_SYS_STAT, bq796x0_DEVICE_XREADY);
    
    delay(500);
    //Check again  
    byte sysStat = registerRead(bq796x0_SYS_STAT);
    if(sysStat & bq796x0_DEVICE_XREADY)
    {
      Serial.println("Device X Ready Not Cleared");
    }
  }

  return true;
}

//Enable or disable the balancing of a given cell
//Give me a cell # and whether you want balancing or not
void enableBalancing(byte cellNumber, boolean enabled)
{
  byte startingBit, cellRegister;

  if(cellNumber < 1 || cellNumber > 15) return; //Out of range
  
  if(cellNumber < 6)
  {
    startingBit = 0;
    cellRegister = bq796x0_CELLBAL1;
  }
  else if(cellNumber < 11)
  {
    startingBit = 6; //The 2nd Cell balancing register starts at CB6
    cellRegister = bq796x0_CELLBAL2; //If the cell number is 6-10 then we are in the 2nd cell balancing register
  }
  else if(cellNumber < 16)
  {
    startingBit = 11;
    cellRegister = bq796x0_CELLBAL3;
  }

  byte cell = registerRead(cellRegister); //Read what is currently there
  
  if(enabled)
    cell |= (1<<(cellNumber - startingBit)); //Set bit for balancing
  else
    cell &= ~(1<<(cellNumber - startingBit)); //Clear bit to disable balancing
  
  registerWrite(cellRegister, cell); //Make it so
}

//Calling this function will put the IC into ultra-low power SHIP mode
//A boot signal is needed to get back to NORMAL mode
void enterSHIPmode(void)
{
  //This function is currently untested but should work
  byte sysValue = registerRead(bq796x0_SYS_CTRL1);
  
  sysValue &= 0xFC; //Step 1: 00
  registerWrite(bq796x0_SYS_CTRL1, sysValue);
  
  sysValue |= 0x03; //Step 2: non-01
  registerWrite(bq796x0_SYS_CTRL1, sysValue);
  
  sysValue &= ~(1<<1); //Step 3: 01
  registerWrite(bq796x0_SYS_CTRL1, sysValue);

  sysValue = (sysValue & 0xFC) | (1<<1); //Step 4: 10
  registerWrite(bq796x0_SYS_CTRL1, sysValue);
  
  //bq should now be in powered down SHIP mode and will not respond to commands
  //Boot on VS1 required to start IC
}

//Given a cell number, return the cell voltage
//Vcell = GAIN * ADC(cell) + OFFSET
//Conversion example from datasheet: 14-bit ADC = 0x1800, Gain = 0x0F, Offset = 0x1E = 2.365V
float readCellVoltage(byte cellNumber)
{
  if(cellNumber < 1 || cellNumber > 15) return(-0); //Return error
  
  Serial.print("Read cell number: ");
  Serial.println(cellNumber);

  //Reduce the caller's cell number by one so that we get register alignment
  cellNumber--;
  
  byte registerNumber = bq796x0_VC1_HI + (cellNumber * 2);
  
  //Serial.print("register: 0x");
  //Serial.println(registerNumber, HEX);

  int cellValue = registerDoubleRead(registerNumber);
  
  //int cellValue = 0x1800; //6,144 - Should return 2.365
  //int cellValue = 0x1F10l; //Should return 3.052
  
  //Cell value should now contain a 14 bit value

  Serial.print("Cell value (dec): ");
  Serial.println(cellValue);

  float cellVoltage = cellValue * gain + offset; //0x1800 * 0.37 + 60 = 3,397mV
  cellVoltage /= (float)1000;

  Serial.print("Cell voltage: ");
  Serial.println(cellVoltage, 3);

  return(cellVoltage);
}

//Given a thermistor number return the temperature in C
//Valid thermistor numbers are 1 to 3 for external and 0 to read the internal die temp
//If you switch between internal die and external TSs this function will delay 2 seconds
int readTemp(byte thermistorNumber)
{
  //There are 3 external thermistors (optional) and an internal temp reading (channel 0)
  if(thermistorNumber < 0 || thermistorNumber > 3) return(-0); //Return error
  
  Serial.print("Read thermistor number: ");
  Serial.println(thermistorNumber);

  byte sysValue = registerRead(bq796x0_SYS_CTRL1);
  
  if(thermistorNumber > 0)
  {
    //See if we need to switch between internal die temp and external thermistor
    if((sysValue & bq796x0_TEMP_SEL) == 0)
    {
      //Bad news, we have to do a switch and wait 2 seconds
      //Set the TEMP_SEL bit
      sysValue |= bq796x0_TEMP_SEL;
      registerWrite(bq796x0_SYS_CTRL1, sysValue); //address, value
      
      Serial.println("Waiting 2 seconds to switch thermistors");
      delay(2000);      
    }
    
    int registerNumber = bq796x0_TS1_HI + ((thermistorNumber - 1) * 2);
    int thermValue = registerDoubleRead(registerNumber);
  
    //Therm value should now contain a 14 bit value
  
    Serial.print("Therm value: 0x");
    Serial.println(thermValue, HEX);
  
    float thermVoltage = thermValue * 382; //0x233C * 382uV/LSB = 3,445,640uV
    float thermResistance = ((float)10000 * thermVoltage) / (3.3 - thermVoltage);
    
    Serial.print("thermVoltage: ");
    Serial.println(thermVoltage / (float)1000000, 3);

    Serial.print("thermResistance: ");
    Serial.println(thermResistance);

    //We now have thermVoltage and resistance. With a datasheet for the NTC 103AT thermistor we could
    //calculate temperature. 
    int temperatureC = thermistorLookup(thermResistance);
    
    Serial.print("temperatureC: ");
    Serial.println(temperatureC);
    
    return(temperatureC);
  }
  else if(thermistorNumber == 0)
  {
    //See if we need to switch between internal die temp and external thermistor
    if((sysValue & 1<<3) != 0)
    {
      //Bad news, we have to do a switch and wait 2 seconds
      //Clear the TEMP_SEL bit
      sysValue &= ~(1<<3);
      registerWrite(bq796x0_SYS_CTRL1, sysValue); //address, value
      
      Serial.println("Waiting 2 seconds to switch to internal die thermistors");
      delay(2000);      
    }
    
    int thermValue = registerDoubleRead(bq796x0_TS1_HI); //There are multiple internal die temperatures. We are only going to grab 1.
  
    //Therm value should now contain a 14 bit value
    Serial.print("Therm value: 0x");
    Serial.println(thermValue, HEX);
  
    float thermVoltage = thermValue * 382; //0x233C * 382uV/LSB = 3,445,640uV
    float temperatureC = 25.0 - ((thermVoltage - 1.2) / 0.0042);
    
    Serial.print("thermVoltage: ");
    Serial.println(thermVoltage / (float)1000000, 3);

    Serial.print("temperatureC: ");
    Serial.println(temperatureC);

    return((int)temperatureC);
  }
  
}

//Returns the coulomb counter value in microVolts
//Example: 84,400uV 
//Coulomb counter is enabled during bqInit(). We do not use oneshot.
//If the counter is enabled in ALWAYS ON mode it will set the ALERT pin every 250ms. You can respond to this however you want.
//Host may clear the CC_READY bit or let it stay at 1.
float readCoulombCounter(void)
{
  int count = registerDoubleRead(bq796x0_CC_HI);
  
  //int count = 0xC350; //Test. Should report -131,123.84
  
  float count_uV = count * 8.44; //count should be naturally in 2's compliment. count_uV is now in uV

  return(count_uV);
}

//Returns the pack voltage in volts
//Vbat = 4 * GAIN * ADC(cell) + (# of cells * offset)
float readPackVoltage(void)
{
  int packADC = registerDoubleRead(bq796x0_BAT_HI);
  
  //int packADC = 0x6DDA; //Test. Should report something like 42.520V
  
  float packVoltage = 4 * gain * packADC + (NUMBER_OF_CELLS * offset); //Should be in mV
  
  return(packVoltage/(float)1000); //Convert to volts
}

//Reads the gain registers and calculates the system's factory trimmed gain
//GAIN = 365uV/LSB + (ADCGAIN<4:0>) * 1uV/LSB
int readGAIN(void)
{
  byte val1 = registerRead(bq796x0_ADCGAIN1);
  byte val2 = registerRead(bq796x0_ADCGAIN2);
  
  //Recombine the bits into one ADCGAIN
  byte adcGain = (val1 << 1) | (val2 >> 5);
  
  int gain = 365 + adcGain;
  
  return(gain);
}

//Returns the factory trimmed ADC offset
//Offset is -127 to 128 in mV
int readADCoffset(void)
{
  //Here we need to convert a 8bit 2's compliment to a 16 bit int
  char offset = registerRead(bq796x0_ADCOFFSET);
  
  //char offset = 0x1E; //Test readCell

  return((int)offset); //8 bit char is now a full 16-bit int. Easier math later on.
}

//Returns the over voltage trip threshold
//Default is 0b.10.OVTRIP(0xAC).1000 = 0b.10.1010.1100.1000 = 0x2AC8 = 10,952
//OverVoltage = (OV_TRIP * GAIN) + ADCOFFSET
//Gain and Offset is different for each IC
//Example: voltage = (10,952 * 0.370) + 56mV = 4.108V
float readOVtrip(void)
{
  int trip = registerRead(bq796x0_OV_TRIP);
  
  trip <<= 4; //Shuffle the bits to align to 0b.10.XXXX.XXXX.1000
  trip |= 0x2008;

  float overVoltage = ((float)trip * gain) + offset;
  overVoltage /= 1000; //Convert to volts
  
  //Serial.print("overVoltage should be around 4.108: ");
  //Serial.println(overVoltage, 3);
  
  return(overVoltage);
}

//Given a voltage (4.22 for example), set the over voltage trip register
//Example: voltage = 4.2V = (4200mV - 56mV) / 0.370mv = 11,200
//11,200 = 0x2BC0 = 
void writeOVtrip(float tripVoltage)
{
  byte val = tripCalculator(tripVoltage); //Convert voltage to an 8-bit middle value
  registerWrite(bq796x0_OV_TRIP, val); //address, value
}

//Returns the under voltage trip threshold
//Default is 0b.01.UVTRIP(0x97).0000 = 0x1970 = 6,512
//UnderVoltage = (UV_TRIP * GAIN) + ADCOFFSET
//Gain and Offset is different for each IC
//Example: voltage = (6,512 * 0.370) + 56mV = 2.465V
float readUVtrip(void)
{
  int trip = registerRead(bq796x0_UV_TRIP);
  
  trip <<= 4; //Shuffle the bits to align to 0b.01.XXXX.XXXX.0000
  trip |= 0x1000;

  float underVoltage = ((float)trip * gain) + offset;
  underVoltage /= 1000; //Convert to volts
  
  //Serial.print("underVoltage should be around 2.465: ");
  //Serial.println(underVoltage, 3);
  
  return(underVoltage);
}

//Given a voltage (2.85V for example), set the under voltage trip register
void writeUVtrip(float tripVoltage)
{
  byte val = tripCalculator(tripVoltage); //Convert voltage to an 8-bit middle value
  registerWrite(bq796x0_UV_TRIP, val); //address, value
}

//Under voltage and over voltage use the same rules for calculating the 8-bit value
//Given a voltage this function uses gain and offset to get a 14 bit value
//Then strips that value down to the middle-ish 8-bits
//No registers are written, that's up to the caller
byte tripCalculator(float tripVoltage)
{
  tripVoltage *= 1000; //Convert volts to mV
  
  //Serial.print("tripVoltage to be: ");
  //Serial.println(tripVoltage, 3);

  tripVoltage -= offset;
  tripVoltage /= gain;

  int tripValue = (int)tripVoltage; //We only want the integer - drop decimal portion.
  
  //Serial.print("tripValue should be something like 0x2BC0: ");
  //Serial.println(tripValue, HEX);
  
  tripValue >>= 4; //Cut off lower 4 bits
  tripValue &= 0x00FF; //Cut off higher bits
  
  //Serial.print("About to report tripValue: ");
  //Serial.println(tripValue, HEX);
  
  return(tripValue);
}

//Write a given value to a given register
void registerWrite(byte regAddress, byte regData)
{
  Wire.beginTransmission(bqI2CAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  
  Wire.beginTransmission(bqI2CAddress);
  Wire.write(regAddress);
  Wire.write(regData);
  Wire.endTransmission();
}

//Returns a given register
byte registerRead(byte regAddress)
{
  Wire.beginTransmission(bqI2CAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  
  Wire.requestFrom(bqI2CAddress, 1);
  
  return(Wire.read());
}

//Returns the atmoic int from two sequentials reads
int registerDoubleRead(byte regAddress)
{
  Wire.beginTransmission(bqI2CAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  
  Wire.requestFrom(bqI2CAddress, 2);
  
  /*byte counter = 0;
  while(Wire.available() < 2)
  {
    Serial.print(".");
    if(counter++ > MAX_I2C_TIME)
    {
      return(-1); //Time out error
    }
    delay(1);
  }*/

  byte reg1 = Wire.read();
  byte reg2 = Wire.read();

  //Serial.print("reg1: 0x");
  //Serial.print(reg1, HEX);
  //Serial.print(" reg2: 0x");
  //Serial.println(reg2, HEX);
  
  int combined = (int)reg1 << 8;
  combined |= reg2;
  
  return(combined);
}

//Given a resistance on a super common 103AT-2 thermistor, return a temperature in C
//This is a poor way of converting the resistance to temp but it works for now
//From: http://www.rapidonline.com/pdf/61-0500e.pdf
int thermistorLookup(float resistance)
{
  //Resistance is coming in as Ohms, this lookup table assume kOhm
  resistance /= 1000; //Convert to kOhm
  
  int temp = 0;
  
  if(resistance > 329.5) temp = -50;
  if(resistance > 247.7) temp = -45;
  if(resistance > 188.5) temp = -40;
  if(resistance > 144.1) temp = -35;
  if(resistance > 111.3) temp = -30;
  if(resistance > 86.43) temp = -25;
  if(resistance > 67.77) temp = -20;
  if(resistance > 53.41) temp = -15;
  if(resistance > 42.47) temp = -10;
  if(resistance > 33.90) temp = -5;
  if(resistance > 27.28) temp = 0;
  if(resistance > 22.05) temp = 5;
  if(resistance > 17.96) temp = 10;
  if(resistance > 14.69) temp = 15;
  if(resistance > 12.09) temp = 20;
  if(resistance > 10.00) temp = 25;
  if(resistance > 8.313) temp = 30;
  
  return(temp);  
}
