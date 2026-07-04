
#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"

// https://github.com/pschatzmann/arduino-audio-tools
// https://github.com/pschatzmann/arduino-libhelix

#include "AudioTools.h"
#include "AudioTools/Communication/AudioHttp.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

using namespace audio_tools;

namespace RadioTochka {

    namespace {
        const char* url =
        //"http://online.radioroks.ua:8000/RadioROKS";
        "http://91.218.213.49:8000/ur1-mp3";

        // I2S pins
        constexpr int PIN_BCLK = 2;
        constexpr int PIN_WS   = 5;
        constexpr int PIN_DOUT = 6;

        URLStream urlStream;
        I2SStream i2s;
        MP3DecoderHelix mp3;
        EncodedAudioStream decoder(&i2s, &mp3);
        StreamCopy copier(decoder, urlStream);
    }

    bool connectToWiFi(String ssid, String password) {
        WiFi.mode(WIFI_STA);
        WiFi.persistent(false);
        WiFi.setSleep(false);
        esp_wifi_set_ps(WIFI_PS_NONE);
        WiFi.setTxPower(WIFI_POWER_17dBm);

        WiFi.disconnect(false);
        delay(500);

        Serial.println("WiFi begin...");
        WiFi.begin(ssid, password);

        uint32_t start = millis();

        while (millis() - start < 20000)
        {
            wl_status_t s = WiFi.status();

            Serial.printf("status=%d RSSI=%d\n", s, WiFi.RSSI());

            if (s == WL_CONNECTED)
            {
                Serial.println("WiFi connected");
                Serial.println(WiFi.localIP());
                return true;
            }

            delay(500);
        }

        Serial.println("WiFi connect timeout");

        WiFi.disconnect(false);
        delay(500);

        return false;
    }

    void begin() {
        AudioLogger::instance().begin(Serial, AudioLogger::Warning);

        auto cfg = i2s.defaultConfig(TX_MODE);
        cfg.pin_bck = PIN_BCLK;
        cfg.pin_ws = PIN_WS;
        cfg.pin_data = PIN_DOUT;
        cfg.bits_per_sample = 16;

        i2s.begin(cfg);
        decoder.begin();
    }

    bool openStream() {
        Serial.println("URL stream begin");
        urlStream.end();
        delay(300);
        if (urlStream.begin(url, "audio/mpeg"))
        {
            Serial.println("URL stream opened");
            return true;
        }
        else {
            Serial.println("URL stream failed");
            return false;
        }
    }

    bool handleLoop() {
        return copier.copy() > 0;
    }

}