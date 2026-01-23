#include "ClockWidget.h"

ClockWidget::ClockWidget(ScreenManager &manager, ConfigManager &config) : Widget(manager, config) {
    m_enabled = true; // Always enabled, do not add a config setting for it
    addConfigToManager();
}

ClockWidget::~ClockWidget() {
}

void ClockWidget::addConfigToManager() {
    String optClockType[4 + USE_CLOCK_CUSTOM] = {
        "Normal Clock",
        "Nixie Clock",
        "Flip Clock",
        "Morph Clock"};
    if (!USE_CLOCK_NIXIE)
        optClockType[(int) ClockType::NIXIE] += " (n/a)";
    if (!USE_CLOCK_FLIP)
        optClockType[(int) ClockType::FLIP] += " (n/a)";
    // if (!USE_CLOCK_MORPH || !m_config.getConfigBool("MorphEnable", m_enableMorph))
    if (!USE_CLOCK_MORPH)
        optClockType[(int) ClockType::MORPH] += " (n/a)";
    for (int i = 0; i < USE_CLOCK_CUSTOM; i++) {
        optClockType[(int) ClockType::CUSTOM0 + i] = "Custom Clock " + String(i);
    }
    m_config.addConfigComboBox("ClockWidget", "defaultType", &m_type, optClockType, 4 + USE_CLOCK_CUSTOM, "Default Clock Type (you can also switch types with the middle button)");
#if USE_CLOCK_CUSTOM > 0
    // Get enabled setting here to know which clocks are valid,
    // because we did not add the config key for it yet (this happens some lines below)
    for (int i = 0; i < USE_CLOCK_CUSTOM; i++) {
        String enKey = Utils::createConstCharBufferAndConcat("clkCust", String(i).c_str(), "en");
        m_customEnabled[i] = m_config.getConfigBool(enKey.c_str(), m_customEnabled[i]);
    }
#endif
    if (!isValidClockType(m_type)) {
        // Invalid Clock Type
        m_type = (int) ClockType::NORMAL;
    }
    String optFormats[] = {"24h mode", "12h mode", "12h mode (with am/pm)"};
    m_config.addConfigComboBox("ClockWidget", "clockFormat", &m_format, optFormats, 3, "Clock Format");
    m_config.addConfigBool("ClockWidget", "showSecondTicks", &m_showSecondTicks, "Show Second Ticks", true);
    m_config.addConfigColor("ClockWidget", "clkColor", &m_fgColor, "Clock Color", true);
    m_config.addConfigBool("ClockWidget", "clkShadowing", &m_shadowing, "Clock Shadowing", true);
    m_config.addConfigColor("ClockWidget", "clkShColor", &m_shadowColor, "Clock Shadow Color", true);
#if USE_CLOCK_NIXIE > 0
    m_config.addConfigColor("ClockWidget", "clkNixieColor", &m_overrideNixieColor, "Override Nixie color (black=disable)", true);
#endif
#if USE_CLOCK_MORPH > 0
    //    m_config.addConfigBool("ClockWidget", "MorphEnable", &m_enableMorph, "Enable MORPH clock type", true);
    m_config.addConfigInt("ClockWidget", "MorphAnimRate", &animDelay, "Morph clock animation rate (in ms)", true);
#endif
#if USE_CLOCK_CUSTOM > 0
    for (int i = 0; i < USE_CLOCK_CUSTOM; i++) {
        const char *enKey = Utils::createConstCharBufferAndConcat("clkCust", String(i).c_str(), "en");
        const char *enDesc = Utils::createConstCharBufferAndConcat("CustomClock", String(i).c_str(), ": Enable");
        m_config.addConfigBool("ClockWidget", enKey, &m_customEnabled[i], enDesc, true);
        const char *tickKey = Utils::createConstCharBufferAndConcat("clkCust", String(i).c_str(), "tckCol");
        const char *tickDesc = Utils::createConstCharBufferAndConcat("CustomClock", String(i).c_str(), ": Second Tick Color");
        m_config.addConfigColor("ClockWidget", tickKey, &m_customTickColor[i], tickDesc, true);
        const char *overrideKey = Utils::createConstCharBufferAndConcat("clkCust", String(i).c_str(), "ovrCol");
        const char *overrideDesc = Utils::createConstCharBufferAndConcat("CustomClock", String(i).c_str(), ": Override color (black=disable)");
        m_config.addConfigColor("ClockWidget", overrideKey, &m_customOverrideColor[i], overrideDesc, true);
    }
#endif
}

