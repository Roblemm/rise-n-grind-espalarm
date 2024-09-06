// Handles Tracking Time

// Project Specific Headers
#include "Alarm.h"
#include "RealTime.h"

// RTC Variables
// CONNECTIONS:
// DS1302 CLK/SCLK --> 5, DS1302 DAT/IO --> 4, DS1302 RST/CE --> 2, DS1302 VCC --> 3.3v - 5v, DS1302 GND --> GND
ThreeWire myWire(4, 5, 2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

// NTP Variables
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Function Reference
void printDateTime(const RtcDateTime &dt);

// RTC Constructor
RealTime::RealTime(Alarm& alarm) : alarm(&alarm) {}

// Initialize the RTC/NTP and Sync Time
void RealTime::initRTC()
{
    /// NTP Setup
    Serial.println("Setting NTP");
    timeClient.begin();               // Begins Client & Connects
    timeClient.setTimeOffset(-14400); // Set Timezone Offset
    timeClient.update();              // Syncs Time
    Serial.println("NTP Finished");

    /// RTC Setup
    Rtc.Begin(); // Begins Real Time Clock

    if (Rtc.GetIsWriteProtected()) // Turn off Write Protected
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning()) // Make sure RTC is running
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    if (timeClient.isTimeSet())
    { // Sets time to NTP if server connected
        // Converts the NTP time to a RTC Date Time Object
        timeClient.update();              // Syncs Time
        RtcDateTime timeToSet;
        timeToSet.InitWithUnix64Time(timeClient.getEpochTime());

        // Set Time
        Rtc.SetDateTime(timeToSet);
        Serial.print("Setting Time to NTP! - ");
        printDateTime(timeToSet);
        Serial.println();
    }
    else // Use Existing RTC time / Update it if incorrect
    {
        // Use the Time the RTC already has!
        Serial.println("Couldn't connect to NTP");

        // Save Compile Time
        RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

        // Check if RTC has valid time
        if (!Rtc.IsDateTimeValid())
        {
            // RTC Doesn't have Valid Time, Update to Compile Time

            // Common Causes:
            //    1) first time you ran and the device wasn't running yet
            //    2) the battery on the device is low or even missing

            Serial.println("RTC lost confidence in the DateTime!");
            Serial.println("Using Compile Time");
            printDateTime(compiled);
            Serial.println();
            Rtc.SetDateTime(compiled);
        }

        // Since we don't have wifi to set time, compare saved time to compile time
        // Update RTC only if time is behind the compile time.
        RtcDateTime now = Rtc.GetDateTime();
        if (now < compiled)
        {
            Serial.println("RTC is older than compile time!  (Updating DateTime)");
            Serial.println("Setting Time to Compile Time.");
            Rtc.SetDateTime(compiled);
        }
        else if (now > compiled) // Don't need to update time
        {
            Serial.println("RTC is newer than compile time. (this is expected)");
        }
        else if (now == compiled) // Don't need to update time
        {
            Serial.println("RTC is the same as compile time! (not expected but all is fine)");
        }
    }
}

void RealTime::runRTCLoop()
{
    static unsigned long timer = millis();

    if (millis() - timer > 1000 || timer == 0)
    {
        // Print Time NOw
        timer = millis();
        // Serial.println("Looping Next");
        // myDFPlayer.next();  //Play next mp3 every 3 second.

        Serial.print("RTC Time: ");
        printDateTime(RealTime::getTimeNow());
        Serial.println();

        // lcd.clear();
        // lcd.setCursor(0, 0);
        // lcd.printf("%02d:%02d:%02d", now.Hour(), now.Minute(), now.Second());

    }
}

// Get Current Time and Makes Sure It's Valid
RtcDateTime RealTime::getTimeNow()
{
    RtcDateTime now = Rtc.GetDateTime();

    if (!now.IsValid())
    {
        // Common Causes:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
    }

    return now;
}

// Prints Date Time Objects as String
void printDateTime(const RtcDateTime &dt)
{
    char datestring[26];

    snprintf_P(datestring,
               countof(datestring),
               PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
               dt.Month(),
               dt.Day(),
               dt.Year(),
               dt.Hour(),
               dt.Minute(),
               dt.Second());
    Serial.print(datestring);
}