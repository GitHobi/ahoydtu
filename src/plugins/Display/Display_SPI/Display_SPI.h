// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <SPI.h>
#include <TFT_eSPI.h>

#include "../../../utils/dbg.h"
#include "../../../utils/helper.h"

#include "Display_SPI_Data.h"
#include "Display_SPI_DataStorage.h"
#include "Display_SPI_InverterPage1.h"



class DisplaySPI {
   public:
    DisplaySPI();

    void init(uint8_t type, uint8_t rot, uint8_t cs, uint8_t dc, uint8_t reset, uint8_t clock, uint8_t data, uint32_t *utcTs, const char *version);
    void config(bool enPowerSafe, bool enScreenSaver, uint8_t lum);
    void loop(void);
    void disp(float totalPower, float totalYieldDay, float totalYieldTotal, uint8_t isprod, DisplayDataSPI *data);

    void tickerDisplay(float totalYieldDay) {
        DPRINTLN(DBG_DEBUG, String(__PRETTY_FUNCTION__) + String(": ") + String(totalYieldDay));
    }

    void tickerMinute(float totalYieldDay) {
        DPRINTLN(DBG_DEBUG, String(__PRETTY_FUNCTION__) + String(": ") + String(totalYieldDay));
        /*
        if (totalYieldDay < oldTotalYieldDay) {
            // Tageswechsel
            oldTotalYieldDay = 0;
        }
        auto delta = totalYieldDay - oldTotalYieldDay;
        if (oldTotalYieldDay != 0)
            dataStorage.pushData(delta);
        oldTotalYieldDay = totalYieldDay;*/


/*
        uint32_t localTime = gTimezone.toLocal(ah::Scheduler::getTimeStamp());
        auto lastMidnight = localTime - (localTime % 86400);
        uint16_t currentHour   = (localTime - lastMidnight) / 3600;
        uint16_t currentMinute = (((localTime - lastMidnight)) - (3600 * currentHour)) / 60 ;

        Serial.println ( "b = " + String(lastMidnight) + " - hour " + String(currentHour));
        Serial.println ( "b = " + String(lastMidnight) + " - minute " + String(currentMinute));

        auto a = lastMidnight + (60*60*1*(currentHour)+(60*(currentMinute+1))+30);
        Serial.println ( "a = " + String(localTime) + " - " + String(a));
        uint32_t nxtTrg = gTimezone.toUTC(a);  // next midnight local time
        onceAt(std::bind(&this, &mDisplay), nxtTrg, "dispm2");
*/

    }

   private:
    DataStorage dataStorage;

    float oldTotalYieldDay = 0;

    int DISP_TIMEOUT = 60 * 10;  // in seconds

    TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
    InverterPage inverterPage = InverterPage(&tft, 600.0);

    int drawStringCentered(uint16_t color, const GFXfont *font, const String &string, int lineWidth, int xpos, int ypos);

    uint8_t mType;
    bool mEnPowerSafe, mEnScreenSaver;
    uint8_t mLuminance;

    int freq = 2000;
    int ledChannel = 8;
    int resolution = 8;

    bool clearScreen = false;

    uint16_t mTimeout;
};
