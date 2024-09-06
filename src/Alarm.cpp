// Handles All Alarm Components and Ringing Time

// Project Specific Headers
#include "Alarm.h"

// External Library Headers

// Alarm Constructor
Alarm::Alarm() : network(nullptr), rtc(nullptr), display(nullptr), sound(nullptr)
{
    network = new Network(*this);
    rtc = new RealTime(*this);
    display = new Display(*this);
    sound = new Sound(*this);
}

// Alarm Destructor
Alarm::~Alarm()
{
    delete network; // Deallocate memory
    delete rtc;     // Deallocate memory
    delete display; // Deallocate memory
    delete sound;   // Deallocate memory
}

void Alarm::initAll()
{
    display->initLCD();      // Start running the LCD
    sound->initSound();      // Setup Alarm Sound
    network->initWiFi();     // Setup Wifi
    rtc->initRTC();          // Start running the RTC
    network->initFirebase(); // Setup Firebase Connection
    initAlarm();             // Load Alarms
    display->clearLCD();     // Clear LCD after Init is Done
}

void Alarm::updateAll()
{
    // Serial.print(1);
    network->runFirebaseLoop();
    // Serial.print(2);
    rtc->runRTCLoop(); // Start running the RTC
    // Serial.print(3);
    display->updateDisplay(); // Update the Display with the new time
    // Serial.print(4);
    updateAlarm(); // Check for Alarms
    // Serial.print(5);
    sound->updateSound(); // Update Sound
    // Serial.print(6);
}

// Loads Alarms
void Alarm::initAlarm()
{
    // Implementation here

    // Set Alarm 45 Seconds From Start Time
    RtcDateTime curTime = rtc->getTimeNow();
    RtcDateTime newTime = curTime.operator+(15);

    addAlarm(newTime);

    // addAlarm(newTime.operator+(80));
}

// Runs Alarm Loop - Checks for Alarms
void Alarm::updateAlarm()
{
    static unsigned long timer = millis();
    static unsigned long debounce = millis(); // Temporarily prohibits turning off alarm during short period.

    // Checks for Attempt to Stop Alarm
    if (millis() - debounce > 500)
    {
        // Only set debounce if you do something
        if (digitalRead(alarmStopPin) == HIGH && turnOffAlarm())
        { // Is true only if Alarm Turned Off
            // Set Debounce
            debounce = millis();
        }
    }

    // Checks for Alarms
    if (millis() - timer > 1000 || timer == 0)
    {
        timer = millis();

        RtcDateTime now = rtc->getTimeNow();

        // Loop through Alarms
        for (size_t i = 0; i < alarms.size(); i++)
        {
            AlarmItem &alarmItem = alarms[i];
            RtcDateTime alarmTime = alarmItem.time;
           

            if (alarmItem.active && !alarmItem.hasRang && !alarmItem.currentlyRinging && (now.operator==(alarmTime) || (now.operator>=(alarmTime) && now.operator<=(alarmTime.operator+(10)))))
            {
                runAlarm(alarmItem);
                Serial.printf("AlarmItem currentlyRinging is true? %s\n", alarmItem.currentlyRinging ? "true" : "false");
            }
            else if (alarmItem.currentlyRinging && now.operator==(alarmTime.operator+(maxRingTime)))
            {
                stopAlarm(alarmItem);
            }
            else if (now.operator==(alarmTime.operator+(maxRingTime)))
            {
                Serial.println("Alarm Item CurrentlyRinging is false ig");
                Serial.println(alarmItem.currentlyRinging);
                stopAlarm(alarmItem);
            }
        }
    }
}

void Alarm::addAlarm(RtcDateTime time)
{
    Serial.printf("New Alarm Set at %02d:%02d:%02d\n", time.Hour(), time.Minute(), time.Second());

    AlarmItem newAlarm(time);
    alarms.push_back(newAlarm);
}

void Alarm::syncAlarms(FirebaseJsonArray &arr)
{
    vector<AlarmItem> newAlarms; // Will be an array of alarms

    FirebaseJsonData result;

    for (size_t i = 0; i < arr.size(); i++)
    {
        // result now used as temporary object to get the parse results
        arr.get(result, i);

        // Print its value
        // Serial.print("Array index: ");
        // Serial.print(i);
        // Serial.print(", type: ");
        // Serial.print(result.type);
        // Serial.print(", value: ");
        // Serial.println(result.to<String>());

        FirebaseJson json;
        // Get FirebaseJson data
        result.get<FirebaseJson>(json);

        // Parse the JSON object as list
        // Get the number of items
        size_t len = json.iteratorBegin();
        FirebaseJson::IteratorValue value;

        int hour;
        int minute;
        String id;
        bool active;

        for (size_t i = 0; i < len; i++)
        {
            value = json.valueAt(i);

            // Serial.printf("%d, Type: %s, Name: %s, Value: %s\n", i, value.type == FirebaseJson::JSON_OBJECT ? "object" : "array", value.key.c_str(), value.value.c_str());

            if (value.key == "hour")
            {
                hour = value.value.toInt();
            }
            else if (value.key == "minute")
            {
                minute = value.value.toInt();
            }
            else if (value.key == "id")
            {
                id = value.value;
            }
            else if (value.key == "active")
            {
                active = value.value == "true" ? true : false;
            }
            else
            {
                // Serial.print("No match: ");
                // Serial.println(value.key);
            }
        }
        // Clear all list to free memory
        json.iteratorEnd();

        RtcDateTime now = rtc->getTimeNow();
        // Add Alarm to newAlarms vector
        newAlarms.push_back(AlarmItem(now, hour, minute, id, active));
    }

    // Update Alarms Array
    alarms = newAlarms;

    // Print Updated Array Vector
        Serial.println("Updated Alarms:");
    for (size_t i = 0; i < alarms.size(); i++)
    {
        AlarmItem &alarmItem = alarms[i];
        Serial.printf("Alarm %s: %02d:%02d\n", alarmItem.id.c_str(), alarmItem.time.Hour(), alarmItem.time.Minute());
    }
}

// Fires Alarm Item & Rings
void Alarm::runAlarm(AlarmItem &alarmItem)
{
    // Stop other alarms
    turnOffAlarm();

    // Make sure Alarm hasn't rang and isn't currently ringing
    if (!alarmItem.hasRang && !alarmItem.currentlyRinging)
    {
        currentAlarm = &alarmItem;
        alarmItem.hasRang = true;          // Alarm has Rang
        alarmItem.currentlyRinging = true; // Currently Ringing now

        Serial.printf("Check again: CurrentAlarm is currently Ringing: %s\n", currentAlarm->currentlyRinging ? "true" : "false");

        sound->startRinging(); // Start Ringing It
    }
}

// Stops Specific Alarm
void Alarm::stopAlarm(AlarmItem &alarmItem)
{
    Serial.println("Attempting to Stop");
    if (alarmItem.currentlyRinging)
    {
        currentAlarm = nullptr;
        alarmItem.currentlyRinging = false; // Stop Ringing
        sound->stopRinging();               // Stop Sound
    }
    else
    {
        Serial.println("Stop failed! Attempting to stop anyways");
        currentAlarm = nullptr;
        alarmItem.currentlyRinging = false; // Stop Ringing
        sound->stopRinging();
    }
}

// Turns off Alarm when button pressed.
bool Alarm::turnOffAlarm()
{
    if (currentAlarm != nullptr)
    {
        stopAlarm(*currentAlarm);
        return true; // Successfully Turned off Alarm
    }
    return false; // Didn't turn off alarm
}