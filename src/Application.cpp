#include "Application.h"

namespace
{
	constexpr uint32_t StartupScreenDurationMs = 3000;
	constexpr uint32_t PinErrorMessageDurationMs = 1500;
}

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

	delay(StartupScreenDurationMs);

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

	if (!m_bleManager.begin(m_storage.getConfig().bleName))
	{
		return false;
	}

	m_display.showAdminPin(0, 0, m_errorCount, m_storage.getConfig().maxErrorCount);

	sendBleStatus();

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

	if (m_bleManager.consumeDisconnected())
	{
		m_bleLoggedIn = false;
		m_hasLastBleStatus = false;
		Serial.println("BLE admin session closed.");
	}

	const char key = m_keypad.getKey();

	if (m_pinErrorMessageActive)
	{
		updatePinErrorMessage();
	}
	else
	{
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

			case Mode::Stopped:
				handleStopped(key);
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

					sendBleStatus();
				}
				break;
		}
	}

	String bleCommand;

	if (m_bleManager.getCommand(bleCommand))
	{
		handleBleCommand(bleCommand);
	}

	sendBleStatusIfChanged();

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

			sendBleStatus();
		}
		else
		{
			m_adminPinInput = "";
			showPinErrorMessage(0);
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

		sendBleStatus();

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

	const bool secondTick = m_timer.consumeSecondTick();

	if (secondTick)
	{
		if (remainingSeconds > 0 && remainingSeconds <= 5 && m_storage.getConfig().soundEnabled)
		{
			m_buzzer.beep(80);
		}
	}

	if (m_timer.isFinished())
	{
		m_mode = Mode::Finished;

		if (m_storage.getConfig().soundEnabled)
		{
			m_buzzer.beep(3000);
		}

		m_display.showFinished(m_errorCount, maxErrorCount);

		sendBleStatus();

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
			Serial.println("Timer stopped by keypad.");

			m_timer.stop();

			m_timerInput = "";
			m_disarmPinInput = "";
			m_errorCount = 0;

			m_mode = Mode::Stopped;
			m_lastDisplayedSeconds = remainingSeconds;
			m_display.showCountdown(remainingSeconds, m_errorCount, maxErrorCount);

			sendBleStatus();
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
					m_display.showMessage("TROPPI ERRORI", "ALLARME", penaltySeconds, m_errorCount, maxErrorCount);
				}
				else
				{
					Serial.println("Penalty countdown disabled. Remaining time unchanged.");

					m_lastDisplayedSeconds = remainingSeconds;
					m_display.showMessage("TROPPI ERRORI", "ALLARME", remainingSeconds, m_errorCount, maxErrorCount);
				}

				sendBleStatus();
			}
			else
			{
				m_lastDisplayedSeconds = remainingSeconds;
				showPinErrorMessage(remainingSeconds);

				sendBleStatus();
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

void Application::handleStopped(char key)
{
	(void)key;

	const uint32_t remainingSeconds = m_timer.getRemainingSeconds();
	const uint32_t maxErrorCount = m_storage.getConfig().maxErrorCount;

	if (remainingSeconds != m_lastDisplayedSeconds)
	{
		m_lastDisplayedSeconds = remainingSeconds;
		m_display.showCountdown(remainingSeconds, m_errorCount, maxErrorCount);
	}
}

void Application::showPinErrorMessage(uint32_t remainingSeconds)
{
	m_pinErrorMessageActive = true;
	m_pinErrorMessageStartedAt = millis();

	m_display.showPinError(
		remainingSeconds,
		m_errorCount,
		m_storage.getConfig().maxErrorCount
	);
}

void Application::updatePinErrorMessage()
{
	if (!m_pinErrorMessageActive)
	{
		return;
	}

	if (millis() - m_pinErrorMessageStartedAt < PinErrorMessageDurationMs)
	{
		return;
	}

	m_pinErrorMessageActive = false;
	restoreDisplayAfterPinError();
}

void Application::restoreDisplayAfterPinError()
{
	const uint32_t remainingSeconds = m_timer.getRemainingSeconds();
	const uint32_t maxErrorCount = m_storage.getConfig().maxErrorCount;

	switch (m_mode)
	{
		case Mode::EnterAdminPin:
			m_display.showAdminPin(m_adminPinInput.length(), 0, m_errorCount, maxErrorCount);
			break;

		case Mode::SetTimer:
			m_display.showSetTimer(m_timerInput, remainingSeconds, m_errorCount, maxErrorCount);
			break;

		case Mode::Running:
			m_lastDisplayedSeconds = remainingSeconds;
			m_display.showCountdown(remainingSeconds, m_errorCount, maxErrorCount);
			break;

		case Mode::Stopped:
			m_lastDisplayedSeconds = remainingSeconds;
			m_display.showCountdown(remainingSeconds, m_errorCount, maxErrorCount);
			break;

		case Mode::Finished:
			m_display.showFinished(m_errorCount, maxErrorCount);
			break;
	}
}

void Application::handleBleCommand(const String &command)
{
	String cleanCommand = command;
	cleanCommand.trim();

	if (cleanCommand.length() == 0)
	{
		m_bleManager.sendResponse("ERR:EMPTY_COMMAND");
		return;
	}

	String upperCommand = cleanCommand;
	upperCommand.toUpperCase();

	if (upperCommand == "PING")
	{
		m_bleManager.sendResponse("OK:PONG");
		return;
	}

	if (upperCommand == "STATUS")
	{
		sendBleStatus();
		return;
	}

	if (upperCommand == "LOGOUT")
	{
		m_bleLoggedIn = false;
		m_bleManager.sendResponse("OK:LOGOUT");
		sendBleStatus();
		return;
	}

	if (upperCommand.startsWith("LOGIN:"))
	{
		String pin = cleanCommand.substring(6);
		pin.trim();

		if (pin == m_storage.getConfig().adminPin)
		{
			m_bleLoggedIn = true;
			m_bleManager.sendResponse("OK:LOGIN");
			sendBleStatus();
		}
		else
		{
			m_bleLoggedIn = false;
			m_bleManager.sendResponse("ERR:LOGIN");
			sendBleStatus();
		}

		return;
	}

	if (!m_bleLoggedIn)
	{
		m_bleManager.sendResponse("ERR:LOGIN_REQUIRED");
		return;
	}

	if (upperCommand == "GETCONFIG")
	{
		sendBleConfig();
		return;
	}

	if (upperCommand.startsWith("SETCONFIG:"))
	{
		const String body = cleanCommand.substring(10);

		if (!applyBleConfig(body))
		{
			m_bleManager.sendResponse("ERR:BAD_CONFIG");
			return;
		}

		m_bleManager.sendResponse("OK:CONFIG_SAVED;restart=1");
		sendBleConfig();
		sendBleStatus();
		return;
	}

	if (upperCommand == "GETUSERS")
	{
		sendBleUsers();
		return;
	}

	if (upperCommand.startsWith("ADDUSER:"))
	{
		const String body = cleanCommand.substring(8);

		if (!addBleUser(body))
		{
			m_bleManager.sendResponse("ERR:ADD_USER");
			return;
		}

		m_bleManager.sendResponse("OK:USER_ADDED");
		delay(20);
		sendBleUsers();
		return;
	}

	if (upperCommand.startsWith("UPDATEUSER:"))
	{
		const String body = cleanCommand.substring(11);

		if (!updateBleUser(body))
		{
			m_bleManager.sendResponse("ERR:UPDATE_USER");
			return;
		}

		m_bleManager.sendResponse("OK:USER_UPDATED");
		delay(20);
		sendBleUsers();
		return;
	}

	if (upperCommand.startsWith("DELUSER:"))
	{
		String idText = cleanCommand.substring(8);
		idText.trim();

		if (!isUnsignedNumber(idText))
		{
			m_bleManager.sendResponse("ERR:BAD_USER_ID");
			return;
		}

		const uint32_t id = static_cast<uint32_t>(idText.toInt());

		if (id == 0 || id > 65535)
		{
			m_bleManager.sendResponse("ERR:BAD_USER_ID");
			return;
		}

		if (!m_users.removeUser(static_cast<uint16_t>(id)))
		{
			m_bleManager.sendResponse("ERR:USER_NOT_FOUND");
			return;
		}

		m_bleManager.sendResponse("OK:USER_DELETED");
		delay(20);
		sendBleUsers();
		return;
	}

	if (upperCommand.startsWith("SETTIME:"))
	{
		if (m_mode == Mode::Running && m_timer.isRunning())
		{
			m_bleManager.sendResponse("ERR:TIMER_RUNNING");
			return;
		}

		String timerText = cleanCommand.substring(8);
		timerText.trim();

		const uint32_t duration = parseTimerText(timerText);

		if (duration == 0)
		{
			m_bleManager.sendResponse("ERR:BAD_TIME");
			return;
		}

		m_timer.setDuration(duration);

		m_timerInput = "";
		m_disarmPinInput = "";
		m_errorCount = 0;
		m_lastDisplayedSeconds = 0xFFFFFFFF;

		m_mode = Mode::SetTimer;
		m_display.showSetTimer(m_timerInput, 0, m_errorCount, m_storage.getConfig().maxErrorCount);

		String response = "OK:TIME_SET:";
		response += duration;

		m_bleManager.sendResponse(response);
		sendBleStatus();

		return;
	}

	if (upperCommand == "START")
	{
		if (m_mode == Mode::Running && m_timer.isRunning())
		{
			m_bleManager.sendResponse("ERR:ALREADY_RUNNING");
			return;
		}

		if (m_timer.getDuration() == 0)
		{
			m_bleManager.sendResponse("ERR:NO_TIME");
			return;
		}

		m_timer.start();

		m_disarmPinInput = "";
		m_errorCount = 0;
		m_lastDisplayedSeconds = 0xFFFFFFFF;
		m_mode = Mode::Running;

		m_bleManager.sendResponse("OK:START");
		sendBleStatus();

		return;
	}

	if (upperCommand == "STOP")
	{
		const uint32_t remainingSeconds = m_timer.getRemainingSeconds();
		const uint32_t maxErrorCount = m_storage.getConfig().maxErrorCount;

		m_timer.stop();

		m_timerInput = "";
		m_disarmPinInput = "";
		m_errorCount = 0;

		m_mode = Mode::Stopped;
		m_lastDisplayedSeconds = remainingSeconds;
		m_display.showCountdown(remainingSeconds, m_errorCount, maxErrorCount);

		m_bleManager.sendResponse("OK:STOP");
		sendBleStatus();

		return;
	}

	if (upperCommand == "RESET")
	{
		m_timer.reset();

		m_timerInput = "";
		m_disarmPinInput = "";
		m_errorCount = 0;
		m_lastDisplayedSeconds = 0xFFFFFFFF;

		m_mode = Mode::SetTimer;
		m_display.showSetTimer(m_timerInput, 0, m_errorCount, m_storage.getConfig().maxErrorCount);

		m_bleManager.sendResponse("OK:RESET");
		sendBleStatus();

		return;
	}

	m_bleManager.sendResponse("ERR:UNKNOWN_COMMAND");
}

void Application::sendBleStatus()
{
	const uint32_t remainingSeconds = m_timer.getRemainingSeconds();
	const uint32_t durationSeconds = m_timer.getDuration();
	const uint32_t maxErrorCount = m_storage.getConfig().maxErrorCount;
	const bool locked = m_errorCount >= maxErrorCount;

	String response = "STATUS:";
	response += modeToString();
	response += ";remaining=";
	response += remainingSeconds;
	response += ";duration=";
	response += durationSeconds;
	response += ";errors=";
	response += m_errorCount;
	response += "/";
	response += maxErrorCount;
	response += ";locked=";
	response += (locked ? "1" : "0");
	response += ";logged=";
	response += (m_bleLoggedIn ? "1" : "0");

	m_bleManager.sendResponse(response);

	m_hasLastBleStatus = true;
	m_lastBleStatusMode = m_mode;
	m_lastBleStatusRemainingSeconds = remainingSeconds;
	m_lastBleStatusDurationSeconds = durationSeconds;
	m_lastBleStatusErrorCount = m_errorCount;
	m_lastBleStatusMaxErrorCount = maxErrorCount;
	m_lastBleStatusLocked = locked;
	m_lastBleStatusLoggedIn = m_bleLoggedIn;
}

void Application::sendBleStatusIfChanged()
{
	const uint32_t remainingSeconds = m_timer.getRemainingSeconds();
	const uint32_t durationSeconds = m_timer.getDuration();
	const uint32_t maxErrorCount = m_storage.getConfig().maxErrorCount;
	const bool locked = m_errorCount >= maxErrorCount;

	if (!m_hasLastBleStatus ||
		m_lastBleStatusMode != m_mode ||
		m_lastBleStatusRemainingSeconds != remainingSeconds ||
		m_lastBleStatusDurationSeconds != durationSeconds ||
		m_lastBleStatusErrorCount != m_errorCount ||
		m_lastBleStatusMaxErrorCount != maxErrorCount ||
		m_lastBleStatusLocked != locked ||
		m_lastBleStatusLoggedIn != m_bleLoggedIn)
	{
		sendBleStatus();
	}
}

void Application::sendBleConfig()
{
	const AppConfig &config = m_storage.getConfig();

	String response;
	response.reserve(182);
	response = "CONFIG:adminPin=";
	response += config.adminPin;
	response += ";bleName=";
	response += config.bleName;
	response += ";soundEnabled=";
	response += (config.soundEnabled ? "1" : "0");
	response += ";rfid=";
	response += (config.rfid ? "1" : "0");
	response += ";fingerprint=";
	response += (config.fingerprint ? "1" : "0");
	response += ";maxErrorCount=";
	response += config.maxErrorCount;
	response += ";errorCountdownSeconds=";
	response += config.errorCountdownSeconds;

	m_bleManager.sendResponse(response);
}

void Application::sendBleUsers()
{
	String beginMessage = "USERS_BEGIN:count=";
	beginMessage += m_users.count();
	m_bleManager.sendResponse(beginMessage);
	delay(20);

	for (uint8_t position = 0; position < m_users.count(); position++)
	{
		UserRecord user;

		if (!m_users.getUserByPosition(position, user))
		{
			continue;
		}

		String response;
		response.reserve(128);
		response = "USER:id=";
		response += user.id;
		response += ";name=";
		response += user.name;
		response += ";uid=";
		response += user.uid;
		response += ";pin=";
		response += user.pin;

		m_bleManager.sendResponse(response);
		delay(20);
	}

	m_bleManager.sendResponse("USERS_END");
}

bool Application::applyBleConfig(const String &body)
{
	const String adminPin = getCommandValue(body, "adminPin");
	const String bleName = getCommandValue(body, "bleName");
	const String soundText = getCommandValue(body, "soundEnabled");
	const String rfidText = getCommandValue(body, "rfid");
	const String fingerprintText = getCommandValue(body, "fingerprint");
	const String maxErrorText = getCommandValue(body, "maxErrorCount");
	const String penaltyText = getCommandValue(body, "errorCountdownSeconds");

	bool soundEnabled = false;
	bool rfid = false;
	bool fingerprint = false;

	if (!isValidPin(adminPin) || !isValidProtocolText(bleName, 48))
	{
		return false;
	}

	if (!parseBooleanValue(soundText, soundEnabled) ||
		!parseBooleanValue(rfidText, rfid) ||
		!parseBooleanValue(fingerprintText, fingerprint))
	{
		return false;
	}

	if (!isUnsignedNumber(maxErrorText) || !isUnsignedNumber(penaltyText))
	{
		return false;
	}

	const uint32_t maxErrorCount = static_cast<uint32_t>(maxErrorText.toInt());
	const uint32_t errorCountdownSeconds = static_cast<uint32_t>(penaltyText.toInt());

	if (maxErrorCount < 1 || maxErrorCount > 10 || errorCountdownSeconds > 3600)
	{
		return false;
	}

	AppConfig config = m_storage.getConfig();
	config.adminPin = adminPin;
	config.bleName = bleName;
	config.soundEnabled = soundEnabled;
	config.rfid = rfid;
	config.fingerprint = fingerprint;
	config.maxErrorCount = maxErrorCount;
	config.errorCountdownSeconds = errorCountdownSeconds;

	return m_storage.saveConfig(config);
}

bool Application::addBleUser(const String &body)
{
	const String name = getCommandValue(body, "name");
	String uid = getCommandValue(body, "uid");
	const String pin = getCommandValue(body, "pin");

	uid.toUpperCase();

	if (!isValidProtocolText(name, 32) || uid.length() > 32 || !isHexString(uid) || !isValidPin(pin))
	{
		return false;
	}

	return m_users.addUser(name, uid, pin);
}

bool Application::updateBleUser(const String &body)
{
	const String idText = getCommandValue(body, "id");
	const String name = getCommandValue(body, "name");
	String uid = getCommandValue(body, "uid");
	const String pin = getCommandValue(body, "pin");

	uid.toUpperCase();

	if (!isUnsignedNumber(idText))
	{
		return false;
	}

	const uint32_t id = static_cast<uint32_t>(idText.toInt());

	if (id == 0 || id > 65535)
	{
		return false;
	}

	if (!isValidProtocolText(name, 32) || uid.length() > 32 || !isHexString(uid) || !isValidPin(pin))
	{
		return false;
	}

	return m_users.updateUser(static_cast<uint16_t>(id), name, uid, pin);
}

String Application::getCommandValue(const String &body, const String &key) const
{
	uint16_t start = 0;

	while (start <= body.length())
	{
		int separator = body.indexOf(';', start);

		if (separator < 0)
		{
			separator = body.length();
		}

		const String part = body.substring(start, separator);
		const int equals = part.indexOf('=');

		if (equals > 0)
		{
			const String currentKey = part.substring(0, equals);

			if (currentKey == key)
			{
				return part.substring(equals + 1);
			}
		}

		if (separator >= static_cast<int>(body.length()))
		{
			break;
		}

		start = separator + 1;
	}

	return "";
}

bool Application::parseBooleanValue(const String &value, bool &result) const
{
	String normalized = value;
	normalized.toLowerCase();

	if (normalized == "1" || normalized == "true")
	{
		result = true;
		return true;
	}

	if (normalized == "0" || normalized == "false")
	{
		result = false;
		return true;
	}

	return false;
}

bool Application::isUnsignedNumber(const String &value) const
{
	if (value.length() == 0)
	{
		return false;
	}

	for (uint16_t i = 0; i < value.length(); i++)
	{
		if (value[i] < '0' || value[i] > '9')
		{
			return false;
		}
	}

	return true;
}

bool Application::isValidPin(const String &pin) const
{
	return pin.length() == 6 && isUnsignedNumber(pin);
}

bool Application::isValidProtocolText(const String &value, uint8_t maxLength) const
{
	if (value.length() == 0 || value.length() > maxLength)
	{
		return false;
	}

	return value.indexOf(';') < 0 && value.indexOf('=') < 0;
}

bool Application::isHexString(const String &value) const
{
	if (value.length() == 0)
	{
		return false;
	}

	for (uint16_t i = 0; i < value.length(); i++)
	{
		const char character = value[i];
		const bool isNumber = character >= '0' && character <= '9';
		const bool isUpperHex = character >= 'A' && character <= 'F';

		if (!isNumber && !isUpperHex)
		{
			return false;
		}
	}

	return true;
}

bool Application::isDigit(char key) const
{
	return key >= '0' && key <= '9';
}

uint32_t Application::parseTimerInput() const
{
	return parseTimerText(m_timerInput);
}

uint32_t Application::parseTimerText(const String &input) const
{
	if (input.length() != 6)
	{
		return 0;
	}

	for (uint8_t i = 0; i < 6; i++)
	{
		if (input[i] < '0' || input[i] > '9')
		{
			return 0;
		}
	}

	const uint8_t hours = (input[0] - '0') * 10 + (input[1] - '0');
	const uint8_t minutes = (input[2] - '0') * 10 + (input[3] - '0');
	const uint8_t seconds = (input[4] - '0') * 10 + (input[5] - '0');

	if (minutes > 59 || seconds > 59)
	{
		return 0;
	}

	return static_cast<uint32_t>(hours) * 3600UL +
		static_cast<uint32_t>(minutes) * 60UL +
		static_cast<uint32_t>(seconds);
}

const char *Application::modeToString() const
{
	switch (m_mode)
	{
		case Mode::EnterAdminPin:
			return "ADMIN_PIN";

		case Mode::SetTimer:
			return "SET_TIMER";

		case Mode::Running:
			return "RUNNING";

		case Mode::Stopped:
			return "STOPPED";

		case Mode::Finished:
			return "FINISHED";
	}

	return "UNKNOWN";
}
