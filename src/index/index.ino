#include <Arduino.h>
#include "WiFiSetup.h"
#include "RadioTochka.h"
#include "Potentiometer.h"

long zeroCopySince = 0;
bool streamActive = true;
Potentiometer potentiometer(3,RadioTochka::MAX_VOLUME);

void setup() {
    Serial.begin(115200);
    delay(1000);

    potentiometer.setSmoothFactor(0.75);

    if (!WifiSetup::hasCredentials()) {
        Serial.println("No Wifi, starting portal");
        WifiSetup::beginPortal();
        return;
    }

    WifiSetup::loadCredentials();

    int attempt = 0;
    while (!WifiSetup::connectToWiFi()) {
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
    delay(10);

    if (WifiSetup::isPortalActive()) {
        WifiSetup::handleLoop();
        return;
    }

    RadioTochka::setVolume(potentiometer.getValueSmoothed());
    const auto copied = RadioTochka::handleLoop();

    //wifi watchdog
    static uint32_t lastCheck = millis();
    if (millis() - lastCheck > 10000) {
        lastCheck = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi lost, trying to reconnect...");
            while (!WifiSetup::connectToWiFi()) {
                delay(3000);
            }
            while (!RadioTochka::openStream()) {
                delay(1000);
            }
        }
    }
    if (copied) {
        zeroCopySince = 0;
    }
    else {
        if (zeroCopySince == 0) zeroCopySince = millis();
        if (millis() - zeroCopySince > 15000) {
            Serial.println("no data: will restart stream");
            while (!RadioTochka::openStream()) {
                Serial.println("Trying to reopen stream...");
                delay(1000);
            }
            zeroCopySince = 0;
        }
    }


}