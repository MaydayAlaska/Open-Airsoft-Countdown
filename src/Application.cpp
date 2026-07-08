#include "Application.h"

bool Application::begin()
{
	if (!m_storage.begin())
	{
		return false;
	}

	if (!m_users.begin())
	{
		return false;
	}

	if (!m_display.begin())
	{
		return false;
	}

	if (!m_timer.begin())
	{
		return false;
	}

	if (!m_buzzer.begin())
	{
		return false;
	}

	if (!m_keypad.begin())
	{
		return false;
	}

	if (!m_nfcReader.begin())
	{
		return false;
	}

	m_display.showAdminPin(0, 0, m_errorCount);

	return true;
}

void Application::update()
{
	m_keypad.update();
	m_nfcReader.update();
	m_timer.update();
	m_buzzer.update();

	if (m_nfcReader.hasNewUid())
	{
		Serial.print("Last NFC UID received by application: ");
		Serial.println(m_nfcReader.getLastUid());

		m_nfcReader.clearNewUid();
	}

	const char key = m_keypad.getKey();

	switch (m_mode)
	{
		case Mode::EnterAdminPin:
			handleAdminPin(key);
			break;

		case Mode::SetTimer:
			handleSetTimer(key);
			break;

		case Mode::Running:
			handleRunning();
			break;

		case Mode::Finished:
			break;
	}

	m_display.update();
}

void Application::handleAdminPin(char key)
{
	if (key == '\0')
	{
		return;
	}

	if (key == '*')
	{
		m_adminPinInput = "";
		m_display.showAdminPin(0, 0, m_errorCount);
		return;
	}

	if (key == '#')
	{
		if (m_adminPinInput == m_storage.getConfig().adminPin)
		{
			m_mode = Mode::SetTimer;
			m_timerInput = "";
			m_display.showSetTimer(m_timerInput, 0, m_errorCount);
		}
		else
		{
			m_adminPinInput = "";
			m_display.showMessage("PIN ADMIN", "ERRATO", 0, m_errorCount);
		}

		return;
	}

	if (isDigit(key) && m_adminPinInput.length() < 6)
	{
		m_adminPinInput += key;
		m_display.showAdminPin(m_adminPinInput.length(), 0, m_errorCount);
	}
}

void Application::handleSetTimer(char key)
{
	if (key == '\0')
	{
		return;
	}

	if (key == '*')
	{
		m_timerInput = "";
		m_display.showSetTimer(m_timerInput, 0, m_errorCount);
		return;
	}

	if (key == '#')
	{
		const uint32_t duration = parseTimerInput();

		if (duration == 0)
		{
			m_timerInput = "";
			m_display.showMessage("TIMER", "NON VALIDO", 0, m_errorCount);
			return;
		}

		m_timer.setDuration(duration);
		m_timer.start();

		m_lastDisplayedSeconds = 0xFFFFFFFF;
		m_mode = Mode::Running;

		return;
	}

	if (isDigit(key) && m_timerInput.length() < 6)
	{
		m_timerInput += key;
		m_display.showSetTimer(m_timerInput, 0, m_errorCount);
	}
}

void Application::handleRunning()
{
	const uint32_t remainingSeconds = m_timer.getRemainingSeconds();

	if (m_timer.consumeSecondTick())
	{
		if (remainingSeconds > 0 && remainingSeconds <= 5)
		{
			m_buzzer.beep(80);
		}
	}

	if (m_timer.isFinished())
	{
		m_mode = Mode::Finished;
		m_buzzer.beep(3000);
		m_display.showFinished(m_errorCount);
		return;
	}

	if (remainingSeconds != m_lastDisplayedSeconds)
	{
		m_lastDisplayedSeconds = remainingSeconds;
		m_display.showCountdown(remainingSeconds, m_errorCount);
	}
}

bool Application::isDigit(char key) const
{
	return key >= '0' && key <= '9';
}

uint32_t Application::parseTimerInput() const
{
	if (m_timerInput.length() != 6)
	{
		return 0;
	}

	const uint8_t hours = (m_timerInput[0] - '0') * 10 + (m_timerInput[1] - '0');
	const uint8_t minutes = (m_timerInput[2] - '0') * 10 + (m_timerInput[3] - '0');
	const uint8_t seconds = (m_timerInput[4] - '0') * 10 + (m_timerInput[5] - '0');

	if (minutes > 59 || seconds > 59)
	{
		return 0;
	}

	return static_cast<uint32_t>(hours) * 3600UL +
		static_cast<uint32_t>(minutes) * 60UL +
		static_cast<uint32_t>(seconds);
}