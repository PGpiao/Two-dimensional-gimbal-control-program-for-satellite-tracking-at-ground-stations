#include <Servo.h>
#include <Wire.h>
#include "easycomm.h"

Servo myservo1, myservo2;
// Define pins
const int azPin = 9;
const int elPin = 10;

int cur1 = 0, cur2 = 0;
int tar1 = 0, tar2 = 0;
unsigned long lastTime;

Servo azimuthServo;
Servo elevationServo;

easycomm *easycom;

void setup() {
  Serial.begin(9600);        // Match the baud rate with your rotctld setup
  azimuthServo.attach(azPin);
  elevationServo.attach(elPin);
  Serial.println("Antenna Tracker Ready");


  Serial.setTimeout(50);

  easycom = new easycomm(&Serial);

  lastTime = millis();
}

void loop() {
  easycom->parse(cur1, cur2);

  int originalAz = easycom->ComAzim;
  int originalEl = easycom->ComElev;

  if (originalAz < 180) {
    tar1 = originalAz;
    tar2 = originalEl;
  } else {
    tar1 = originalAz - 180;
    tar2 = originalEl + 90;
  }
  
  azimuthServo.write(map(tar1, 0, 180, 0, 255));
  elevationServo.write(map(tar2, 0, 180, 0, 255));
  cur1=tar1;
  cur2=tar2;
}