void ClockWidget::setup() {
    m_lastDisplay1Digit = "";
    m_lastDisplay2Digit = "";
    m_lastDisplay4Digit = "";
    m_lastDisplay5Digit = "";
}

void ClockWidget::draw(bool force) {
    m_manager.setFont(CLOCK_FONT);
    GlobalTime *time = GlobalTime::getInstance();

    if (m_secondSingle != m_lastSecondSingle || force) {
        if (m_secondSingle % 2 == 0) {
            displayDigit(2, "", ":", m_fgColor, false);
        } else {
            displayDigit(2, "", ":", m_shadowColor, false);
        }
        if (m_showSecondTicks) {
            if (!isCustomClock(m_type)) {
                // not a custom clock -> clear background
                displaySeconds(2, m_lastSecondSingle, TFT_BLACK);
            }
            displaySeconds(2, m_secondSingle, m_fgColor);
        }
        if (m_type == (int) ClockType::NORMAL || m_type == (int) ClockType::MORPH) {
            if (m_format == CLOCK_FORMAT_12_HOUR_AMPM) {
                if (m_amPm != m_lastAmPm) {
                    // Clear old AM/PM
                    displayAmPm(m_lastAmPm, TFT_BLACK);
                    m_lastAmPm = m_amPm;
                }
                displayAmPm(m_amPm, m_fgColor);
            }
        }
        m_lastSecondSingle = m_secondSingle;
        m_manager.setFont(CLOCK_FONT);
    }

    if (m_lastDisplay5Digit != m_display5Digit || force) {
        if (force)
            m_lastDisplay5Digit = m_display5Digit;
        displayDigit(4, m_lastDisplay5Digit, m_display5Digit, m_fgColor);
        m_lastDisplay5Digit = m_display5Digit;
    }

    if (m_lastDisplay4Digit != m_display4Digit || force) {
        if (force)
            m_lastDisplay4Digit = m_display4Digit;
        displayDigit(3, m_lastDisplay4Digit, m_display4Digit, m_fgColor);
        m_lastDisplay4Digit = m_display4Digit;
    }

    if (m_lastDisplay2Digit != m_display2Digit || force) {
        if (force)
            m_lastDisplay2Digit = m_display2Digit;
        displayDigit(1, m_lastDisplay2Digit, m_display2Digit, m_fgColor);
        m_lastDisplay2Digit = m_display2Digit;
    }

    if (m_lastDisplay1Digit != m_display1Digit || force) {
        if (force)
            m_lastDisplay1Digit = m_display1Digit;
        displayDigit(0, m_lastDisplay1Digit, m_display1Digit, m_fgColor);
        m_lastDisplay1Digit = m_display1Digit;
    }
}

void ClockWidget::displayAmPm(String &amPm, uint32_t color) {
    m_manager.selectScreen(2);
    m_manager.setFontColor(color, TFT_BLACK);
    // Workaround for 12h AM/PM problem
    // The colon is slightly offset and that's a problem because to remove them, we paint over them
    // I think this is related to the TTF cache
    // The problem disappears if we reload the font here
    if (CLOCK_FONT == TTF_Font::DSEG7) {
        // We set a new font anyway
        m_manager.setFont(TTF_Font::DSEG14);
    } else {
        // Force reloading the font
        m_manager.setFont(TTF_Font::NONE);
        m_manager.setFont(CLOCK_FONT);
    }
    m_manager.drawString(amPm, SCREEN_SIZE / 5 * 4, SCREEN_SIZE / 2, 25, Align::MiddleCenter);
}

