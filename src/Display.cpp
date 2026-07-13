#include "Display.h"

#include <Arduino.h>
#include <Wire.h>

namespace
{
	constexpr uint8_t I2cSdaPin = 8;
	constexpr uint8_t I2cSclPin = 9;
}

Display::Display() :
	m_display(U8G2_R0, U8X8_PIN_NONE)
{
}

bool Display::begin()
{
	Serial.println("Initializing display...");

	Wire.begin(I2cSdaPin, I2cSclPin);

	m_display.begin();
	m_display.clearBuffer();

	m_display.setFont(u8g2_font_ncenB08_tr);
	m_display.drawStr(8, 18, "Open Airsoft");
	m_display.drawStr(18, 34, "Countdown");
	m_display.drawStr(38, 56, "v0.1.0");

	m_display.sendBuffer();

	Serial.println("Display initialized.");

	return true;
}

void Display::update()
{
}

void Display::showAdminPin(uint8_t enteredDigits, uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, true);

	m_display.setFont(u8g2_font_ncenB08_tr);
	m_display.drawStr(18, 25, "PIN ADMIN");

	const String mask = formatPinMask(enteredDigits);

	m_display.setFont(u8g2_font_ncenB14_tr);
	m_display.drawStr(18, 48, mask.c_str());

	m_display.setFont(u8g2_font_5x8_tr);
	m_display.drawStr(8, 63, "# conferma  * cancella");

	m_display.sendBuffer();
}

void Display::showDisarmPin(uint8_t enteredDigits, uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, true);

	m_display.setFont(u8g2_font_ncenB08_tr);
	m_display.drawStr(16, 25, "PIN DISARMO");

	const String mask = formatPinMask(enteredDigits);

	m_display.setFont(u8g2_font_ncenB14_tr);
	m_display.drawStr(18, 48, mask.c_str());

	m_display.setFont(u8g2_font_5x8_tr);
	m_display.drawStr(8, 63, "# conferma  * cancella");

	m_display.sendBuffer();
}

void Display::showSetTimer(const String &input, uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, false);

	m_display.setFont(u8g2_font_ncenB08_tr);
	m_display.drawStr(18, 24, "IMPOSTA TIMER");

	const String formattedInput = formatTimerInput(input);

	m_display.setFont(u8g2_font_ncenB14_tr);
	m_display.drawStr(10, 48, formattedInput.c_str());

	m_display.setFont(u8g2_font_5x8_tr);
	m_display.drawStr(12, 63, "* cancella   # ok");

	m_display.sendBuffer();
}

void Display::showCountdown(uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, false);

	const String timeText = formatSeconds(remainingSeconds);

	m_display.setFont(u8g2_font_ncenB18_tr);

	const int16_t timeX = (128 - m_display.getStrWidth(timeText.c_str())) / 2;

	m_display.drawStr(timeX, 45, timeText.c_str());

	m_display.sendBuffer();
}

void Display::showMessage(const String &line1, const String &line2, uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, true);

	m_display.setFont(u8g2_font_ncenB08_tr);
	m_display.drawStr(4, 30, line1.c_str());
	m_display.drawStr(4, 48, line2.c_str());

	m_display.sendBuffer();
}

void Display::showFinished(uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(0, errorCount, maxErrorCount, true);

	const char *line1 = "TEMPO SCADUTO";
	const char *line2 = "PREMI # PER";
	const char *line3 = "NUOVO TIMER";

	m_display.setFont(u8g2_font_6x10_tr);

	const int16_t line1X = (128 - m_display.getStrWidth(line1)) / 2;
	const int16_t line2X = (128 - m_display.getStrWidth(line2)) / 2;
	const int16_t line3X = (128 - m_display.getStrWidth(line3)) / 2;

	m_display.drawStr(line1X, 28, line1);
	m_display.drawStr(line2X, 44, line2);
	m_display.drawStr(line3X, 60, line3);

	m_display.sendBuffer();
}

void Display::drawHeader(uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount, bool showTime)
{
	m_display.setFont(u8g2_font_6x10_tr);

	if (showTime)
	{
		const String timeText = "T " + formatSeconds(remainingSeconds);
		m_display.drawStr(0, 9, timeText.c_str());
	}

	if (errorCount > 0)
	{
		char buffer[20];

		snprintf(
			buffer,
			sizeof(buffer),
			"Errore %u/%u",
			static_cast<unsigned int>(errorCount),
			static_cast<unsigned int>(maxErrorCount)
		);

		const int16_t errorX = 128 - m_display.getStrWidth(buffer);

		m_display.drawStr(errorX, 9, buffer);
	}

	m_display.drawLine(0, 12, 127, 12);
}

String Display::formatSeconds(uint32_t seconds) const
{
	const uint32_t hours = seconds / 3600;
	const uint32_t minutes = (seconds % 3600) / 60;
	const uint32_t remaining = seconds % 60;

	char buffer[16];

	if (hours > 0)
	{
		snprintf(
			buffer,
			sizeof(buffer),
			"%02u:%02u:%02u",
			static_cast<unsigned int>(hours),
			static_cast<unsigned int>(minutes),
			static_cast<unsigned int>(remaining)
		);
	}
	else
	{
		snprintf(
			buffer,
			sizeof(buffer),
			"%02u:%02u",
			static_cast<unsigned int>(minutes),
			static_cast<unsigned int>(remaining)
		);
	}

	return String(buffer);
}

String Display::formatPinMask(uint8_t enteredDigits) const
{
	String mask;

	for (uint8_t i = 0; i < 6; i++)
	{
		if (i < enteredDigits)
		{
			mask += "*";
		}
		else
		{
			mask += "_";
		}

		if (i < 5)
		{
			mask += " ";
		}
	}

	return mask;
}

String Display::formatTimerInput(const String &input) const
{
	char buffer[] = "__:__:__";
	const uint8_t positions[] = { 0, 1, 3, 4, 6, 7 };

	for (uint8_t i = 0; i < input.length() && i < 6; i++)
	{
		buffer[positions[i]] = input[i];
	}

	return String(buffer);
}