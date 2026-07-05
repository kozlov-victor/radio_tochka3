
class Potentiometer {
private:
    int pin;
    float maxValue;
    int adcMax;
    float filtered = 0.0f;
    int lastStep = -1;
    bool initialized = false;
    float smoothFactor = 0.85f; // 0.7 - smoother

public:
    Potentiometer(int pin, float maxValue, int adcMax = 4095):pin(pin),maxValue(maxValue),adcMax(adcMax) {}

    float getValueSmoothed() {
        int raw = analogRead(pin);

        float value = (float)raw / adcMax * maxValue;

        if (!initialized) {
            filtered = value;
            lastStep = (int)roundf(filtered * 10.0f);
            initialized = true;
            return lastStep / 10.0f;
        }

        filtered = filtered * smoothFactor + value * (1.0f - smoothFactor);

        int step = (int)roundf(filtered * 10.0f);

        int maxStep = (int)roundf(maxValue * 10.0f);

        if (step < 0) step = 0;
        if (step > maxStep) step = maxStep;

        lastStep = step;

        return step / 10.0f;
    }

    void setSmoothFactor(float factor) {
        smoothFactor = factor;
    }

};