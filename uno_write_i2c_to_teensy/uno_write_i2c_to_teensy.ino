// Wire Master Writer
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Writes data to an I2C/TWI slave device
// Refer to the "Wire Slave Receiver" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

void setup() {
  Serial.begin(9600);
  Wire.begin(); // join i2c bus (address optional for master)
}

void writeNum(byte tube, byte num)
{
  char messageBuf[7] = {0};
  sprintf (messageBuf, "%u %u 1\n",tube,num);
  Serial.print(messageBuf);
  Wire.beginTransmission(9); // transmit to device #8
  for(int i = 0; i < 7; i++)
  {
    Wire.write(messageBuf[i]);              // sends one byte
  }
  Wire.endTransmission();    // stop transmitting
}

void loop() {
  for (byte val=0; val < 10; val++)
  {
    for (byte tube=0; tube < 4; tube++)
    {
      writeNum(tube, val);
      delay(250);
    }
  }
}
