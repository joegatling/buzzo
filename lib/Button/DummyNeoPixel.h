#pragma once
#include <Arduino.h>


class DummyNeoPixel 
{
public:
    DummyNeoPixel(uint16_t countPixels, uint8_t pin) {}
    ~DummyNeoPixel() {} 

    void ClearTo(RgbColor color) {};
    void Show() {};
    void Begin() {};

    void SetPixelColor(uint16_t index, RgbColor color) {};


    uint16_t PixelCount() const { return 0; }; 

    void SetLuminance(uint8_t luminance) {};
};
