// Handles Connection to Network and Firebase (Wifi)

// Standard Libraries
#include <string>

// External Library Headers
#include <WiFi.h>
#include <WiFiMulti.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// Project Specific Headers
#include "secrets.h"
#include "Alarm.h"
#include "Network.h"

// WIFI Variables
WiFiMulti wifiMulti;

const int WIFI_TIMEOUT = 100000; // Maximum time for Wifi to connect. Increase if necessary.

// Firebase Variables
FirebaseData fbdo;
FirebaseData stream;
unsigned long sendDataPrevMillis = 0;
bool firebaseChanged = false; // Flag to check if firebase data has been changed

// Network Constructor
Network::Network(Alarm &alarm) : alarm(&alarm) {}

// Wifi Events
void WiFiEventConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("WIFI CONNECTED! " + WiFi.SSID());
}

void WiFiEventGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("LOCAL IP ADDRESS: " + WiFi.localIP().toString());
}

void WiFiEventDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("WIFI DISCONNECTED!");
    // WiFi SHOULD automatically reconnect!
}

// Setup Wifi Networks
bool Network::initWiFi()
{
    WiFi.disconnect(); // Disconnects on init in case it was already connected

    // Connect Events
    WiFi.onEvent(WiFiEventConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiEventGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiEventDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    // Add Wifi Networks
    wifiMulti.addAP("NACS-Guest", "");
    wifiMulti.addAP(JAT_HOTSPOT[0], JAT_HOTSPOT[1]);
    wifiMulti.addAP(JAT_NETWORK[0], JAT_NETWORK[1]);
    wifiMulti.addAP(NOLAN_NETWORK[0], NOLAN_NETWORK[1]);

    // Attempt Connection until Successful
    while (connectWiFi() != true)
    {
        delay(100);
    }

    return true;
}

// Attempt to Connect to Wifi
bool Network::connectWiFi()
{
    // Attempt to Connect
    Serial.println("Connecting Wifi...");
    if (wifiMulti.run(WIFI_TIMEOUT) != WL_CONNECTED)
    {
        // Connection Failed
        Serial.println("WiFi failed to connect!");
        return false; // Failure
    }
    else
    {
        // Connection Success
        return true; // Success
    }
}

void streamCallback(FirebaseStream data)
{
    firebaseChanged = true;

    Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                  data.streamPath().c_str(),
                  data.dataPath().c_str(),
                  data.dataType().c_str(),
                  data.eventType().c_str());
    printResult(data); // see addons/RTDBHelper.h
    Serial.println();

    // This is the size of stream payload received (current and max value)
    // Max payload size is the payload size under the stream path since the stream connected
    // and read once and will not update until stream reconnection takes place.
    // This max value will be zero as no payload received in case of ESP8266 which
    // BearSSL reserved Rx buffer size is less than the actual stream payload.
    Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());

    // Due to limited of stack memory, do not perform any task that used large memory here especially starting connect to server.
    // Just set this flag and check it status later.
}

void streamTimeoutCallback(bool timeout)
{
    if (timeout)
        Serial.println("stream timed out, resuming...\n");

    if (!stream.httpConnected())
        Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

// Setup Firebase Connection
void Network::initFirebase()
{
    // Sets API & Database URL for connection
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    config.token_status_callback = tokenStatusCallback;

    // User Authentication
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    // WiFi Reconnection will be handled by Firebase
    Firebase.reconnectNetwork(true);

    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
    stream.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

    // Begins Firebase Connection using Authetication Information
    Firebase.begin(&config, &auth);

    // Stream Setup
    stream.keepAlive(5, 5, 1); // TCP KeepAlive For more reliable stream operation and tracking the server connection status

    // Getting the user UID might take a few seconds
    Serial.println("Getting User UID");
    while ((auth.token.uid) == "")
    {
        Serial.print('.');
        delay(1000);
    }

    uid = String(auth.token.uid.c_str());

    String path = String("/users/") + uid;

    Serial.print("Found Path: ");
    Serial.println(path);

    if (!Firebase.RTDB.beginStream(&stream, path))
    {
        Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());
    }
    else
    {
        Serial.println("Firebase Started");
    }

    Serial.println("Firebase Started");
    Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
}

void Network::runFirebaseLoop()
{
    if (Firebase.ready() && ((firebaseChanged && millis() - sendDataPrevMillis > 5000) || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();
        firebaseChanged = false;

        String path = String("/users/") + uid + "/alarms";

        Serial.println("Looking for Data...");
        if (Firebase.RTDB.getArray(&fbdo, path))
        {
            if (fbdo.dataType() == "array")
            {
                FirebaseJsonArray &arr = fbdo.to<FirebaseJsonArray>();
                // Deserialize to serial with prettify option
                // Serial.println("Data!");
                alarm->syncAlarms(arr);
            }
            else
            {
                Serial.print("Data Type Mismatch: ");
                Serial.println(fbdo.dataType());
            }
        }
    }
    // After calling stream.keepAlive, now we can track the server connecting status
    if (!stream.httpConnected())
    {
        // Server was disconnected!
    }
}
