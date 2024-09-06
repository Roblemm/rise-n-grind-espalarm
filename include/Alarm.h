// Handles All Alarm Components and Ringing Time 

#ifndef Alarm_H_
#define Alarm_H_


#include "Network.h"
#include "RealTime.h"
#include "Display.h"
#include "Sound.h"

using std::vector;

class AlarmItem;

class Alarm {
    private:
        long lastPressed = 0; // When button to turn off alarm was last pressed
        vector<AlarmItem> alarms; // Will be an array of alarms


        int alarmStopPin = 12; // Gray

    public:
        Alarm();
        ~Alarm();

        // Public Alarm Components
        Network *network;
        RealTime *rtc;
        Display *display;
        Sound *sound;

        // Tracks Current Alarm
        AlarmItem *currentAlarm = nullptr;
        int maxRingTime = 60;

        void initAll(); // Initializes All Alarm Components 
        void updateAll(); // Updates All Alarm Components

        void initAlarm(); // Loads Alarms
        void updateAlarm();  // Runs Alarm Loop - Checks for Alarms
        
        void addAlarm(RtcDateTime time); // Add New Alarm to Ring at Time
        void syncAlarms(FirebaseJsonArray& arr); // Syncs Alarms from Firebase

        void runAlarm(AlarmItem& alarmItem); // Fires Alarm Item & Rings

        void stopAlarm(AlarmItem& alarmItem); // Stops Specific Alarm
        bool turnOffAlarm(); // Turns off Alarm when button pressed.
};

class AlarmItem {
    public:
        RtcDateTime time;

        int hour;
        int minute;
        String id;
        bool active;
        
        bool hasRang = false;
        bool currentlyRinging = false;


        AlarmItem(RtcDateTime time) : time(time) {};
        AlarmItem(RtcDateTime time, bool hasRang) : time(time), hasRang(hasRang) {};

        AlarmItem(RtcDateTime now, int hour, int minute, String id, bool active) : hour(hour), minute(minute), id(id), active(active) {
            time = RtcDateTime(now.Year(), now.Month(), now.Day(), hour, minute, 0);
        };
};

#endif