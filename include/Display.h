// Handles Display of LCD Screen

#ifndef Display_H_
#define Display_H_

class Alarm;
class RtcDateTime;

class Display {
    private:
        RtcDateTime getTimeInformation(); // Returns the Date and Time
        void getWeatherInformation(); // Returns Weather Information

        Alarm *alarm;

    public:
        Display(Alarm &alarm);

        void initLCD(); // Starts LCD up
        void clearLCD(); // Clears LCD Screen

        void updateDisplay(); // Updates Time, Date, and Weather on Screen
        
        void blinkScreen(); // Blinks when Alarm is Running
        void showVolume(); // Shows Volume

};

#endif