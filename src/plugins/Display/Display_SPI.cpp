// SPDX-License-Identifier: GPL-2.0-or-later
#include "Display_SPI.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif




DisplaySPI::DisplaySPI() {
    mEnPowerSafe = true;
    mEnScreenSaver = true;
    mLuminance = 60;
    mTimeout = DISP_TIMEOUT;

    freq = 5000;
    ledChannel = 8;
    resolution = 8;
}



void DisplaySPI::init(uint8_t type, uint8_t rotation, uint8_t cs, uint8_t dc, uint8_t reset, uint8_t clock, uint8_t data, uint32_t *utcTs, const char *version) {
#ifdef DisplaySPI_DEBUG
    DPRINTLN(DBG_DEBUG, "DisplaySPI init ----------------");
    DPRINTLN(DBG_DEBUG, String("SCK:  ") + String(SCK) + String(" - ") + String(TFT_SCLK));
    DPRINTLN(DBG_DEBUG, String("CS:    ") + String(SS) + String("   -   ") + String(TFT_CS));
    DPRINTLN(DBG_DEBUG, String("RST:  --") + String("   -   ") + String(TFT_RST));
    DPRINTLN(DBG_DEBUG, String("DC:   --") + String("   -   ") + String(TFT_DC));
    DPRINTLN(DBG_DEBUG, String("BL:   --") + String("   -   ") + String(TFT_BL));
#endif

    tft.init();
    tft.setRotation(rotation);

    tft.fillScreen(TFT_BLACK);

    tft.setFreeFont(&FreeSansBold24pt7b);
    auto vh = tft.fontHeight();
    auto ypos = (tft.height() - vh) >> 1;
    auto dy = drawStringCentered(TFT_GOLD, &FreeSansBold24pt7b, "AHOY dtu!",tft.width(), 0, ypos);
    dy = drawStringCentered(tft.color565(32, 32, 32), &FreeSansBold12pt7b, "Hobi Version", tft.width(), 0, ypos + dy);
    dy = drawStringCentered(TFT_WHITE, &FreeSansBold12pt7b, version, tft.width(), 0, ypos + dy);


    clearScreen = true;

    if (TFT_BL != -1) {
        DPRINTLN(DBG_DEBUG, F("Setting up PWM for panel backlight"));

        ledcSetup(ledChannel, freq, resolution);
        ledcAttachPin(TFT_BL, ledChannel);
        ledcWrite(ledChannel, (int)((mLuminance * 255.0) / 100.0));
    }

    //tft.pushImage (0,0,153,159,logo);
}

void DisplaySPI::config(bool enPowerSafe, bool enScreenSaver, uint8_t lum) {
#ifdef DisplaySPI_DEBUG
    DPRINTLN(DBG_DEBUG, "DisplaySPI config ----------------");
    DPRINTLN(DBG_DEBUG, "DisplaySPI config mEnPowerSafe    " + String(mEnPowerSafe));
    DPRINTLN(DBG_DEBUG, "DisplaySPI config mEnScreenSaver  " + String(mEnScreenSaver));
    DPRINTLN(DBG_DEBUG, "DisplaySPI config mLuminance      " + String(mLuminance));
#endif

    mEnPowerSafe = enPowerSafe;
    mEnScreenSaver = enScreenSaver;
    mLuminance = lum;
}

void DisplaySPI::loop(void) {
    DBGPRINTLN("DisplaySPI loop ----------------");
    DBGPRINTLN("DisplaySPI config mEnPowerSafe " + String(mEnPowerSafe));
    DBGPRINTLN("DisplaySPI config mTimeout     " + String(mTimeout));
    if (mEnPowerSafe)
        if (mTimeout != 0)
            mTimeout--;
}

int DisplaySPI::drawStringCentered(uint16_t color, const GFXfont *font, const String &string, int lineWidth, int xpos, int ypos) {
    tft.setFreeFont(font);
    auto tw = tft.textWidth(string);

    tft.setTextColor(color, TFT_BLACK);
    tft.drawString(string, xpos + (lineWidth - tw) >> 1, ypos);

    return tft.fontHeight();
}