void ClockWidget::update(bool force) {
    if (millis() - m_secondTimerPrev < m_secondTimer && !force) {
        return;
    }

    GlobalTime *time = GlobalTime::getInstance();
    if (force) {
        time->updateTime(true);
    }

    m_hourSingle = time->getHour();
    m_minuteSingle = time->getMinute();
    m_secondSingle = time->getSecond();
    m_amPm = time->isPM() ? "PM" : "AM";

    if (m_lastHourSingle != m_hourSingle || force) {
        if (m_hourSingle < 10) {
            if (m_format == CLOCK_FORMAT_24_HOUR) {
                m_display1Digit = "0";
            } else {
                m_display1Digit = " ";
            }
        } else {
            m_display1Digit = int(m_hourSingle / 10);
        }
        m_display2Digit = m_hourSingle % 10;

        m_lastHourSingle = m_hourSingle;
    }

    if (m_lastMinuteSingle != m_minuteSingle || force) {
        String currentMinutePadded = String(m_minuteSingle).length() == 1 ? "0" + String(m_minuteSingle) : String(m_minuteSingle);

        m_display4Digit = currentMinutePadded.substring(0, 1);
        m_display5Digit = currentMinutePadded.substring(1, 2);

        m_lastMinuteSingle = m_minuteSingle;
    }
}

void ClockWidget::changeFormat() {
    GlobalTime *time = GlobalTime::getInstance();
    m_format++;
    if (m_format > 2) {
        m_format = 0;
    }
    time->setFormat24Hour(m_format == CLOCK_FORMAT_24_HOUR);
    m_manager.clearAllScreens();
    update(true);
    draw(true);
}

bool ClockWidget::isCustomClock(int clockType) {
    return (clockType >= (int) ClockType::CUSTOM0 && clockType <= (int) ClockType::CUSTOM9);
}

bool ClockWidget::isValidClockType(int clockType) {
    if (clockType == (int) ClockType::NORMAL)
        return true; // Always enabled
    else if (clockType == (int) ClockType::NIXIE)
        return USE_CLOCK_NIXIE > 0;
    else if (clockType == (int) ClockType::FLIP)
        return USE_CLOCK_FLIP > 0;
    else if (clockType == (int) ClockType::MORPH)
        //    return m_enableMorph; // USE_CLOCK_MORPH;
        return USE_CLOCK_MORPH > 0; //?? oder doch das hier ??
    else if (isCustomClock(clockType)) {
        int customClockNumber = clockType - (int) ClockType::CUSTOM0;
        return USE_CLOCK_CUSTOM > customClockNumber && m_customEnabled[customClockNumber];
    } else
        return false;
}

void ClockWidget::changeClockType() {
    m_type++;
    if (m_type >= CLOCK_TYPE_NUM) {
        m_type = 0;
    }
    if (!isValidClockType(m_type)) {
        // Call recursively until a valid clock type is found
        changeClockType();
    } else {
        m_manager.clearAllScreens();
        draw(true);
    }
}

void ClockWidget::buttonPressed(uint8_t buttonId, ButtonState state) {
    if (buttonId == BUTTON_OK && state == BTN_SHORT) {
        changeClockType();
    } else if (buttonId == BUTTON_OK && state == BTN_MEDIUM) {
        changeFormat();
    }
}

DigitOffset ClockWidget::getOffsetForDigit(const String &digit) {
    if (digit.length() > 0) {
        char c = digit.charAt(0);
        if (c >= '0' && c <= '9') {
            // get digit offsets
            return m_digitOffsets[c - '0'];
        }
    }
    // not a valid digit
    return {0, 0};
}

