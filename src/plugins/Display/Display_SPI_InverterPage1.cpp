#include "Display_SPI_InverterPage1.h"



// uint16_t ThemeColor[] = {TFT_DARKGREEN, TFT_GREENYELLOW, TFT_SKYBLUE, TFT_BLUE};
uint16_t ThemeColor[] = {TFT_BLUE, TFT_SKYBLUE, TFT_SKYBLUE, TFT_BLUE, TFT_DARKGREY, TFT_SILVER};
#define SectionColor 0
#define SectionValue 1
#define BarColor1 2
#define BarColor2 3
#define SubSectionColor 4
#define SubSectionValue 5

void InverterPage::displayData(std::vector<float> data, float currentPower, float todayPower, float totalPower, DisplayDataSPI *inverter_data) {
    if (currentPower != oldtotalPower) {
        meter(currentPower, 0, 0, 200, 200, inverter_data);
        oldtotalPower = currentPower;
    }

    if (todayPower != oldtotalYieldDay) {
        meterToday(todayPower, 201, 0, 119, 100);
        oldtotalYieldDay = todayPower;
    }

    if (totalPower != oldtotalYieldTotal) {
        meterTotal(totalPower, 201, 90, 119, 100);
        oldtotalYieldTotal = totalPower;
    }

    // meterHistory(data, 0, 175, 320, 62);
    meterStatistics(inverter_data, 0, 175, 320, 62);
}

int InverterPage::drawData(TFT_eSprite &img, String label, String value, String unit, uint16_t posX, uint16_t posY, uint16_t width) {
    char str[20];
    auto px = width >> 1;
    int tw = 0;
    int th = 0;

    img.setTextColor(ThemeColor[SubSectionValue], TFT_BLACK);
    img.setFreeFont(&FreeSans9pt7b);
    tw = img.textWidth(unit);
    px = px - tw;
    img.drawString(unit, posX + px, posY);

    img.setTextColor(ThemeColor[SubSectionValue], TFT_BLACK);
    img.setFreeFont(&FreeSansBold9pt7b);
    tw = img.textWidth(value) + 3;
    px = px - tw;
    img.drawString(value, posX + px, posY);

    img.setTextColor(ThemeColor[SubSectionColor], TFT_BLACK);
    img.setFreeFont(&FreeSans9pt7b);
    auto h = img.fontHeight();
    img.drawString(label, posX, posY);

    return h;
}

void InverterPage::meterStatistics(DisplayDataSPI *data, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if (tft == nullptr)
        return;

    TFT_eSprite img = TFT_eSprite(tft);
    img.setColorDepth(16);
    img.createSprite(width, height);
    img.fillSprite(TFT_BLACK);

    int h = 0;

    {
        char str[20];
        snprintf(str, sizeof(str), "%3.2f", data->CH0_Irradiation);
        h += drawData(img, F("Exposure: "), String(str), F("%"), 0, h, width - 10);
    }

    {
        char str[20];
        snprintf(str, sizeof(str), "%3.0f", data->CH0_YieldDay);
        h += drawData(img, F("Today: "), String(str), F("Wh"), 0, h, width - 10);
    }

    {
        char str[20];
        snprintf(str, sizeof(str), "%3.1f", data->CH0_Power);
        h += drawData(img, F("Power: "), String(str), F("W"), 0, h, width - 10);
    }

    // ----

    h = 0;
    {
        char str[20];
        snprintf(str, sizeof(str), "%3.2f", data->CH1_Irradiation);
        h += drawData(img, F("Exposure: "), String(str), F("%"), (width >> 1) + 5, h, width - 10);
    }

    {
        char str[20];
        snprintf(str, sizeof(str), "%3.0f", data->CH1_YieldDay);
        h += drawData(img, F("Today: "), String(str), F("Wh"), (width >> 1) + 5, h, width - 10);
    }

    {
        char str[20];
        snprintf(str, sizeof(str), "%3.1f", data->CH1_Power);
        h += drawData(img, F("Power: "), String(str), F("W"), (width >> 1) + 5, h, width - 10);
    }



    img.pushSprite(x, y);
}

