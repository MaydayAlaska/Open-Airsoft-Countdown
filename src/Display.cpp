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