void ClockWidget::displayDigit(int displayIndex, const String &lastDigit, const String &digit, uint32_t color, bool shadowing) {
    uint32_t start = millis();
    if (m_type == (int) ClockType::NIXIE || m_type == (int) ClockType::FLIP || isCustomClock(m_type)) {
        if (digit == ":" && color == m_shadowColor) {
            // Show colon off
            displayDigitImage(displayIndex, " ");
        } else {
            displayDigitImage(displayIndex, digit);
        }
    } else {
        m_manager.selectScreen(displayIndex);
        if (displayIndex == 2) {
            if (digit == ":") {
                m_manager.fillCircle(120, (120 - 40), 12, color);
                m_manager.fillCircle(120, (120 + 40), 12, color);
            } else {
                m_manager.fillCircle(120, (120 - 40), 12, TFT_BLACK);
                m_manager.fillCircle(120, (120 + 40), 12, TFT_BLACK);
            }
        } else {
            if (m_type == (int) ClockType::MORPH) {
                displayMorphDigit(displayIndex, lastDigit, digit, color);
            } else {
                // Normal clock
                int fontSize = CLOCK_FONT_SIZE;
                char c = digit.charAt(0);
                bool isDigit = c >= '0' && c <= '9' || c == ' ';
                int defaultX = SCREEN_SIZE / 2 + (isDigit ? CLOCK_OFFSET_X_DIGITS : CLOCK_OFFSET_X_COLON);
                int defaultY = SCREEN_SIZE / 2;
                DigitOffset digitOffset = getOffsetForDigit(digit);
                DigitOffset lastDigitOffset = getOffsetForDigit(lastDigit);
                m_manager.selectScreen(displayIndex);
                if (shadowing) {
                    m_manager.setFontColor(m_shadowColor, TFT_BLACK);
                    if (CLOCK_FONT == DSEG14) {
                        // DSEG14 (from DSEGstended) uses # to fill all segments
                        m_manager.drawString("#", defaultX, defaultY, fontSize, Align::MiddleCenter);
                    } else if (CLOCK_FONT == DSEG7) {
                        // DESG7 uses 8 to fill all segments
                        m_manager.drawString("8", defaultX, defaultY, fontSize, Align::MiddleCenter);
                    } else {
                        // Other fonts can't be shadowed
                        m_manager.setFontColor(TFT_BLACK, TFT_BLACK);
                        m_manager.drawString(lastDigit, defaultX + lastDigitOffset.x, defaultY + lastDigitOffset.y, fontSize, Align::MiddleCenter);
                    }
                } else {
                    m_manager.setFontColor(TFT_BLACK, TFT_BLACK);
                    m_manager.drawString(lastDigit, defaultX + lastDigitOffset.x, defaultY + lastDigitOffset.y, fontSize, Align::MiddleCenter);
                }
                m_manager.setFontColor(color, TFT_BLACK);
                m_manager.drawString(digit, defaultX + digitOffset.x, defaultY + digitOffset.y, fontSize, Align::MiddleCenter);
            }
        }
    }
    uint32_t end = millis();
#ifdef CLOCK_DEBUG
    Serial.printf("displayDigit(%s) took %dms\n", digit, end - start);
#endif
}

void ClockWidget::displayDigit(int displayIndex, const String &lastDigit, const String &digit, uint32_t color) {
    displayDigit(displayIndex, lastDigit, digit, color, m_shadowing);
}

void ClockWidget::displaySeconds(int displayIndex, int seconds, int color) {
    if (color != m_fgColor && (isCustomClock(m_type) || m_type == (int) ClockType::NIXIE || m_type == (int) ClockType::FLIP)) {
        // ignore clear tick (we draw the whole image anyway)
        return;
    }
    if (m_type == (int) ClockType::NIXIE) {
        if (m_overrideNixieColor != TFT_BLACK) {
            // Selected override color for nixie
            color = m_overrideNixieColor;
        } else {
            // Special color (orange) for nixie
            color = 0xfd40;
        }
    } else if (isCustomClock(m_type)) {
        String tickColorKey = "clkCust" + String(m_type - (int) ClockType::CUSTOM0) + "tckCol";
        color = m_config.getConfigInt(tickColorKey.c_str(), TFT_WHITE);
    }
    m_manager.selectScreen(displayIndex);
    int startA = ((seconds * 6) + 180 - 3) % 360;
    int endA = ((seconds * 6) + 180 + 3) % 360;
    m_manager.drawSmoothArc(SCREEN_SIZE / 2, SCREEN_SIZE / 2, 120, 115, startA, endA, color, TFT_BLACK);
}

void ClockWidget::displayDigitImage(int displayIndex, const String &digit) {
    if (digit.length() != 1) {
        return;
    }
    char c = digit.charAt(0);
    uint8_t index;
    if (c == ' ')
        index = 10;
    else if (c == ':')
        index = 11;
    else
        index = c - '0';
    if (index >= 12) {
        return;
    }
    if (m_type == (int) ClockType::NIXIE) {
        displayNixie(displayIndex, index);
    } else if (m_type == (int) ClockType::FLIP) {
        displayFlip(displayIndex, index);
    } else if (isCustomClock(m_type)) {
        displayCustom(displayIndex, m_type - (int) ClockType::CUSTOM0, index);
    }
}

void ClockWidget::displayNixie(int displayIndex, uint8_t index) {
#if USE_CLOCK_NIXIE > 0
    displayClockGraphics(displayIndex, clock_nixie, index, m_overrideNixieColor);
#endif
}

