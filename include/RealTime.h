// Handles Tracking Time

#ifndef RealTime_H_
#define RealTime_H_

// External Library Headers
#include <RtcDS1302.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h> 


class RealTime {
    private:
        Alarm *alarm; // Reference to the Alarm Object

    public:
        RealTime(Alarm& alarm); // Initialize Real Time Clock and Sync Time
        
        void initRTC(); // Initialize the RTC/NCP and Sync Time
        void runRTCLoop(); // Runs RTC Loop

        RtcDateTime getTimeNow(); // Returns the current time

};

#endif