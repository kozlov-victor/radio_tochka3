#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

namespace WifiSetup {

    namespace {
        WebServer server(80);
        Preferences prefs;

        bool portalActive = false;

        const char* AP_NAME = "RadioTochkaSetup";
        const char* AP_PASS = "12345678";

        //language=HTML
        const char HTML[] PROGMEM = R"HTML(
        <!doctype html>
        <html lang="en">
        <head>
          <meta charset="utf-8">
          <title>RadioTochka Setup</title>
          <meta name="viewport" content="width=device-width, initial-scale=1">
        </head>
        <body>
          <h2>RadioTochka WiFi Setup</h2>
          <form method="POST" action="/save">
            <input name="ssid" placeholder="WiFi name" required><br><br>
            <input name="pass" placeholder="WiFi password" type="password"><br><br>
            <button type="submit">Save</button>
          </form>
        </body>
        </html>
        )HTML";

        void saveCredentials(const String& ssid, const String& pass) {
            prefs.begin("wifi", false);
            prefs.putString("ssid", ssid);
            prefs.putString("pass", pass);
            prefs.end();
        }

        void handleRoot() {
            server.send_P(200, "text/html", HTML);
        }

        void handleSave() {
            String ssid = server.arg("ssid");
            String pass = server.arg("pass");

            ssid.trim();
            pass.trim();

            if (ssid.length() == 0)
            {
                server.send(400, "text/plain", "SSID is empty");
                return;
            }

            saveCredentials(ssid, pass);

            server.send(
                200,
                "text/html",
                "<h2>Saved</h2><p>Device will restart.</p>"
            );

            delay(1000);
            ESP.restart();
        }
    }

    inline bool loadCredentials(String& ssid, String& pass) {
        prefs.begin("wifi", true);
        ssid = prefs.getString("ssid", "");
        pass = prefs.getString("pass", "");
        prefs.end();
        return ssid.length() > 0;
    }

    inline bool hasCredentials() {
        String ssid, pass;
        return loadCredentials(ssid, pass);
    }

    inline void clearCredentials() {
        prefs.begin("wifi", false);
        prefs.clear();
        prefs.end();
    }

    inline void beginPortal()
    {
        WiFi.mode(WIFI_OFF);
        delay(1000);

        WiFi.mode(WIFI_AP);
        delay(500);

        WiFi.setTxPower(WIFI_POWER_17dBm);

        bool ok = WiFi.softAP(
            AP_NAME,
            AP_PASS,
            6,      // channel
            false,  // hidden
            4       // max connections
        );

        delay(500);

        Serial.print("softAP result: ");
        Serial.println(ok ? "OK" : "FAIL");

        Serial.print("AP mode: ");
        Serial.println(WiFi.getMode());

        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());

        Serial.print("Stations: ");
        Serial.println(WiFi.softAPgetStationNum());

        server.on("/", HTTP_GET, handleRoot);
        server.on("/save", HTTP_POST, handleSave);
        server.onNotFound([]() {
            server.send_P(200, "text/html", HTML);
        });

        server.begin();
        portalActive = true;
    }

    inline void handleLoop() {
        if (portalActive)
            server.handleClient();
    }

    inline bool isPortalActive() {
        return portalActive;
    }
}