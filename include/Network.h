// Handles Connection to Network and Firebase

#ifndef Network_H_
#define Network_H_

// Standard Libraries
#include <string>

#include <WiFi.h>
#include <Firebase_ESP_Client.h>

class Alarm;

class Network {
    private:
        Alarm* alarm; // Reference to Alarm

        FirebaseAuth auth;
        FirebaseConfig config;

        String uid;

        friend void WiFiEventConnected(WiFiEvent_t event, WiFiEventInfo_t info);
        friend void WiFiEventGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
        friend void WiFiEventDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);


    public:
        Network(Alarm& alarm);

        bool initWiFi();
        void initFirebase();
        void runFirebaseLoop();
        bool connectWiFi();
        void firebaseDataUpdate();
};

#endif