// =========================================================================
// Get coordinates of two ends of a line from r1 to r2, pivot at x,y, angle a
// =========================================================================
// Coordinates are returned to caller via the xp and yp pointers
#define DEG2RAD 0.0174532925
void InverterPage::getCoord(int16_t x, int16_t y, float *xp1, float *yp1, float *xp2, float *yp2, int16_t r1, int16_t r2, float a) {
    float sx = cos((a - 90) * DEG2RAD);
    float sy = sin((a - 90) * DEG2RAD);
    *xp1 = sx * r1 + x;
    *yp1 = sy * r1 + y;
    *xp2 = sx * r2 + x;
    *yp2 = sy * r2 + y;
}

// =========================================================================
// Return a 16 bit rainbow colour
// =========================================================================
unsigned int InverterPage::rainbow(byte value) {
    // Value is expected to be in range 0-127
    // The value is converted to a spectrum colour from 0 = blue through to 127 = red

    byte red = 0;    // Red is the top 5 bits of a 16 bit colour value
    byte green = 0;  // Green is the middle 6 bits
    byte blue = 0;   // Blue is the bottom 5 bits

    byte quadrant = value / 32;

    if (quadrant == 0) {
        blue = 31;
        green = 2 * (value % 32);
        red = 0;
    }
    if (quadrant == 1) {
        blue = 31 - (value % 32);
        green = 63;
        red = 0;
    }
    if (quadrant == 2) {
        blue = 0;
        green = 63;
        red = value % 32;
    }
    if (quadrant == 3) {
        blue = 0;
        green = 63 - 2 * (value % 32);
        red = 31;
    }
    return (red << 11) + (green << 5) + blue;
}

void InverterPage::meterHistory(std::vector<float> data, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if (tft == nullptr)
        return;

    TFT_eSprite img = TFT_eSprite(tft);
    img.setColorDepth(16);
    img.createSprite(width, height);
    img.fillSprite(TFT_BLACK);

    // tft.fillRect(x, y, x + width, y + width, TFT_BLACK);

    /// uint16_t data[] = {2312, 2543, 123, 2343, 1235, 1211, 1000, 500, 543, 2400, 2342};
    // tft.drawRect(x, y, width, height, TFT_RED);
    // auto dataSize = sizeof(data) / sizeof(uint16_t);
    auto dataSize = data.size();
    if (dataSize == 0)
        dataSize = 1;

    auto dist = 3;
    auto elementWidth = (width - ((dataSize - 1) * dist)) / dataSize;
    auto maxValue = 50;
    for (auto i : data) {
        if (i > maxValue)
            maxValue = i;
    }

    for (auto i = 0; i < maxValue; i += 100) {
        auto h = (float)height * ((float)i / (float)maxValue);
        img.drawLine(0, 0 + height - h, 0 + width, 0 + height - h, TFT_DARKGREY);
    }

    auto xpos = 0;
    for (auto i : data) {
        auto h = (float)height * ((float)i / (float)maxValue);
        // tft.fillRectVGradient ( xpos, y+height-h, elementWidth, h, TFT_DARKGREEN, TFT_GREENYELLOW );
        img.fillRectVGradient(xpos, 0 + height - h, elementWidth, h, ThemeColor[BarColor1], ThemeColor[BarColor2]);
        xpos += (elementWidth + dist);
    }

    img.pushSprite(x, y);
}

