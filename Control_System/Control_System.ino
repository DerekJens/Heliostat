#include <DS3232RTC.h>
#include "src/SolarPosition.h"
#include <Servo.h>

// choose your RTC library (DS1307 lib can also read from the DS3231 IC):

// number of decimal digits to print
const uint8_t digits = 3;

bool hasRun = false;
// program begins
Servo elevationServo;
Servo azimuthServo;
DS3232RTC RTC;

SolarPosition SanDiego(32.7157, -117.1611);
float mirrorAzimuth = 90;
float mirrorElevation = 0;
float targetAzimuth;
float targetElevation;

void setup()
{
  elevationServo.attach(11);
  azimuthServo.attach(10);
  Serial.begin(9600);
  RTC.begin();
  elevationServo.write(0);
  azimuthServo.write(0);
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


  if(hasRun == false){
    targetAzimuth = 2*mirrorAzimuth - solarAzimuth;
    targetElevation = 2*mirrorElevation - solarElevation;
    hasRun = true;
  }
  mirrorElevation = (targetElevation + solarElevation)/2;
  mirrorAzimuth = (targetAzimuth + solarElevation)/2;

  elevationServo.write(mirrorElevation);
  azimuthServo.write(mirrorAzimuth - 90);

  printSolarPosition(SanDiego.getSolarPosition(), digits);
  printTime(RTC.get());
  
  delay(15000);
  
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


