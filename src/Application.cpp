#include "Application.h"

bool Application::begin()
{
	if (!m_storage.begin())
	{
		return false;
	}

	m_storage.printFileSystem();

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

	if (m_storage.getConfig().rfid)
	{
		if (!m_nfcReader.begin())
		{
			return false;
		}
	}
	else
	{
		Serial.println("RFID authentication disabled by config.");
	}

	m_display.showAdminPin(0, 0, m_errorCount, m_storage.getConfig().maxErrorCount);

	return true;
}

void Application::update()
{
	m_keypad.update();

	if (m_storage.getConfig().rfid)
	{
		m_nfcReader.update();

		if (m_nfcReader.hasNewUid())
		{
			Serial.print("Last NFC UID received by application: ");
			Serial.println(m_nfcReader.getLastUid());

			m_nfcReader.clearNewUid();
		}
	}

	m_timer.update();
	m_buzzer.update();

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
			handleRunning(key);
			break;

		case Mode::Finished:
			if (key == '#')
			{
				m_timer.reset();
				m_timerInput = "";
				m_disarmPinInput = "";
				m_errorCount = 0;
				m_lastDisplayedSeconds = 0xFFFFFFFF;

				m_mode = Mode::SetTimer;
				m_display.showSetTimer(m_timerInput, 0, m_errorCount, m_storage.getConfig().maxErrorCount);
			}
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
		if (m_adminPinInput.length() > 0)
		{
			m_adminPinInput.remove(m_adminPinInput.length() - 1);
		}

		m_display.showAdminPin(m_adminPinInput.length(), 0, m_errorCount, m_storage.getConfig().maxErrorCount);
		return;
	}

	if (key == '#')
	{
		if (m_adminPinInput == m_storage.getConfig().adminPin)
		{
			m_adminPinInput = "";
			m_errorCount = 0;

			m_mode = Mode::SetTimer;
			m_timerInput = "";
			m_display.showSetTimer(m_timerInput, 0, m_errorCount, m_storage.getConfig().maxErrorCount);
		}
		else
		{
			m_adminPinInput = "";
			m_display.showMessage("PIN ADMIN", "ERRATO", 0, m_errorCount, m_storage.getConfig().maxErrorCount);
		}

		return;
	}

	if (isDigit(key) && m_adminPinInput.length() < 6)
	{
		m_adminPinInput += key;
		m_display.showAdminPin(m_adminPinInput.length(), 0, m_errorCount, m_storage.getConfig().maxErrorCount);
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
		if (m_timerInput.length() > 0)
		{
			m_timerInput.remove(m_timerInput.length() - 1);
		}

		m_display.showSetTimer(m_timerInput, 0, m_errorCount, m_storage.getConfig().maxErrorCount);
		return;
	}

	if (key == '#')
	{
		const uint32_t duration = parseTimerInput();

		if (duration == 0)
		{
			m_timerInput = "";
			m_display.showMessage("TIMER", "NON VALIDO", 0, m_errorCount, m_storage.getConfig().maxErrorCount);
			return;
		}

		m_timer.setDuration(duration);
		m_timer.start();

		m_disarmPinInput = "";
		m_errorCount = 0;
		m_lastDisplayedSeconds = 0xFFFFFFFF;
		m_mode = Mode::Running;

		return;
	}

	if (isDigit(key) && m_timerInput.length() < 6)
	{
		m_timerInput += key;
		m_display.showSetTimer(m_timerInput, 0, m_errorCount, m_storage.getConfig().maxErrorCount);
	}
}

void Application::handleRunning(char key)
{
	const uint32_t remainingSeconds = m_timer.getRemainingSeconds();
	const uint32_t maxErrorCount = m_storage.getConfig().maxErrorCount;

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
		m_display.showFinished(m_errorCount, maxErrorCount);
		return;
	}

	if (m_errorCount >= maxErrorCount)
	{
		if (remainingSeconds != m_lastDisplayedSeconds)
		{
			m_lastDisplayedSeconds = remainingSeconds;
			m_display.showCountdown(remainingSeconds, m_errorCount, maxErrorCount);
		}

		return;
	}

	if (key == '*')
	{
		if (m_disarmPinInput.length() > 0)
		{
			m_disarmPinInput.remove(m_disarmPinInput.length() - 1);
			m_lastDisplayedSeconds = remainingSeconds;
			m_display.showDisarmPin(m_disarmPinInput.length(), remainingSeconds, m_errorCount, maxErrorCount);
		}
		else
		{
			m_lastDisplayedSeconds = remainingSeconds;
			m_display.showCountdown(remainingSeconds, m_errorCount, maxErrorCount);
		}

		return;
	}

	if (key == '#')
	{
		if (m_disarmPinInput == m_storage.getConfig().adminPin)
		{
			Serial.println("Timer disarmed.");

			m_timer.stop();

			m_timerInput = "";
			m_disarmPinInput = "";
			m_errorCount = 0;
			m_lastDisplayedSeconds = 0xFFFFFFFF;

			m_mode = Mode::SetTimer;
			m_display.showSetTimer(m_timerInput, 0, m_errorCount, maxErrorCount);
		}
		else
		{
			Serial.println("Wrong disarm PIN.");

			m_disarmPinInput = "";

			if (m_errorCount < maxErrorCount)
			{
				m_errorCount++;
			}

			if (m_errorCount >= maxErrorCount)
			{
				const uint32_t penaltySeconds = m_storage.getConfig().errorCountdownSeconds;

				Serial.println("Maximum errors reached. Disarm locked.");

				if (penaltySeconds > 0)
				{
					Serial.print("Forcing timer to ");
					Serial.print(penaltySeconds);
					Serial.println(" seconds.");

					m_timer.setRemainingSeconds(penaltySeconds);

					m_lastDisplayedSeconds = 0xFFFFFFFF;
					m_display.showMessage("TROPPI ERRORI", "DISARMO BLOCCATO", penaltySeconds, m_errorCount, maxErrorCount);
				}
				else
				{
					Serial.println("Penalty countdown disabled. Remaining time unchanged.");

					m_lastDisplayedSeconds = remainingSeconds;
					m_display.showMessage("TROPPI ERRORI", "DISARMO BLOCCATO", remainingSeconds, m_errorCount, maxErrorCount);
				}
			}
			else
			{
				m_lastDisplayedSeconds = remainingSeconds;
				m_display.showMessage("PIN DISARMO", "ERRATO", remainingSeconds, m_errorCount, maxErrorCount);
			}
		}

		return;
	}

	if (isDigit(key) && m_disarmPinInput.length() < 6)
	{
		m_disarmPinInput += key;
		m_lastDisplayedSeconds = remainingSeconds;
		m_display.showDisarmPin(m_disarmPinInput.length(), remainingSeconds, m_errorCount, maxErrorCount);
		return;
	}

	if (m_disarmPinInput.length() > 0)
	{
		if (remainingSeconds != m_lastDisplayedSeconds)
		{
			m_lastDisplayedSeconds = remainingSeconds;
			m_display.showDisarmPin(m_disarmPinInput.length(), remainingSeconds, m_errorCount, maxErrorCount);
		}

		return;
	}

	if (remainingSeconds != m_lastDisplayedSeconds)
	{
		m_lastDisplayedSeconds = remainingSeconds;
		m_display.showCountdown(remainingSeconds, m_errorCount, maxErrorCount);
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

String Application::formatDisarmPinMask() const
{
	String mask;

	for (uint8_t i = 0; i < 6; i++)
	{
		if (i < m_disarmPinInput.length())
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