void InverterPage::meterToday(float power, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if (tft == nullptr)
        return;

    TFT_eSprite img = TFT_eSprite(tft);
    img.setColorDepth(16);
    img.createSprite(width, height);
    img.fillSprite(TFT_BLACK);

    char str[20] = "TODAY";
    img.setTextColor(ThemeColor[SectionColor], TFT_BLACK);
    img.setFreeFont(&FreeSans9pt7b);
    auto tw = img.textWidth(str);
    auto vh = img.fontHeight();
    img.drawString(str, 0 + width - tw, 0);

    img.setTextColor(ThemeColor[SectionValue], TFT_BLACK);
    img.setFreeFont(&FreeSansBold12pt7b);
    snprintf(str, sizeof(str), "%3.1f", power);
    tw = img.textWidth(str);
    auto vh2 = img.fontHeight();
    img.drawString(str, 0 + width - tw, 0 + vh + 2);

    img.setTextColor(ThemeColor[SectionValue], TFT_BLACK);
    img.setFreeFont(&FreeSans9pt7b);
    snprintf(str, sizeof(str), "%s", "Wh");
    tw = img.textWidth(str);
    img.drawString(str, 0 + width - tw, 0 + vh + vh2 - 5);

    img.pushSprite(x, y);
}

void InverterPage::meterTotal(float power, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if (tft == nullptr)
        return;

    TFT_eSprite img = TFT_eSprite(tft);
    img.setColorDepth(16);
    img.createSprite(width, height);
    img.fillSprite(TFT_BLACK);

    char str[20] = "TOTAL";
    char unit[20];

    img.setTextColor(ThemeColor[SectionColor], TFT_BLACK);
    img.setFreeFont(&FreeSans9pt7b);
    auto tw = img.textWidth(str);
    auto vh = img.fontHeight();
    img.drawString(str, 0 + width - tw, 0);

    // if (power > 999) {
    //     snprintf(str, sizeof(str), "%2.2f", (power / 1000.0));
    snprintf(unit, sizeof(unit), "%s", "kWh");
    //} else {
    snprintf(str, sizeof(str), "%.2f", power);
    //    snprintf(unit, sizeof(unit), "%s", "W");
    // }

    img.setTextColor(ThemeColor[SectionValue], TFT_BLACK);
    img.setFreeFont(&FreeSansBold12pt7b);
    tw = img.textWidth(str);
    auto vh2 = img.fontHeight();
    img.drawString(str, 0 + width - tw, 0 + vh + 2);

    img.setTextColor(ThemeColor[SectionValue], TFT_BLACK);
    img.setFreeFont(&FreeSans9pt7b);
    tw = img.textWidth(unit);
    img.drawString(unit, 0 + width - tw, 0 + vh + vh2 -5 ) ;

    img.pushSprite(x, y);
}

unsigned int InverterPage::adjustColor(unsigned int color) {
    auto r = (color >> 8) & 0xF8;
    auto g = (color >> 3) & 0xFC;
    auto b = (color << 3) & 0xF8;

    uint8_t scaled_r = (r * 255) / 248;
    uint8_t scaled_g = (g * 255) / 252;
    uint8_t scaled_b = (b * 255) / 248;

    return tft->color565(r / 3, g / 3, b / 3);
}

