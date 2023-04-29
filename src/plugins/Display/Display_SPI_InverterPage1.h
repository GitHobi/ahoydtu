#pragma once

#include <TFT_eSPI.h>  // Hardware-specific library

#include <vector>



class InverterPage {
   public:
    InverterPage(TFT_eSPI *aTFT, float aMaxPower = 600)
        : tft(aTFT), maxPower(aMaxPower) {
    }

    void displayData(std::vector<float> data, float currentPower, float todayPower, float totalPower);



   private:
    TFT_eSPI *tft;
    float maxPower = 600;


    void meterHistory(std::vector<float> data, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void meterToday(float power, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void meterTotal(float power, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void meter(float power, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

    void getCoord(int16_t x, int16_t y, float *xp1, float *yp1, float *xp2, float *yp2, int16_t r1, int16_t r2, float a);
    unsigned int rainbow(byte value);

    float oldtotalPower = -1;
    float oldtotalYieldDay = -1;
    float oldtotalYieldTotal = -1;
};