void ClockWidget::displayFlip(int displayIndex, uint8_t index) {
#if USE_CLOCK_FLIP > 0
    displayFlipClockGraphics(displayIndex, clock_flip, index, m_overrideNixieColor);
#endif
}

void ClockWidget::displayCustom(int displayIndex, uint8_t clockNumber, uint8_t index) {
#if USE_CLOCK_CUSTOM > 0
    String ovrColorKey = "clkCust" + String(clockNumber) + "ovrCol";
    int ovrColor = m_config.getConfigInt(ovrColorKey.c_str(), TFT_BLACK);
    m_manager.selectScreen(displayIndex);
    String name = "/CustomClock" + String(clockNumber) + "/" + String(index) + ".jpg";
    m_manager.drawFsJpg(0, 0, name.c_str(), 1, ovrColor);
#endif
}

void ClockWidget::displayClockGraphics(int displayIndex, const byte *clockArray[12][2], uint8_t index, int colorOverride) {
    m_manager.selectScreen(displayIndex);
    const byte *start = clockArray[index][0];
    const byte *end = clockArray[index][1];
    m_manager.drawJpg(0, 0, start, end - start, 1, colorOverride);
}

void ClockWidget::displayFlipClockGraphics(int displayIndex, const byte *clockFlipArray[80][2], uint8_t index, int colorOverride) {
    m_manager.selectScreen(displayIndex);
    if (displayIndex != 2) {
        if ((m_lastDisplay1Digit != "" || m_lastDisplay2Digit != "" || m_lastDisplay4Digit != "" || m_lastDisplay5Digit != "") && flipanimation == false) {
            flipanimation = true;
        } else {
            const byte *start = clockFlipArray[index][0];
            const byte *end = clockFlipArray[index][1];
            m_manager.drawJpg(0, 0, start, end - start, 1, colorOverride);
        }
        if (flipanimation == true) {
            animIndex = 0;
            while (animIndex < 4) {
                // Numbers can be either from 0-1 or 0-2 depending on 12/24h format
                if (displayIndex == 0) {
                    if (m_format == CLOCK_FORMAT_24_HOUR) {
                        if (index == 0) {
                            flipindex = 56 + animIndex;
                        } else {
                            flipindex = ((index - 1) * 4) + 12 + animIndex;
                        }
                    }
                    if (m_format == CLOCK_FORMAT_12_HOUR) {
                        if (index == 0) {
                            flipindex = 52 + animIndex;
                        } else {
                            flipindex = ((index - 1) * 4) + 12 + animIndex;
                        }
                    }
                }
                // Number are from 0-9 and can switch from 3->0
                if (displayIndex == 1) {
                    if (index == 0) {
                        if (m_format == CLOCK_FORMAT_12_HOUR && m_amPm == "PM") {
                            // Switch from 2 to 0 when PM
                            flipindex = 56 + animIndex;
                        }
                        if (m_format == CLOCK_FORMAT_12_HOUR && m_amPm == "AM") {
                            // Switch from 2 to 1 when AM
                            flipindex = 64 + animIndex;
                        }
                        if (m_format == CLOCK_FORMAT_24_HOUR && m_amPm == "PM") {
                            // Switch from 3 to 0 when AM
                            flipindex = 68 + animIndex;
                        }
                    } else {
                        flipindex = ((index - 1) * 4) + 12 + animIndex;
                    }
                }
                // Numbers are from 0-5
                if (displayIndex == 3) {
                    if (index == 0) {
                        flipindex = 60 + animIndex;
                    } else {
                        flipindex = ((index - 1) * 4) + 12 + animIndex;
                    }
                }
                // Numbers are from 0-9
                if (displayIndex == 4) {
                    if (index == 0) {
                        flipindex = (9 * 4) + 11 + animIndex;
                    } else {
                        flipindex = ((index - 1) * 4) + 12 + animIndex;
                    }
                }
                const byte *start = clockFlipArray[flipindex][0];
                const byte *end = clockFlipArray[flipindex][1];
                m_manager.drawJpg(0, 0, start, end - start, 1, colorOverride);
                animIndex += 1;
            }
            flipanimation = false;
            const byte *start = clockFlipArray[index][0];
            const byte *end = clockFlipArray[index][1];
            m_manager.drawJpg(0, 0, start, end - start, 1, colorOverride);
        } else {
            const byte *start = clockFlipArray[index][0];
            const byte *end = clockFlipArray[index][1];
            m_manager.drawJpg(0, 0, start, end - start, 1, colorOverride);
        }
    } else {
        if (colonanimation == false) {
            colonanimation = true;
        }
        if (colonanimation == true) {
            animcolon = 0;
            while (animcolon < 4) {
                // Switch colon from on to off
                if (index == 10) {
                    colonindex = 72 + animcolon;
                }
                // Switch colon from off to on
                if (index == 11) {
                    colonindex = 76 + animcolon;
                }
                const byte *start = clockFlipArray[colonindex][0];
                const byte *end = clockFlipArray[colonindex][1];
                m_manager.drawJpg(0, 0, start, end - start, 1, colorOverride);
                animcolon += 1;
            }
        }
        colonanimation = false;
        const byte *start = clockFlipArray[index][0];
        const byte *end = clockFlipArray[index][1];
        m_manager.drawJpg(0, 0, start, end - start, 1, colorOverride);
    }
}

