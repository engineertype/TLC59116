// Using Arduino to control LEDs using the TI TLC59116
// Written by Peter Easton (whizoo.com 2013)
// Distributed under Open Software License v3.0


#include <Wire.h>

// I2C bus addresses (excludes the R/W bit)
#define ADDRESS                        0b1100000  // Assumes A0 to A3 are connected to ground
#define ALLCALL_ADDRESS                0b1101000
#define RESET_ADDRESS                  0b1101011

// I2C R/W
#define I2C_READ                       1
#define I2C_WRITE                      0

// Control register (three MSB control auto-increment)
#define NO_AUTO_INCREMENT              0b00000000
#define AUTO_INCREMENT_ALL_REGISTERS   0b10000000
#define AUTO_INCREMENT_BRIGHTNESS      0b10100000
#define AUTO_INCREMENT_CONTROL         0b11000000
#define AUTO_INCREMENT_BRIGHT_CONTROL  0b11100000

// TLC59116 registers
#define TLC59116_GRPPWM                0x12
#define TLC59116_LEDOUT0               0x14

// LED output state for LEDOUT0 to LEDOUT3
#define LED_OUTPUT_OFF                 0b00
#define LED_OUTPUT_GROUP               0b11


void setup()
{
  Wire.begin();
  Serial.begin(9600);
  
  // Transmit to the TLC59116
  Wire.beginTransmission(ADDRESS);
  // Send the control register.  All registers will be written to, starting at register 0
  Wire.write(byte(AUTO_INCREMENT_ALL_REGISTERS));
  // Set MODE1: no sub-addressing
  Wire.write(byte(0));
  // Set MODE2: dimming
  Wire.write(byte(0));
  // Set individual brightness control to maximum
  for (int i=0; i< 16; i++)
    Wire.write(byte(0xFF));
  // Set GRPPWM: Full brightness
  Wire.write(byte(0xFF));
  // Set GRPFREQ: Not blinking, must be 0
  Wire.write(byte(0));
  // Set LEDs off for now
  for (int i=0; i< 4; i++)
    Wire.write(byte(0));
  // Set the I2C all-call and sub addresses (if needed)
  Wire.endTransmission();

  delay(100);
}


void loop()
{
  // Loop through LEDs from 0 to 15
  for (int i=0; i<16; i++) {
    setLEDs(1<<i);
    delay(100);
  }
  
  // Light all LEDs and slowly dim them
  setLEDs(0xFFFF);
  for (int i=255; i > 0; i--) {
    setBrightness(i);
    delay(20);
  }
  setLEDs(0);
  setBrightness(0xFF);
}


// Turn the LEDs on or off.  LEDs is a 16-bit int corresponding to OUT0 (LSB) to OUT15 (MSB)
void setLEDs(int LEDs)
{
  int registerVal=0;
  int registerIncrement = LED_OUTPUT_GROUP;
  
  Wire.beginTransmission(ADDRESS);
  // Write to consecutive registers, starting with LEDOUT0
  Wire.write(byte(AUTO_INCREMENT_ALL_REGISTERS + TLC59116_LEDOUT0));
  
  // Write the value for LEDs
  for (int i=0; i< 16; i++) {
    if (LEDs & 0x01)
      registerVal += registerIncrement;
    // Move to the next LED
    LEDs >>= 1;
    // Are 4 LED values in the register now?
    if (registerIncrement == LED_OUTPUT_GROUP<<6) {
      // The register can be written out now
      Wire.write((byte) registerVal);
      registerVal = 0;
      registerIncrement = LED_OUTPUT_GROUP;
    }
    else {
      // Move to the next increment
      registerIncrement <<= 2;
    }
  }
  Wire.endTransmission();
}


// Set the brightness from 0 to 0xFF
void setBrightness(int brightness)
{
  Wire.beginTransmission(ADDRESS);
  // Write to the GRPPWM register
  Wire.write(byte(NO_AUTO_INCREMENT + TLC59116_GRPPWM));
  Wire.write(byte(brightness));
  Wire.endTransmission();
}

// Reset all TLC59116's on the I2C bus
void reset()
{
  Wire.beginTransmission(RESET_ADDRESS);
  Wire.write(byte(0));
  Wire.endTransmission();
}


