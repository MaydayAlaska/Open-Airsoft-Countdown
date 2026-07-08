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

void Display::showAdminPin(uint8_t enteredDigits, uint32_t remainingSeconds, uint8_t errorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount);

	m_display.setFont(u8g2_font_ncenB08_tr);
	m_display.drawStr(18, 28, "PIN ADMIN");

	const String mask = formatPinMask(enteredDigits);

	m_display.setFont(u8g2_font_ncenB14_tr);
	m_display.drawStr(18, 54, mask.c_str());

	m_display.sendBuffer();
}

void Display::showSetTimer(const String &input, uint32_t remainingSeconds, uint8_t errorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount);

	m_display.setFont(u8g2_font_ncenB08_tr);
	m_display.drawStr(18, 24, "IMPOSTA TIMER");

	const String formattedInput = formatTimerInput(input);

	m_display.setFont(u8g2_font_ncenB14_tr);
	m_display.drawStr(10, 48, formattedInput.c_str());

	m_display.setFont(u8g2_font_5x8_tr);
	m_display.drawStr(12, 63, "* cancella   # ok");

	m_display.sendBuffer();
}

void Display::showCountdown(uint32_t remainingSeconds, uint8_t errorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount);

	const String timeText = formatSeconds(remainingSeconds);

	m_display.setFont(u8g2_font_ncenB18_tr);
	m_display.drawStr(16, 45, timeText.c_str());

	m_display.sendBuffer();
}

void Display::showMessage(const String &line1, const String &line2, uint32_t remainingSeconds, uint8_t errorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount);

	m_display.setFont(u8g2_font_ncenB08_tr);
	m_display.drawStr(4, 30, line1.c_str());
	m_display.drawStr(4, 48, line2.c_str());

	m_display.sendBuffer();
}

void Display::showFinished(uint8_t errorCount)
{
	m_display.clearBuffer();

	drawHeader(0, errorCount);

	m_display.setFont(u8g2_font_ncenB10_tr);
	m_display.drawStr(12, 34, "TEMPO SCADUTO");

	m_display.setFont(u8g2_font_ncenB14_tr);
	m_display.drawStr(34, 58, "00:00");

	m_display.sendBuffer();
}

void Display::drawHeader(uint32_t remainingSeconds, uint8_t errorCount)
{
	const String timeText = "T " + formatSeconds(remainingSeconds);

	m_display.setFont(u8g2_font_6x10_tr);
	m_display.drawStr(0, 9, timeText.c_str());

	if (errorCount > 0)
	{
		char buffer[8];
		snprintf(buffer, sizeof(buffer), "E %u/3", errorCount);
		m_display.drawStr(94, 9, buffer);
	}

	m_display.drawLine(0, 12, 127, 12);
}

String Display::formatSeconds(uint32_t seconds) const
{
	const uint32_t hours = seconds / 3600;
	const uint32_t minutes = (seconds % 3600) / 60;
	const uint32_t remaining = seconds % 60;

	char buffer[12];

	if (hours > 0)
	{
		snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, remaining);
	}
	else
	{
		snprintf(buffer, sizeof(buffer), "%02lu:%02lu", minutes, remaining);
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