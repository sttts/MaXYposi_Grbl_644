/*
  SD card file dump

 This example shows how to read a file from the SD card using the
 SD library and send it over the serial port.

 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

 created  22 December 2010
 by Limor Fried
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#include <SPI.h>
#include <SD.h>

const int chipSelect = 8;
String inputString = "";         // a string to hold incoming data
boolean ready_to_run = false;

void setup() {
  inputString.reserve(200);
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void serial_readln(){
  boolean got_cr = false;
  inputString = "";
	do {
	  while (Serial.available()) {
	    char inChar = (char)Serial.read();
	    inputString += inChar;
	    if (inChar == 13) {
	      got_cr = true;
	    }
	  }
	}	while (!got_cr);
}

void file_send(){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  // SerialEvent will not work while active!
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("(SD Card failed or not present)");
  }
//  Serial.println("(SD card initialized)");

  File dataFile = SD.open("gcode.txt");
  char c;
  // if the file is available, read from it:
  if (dataFile) {
    while (dataFile.available()) {
      c = dataFile.read();
      if (c >= 32) Serial.write(c);
      if (c==13) {
  			while (Serial.available()) Serial.read();
      	Serial.write(c);
        serial_readln();
        delay(25);
			  if (inputString.indexOf("Pn:R") >= 0) break;
      }
    }
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("(error opening gcode.txt)");
  }
  delay(500);
}

void loop() {

  char c;
  while (Serial.available()) Serial.read();
  Serial.write('?');
  serial_readln();
  if (inputString.indexOf("Idle") >= 0)  ready_to_run = true;
  if (inputString.indexOf("Run") >= 0)   ready_to_run = false;
  if (inputString.indexOf("Hold") >= 0)  ready_to_run = false;
  if (inputString.indexOf("Alarm") >= 0) ready_to_run = false;
  if (ready_to_run && (inputString.indexOf("Pn:S")) >= 0){
    file_send();
  }
  delay(100);
}

