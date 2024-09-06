// Handles Ringing

// Project Specific Headers
#include "Alarm.h"
#include "Sound.h"

// DF Setup
#include "DFRobotDFPlayerMini.h"
#define FPSerial Serial1
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

// Sound Constructor
Sound::Sound(Alarm& alarm) : alarm(&alarm) {}

Sound *instance = nullptr; // Will be set to the sound instance when it is created. 
void inc1(){
    instance->incrementVolume(1);
}

void dec1(){
    instance->incrementVolume(-1);
}

// Setup Sound
void Sound::initSound(){
    FPSerial.begin(9600, SERIAL_8N1, /*tx =*/26, /*rx =*/27);
    Serial.println();
    Serial.println(F("DFRobot DFPlayer Mini Demo"));
    Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));



    while (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
        Serial.println(F("Unable to begin:"));
        Serial.println(F("1.Please recheck the connection!"));
        Serial.println(F("2.Please insert the SD card!"));
        delay(1000);
    }
    Serial.println(F("DFPlayer Mini online."));

    myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms

    //----Set volume----
    myDFPlayer.volume(volume);  //Set volume value (0~30).

    //----Read information----
    Serial.println("Read Information");
    Serial.println(myDFPlayer.readState()); //read mp3 state
    Serial.println(myDFPlayer.readVolume()); //read current volume
    Serial.println(myDFPlayer.readEQ()); //read EQ setting
    Serial.println(myDFPlayer.readFileCounts()); //read all file counts in SD card
    Serial.println(myDFPlayer.readCurrentFileNumber()); //read current play file number
    Serial.println(myDFPlayer.readFileCountsInFolder(3)); //read file counts in folder SD:/03


    // Serial.println("Playing Ringtone");
    // myDFPlayer.loop(1);  //Loop the first mp3

    pinMode(volumeIncreasePin, INPUT_PULLDOWN); // Volume Increase Button (1 when Pushed, 0 when not Pushed)
    pinMode(volumeDecreasePin, INPUT_PULLDOWN); // Volume Decrease Button (1 when Pushed, 0 when not Pushed)
    
    instance = this; // Update global instance

    // attachInterrupt(digitalPinToInterrupt(volumeIncreasePin), inc1, RISING);
    // attachInterrupt(digitalPinToInterrupt(volumeDecreasePin), dec1, RISING);
}

// Handles Updating Sound (Turning it off or on)
void Sound::updateSound(){
    // MAY NEED TO UPDATE SOUND WHILE PLAYING. 
    // WILL NOT HANDLE TURNING ITSELF ON AND OFF UNLESS NECCESSARY
    static unsigned long timer = millis();
    static short volumeIncreaseState = digitalRead(volumeIncreasePin);
    static short volumeDecreaseState = digitalRead(volumeDecreasePin);
    static unsigned long debounce =  millis(); // Temporarily prohibits increasing volume during short period. 
    int curIncState = digitalRead(volumeIncreasePin);
    int curDecState = digitalRead(volumeDecreasePin);

    if(millis() - debounce > 500) { // If at least .5 second since press button reaction
        if(curIncState == HIGH){
            debounce = millis(); // Update Debounce
            recentlyChangedVolume = true;
            incrementVolume(1);
        }
        if(curDecState == HIGH){
            recentlyChangedVolume = true;
            debounce = millis();
            incrementVolume(-1);
        }
        // Turn off Recently Changed Volume after 1.5s
        if(recentlyChangedVolume && millis() - debounce > 1500 && curDecState == LOW && curIncState == LOW){
            recentlyChangedVolume = false;
        }
    }


    // if(millis() - timer > 500) {
    //     timer = millis();
        
    //     if(curState == HIGH){
    //         Serial.printf("Volume Up State: HIGH ");
    //     } else {
    //         Serial.printf("Volume Up State: LOW ");
    //     }
    //     if(curState2 == HIGH){
    //         Serial.printf("Volume Down State: HIGH\n");
    //     } else {
    //         Serial.printf("Volume Down State: LOW\n");
    //     }


    //     // incrementVolume(1);
    // }


    if (myDFPlayer.available()) {
        printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
    }
} 
// Starts Alarm Ringing
void Sound::startRinging(){
    // NEEDS TO BE UPDATED WITH RING SOUND LOGIC

    Serial.println("Playing Ringtone");
    myDFPlayer.loop(1);  //Loop the first mp3
}
// Stops Alarm Ringing
void Sound::stopRinging(){
    Serial.println("Stopping Ringtone");
    myDFPlayer.stop();
} 
// Returns if the Alarm is ringing or not
bool Sound::checkIsRinging(){
    return false; // TODO
} 

// Set Volume to Amount
void Sound::setVolume(int amount){
    volume = amount;
    myDFPlayer.volume(amount);
} 
// Change Volume by Amount
int Sound::incrementVolume(int amount){
    Serial.printf("Changing Volume at %d by %d\n", volume,  amount);
    int newVolume = volume + amount;
    if(newVolume <= 30 && 0 <= newVolume) {
        volume = newVolume;
        setVolume(volume);
    }
    return volume;
}

// Return Volume of Device
int Sound::getVolume(){
    return volume;
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

}