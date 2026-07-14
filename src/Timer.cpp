#include "Timer.h"

bool Timer::begin()
{
	Serial.println("Initializing timer...");
	Serial.println("Status LED pin: GPIO14");

	pinMode(StatusLedPin, OUTPUT);
	digitalWrite(StatusLedPin, LOW);

	m_durationSeconds = 0;
	m_remainingSeconds = 0;
	m_lastTickMillis = millis();
	m_running = false;
	m_finished = false;
	m_ledState = false;
	m_secondTick = false;

	Serial.println("Timer initialized.");

	return true;
}

void Timer::update()
{
	m_secondTick = false;

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

	m_ledState = !m_ledState;
	digitalWrite(StatusLedPin, m_ledState ? HIGH : LOW);

	if (m_remainingSeconds > 0)
	{
		m_remainingSeconds--;
		m_secondTick = true;

		Serial.print("Remaining seconds: ");
		Serial.println(m_remainingSeconds);
	}

	if (m_remainingSeconds == 0)
	{
		m_running = false;
		m_finished = true;

		digitalWrite(StatusLedPin, HIGH);

		Serial.println("Timer finished.");
	}
}

void Timer::setDuration(uint32_t seconds)
{
	m_durationSeconds = seconds;
	m_remainingSeconds = seconds;
	m_finished = false;
	m_secondTick = false;
	m_ledState = false;

	digitalWrite(StatusLedPin, LOW);

	Serial.print("Timer duration set to ");
	Serial.print(seconds);
	Serial.println(" seconds.");
}

void Timer::setRemainingSeconds(uint32_t seconds)
{
	m_remainingSeconds = seconds;
	m_finished = false;
	m_secondTick = false;
	m_lastTickMillis = millis();

	if (seconds == 0)
	{
		m_running = false;
		m_finished = true;
		digitalWrite(StatusLedPin, HIGH);
		return;
	}

	if (m_running)
	{
		Serial.print("Timer remaining seconds forced to ");
		Serial.print(seconds);
		Serial.println(" seconds.");
	}
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
	m_secondTick = false;
	m_ledState = false;

	digitalWrite(StatusLedPin, LOW);

	Serial.println("Timer started.");
}

void Timer::stop()
{
	m_running = false;
	m_secondTick = false;
	m_ledState = false;

	digitalWrite(StatusLedPin, LOW);

	Serial.println("Timer stopped.");
}

void Timer::reset()
{
	m_running = false;
	m_finished = false;
	m_secondTick = false;
	m_remainingSeconds = m_durationSeconds;
	m_ledState = false;

	digitalWrite(StatusLedPin, LOW);

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

bool Timer::consumeSecondTick()
{
	const bool tick = m_secondTick;

	m_secondTick = false;

	return tick;
}

uint32_t Timer::getDuration() const
{
	return m_durationSeconds;
}

uint32_t Timer::getRemainingSeconds() const
{
	return m_remainingSeconds;
}
