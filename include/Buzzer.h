#pragma once

#include <Arduino.h>

class Buzzer
{
public:
	bool begin();
	void update();

	void beep(uint16_t durationMs);

private:
	static constexpr uint8_t BuzzerPin = 48;
	static constexpr uint8_t BuzzerOnLevel = LOW;
	static constexpr uint8_t BuzzerOffLevel = HIGH;

	bool m_active = false;
	uint32_t m_startedAt = 0;
	uint16_t m_durationMs = 0;
};
