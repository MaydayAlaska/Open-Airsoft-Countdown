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

void Display::setLanguage(const String &language)
{
	m_language = language;
	m_language.trim();
	m_language.toLowerCase();

	if (m_language != "en")
	{
		m_language = "it";
	}
}

bool Display::begin()
{
	Serial.println("Initializing display...");

	Wire.begin(I2cSdaPin, I2cSclPin);

	m_display.begin();
	m_display.clearBuffer();

	m_display.setFont(u8g2_font_ncenB08_tr);
	drawCentered("Open Airsoft", 14);
	drawCentered("Countdown", 29);
	drawCentered("v1.8", 43);

	m_display.setFont(u8g2_font_5x8_tr);
	drawCentered("maydayalaska - GitHub", 62);

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
	drawCentered(isEnglish() ? "ADMIN PIN" : "PIN ADMIN", 25);

	const String mask = formatPinMask(enteredDigits);

	m_display.setFont(u8g2_font_ncenB14_tr);
	drawCentered(mask.c_str(), 48);

	m_display.setFont(u8g2_font_5x8_tr);
	m_display.drawStr(8, 63, isEnglish() ? "# confirm  * delete" : "# conferma  * cancella");

	m_display.sendBuffer();
}

void Display::showDisarmPin(uint8_t enteredDigits, uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, true);

	m_display.setFont(u8g2_font_ncenB08_tr);
	drawCentered(isEnglish() ? "DISARM PIN" : "PIN DISARMO", 25);

	const String mask = formatPinMask(enteredDigits);

	m_display.setFont(u8g2_font_ncenB14_tr);
	drawCentered(mask.c_str(), 48);

	m_display.setFont(u8g2_font_5x8_tr);
	m_display.drawStr(8, 63, isEnglish() ? "# confirm  * delete" : "# conferma  * cancella");

	m_display.sendBuffer();
}

void Display::showSetTimer(const String &input, uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, false);

	m_display.setFont(u8g2_font_ncenB08_tr);
	drawCentered(isEnglish() ? "SET TIMER" : "IMPOSTA TIMER", 24);

	const String formattedInput = formatTimerInput(input);

	m_display.setFont(u8g2_font_ncenB14_tr);
	drawCentered(formattedInput.c_str(), 48);

	m_display.setFont(u8g2_font_5x8_tr);
	m_display.drawStr(12, 63, isEnglish() ? "* delete    # ok" : "* cancella   # ok");

	m_display.sendBuffer();
}

void Display::showCountdown(uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, false);

	const String timeText = formatSeconds(remainingSeconds);

	m_display.setFont(u8g2_font_ncenB18_tr);
	drawCentered(timeText.c_str(), 45);

	m_display.sendBuffer();
}

void Display::showMessage(const String &line1, const String &line2, uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, true);

	m_display.setFont(u8g2_font_ncenB08_tr);
	drawCentered(line1.c_str(), 30);
	drawCentered(line2.c_str(), 48);

	m_display.sendBuffer();
}

void Display::showPinError(uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, true);

	m_display.setFont(u8g2_font_ncenB12_tr);
	drawCentered(isEnglish() ? "WRONG PIN" : "PIN ERRATO", 42);

	m_display.sendBuffer();
}

void Display::showMaximumError(uint32_t remainingSeconds, uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, true);

	m_display.setFont(u8g2_font_ncenB08_tr);
	drawCentered(isEnglish() ? "TOO MANY ERRORS" : "TROPPI ERRORI", 30);
	drawCentered(isEnglish() ? "ALARM" : "ALLARME", 48);

	m_display.sendBuffer();
}

void Display::showUserGreeting(
	const String &name,
	uint32_t remainingSeconds,
	uint8_t errorCount,
	uint32_t maxErrorCount
)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, true);

	m_display.setFont(u8g2_font_ncenB10_tr);
	drawCentered(isEnglish() ? "HELLO" : "SALVE", 30);

	m_display.setFont(u8g2_font_ncenB12_tr);
	drawCentered(name.c_str(), 51);

	m_display.sendBuffer();
}

void Display::showAuthenticationUnavailable(
	uint32_t remainingSeconds,
	uint8_t errorCount,
	uint32_t maxErrorCount
)
{
	m_display.clearBuffer();

	drawHeader(remainingSeconds, errorCount, maxErrorCount, true);

	m_display.setFont(u8g2_font_6x10_tr);
	drawCentered(isEnglish() ? "FINGERPRINT" : "IMPRONTA", 31);
	drawCentered(isEnglish() ? "NOT AVAILABLE" : "NON DISPONIBILE", 49);

	m_display.sendBuffer();
}

void Display::showFinished(uint8_t errorCount, uint32_t maxErrorCount)
{
	m_display.clearBuffer();

	drawHeader(0, errorCount, maxErrorCount, true);

	const char *line1 = isEnglish() ? "TIME EXPIRED" : "TEMPO SCADUTO";
	const char *line2 = isEnglish() ? "PRESS # FOR" : "PREMI # PER";
	const char *line3 = isEnglish() ? "NEW TIMER" : "NUOVO TIMER";

	m_display.setFont(u8g2_font_6x10_tr);
	drawCentered(line1, 28);
	drawCentered(line2, 44);
	drawCentered(line3, 60);

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
			isEnglish() ? "Error %u/%u" : "Errore %u/%u",
			static_cast<unsigned int>(errorCount),
			static_cast<unsigned int>(maxErrorCount)
		);

		const int16_t errorX = 128 - m_display.getStrWidth(buffer);
		m_display.drawStr(errorX, 9, buffer);
	}

	m_display.drawLine(0, 12, 127, 12);
}

void Display::drawCentered(const char *text, int16_t y)
{
	const int16_t x = (128 - m_display.getStrWidth(text)) / 2;
	m_display.drawStr(x < 0 ? 0 : x, y, text);
}

bool Display::isEnglish() const
{
	return m_language == "en";
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
