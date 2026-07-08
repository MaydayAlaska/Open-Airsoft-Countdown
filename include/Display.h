#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

class Display
{
public:
	Display();

	bool begin();
	void update();

	void showAdminPin(uint8_t enteredDigits, uint32_t remainingSeconds, uint8_t errorCount);
	void showSetTimer(const String &input, uint32_t remainingSeconds, uint8_t errorCount);
	void showCountdown(uint32_t remainingSeconds, uint8_t errorCount);
	void showMessage(const String &line1, const String &line2, uint32_t remainingSeconds, uint8_t errorCount);
	void showFinished(uint8_t errorCount);

private:
	void drawHeader(uint32_t remainingSeconds, uint8_t errorCount);
	String formatSeconds(uint32_t seconds) const;
	String formatPinMask(uint8_t enteredDigits) const;
	String formatTimerInput(const String &input) const;

	U8G2_SH1106_128X64_NONAME_F_HW_I2C m_display;
};