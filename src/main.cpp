// Libraries for Use

#include <Arduino.h>


#include "Alarm.h"


// Library Objects / Variables

Alarm alarmObject;

////////////////////////
// Firebase Functions //
///////////////////////


// Base setup Code
void setup()
{
  Serial.begin(115200);
  Serial.println("Setup");

  
  alarmObject.initAll(); // Initializes all Alarm Components
  // Display, Network, RTC, and Firebase
}

void loop()
{
  // put your main code here, to run repeatedly:

  alarmObject.updateAll(); // Updates every alarm component
  
  // Alarm.updateTime() // Updates Displayed Time
  // Alarm.checkAlarmNow() // Checks if there is an alarm
}