#pragma once

#include <Arduino.h>

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
        //"http://91.218.213.49:8000/ur1-mp3";
        "https://radio2.ukr.radio/ur1-mp3-m";

        // I2S pins
        constexpr int PIN_BCLK = 7;
        constexpr int PIN_WS   = 4;
        constexpr int PIN_DOUT = 6;

        URLStream urlStream;
        I2SStream i2s;

        // volume пише у фізичний I2S
        VolumeStream volume(i2s);

        MP3DecoderHelix mp3;

        // decoder пише у volume
        EncodedAudioStream decoder(&volume, &mp3);

        StreamCopy copier(decoder, urlStream);

        double currentVolume = -1;
    }

    constexpr double MAX_VOLUME = 1.0;

    inline void begin() {
        AudioLogger::instance().begin(Serial, AudioLogger::Warning);

        auto cfg = i2s.defaultConfig(TX_MODE);
        cfg.pin_bck = PIN_BCLK;
        cfg.pin_ws = PIN_WS;
        cfg.pin_data = PIN_DOUT;
        cfg.bits_per_sample = 16;

        i2s.begin(cfg);

        auto info = i2s.audioInfo();

        Serial.printf(
            "sr=%d ch=%d bits=%d\n",
            info.sample_rate,
            info.channels,
            info.bits_per_sample
        );


        auto vcfg = volume.defaultConfig();
        vcfg.copyFrom(cfg);
        vcfg.allow_boost = true;
        volume.setVolume(0);
        volume.begin(vcfg);

        decoder.begin();
    }

    inline void setVolume(double value) {
        //if (value < MAX_VOLUME / 100.0) value = 0.0; // коли звук викручений в 0, все одно звук може пробиватись і "гавкати", приберем ці осціляції
        if (value > MAX_VOLUME) value = MAX_VOLUME;
        if (currentVolume == value) return;
        currentVolume = value;
        volume.setVolume(currentVolume);
        Serial.printf("Volume set to %f\n",value);
    }

    inline float getVolume() {
        return currentVolume;
    }

    inline void volumeUp() {
        setVolume(currentVolume + 0.1f);
    }

    inline void volumeDown() {
        setVolume(currentVolume - 0.1f);
    }

    inline bool openStream() {
        setVolume(0);
        Serial.println("URL stream begin");
        urlStream.end();
        delay(300);

        if (urlStream.begin(url, "audio/mpeg")) {
            Serial.println("URL stream opened");
            return true;
        }
        else {
            Serial.println("URL stream failed");
            return false;
        }
    }

    inline bool handleLoop() {
        return copier.copy() > 0;
    }

}