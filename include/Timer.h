#pragma once

#include <Arduino.h>

class Timer
{
public:
	bool begin();
	void update();

	void setDuration(uint32_t seconds);
	void setRemainingSeconds(uint32_t seconds);

	void start();
	void stop();
	void reset();

	bool isRunning() const;
	bool isFinished() const;
	bool consumeSecondTick();

	uint32_t getDuration() const;
	uint32_t getRemainingSeconds() const;

private:
	static constexpr uint8_t StatusLedPin = 14;

	uint32_t m_durationSeconds = 0;
	uint32_t m_remainingSeconds = 0;
	uint32_t m_lastTickMillis = 0;

	bool m_running = false;
	bool m_finished = false;
	bool m_ledState = false;
	bool m_secondTick = false;
};
