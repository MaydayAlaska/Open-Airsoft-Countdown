#include "KeypadInput.h"

#include <Wire.h>

namespace
{
	const char KeyMap[4][4] =
	{
		{ '1', '2', '3', 'A' },
		{ '4', '5', '6', 'B' },
		{ '7', '8', '9', 'C' },
		{ '*', '0', '#', 'D' }
	};
}

bool KeypadInput::begin()
{
	Serial.println("Initializing I2C keypad...");

	Wire.begin(I2cSdaPin, I2cSclPin);

	writePcf8574(0xFF);

	Serial.println("I2C keypad initialized.");

	return true;
}

void KeypadInput::update()
{
	const char key = scanKey();

	if (key != '\0' && m_lastKey == '\0')
	{
		const uint32_t currentMillis = millis();

		if (currentMillis - m_lastPressMillis >= DebounceMillis)
		{
			m_pendingKey = key;
			m_lastPressMillis = currentMillis;
		}
	}

	m_lastKey = key;
}

char KeypadInput::getKey()
{
	const char key = m_pendingKey;

	m_pendingKey = '\0';

	return key;
}

char KeypadInput::scanKey()
{
	for (uint8_t row = 0; row < 4; row++)
	{
		uint8_t output = 0xFF;
		output &= ~(1 << row);

		writePcf8574(output);
		delayMicroseconds(100);

		const uint8_t input = readPcf8574();

		for (uint8_t col = 0; col < 4; col++)
		{
			const uint8_t colBit = 4 + col;

			if ((input & (1 << colBit)) == 0)
			{
				writePcf8574(0xFF);
				return KeyMap[row][col];
			}
		}
	}

	writePcf8574(0xFF);

	return '\0';
}

void KeypadInput::writePcf8574(uint8_t value)
{
	Wire.beginTransmission(I2cAddress);
	Wire.write(value);
	Wire.endTransmission();
}

uint8_t KeypadInput::readPcf8574()
{
	Wire.requestFrom(I2cAddress, static_cast<uint8_t>(1));

	if (Wire.available() == 0)
	{
		return 0xFF;
	}

	return Wire.read();
}