// Used for drawing segments which have not changed and initial set-up
void ClockWidget::drawSegment(int seg, uint32_t color) {
    m_manager.S_fillRoundRect(segLoc[seg][0], segLoc[seg][1], segLoc[seg][2], segLoc[seg][3], 8, color);
}

void ClockWidget::drawDisappearBT(int seg, int seq) {
    int x;
    int y;
    int l;
    int w;
    x = segLoc[seg][0];
    y = segLoc[seg][1];
    l = segLoc[seg][2];
    if (seg == 0 || seg == 3 || seg == 6) {
    } else {
        w = (20 * (4 - seq));
        if (seq == 4)
            l = 0;
    }
    m_manager.S_fillRoundRect(x, y, l, w, 8, m_fgColor);
}

void ClockWidget::drawDisappearTB(int seg, int seq) {
    int x;
    int y;
    int l;
    int w;
    x = segLoc[seg][0];
    l = segLoc[seg][2];
    if (seg == 0 || seg == 3 || seg == 6) {
    } else {
        y = segLoc[seg][1] + (20 * (seq));
        w = (20 * (4 - seq));
        if (seq == 4)
            l = 0;
    }
    m_manager.S_fillRoundRect(x, y, l, w, 8, m_fgColor);
}

void ClockWidget::drawAppearTB(int seg, int seq) {
    int x;
    int y;
    int l;
    int w;
    x = segLoc[seg][0];
    y = segLoc[seg][1];
    l = segLoc[seg][2];
    if (seg == 0 || seg == 3 || seg == 6) {
    } else {
        w = (20 * (seq));
        if (seq == 0)
            l = 0;
    }
    m_manager.S_fillRoundRect(x, y, l, w, 8, m_fgColor);
}

void ClockWidget::drawAppearBT(int seg, int seq) {
    int x;
    int y;
    int l;
    int w;
    x = segLoc[seg][0];
    l = segLoc[seg][2];
    if (seg == 0 || seg == 3 || seg == 6) {
    } else {
        y = segLoc[seg][1] + (20 * (4 - seq));
        w = (20 * (seq));
        if (seq == 0)
            l = 0;
    }
    m_manager.S_fillRoundRect(x, y, l, w, 8, m_fgColor);
}

void ClockWidget::drawAppearRL(int seg, int seq) {
    int x;
    int y;
    int l;
    int w;
    y = segLoc[seg][1];
    w = segLoc[seg][3];
    if (seg == 0 || seg == 3 || seg == 6) {
        x = segLoc[seg][0] + (20 * (4 - seq));
        l = ((seq) * 20);
    } else {
        x = segLoc[seg][0] + (25 * (4 - seq));
        l = segLoc[seg][2];
        if (seq > 0 && seq < 4) {
            y = y + 8;
            w = w - 20;
        }
    }
    m_manager.S_fillRoundRect(x, y, l, w, 8, m_fgColor);
}

void ClockWidget::drawAppearLR(int seg, int seq) {
    int x;
    int y;
    int l;
    int w;
    y = segLoc[seg][1];
    w = segLoc[seg][3];
    if (seg == 0 || seg == 3 || seg == 6) {
        x = segLoc[seg][0];
        l = ((seq) * 20);
    } else {
        x = segLoc[seg][0] - (25 * (4 - seq));
        l = segLoc[seg][2];
        if (seq > 0 && seq < 4) {
            y = y + 8;
            w = w - 20;
        }
    }
    m_manager.S_fillRoundRect(x, y, l, w, 8, m_fgColor);
}

