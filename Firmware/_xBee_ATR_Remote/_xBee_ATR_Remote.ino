/*********************************************************

xBee Remote Example
Author: Jason Traud  Jason@SDRobots.com
SuperDroid Robots
May 7th, 2015

This code uses an Arduino Uno to transmit a basic robot command packet

***********************************************************/

// Analog Input Variables
int FB;     // Forward and Back
int LR;     // Left and Right
int Spin;   // Rotation (used on vectoring robots)

// Temporary variables used in calculations
double tempFB, tempLR, tempSpin;

// Transmitted packet
byte byteFB;
byte byteLR;
byte Digital1;
byte checksum;

// Reverse bits
bool revA0 = 1;  // These are used to easily flip
bool revA1 = 0;  // the direction of the joystick

// Misc variables
int deadband = 5; 

void setup() {
  Serial.begin(9600);   // COM to xBee 
}

void loop() {
  reinitVars();		// nulls out temporary variables
  captureData();	// retrieves new data
  packetizeData();	// assembles data to be sent
  transmitData();       // send data off to xBee
  //debugData();        // print out formatted debug data
	
  delay(20);	// Delay needed for consistant communication
}

// Reinitialize temporary variables
void reinitVars() {
  Digital1 = 0;	
  tempFB = 0;
  tempLR = 0;
}

// Retrieve analog input data and conform to specs
void captureData() {
  FB = analogRead(A0);
  LR = analogRead(A1);

  // Reverse input analogs if flagged
  if (revA0)	{ FB = 1024 - FB; }
  if (revA1)	{ LR = 1024 - LR; }

  // The switch is towards the joystick (full speed)
  tempFB = map(FB, 0, 1024, -126, 126);	
  tempLR = map(LR, 0, 1024, -126, 126);
  
  if ((tempFB > (0 - deadband)) && (tempFB < deadband) )	
  { tempFB = 0; }
  if ((tempLR > (0 - deadband)) && (tempLR < deadband) )	
  { tempLR = 0; } 

}

// Assembles data to be sent
void packetizeData() {

  // Shift analog data up so we have a range of
  // 0-255
  tempFB = tempFB + 127;
  tempLR = tempLR + 127;

  // Assemble the digital packet
  // Add status of switches to digital1 here
  
  // convert transmitted data to bytes
  byteFB = byte(tempFB);
  byteLR = byte(tempLR);

  // calculate checksum
  checksum = byteFB + byteLR + byte(Digital1);
}

// xBee Communication
void transmitData() {  
  Serial.write("S");	
  Serial.write("D");
  Serial.write("R");	
  Serial.write(byteFB);
  Serial.write(byteLR);
  Serial.write(byte(Digital1));
  Serial.write(checksum);
}

// Print out values for easy debugging
void debugData() {
  Serial.print("[FB: ");  Serial.print(byteFB, HEX);          Serial.print("]");
  Serial.print("[LR: ");  Serial.print(byteLR, HEX);          Serial.print("]");
  Serial.print("[D1: ");  Serial.print(byte(Digital1), BIN);  Serial.print("]");
  Serial.println("");
}