void DisplaySPI::disp(float totalPower, float totalYieldDay, float totalYieldTotal, uint8_t isprod) {
#ifndef DisplaySPI_DEBUG
    DPRINTLN(DBG_DEBUG, "DisplaySPI disp ----------------");
    DPRINTLN(DBG_DEBUG, "DisplaySPI disp totalPower      " + String(totalPower));
    DPRINTLN(DBG_DEBUG, "DisplaySPI disp totalYieldDay   " + String(totalYieldDay));
    DPRINTLN(DBG_DEBUG, "DisplaySPI disp totalYieldTotal " + String(totalYieldTotal));
    DPRINTLN(DBG_DEBUG, "DisplaySPI disp isprod          " + String(isprod));
    DPRINTLN(DBG_DEBUG, "DisplaySPI disp TFT_BL          " + String(TFT_BL));
    DPRINTLN(DBG_DEBUG, "DisplaySPI disp LEDCHannel      " + String(ledChannel));
#endif

    if (clearScreen) {
        clearScreen = false;
        tft.fillScreen(TFT_BLACK);
    }

    //std::vector<int> data;

    if ( !isprod && mTimeout == 0 ) {
        ledcWrite(ledChannel, 0);
        DPRINTLN(DBG_DEBUG, F("DisplaySPI blanking screen"));
        return;
    }



    if ( isprod && mTimeout == 0 ) {
        ledcWrite(ledChannel, mLuminance );
        DPRINTLN(DBG_DEBUG, F("DisplaySPI unblanking screen"));
    }



    inverterPage.displayData (dataStorage.getData(), totalPower, totalYieldDay, totalYieldTotal);

    if ( !isprod && mTimeout > 0  )
    {
        return;
    }

    mTimeout = DISP_TIMEOUT;


/*
    auto mainValueColor = tft.color565(64, 64, 64);
    auto subValueColor = tft.color565(32, 32, 32);
    auto labelColor = tft.color565(8, 8, 8);

    if (totalPower == 0 && isprod == 0) {
        if (mTimeout == 0) {
            DBGPRINTLN("DisplaySPI disp mTimeOut");

            if (TFT_BL != -1) {
                ledcWrite(ledChannel, 0);
                DBGPRINTLN("DisplaySPI disp blanking screen");
            } else {
                tft.fillScreen(TFT_BLACK);
            }
        }
    } else {
        if (TFT_BL != -1) {
            ledcWrite(ledChannel, (int)((mLuminance * 255.0) / 100.0));
            DBGPRINTLN("DisplaySPI disp unblanking screen");
        }
    }

    // tft.fillScreen(TFT_BLACK);
    char str[20];
    if (oldtotalPower != totalPower) {
        tft.setFreeFont(&FreeSansBold24pt7b);
        auto vh = tft.fontHeight();
        auto mainValueColorToUse = mainValueColor;
        if (isprod == 0) {
            mainValueColorToUse = TFT_RED;
            snprintf(str, sizeof(str), "%s", "OFFLINE");
            mTimeout = DISP_TIMEOUT;

        } else {
            mTimeout = DISP_TIMEOUT;
            if (TFT_BL != -1) {
                ledcWrite(ledChannel, 255);
            }

            if (totalPower > 999) {
                snprintf(str, sizeof(str), "%2.2f kW", (totalPower / 1000));
            } else {
                snprintf(str, sizeof(str), "%3.0f W", totalPower);
            }
        }
        drawStringCentered(mainValueColorToUse, &FreeSansBold24pt7b, String(str), TFT_WIDTH, 0, (TFT_HEIGHT - vh) >> 1);

        oldtotalPower = totalPower;
    }

    if (oldtotalYieldDay != totalYieldDay) {
        auto h = drawStringCentered(labelColor, &FreeSans9pt7b, "TODAY", TFT_WIDTH, 0, 25);
        drawStringCentered(subValueColor, &FreeSansBold12pt7b, String((int)totalYieldDay) + " Wh", TFT_WIDTH, 0, 25 + h);
        oldtotalYieldDay = totalYieldDay;
    }

    if (oldtotalYieldTotal != totalYieldTotal) {
        snprintf(str, sizeof(str), "%.1f kWh", totalYieldTotal);
        auto h = drawStringCentered(subValueColor, &FreeSansBold12pt7b, String(str), TFT_WIDTH, 0, 155);
        drawStringCentered(labelColor, &FreeSans9pt7b, "TOTAL", TFT_WIDTH, 0, 155 + h);
        oldtotalYieldTotal = totalYieldTotal;
    }

    IPAddress ip = WiFi.localIP();
    drawStringCentered(tft.color565(4, 4, 4), &FreeSans9pt7b, ip.toString(), TFT_WIDTH, 0, TFT_HEIGHT - tft.fontHeight() - 2);
*/
}