void ClockWidget::drawDisappearLR(int seg, int seq) {
    int x;
    int y;
    int l;
    int w;
    y = segLoc[seg][1];
    w = segLoc[seg][3];
    if (seg == 0 || seg == 3 || seg == 6) {
        x = segLoc[seg][0] + (20 * (seq));
        l = ((4 - seq) * 20);
    } else {
        x = segLoc[seg][0] + (25 * (seq));
        l = segLoc[seg][2];
        if (seq > 0 && seq < 4) {
            y = y + 8;
            w = w - 20;
        }
    }
    m_manager.S_fillRoundRect(x, y, l, w, 8, m_fgColor);
}

void ClockWidget::drawDisappearRL(int seg, int seq) {
    int x;
    int y;
    int l;
    int w;
    y = segLoc[seg][1];
    w = segLoc[seg][3];
    if (seg == 0 || seg == 3 || seg == 6) {
        x = segLoc[seg][0];
        l = ((4 - seq) * 20);
    } else {
        x = segLoc[seg][0] + (25 * (seq));
        l = segLoc[seg][2];
        if (seq > 0 && seq < 4) {
            y = y + 8;
            w = w - 20;
        }
    }
    m_manager.S_fillRoundRect(x, y, l, w, 8, m_fgColor);
}

void ClockWidget::displayMorphDigit(int orb, const String m_digitlast, const String m_digit, uint32_t color) {
    int tranForSeg;
    int m_dig = 0;
    if (m_digit != " ")
        m_dig = m_digit.toInt();

    for (int lv_anim = 0; lv_anim < 5; lv_anim++) { // Loop for the animation
        m_manager.S_fillSprite(TFT_BLACK); // Clear sprite
        for (int lv_seg = 0; lv_seg < 7; lv_seg++) { // Draw each segment in the sprite
            if (m_digit == " " && m_digitlast == " ")
                tranForSeg = 9;
            else if (m_digit == " " || m_digitlast == " ")
                tranForSeg = trans_0_B[m_dig][lv_seg];
            else if (m_digit == m_digitlast)
                tranForSeg = 0;
            else if (m_digit == "0") {
                if (m_digitlast == "2")
                    tranForSeg = trans_0_2[m_dig][lv_seg];
                if (m_digitlast == "3")
                    tranForSeg = trans_0_3[m_dig][lv_seg];
                if (m_digitlast == "5")
                    tranForSeg = trans_0_5[m_dig][lv_seg];
                if (m_digitlast == "9")
                    tranForSeg = trans_0_9[m_dig][lv_seg];
            } else if (m_digit == "1") {
                if (m_digitlast == "0")
                    tranForSeg = trans_0_9[m_dig][lv_seg];
                if (m_digitlast == "2")
                    tranForSeg = trans_0_2[m_dig][lv_seg];
            } else
                tranForSeg = trans_0_9[m_dig][lv_seg];

            switch (tranForSeg) { // Use the transition on each segment
            case 0:
                if (digits[m_dig][lv_seg] == 1)
                    drawSegment(lv_seg, m_fgColor);
                break; // Draw the segment
            case 1:
                drawDisappearBT(lv_seg, lv_anim);
                break;
            case 2:
                drawAppearTB(lv_seg, lv_anim);
                break;
            case 3:
                drawDisappearRL(lv_seg, lv_anim);
                break;
            case 4:
                drawDisappearLR(lv_seg, lv_anim);
                break;
            case 5:
                drawAppearRL(lv_seg, lv_anim);
                break;
            case 6:
                drawAppearLR(lv_seg, lv_anim);
                break;
            case 7:
                drawDisappearTB(lv_seg, lv_anim);
                break;
            case 8:
                drawAppearBT(lv_seg, lv_anim);
                break;
            case 9:
                break;
            }
        }
        delay(animDelay);
        m_manager.S_pushSprite(62, 28);
        // Serial.printf("displayDigit : %i + %s + %s + %d + %d\n",orb, m_digitlast, m_digit, m_dig, tranForSeg);
    }
}

String ClockWidget::getName() {
    return "Clock";
}
