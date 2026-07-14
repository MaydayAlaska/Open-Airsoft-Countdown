#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

class Display
{
public:
	Display();

	void setLanguage(const String &language);
	bool begin();
	void update();

	void showAdminPin(uint8_t enteredDigits, uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount);
	void showDisarmPin(uint8_t enteredDigits, uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount);
	void showSetTimer(const String &input, uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount);
	void showCountdown(uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount);
	void showMessage(const String &line1, const String &line2, uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount);
	void showPinError(uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount);
	void showMaximumError(uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount);
	void showFinished(uint8_t errorCount, uint32_t maxErrorCount);

private:
	void drawHeader(uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount, bool showTime);
	void drawCentered(const char *text, int16_t y);
	bool isEnglish() const;
	String formatSeconds(uint32_t seconds) const;
	String formatPinMask(uint8_t enteredDigits) const;
	String formatTimerInput(const String &input) const;

	U8G2_SH1106_128X64_NONAME_F_HW_I2C m_display;
	String m_language = "it";
};
