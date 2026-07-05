#include <Arduino.h>
#include "WiFiSetup.h"
#include "RadioTochka.h"
#include "Potentiometer.h"

String ssid = "";
String password = "";
Potentiometer potentiometer(3,2.0);

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (!WifiSetup::hasCredentials()) {
        Serial.println("No Wifi, starting portal");
        WifiSetup::beginPortal();
        return;
    }

    WifiSetup::loadCredentials(ssid, password);

    int attempt = 0;
    while (!RadioTochka::connectToWiFi(ssid, password)) {
        delay(3000);
        attempt++;
        if (attempt > 10) {
            Serial.println("WiFi lost, trying to configure...");
            WifiSetup::beginPortal();
            return;
        }
    }

    RadioTochka::begin();

    while (!RadioTochka::openStream()) {
        Serial.println("Trying to open stream...");
        delay(1000);
    }

    Serial.println("Player started");
}

void loop() {
    delay(1);

    if (WifiSetup::isPortalActive()) {
        WifiSetup::handleLoop();
        return;
    }

    RadioTochka::handleLoop();
    RadioTochka::setVolume(potentiometer.getValueSmoothed());

    //wifi watchdog
    static uint32_t lastCheck = millis();
    if (millis() - lastCheck > 10000) {
        lastCheck = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi lost, trying to reconnect...");
            while (!RadioTochka::connectToWiFi(ssid, password)) {
                delay(3000);
            }
            while (!RadioTochka::openStream()) {
                delay(1000);
            }
        }
    }


}