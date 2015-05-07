/*********************************************************

xBee Robot Example
Author: Jason Traud
SuperDroid Robots
May 7th, 2015

This code uses an Arduino Uno mounted on an ATR platform. 
The robot commands a single Sabertooth motor controller.

The goal of this firmware is provide control of the robot over an
xBee interface

Motor Controller:
Sabertooth Dual 25A Motor Driver (TE-091-225)
http://www.superdroidrobots.com/shop/item.aspx/sabertooth-dual-25a-motor-driver/822/

***********************************************************/

#include "Arduino.h"	// Arduino

// Since the Arduino is limited in the amount of serial COMs it has,
// we will control the motor controller over a software Serial port
#include <SoftwareSerial.h>
#include <Sabertooth.h>
SoftwareSerial SWSerial(2, 3); // RX on pin 2 (unused), TX on pin 3 (to S1).
Sabertooth ST(128, SWSerial); // Address 128, and use SWSerial as the serial port.

// Networking related RAM
bool sdrFound = false;		// Master Flag for SDR
bool sFound = false;		// Flag for SDR
bool dFound = false;		// Flag for SDR
bool rFound = false;		// Flag for SDR

// Recieved RAM
byte byteFB;
byte byteLR;
byte Digital1;
byte checksum;

// Define RAM
byte Bad_Tx;
word No_Tx = 0;

// Used for timeouts
unsigned long currentTime;
unsigned long timeOfLastGoodPacket = 0;

//******************************************************************************
// Sets up our serial com, hardware pins, etc
// RETURNS: Nothing
//******************************************************************************
void setup() {
  
  delay(2000);           // Short delay to allow the motor controllers
                         // to power up before we attempt to send it commands.
  
  Serial.begin(9600);      // Serial for the debug output
  SWSerial.begin(9600);    // Serial for the motor controller
  
  allStop();		// Make sure all motors are stopped for safety
}

//******************************************************************************
// This is our main program loop and is wrapped in an implied while(true) loop.
// RETURNS: Nothing
//******************************************************************************
void loop() {
  
  currentTime = millis();
  
  processSerial();     // Handle incoming data from xBee
  timeout();           // Stop the robot if we lost connection
  
  delay(10);
}

//******************************************************************************
// Sets the speed of all motor controllers to zero and sets all ESTOPs
// RETURNS: NONE
//******************************************************************************
void allStop() {
  ST.motor(1,0);  // Zero motors
  ST.motor(2,0);  
}

//******************************************************************************
// Here we're trying to find our start bytes. We have a flag for each of the 
// three bytes (SDR) and we raise the flag as they are found in sequence. If
// the next letter is not found, all of the flags are cleared and we start over.
// Once we have our flags we wait until we have the expected size of the packet
// and then we process the data
// RETURNS: NONE
//******************************************************************************
void processSerial() {
  
  unsigned char inputBufferTemp;
  byte chksumTest;
  
  // Debug Code
  //Serial1.println("[Serial not Available]");

  // Wait for serial
  if (Serial.available() > 0) {

    // Debug Code
    //Serial.println("[Serial is Available]");

    if (!sFound) {
      inputBufferTemp = Serial.read();
      if(inputBufferTemp == 0x53) { sFound = true; } 
      else { sFound = false; dFound = false; rFound = false; sdrFound = false; }
    }
    
    if (!dFound) {
      inputBufferTemp = Serial.read();
      if(inputBufferTemp == 0x44) { dFound = true; } 
      else { sFound = false; dFound = false; rFound = false; sdrFound = false; }
    }
    
    if (!rFound) {
      inputBufferTemp = Serial.read();
      if(inputBufferTemp == 0x52) { rFound = true; sdrFound = true;} 
      else { sFound = false; dFound = false; rFound = false; sdrFound = false; }
    }

    // Debug Code
    //Serial.print("["); Serial.print(sFound); Serial.print(dFound); Serial.print(rFound); Serial.print("]");

    if (sdrFound && (Serial.available()  > 3 )) {
      
      // store bytes into the appropriate variables
      byteFB    = Serial.read();
      byteLR    = Serial.read();
      Digital1  = Serial.read();
      checksum  = Serial.read();
      
      // Debug Code
      //Serial1.print("["); Serial1.print(byteFB, HEX); Serial1.print("]");
      //Serial1.print("["); Serial1.print(byteLR, HEX); Serial1.print("]");
      //Serial1.print("["); Serial1.print(Digital1, BIN); Serial1.print("]");
      //Serial1.print("["); Serial1.print(checksum, HEX); Serial1.print("]");
      //Serial1.println("");
      
      // Clear flags
      sdrFound = false;
      sFound = false; 
      dFound = false; 
      rFound = false;
      
      // Calculate our checksum
      chksumTest = byteFB + byteLR + Digital1;
      
      // Debug Code
      //Serial.print("["); Serial.print(checksum); Serial.print("]");
      //Serial.print("["); Serial.print(chksumTest); Serial.print("]");
      
      // Compare our calculated checksum to the expected
      if (chksumTest != checksum) {	
	return;	
      }
      
      // We found a good packet, so set current time
      timeOfLastGoodPacket = currentTime;
      
      ST.drive(byteFB);
      ST.turn(byteLR);      
    } 
  } 
}

//******************************************************************************
// We need to be cautios of data connection losses. Here we are comparing 
// the current time and when we recieved our last good packet. If a second
// has passed then we need to stop our motors
// RETURNS: NONE
//******************************************************************************
void timeout() {
  if (currentTime > (timeOfLastGoodPacket + 1000)) {
    allStop();
    timeOfLastGoodPacket = currentTime;
  }
}
