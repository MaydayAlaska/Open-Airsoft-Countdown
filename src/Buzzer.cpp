#include "Buzzer.h"

bool Buzzer::begin()
{
	Serial.println("Initializing buzzer...");

	digitalWrite(BuzzerPin, BuzzerOffLevel);
	pinMode(BuzzerPin, OUTPUT);

	Serial.println("Buzzer initialized.");

	return true;
}

void Buzzer::update()
{
	if (!m_active)
	{
		return;
	}

	const uint32_t currentMillis = millis();

	if (currentMillis - m_startedAt >= m_durationMs)
	{
		digitalWrite(BuzzerPin, BuzzerOffLevel);
		m_active = false;
	}
}

void Buzzer::beep(uint16_t durationMs)
{
	m_startedAt = millis();
	m_durationMs = durationMs;
	m_active = true;

	digitalWrite(BuzzerPin, BuzzerOnLevel);
}
