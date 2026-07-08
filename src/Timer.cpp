#include "Timer.h"

bool Timer::begin()
{
	Serial.println("Initializing timer...");

	m_durationSeconds = 0;
	m_remainingSeconds = 0;
	m_lastTickMillis = millis();
	m_running = false;
	m_finished = false;

	Serial.println("Timer initialized.");

	return true;
}

void Timer::update()
{
	if (!m_running)
	{
		return;
	}

	const uint32_t currentMillis = millis();

	if (currentMillis - m_lastTickMillis < 1000)
	{
		return;
	}

	m_lastTickMillis = currentMillis;

	if (m_remainingSeconds > 0)
	{
		m_remainingSeconds--;

		Serial.print("Remaining seconds: ");
		Serial.println(m_remainingSeconds);
	}

	if (m_remainingSeconds == 0)
	{
		m_running = false;
		m_finished = true;

		Serial.println("Timer finished.");
	}
}

void Timer::setDuration(uint32_t seconds)
{
	m_durationSeconds = seconds;
	m_remainingSeconds = seconds;
	m_finished = false;

	Serial.print("Timer duration set to ");
	Serial.print(seconds);
	Serial.println(" seconds.");
}

void Timer::start()
{
	if (m_remainingSeconds == 0)
	{
		m_remainingSeconds = m_durationSeconds;
	}

	if (m_remainingSeconds == 0)
	{
		Serial.println("Timer cannot start: duration is 0.");
		return;
	}

	m_lastTickMillis = millis();
	m_running = true;
	m_finished = false;

	Serial.println("Timer started.");
}

void Timer::stop()
{
	m_running = false;

	Serial.println("Timer stopped.");
}

void Timer::reset()
{
	m_running = false;
	m_finished = false;
	m_remainingSeconds = m_durationSeconds;

	Serial.println("Timer reset.");
}

bool Timer::isRunning() const
{
	return m_running;
}

bool Timer::isFinished() const
{
	return m_finished;
}

uint32_t Timer::getDuration() const
{
	return m_durationSeconds;
}

uint32_t Timer::getRemainingSeconds() const
{
	return m_remainingSeconds;
}