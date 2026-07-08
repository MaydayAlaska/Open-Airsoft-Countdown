#pragma once

#include <Arduino.h>

class KeypadInput
{
public:
	bool begin();
	void update();

	char getKey();

private:
	static constexpr uint8_t I2cAddress = 0x20;
	static constexpr uint8_t I2cSdaPin = 8;
	static constexpr uint8_t I2cSclPin = 9;
	static constexpr uint16_t DebounceMillis = 180;

	char scanKey();

	void writePcf8574(uint8_t value);
	uint8_t readPcf8574();

	char m_lastKey = '\0';
	char m_pendingKey = '\0';

	uint32_t m_lastPressMillis = 0;
};