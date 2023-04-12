// SPDX-License-Identifier: GPL-2.0-or-later
#include "Display_SPI.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include "../../utils/dbg.h"
#include "../../utils/helper.h"

// #ifdef U8X8_HAVE_HW_SPI
// #include <SPI.h>
// #endif
// #ifdef U8X8_HAVE_HW_I2C
// #include <Wire.h>
// #endif

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
    DBGPRINTLN("DisplaySPI init ----------------");
    Serial.print("SCK:  ");
    Serial.print(SCK);
    Serial.print("   -   ");
    Serial.println(TFT_SCLK);

    Serial.print("CS:    ");
    Serial.print(SS);
    Serial.print("   -   ");
    Serial.println(TFT_CS);

    Serial.print("RST:  --");
    Serial.print("   -   ");
    Serial.println(TFT_RST);

    Serial.print("DC:   --");
    Serial.print("   -   ");
    Serial.println(TFT_DC);

    Serial.print("BL:   --");
    Serial.print("   -   ");
    Serial.println(TFT_BL);

    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);

    tft.setFreeFont(&FreeSansBold24pt7b);
    auto vh = tft.fontHeight();
    auto ypos = (TFT_HEIGHT - vh) >> 1;
    auto dy = drawStringCentered(TFT_GOLD, &FreeSansBold24pt7b, "AHOY dtu!", TFT_WIDTH, 0, ypos);
    dy = drawStringCentered(tft.color565(32, 32, 32), &FreeSansBold12pt7b, "Hobi Version", TFT_WIDTH, 0, ypos + dy);
    dy = drawStringCentered(TFT_WHITE, &FreeSansBold12pt7b, version, TFT_WIDTH, 0, ypos + dy);

    // tft.drawBitmap(0, 0, logo, 200, 200, TFT_WHITE);

    clearScreen = true;

    if (TFT_BL != -1) {
        Serial.println("Setting up PWM for panel backlight");

        ledcSetup(ledChannel, freq, resolution);
        ledcAttachPin(TFT_BL, ledChannel);
        ledcWrite(ledChannel, (int)((mLuminance * 255.0) / 100.0));
    }
}

void DisplaySPI::config(bool enPowerSafe, bool enScreenSaver, uint8_t lum) {
    DBGPRINTLN("DisplaySPI config ----------------");
    DBGPRINTLN("DisplaySPI config mEnPowerSafe    " + String(mEnPowerSafe));
    DBGPRINTLN("DisplaySPI config mEnScreenSaver  " + String(mEnScreenSaver));
    DBGPRINTLN("DisplaySPI config mLuminance      " + String(mLuminance));

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
    DBGPRINTLN("DisplaySPI disp ----------------");
    DBGPRINTLN("DisplaySPI disp totalPower      " + String(totalPower));
    DBGPRINTLN("DisplaySPI disp totalYieldDay   " + String(totalYieldDay));
    DBGPRINTLN("DisplaySPI disp totalYieldTotal " + String(totalYieldTotal));
    DBGPRINTLN("DisplaySPI disp isprod          " + String(isprod));
    DBGPRINTLN("DisplaySPI disp TFT_BL          " + String(TFT_BL));
    DBGPRINTLN("DisplaySPI disp LEDCHannel      " + String(ledChannel));

    if (clearScreen) {
        clearScreen = false;
        tft.fillScreen(TFT_BLACK);
    }

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

}


