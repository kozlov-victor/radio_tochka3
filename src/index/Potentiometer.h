#pragma once

#include <Arduino.h>

#include "RadioTochka.h"

class Potentiometer {
private:
    int pin;
    float maxValue;
    int adcMax;
    float filtered = 0.0f;
    float smoothFactor = 0.85f; // 0.7 - smoother
    float roundTo = 0.05;

public:
    Potentiometer(const int pin,const float maxValue, const int adcMax = 4095):pin(pin),maxValue(maxValue),adcMax(adcMax) {}

    float getValueSmoothed() {
        const int raw = analogRead(pin);
        const float value = (float)raw / adcMax * maxValue;
        filtered = filtered * smoothFactor + value * (1.0f - smoothFactor);
        if (filtered > maxValue) filtered = maxValue;
        return roundf(filtered / roundTo) * roundTo;
    }

    void setSmoothFactor(float factor) {
        smoothFactor = factor;
    }

    void setRoundTo(float value) {
        roundTo = value;
    }

};
