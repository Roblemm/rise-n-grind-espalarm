// Handles Display of LCD Screen

// Project Specific Headers
#include "Display.h"
#include "RealTime.h"
#include "Alarm.h"

// External Library Headers
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// LCD Variables
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
// Default SDA = 21, SCL = 22


// Display Constructor
Display::Display(Alarm &alarm) : alarm(&alarm) {}


// Returns the Date and Time
RtcDateTime Display::getTimeInformation(){
    return alarm->rtc->getTimeNow();
} 
// Returns Weather Information
void Display::getWeatherInformation(){

}

// Starts LCD up
void Display::initLCD() {
    lcd.init();   // initialize the lcd 
    // Print a message to the LCD.
    lcd.backlight();
    lcd.setCursor(2,1);
    lcd.print("Loading...");
}

// Clears LCD Screen
void Display::clearLCD() {
    lcd.clear();
}


// Updates Time, Date, and Weather on Screen
void Display::updateDisplay() {
    static unsigned long timer = millis();
    static unsigned long timer2 = millis();
    static RtcDateTime lastTime = getTimeInformation();
    static int lastVolume = alarm->sound->getVolume();
    

    if (millis() - timer > 1000 || timer == 0) {
        timer = millis();

        // lcd.clear();

        // Set the Time
        lcd.setCursor(0,0);

        RtcDateTime now = getTimeInformation();

        // First, Clear what needs to be cleared.
        // If the Hour Changes, Clear the Whole Row first
        if(now.HourAmPm().Hour()!=lastTime.HourAmPm().Hour()){
            lcd.print("                "); // Clears First Row
            lcd.setCursor(0,0); // Sets Cursor back at the beginning
        }

        lcd.printf("%d:%02d:%02d %s",now.HourAmPm().Hour(), now.Minute(), now.Second(), now.HourAmPm().Meridiem() == Rtc_AM ? "AM" : "PM");

        // Set the Date (Shouldn't need to ever clear)
        lcd.setCursor(0,1);
        lcd.printf("%02d/%02d/%04d", now.Month(), now.Day(), now.Year());     

        // Update Last Time
        lastTime = now;   
    }

    if(millis() - timer2 > 500 || timer2 == 0){
        timer2 = millis();
        // Show Volume if recently edited
        if(alarm->sound->recentlyChangedVolume){
            int volume = alarm->sound->getVolume();

            // Only change if volume is different than before
            if(volume != lastVolume) {
                lastVolume = volume;
                if(volume < 10){
                    lcd.setCursor(11,1);
                    lcd.printf(" %d/%d",volume, alarm->sound->maxVolume);
                } else {
                    lcd.setCursor(11,1);
                    lcd.printf("%d/%d",volume, alarm->sound->maxVolume);
                }
            }

            
        } else {
            // Volume should not be showing, clear those lines
            lcd.setCursor(11,1);
            lcd.print("     ");
        }
    }
}

// Blinks when Alarm is Running
void Display::blinkScreen(){

}

// Shows Volume
void Display::showVolume(){

}