void InverterPage::meter(float power, uint8_t x, uint8_t y, uint8_t width, uint8_t height, DisplayDataSPI *inverter_data) {
    if (tft == nullptr)
        return;

    TFT_eSprite img = TFT_eSprite(tft);
    img.setColorDepth(16);
    /*
        Where do the magic numer 170 come from?
        We are not drawing a full circle, but an arc, spanning 270°. So we do not cover the full height of the circle.
        The circle has a radius of 100 pixel. (give by the values I used)
        We need to calculate the position of the starting point of the arc - concret: We need the distance on the y-achsis to the center of the arc.
        The distance = sin(-135) * 100 = ~70.
        This this value we add the upper part of the arc (above the center), which is 100px.
        So the absolute height will be ~170pixel.
    */
    img.createSprite(width, 170);
    img.fillSprite(TFT_BLACK);



    //img.fillRect(x, y, x + width, y + width, TFT_BLACK);

    auto isprod = power > 0;
    auto lastColor = TFT_BLACK;

    {
        // -135 ... 135
        //      270
        float value = (270.0 * (power / maxPower)) - 135.0;

        // Draw a segmented ring meter type display
        // Centre of screen
        // int cx = tft.width()  / 2;
        // int cy = tft.height() / 2;
        int cx = 0 + width >> 1;
        int cy = 0 + height >> 1;

        // Inner and outer radius of ring
        float r1 = min(cx, cy) - 40.0;
        float r2 = min(cx, cy) - 10.0;

        // Inner and outer line width
        int w1 = r1 / 25;
        int w2 = r2 / 20;

        // The following will be updated by the getCoord function
        float px1 = 0.0;
        float py1 = 0.0;
        float px2 = 0.0;
        float py2 = 0.0;

        // Wedge line function, an anti-aliased wide line between 2 points, with different
        // line widths at the two ends. Background colour is black.

        for (int angle = -130; angle <= 130; angle += 10) {
            getCoord(cx, cy, &px1, &py1, &px2, &py2, r1, r2, angle);
            uint16_t colour = rainbow(map(angle, -130, 130, 0, 127));

            if (angle > value)
                // colour = tft->color565(16, 16, 16);
                colour = adjustColor(colour);

            else
                lastColor = colour;
            img.drawWedgeLine(px1, py1, px2, py2, w1, w2, colour, TFT_BLACK);
        }

        // Smooth dark red filled circle
        img.fillSmoothCircle(cx, cy, r1 - 8, TFT_BLACK, TFT_BLACK);

        // getCoord(cx, cy, &px1, &py1, &px2, &py2, 0, r1 - 10, oldValue);
        // tft.drawWedgeLine(cx, cy, px2, py2, 10, 0, TFT_BLACK, TFT_BLACK);

        // Draw a white dial pointer using wedge line function
        // getCoord(cx, cy, &px1, &py1, &px2, &py2, 0, r1 - 10, value);
        // Magenta wedge line pointer on red background
        // Line tapers from radius 5 to zero
        // tft.drawWedgeLine(cx, cy, px2, py2, 10, 0, TFT_WHITE, TFT_BLACK);

        // ----
    }

    //auto mainValueColorToUse = lastColor;

    char str[20];
    char unit[5];

    if (isprod == 0) {
        img.setFreeFont(&FreeSansBold18pt7b);
        img.setTextColor(TFT_RED);
        snprintf(str, sizeof(str), "%s", "OFFLINE");
        snprintf(unit, sizeof(unit), "%s", "");
    } else {
        img.setFreeFont(&FreeSansBold24pt7b);
        img.setTextColor(ThemeColor[SectionValue], TFT_BLACK);
        if (power > 999) {
            snprintf(str, sizeof(str), "%2.2f", (power / 1000));
            snprintf(unit, sizeof(unit), "%s", "kW");
        } else {
            snprintf(str, sizeof(str), "%.0f", power);
            snprintf(unit, sizeof(unit), "%s", "W");
        }
    }


    auto tw = img.textWidth(str);
    auto vh = img.fontHeight();
    auto py = y + (height - vh) >> 1;
    //

    img.drawString(str, x + (width - tw) >> 1, py+5);

    img.setFreeFont(&FreeSans12pt7b);
    tw = img.textWidth(unit);
    py += vh;
    img.drawString(unit, x + (width - tw) >> 1, py-7);

    if (inverter_data != nullptr) {
        auto t = inverter_data->Temperature > 70.0f ? 70.0f : inverter_data->Temperature;
        t -= 20.0f;
        t = t < 0.0f ? 0.0 : t;
        auto color = rainbow ( (t / 50.0) * 127.0 ); // we show temperature in range of 20-70
        img.setTextColor(color, TFT_BLACK);

        snprintf(str, sizeof(str), "%2.1f C", inverter_data->Temperature);
        img.setFreeFont(&FreeSans9pt7b);
        tw = img.textWidth(str);
        img.drawString(str, x + (width - tw) >> 1, 55);

        img.drawSmoothCircle(x + ((width - tw) >> 1) + tw - img.textWidth("C") -1, 55, 2, color, TFT_BLACK);
    }

    // tft.drawSmoothCircle(cx, cy, r2+w2, TFT_RED, TFT_BLACK);

    img.pushSprite(x, y);
}
