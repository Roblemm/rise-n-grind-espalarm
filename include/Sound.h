// Handles Ringing

#ifndef Sound_H_
#define Sound_H_

class Alarm;

class Sound {
    private:
        bool ringing = false;
        int volume = 15;
        Alarm *alarm;


        int volumeDecreasePin = 13; // Tan
        int volumeIncreasePin = 14; // Green

    public:
        Sound(Alarm &alarm);

        bool recentlyChangedVolume = false; // When true, it will display the volume
        int maxVolume = 30;

        void initSound();

        void updateSound(); // Handles Updating Sound (Turning it off or on)
        void startRinging(); // Starts Alarm Ringing
        void stopRinging(); // Stops Alarm Ringing
        bool checkIsRinging(); // Returns if the Alarm is ringing or not
        
        void setVolume(int amount); // Set Volume to Amount
        int incrementVolume(int amount); // Change Volume by Amount

        int getVolume();

};

#endif