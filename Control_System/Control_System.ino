#include <DS3232RTC.h>
#include "src/SolarPosition.h"
#include <Servo.h>

// choose your RTC library (DS1307 lib can also read from the DS3231 IC):

// number of decimal digits to print
const uint8_t digits = 3;

bool hasRun = true;
volatile byte buttonState = HIGH;
const int setPin = 3;
const int resetPin = 2;
const int upPin = 4;
const int downPin = 5;
const int leftPin = 6;
const int rightPin = 7;
// program begins
Servo elevationServo;
Servo azimuthServo;
DS3232RTC RTC;

SolarPosition SanDiego(32.7157, -117.1611);
float mirrorAzimuth;
float mirrorElevation;
float volatile targetAzimuth = 180;
float volatile targetElevation = 10;

void setup()
{
  pinMode(setPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(setPin), setMirror, FALLING);
  pinMode(resetPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(resetPin), resetMirror, FALLING);
  pinMode(rightPin, INPUT_PULLUP);
  pinMode(leftPin, INPUT_PULLUP);
  pinMode(upPin, INPUT_PULLUP);
  pinMode(downPin, INPUT_PULLUP);
  
  elevationServo.attach(11);
  azimuthServo.attach(10);
  Serial.begin(9600);
  RTC.begin();
  SolarPosition::setTimeProvider(RTC.get);
  // set the Time service as the time provider
}

void loop()
{
  //Get sun angle
  //Have set target angle
  //stepper angle should be (Sun angle + target angle)/2
  //delay (15 min)
  float solarAzimuth = SanDiego.getSolarAzimuth();
  float solarElevation = SanDiego.getSolarElevation();

  if(0 < solarElevation){
    mirrorElevation = (targetElevation + solarElevation)/2;
    mirrorAzimuth = (targetAzimuth + solarAzimuth)/2;
  }
  else{
    mirrorElevation = 0;
    mirrorAzimuth = 180;
  }

  if(mirrorElevation < 0){
    mirrorElevation = 0;
  }
  else if(mirrorElevation > 120){
    mirrorElevation = 120;
  }
  if(mirrorAzimuth > 270){
    mirrorAzimuth = 270;
  }
  else if(mirrorAzimuth < 90){
    mirrorAzimuth = 90;
  }
  elevationServo.write(mirrorElevation);
  delay(1000);
  azimuthServo.write(-mirrorAzimuth + 270);

  Serial.print("target Elevation: ");
  Serial.println(targetElevation);
  Serial.print("target Azimuth: ");
  Serial.println(targetAzimuth);
  Serial.print("Mirror Elevation: ");
  Serial.println(mirrorElevation);
  Serial.print("mirror Azimuth: ");
  Serial.println(mirrorAzimuth);
  Serial.print("Solar EV: ");
  Serial.println(solarElevation, 3);
  Serial.print("Solar AZ: ");
  Serial.println(solarAzimuth, 3);
  //printSolarPosition(SanDiego.getSolarPosition(), digits);
  printTime(RTC.get());
  
  delay(3000000);
  
}

// Print a solar position to serial
//
void printSolarPosition(SolarPosition_t pos, int numDigits)
{
  Serial.print(F("el: "));
  Serial.print(pos.elevation, numDigits);
  Serial.print(F(" deg\t"));

  Serial.print(F("az: "));
  Serial.print(pos.azimuth, numDigits);
  Serial.println(F(" deg"));
}

// Print a time to serial
//
void printTime(time_t t)
{
  tmElements_t someTime;
  breakTime(t, someTime);

  Serial.print(someTime.Hour);
  Serial.print(F(":"));
  Serial.print(someTime.Minute);
  Serial.print(F(":"));
  Serial.print(someTime.Second);
  Serial.print(F(" UTC on "));
  Serial.print(dayStr(someTime.Wday));
  Serial.print(F(", "));
  Serial.print(monthStr(someTime.Month));
  Serial.print(F(" "));
  Serial.print(someTime.Day);
  Serial.print(F(", "));
  Serial.println(tmYearToCalendar(someTime.Year));
}

void setMirror(){
  float solarAzimuth = SanDiego.getSolarAzimuth();
  float solarElevation = SanDiego.getSolarElevation();
  Serial.println("Setting mirror:");
  while(!digitalRead(setPin)){
    if (!digitalRead(upPin)){
      mirrorElevation = mirrorElevation + 10;
      elevationServo.write(mirrorElevation);
      Serial.println("move up");
      delay(100);
    }
    else if(!digitalRead(downPin)){
      mirrorElevation = mirrorElevation - 10;
      elevationServo.write(mirrorElevation);
      Serial.println("move down");
      delay(100);
    }
    else if(!digitalRead(leftPin)){
      mirrorAzimuth = mirrorAzimuth - 10;
      azimuthServo.write(-mirrorAzimuth+270);
      Serial.println("move left");
      delay(100);
    }
    else if(!digitalRead(rightPin)){
      mirrorAzimuth = mirrorAzimuth + 10;
      azimuthServo.write(-mirrorAzimuth+270);
      Serial.println("move right");
      delay(100);
    }
  }
  targetAzimuth = 2*mirrorAzimuth - solarAzimuth;
  targetElevation = 2*mirrorElevation - solarElevation;
}

void resetMirror(){
  Serial.println("resetting Mirror");
  float solarAzimuth = SanDiego.getSolarAzimuth();
  float solarElevation = SanDiego.getSolarElevation();
  while(!digitalRead(resetPin)){
    azimuthServo.write(90);
    elevationServo.write(0);
    mirrorElevation = 0;
    mirrorAzimuth = 180;
    delay(1000);
  }
  targetAzimuth = 2*mirrorAzimuth - solarAzimuth;
  targetElevation = 2*mirrorElevation - solarElevation;
}

