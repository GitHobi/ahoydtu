// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <SPI.h>
#include <TFT_eSPI.h>





class DisplaySPI {
   public:
      DisplaySPI();

      void init(uint8_t type, uint8_t rot, uint8_t cs, uint8_t dc, uint8_t reset, uint8_t clock, uint8_t data, uint32_t *utcTs, const char* version);
      void config(bool enPowerSafe, bool enScreenSaver, uint8_t lum);
      void loop(void);
      void disp(float totalPower, float totalYieldDay, float totalYieldTotal, uint8_t isprod);

   private:

    int DISP_TIMEOUT= 60*10;  // in seconds

    TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

    int drawStringCentered ( uint16_t color, const GFXfont *font, const String &string, int lineWidth, int xpos, int ypos);

    float oldtotalPower = -1;
    float oldtotalYieldDay = -1;
    float oldtotalYieldTotal =-1;



      uint8_t mType;
      bool mEnPowerSafe, mEnScreenSaver;
      uint8_t mLuminance;

        int freq = 2000;
        int ledChannel = 8;
        int resolution = 8;

        bool clearScreen = false;

      uint16_t mTimeout;

};
