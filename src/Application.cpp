#include "Application.h"

namespace
{
	constexpr uint32_t StartupScreenDurationMs = 3000;
	constexpr uint32_t PinErrorMessageDurationMs = 1500;
	constexpr uint32_t MaximumErrorMessageDurationMs = 2000;
	constexpr uint32_t MaximumErrorCompensationSeconds = MaximumErrorMessageDurationMs / 1000;
	constexpr uint32_t UserGreetingMessageDurationMs = 2000;
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

	if (!loadAuthorizedUsers())
	{
		Serial.println("Invalid authorized user configuration.");
		return false;
	}

	m_display.setLanguage(m_storage.getConfig().language);

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

			if (m_mode == Mode::Running)
			{
				m_disarmUidInput = m_nfcReader.getLastUid();
				m_disarmUidInput.trim();
				m_disarmUidInput.toUpperCase();
			}

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
	else if (m_maximumErrorMessageActive)
	{
		updateMaximumErrorMessage();
	}
	else if (m_userGreetingMessageActive)
	{
		updateUserGreetingMessage();
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
					resetDisarmAuthentication();
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
		if (!processDisarmAttempt(remainingSeconds))
		{
			Serial.println("Wrong or incomplete user authentication.");

			m_disarmPinInput = "";
			m_disarmUidInput = "";

			if (m_errorCount < maxErrorCount)
			{
				m_errorCount++;
			}

			if (m_errorCount >= maxErrorCount)
			{
				const uint32_t penaltySeconds = m_storage.getConfig().errorCountdownSeconds;
				uint32_t displayedRemainingSeconds = remainingSeconds;

				Serial.println("Maximum errors reached. Disarm locked.");

				if (penaltySeconds > 0)
				{
					const uint32_t compensatedPenaltySeconds =
						penaltySeconds + MaximumErrorCompensationSeconds;

					m_timer.setRemainingSeconds(compensatedPenaltySeconds);
					displayedRemainingSeconds = compensatedPenaltySeconds;
				}

				m_lastDisplayedSeconds = 0xFFFFFFFF;
				showMaximumErrorMessage(displayedRemainingSeconds);
			}
			else
			{
				m_lastDisplayedSeconds = remainingSeconds;
				showPinErrorMessage(remainingSeconds);
			}

			sendBleStatus();
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

void Application::showMaximumErrorMessage(uint32_t remainingSeconds)
{
	m_maximumErrorMessageActive = true;
	m_maximumErrorMessageStartedAt = millis();

	m_display.showMaximumError(
		remainingSeconds,
		m_errorCount,
		m_storage.getConfig().maxErrorCount
	);
}

void Application::updateMaximumErrorMessage()
{
	if (!m_maximumErrorMessageActive)
	{
		return;
	}

	if (millis() - m_maximumErrorMessageStartedAt < MaximumErrorMessageDurationMs)
	{
		return;
	}

	m_maximumErrorMessageActive = false;
	restoreDisplayAfterMaximumError();
}

void Application::restoreDisplayAfterMaximumError()
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
		case Mode::Stopped:
			m_lastDisplayedSeconds = remainingSeconds;
			m_display.showCountdown(remainingSeconds, m_errorCount, maxErrorCount);
			break;

		case Mode::Finished:
			m_display.showFinished(m_errorCount, maxErrorCount);
			break;
	}
}

bool Application::loadAuthorizedUsers()
{
	const String value = m_storage.getConfig().authorizedUserIds;
	m_requiredUserCount = 0;
	m_requireAllUsers = value.indexOf(',') >= 0;

	const char separator = m_requireAllUsers ? ',' : ';';
	uint16_t start = 0;

	for (uint16_t i = 0; i <= value.length(); i++)
	{
		if (i < value.length() && value[i] != separator)
		{
			continue;
		}

		if (m_requiredUserCount >= MaxRequiredUsers)
		{
			return false;
		}

		const uint16_t id = static_cast<uint16_t>(value.substring(start, i).toInt());
		UserRecord user;

		if (id == 0 || !m_users.getUserById(id, user))
		{
			Serial.print("Authorized user ID not found: ");
			Serial.println(id);
			return false;
		}

		for (uint8_t existing = 0; existing < m_requiredUserCount; existing++)
		{
			if (m_requiredUserIds[existing] == id)
			{
				return false;
			}
		}

		m_requiredUserIds[m_requiredUserCount] = id;
		m_authenticatedUsers[m_requiredUserCount] = false;
		m_requiredUserCount++;
		start = i + 1;
	}

	return m_requiredUserCount > 0;
}

void Application::resetDisarmAuthentication()
{
	m_disarmPinInput = "";
	m_disarmUidInput = "";

	for (uint8_t i = 0; i < MaxRequiredUsers; i++)
	{
		m_authenticatedUsers[i] = false;
	}

	m_userGreetingMessageActive = false;
	m_stopAfterUserGreeting = false;
}

bool Application::processDisarmAttempt(uint32_t remainingSeconds)
{
	const AppConfig &config = m_storage.getConfig();

	if (config.fingerprint)
	{
		m_display.showAuthenticationUnavailable(
			remainingSeconds,
			m_errorCount,
			config.maxErrorCount
		);
		return false;
	}

	for (uint8_t i = 0; i < m_requiredUserCount; i++)
	{
		if (m_authenticatedUsers[i])
		{
			continue;
		}

		UserRecord user;

		if (!m_users.getUserById(m_requiredUserIds[i], user))
		{
			continue;
		}

		if (user.pin != m_disarmPinInput)
		{
			continue;
		}

		if (config.rfid && user.uid != m_disarmUidInput)
		{
			continue;
		}

		completeUserAuthentication(user, remainingSeconds);
		m_disarmPinInput = "";
		m_disarmUidInput = "";
		return true;
	}

	return false;
}

bool Application::isUserRequired(uint16_t id) const
{
	for (uint8_t i = 0; i < m_requiredUserCount; i++)
	{
		if (m_requiredUserIds[i] == id)
		{
			return true;
		}
	}

	return false;
}

bool Application::isUserAlreadyAuthenticated(uint16_t id) const
{
	for (uint8_t i = 0; i < m_requiredUserCount; i++)
	{
		if (m_requiredUserIds[i] == id)
		{
			return m_authenticatedUsers[i];
		}
	}

	return false;
}

bool Application::areAllRequiredUsersAuthenticated() const
{
	for (uint8_t i = 0; i < m_requiredUserCount; i++)
	{
		if (!m_authenticatedUsers[i])
		{
			return false;
		}
	}

	return true;
}

void Application::completeUserAuthentication(const UserRecord &user, uint32_t remainingSeconds)
{
	for (uint8_t i = 0; i < m_requiredUserCount; i++)
	{
		if (m_requiredUserIds[i] == user.id)
		{
			m_authenticatedUsers[i] = true;
			break;
		}
	}

	m_stopAfterUserGreeting =
		!m_requireAllUsers || areAllRequiredUsersAuthenticated();

	showUserGreetingMessage(user, remainingSeconds);
}

void Application::showUserGreetingMessage(const UserRecord &user, uint32_t remainingSeconds)
{
	m_userGreetingMessageActive = true;
	m_userGreetingMessageStartedAt = millis();

	m_display.showUserGreeting(
		user.name,
		remainingSeconds,
		m_errorCount,
		m_storage.getConfig().maxErrorCount
	);
}

void Application::updateUserGreetingMessage()
{
	if (!m_userGreetingMessageActive)
	{
		return;
	}

	if (millis() - m_userGreetingMessageStartedAt < UserGreetingMessageDurationMs)
	{
		return;
	}

	m_userGreetingMessageActive = false;

	if (m_stopAfterUserGreeting)
	{
		stopTimerAfterAuthentication();
		return;
	}

	restoreDisplayAfterUserGreeting();
}

void Application::restoreDisplayAfterUserGreeting()
{
	const uint32_t remainingSeconds = m_timer.getRemainingSeconds();
	m_lastDisplayedSeconds = remainingSeconds;
	m_display.showCountdown(
		remainingSeconds,
		m_errorCount,
		m_storage.getConfig().maxErrorCount
	);
}

void Application::stopTimerAfterAuthentication()
{
	const uint32_t remainingSeconds = m_timer.getRemainingSeconds();
	const uint32_t maxErrorCount = m_storage.getConfig().maxErrorCount;

	Serial.println("Timer stopped by authorized user authentication.");

	m_timer.stop();
	m_timerInput = "";
	resetDisarmAuthentication();
	m_errorCount = 0;
	m_mode = Mode::Stopped;
	m_lastDisplayedSeconds = remainingSeconds;
	m_display.showCountdown(remainingSeconds, m_errorCount, maxErrorCount);

	sendBleStatus();
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
		resetDisarmAuthentication();
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

		resetDisarmAuthentication();
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
		resetDisarmAuthentication();
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
		resetDisarmAuthentication();
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
	response.reserve(240);
	response = "CONFIG:adminPin=";
	response += config.adminPin;
	response += ";bleName=";
	response += config.bleName;
	response += ";language=";
	response += config.language;
	response += ";authorizedUserIds=";
	response += config.authorizedUserIds;
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
	String language = getCommandValue(body, "language");
	String authorizedUserIds = getCommandValue(body, "authorizedUserIds");
	const String soundText = getCommandValue(body, "soundEnabled");
	const String rfidText = getCommandValue(body, "rfid");
	const String fingerprintText = getCommandValue(body, "fingerprint");
	const String maxErrorText = getCommandValue(body, "maxErrorCount");
	const String penaltyText = getCommandValue(body, "errorCountdownSeconds");

	bool soundEnabled = false;
	bool rfid = false;
	bool fingerprint = false;

	if (language.length() == 0)
	{
		language = m_storage.getConfig().language;
	}

	if (authorizedUserIds.length() == 0)
	{
		authorizedUserIds = m_storage.getConfig().authorizedUserIds;
	}

	authorizedUserIds.trim();
	authorizedUserIds.replace(" ", "");

	language.trim();
	language.toLowerCase();

	if (!isValidPin(adminPin) ||
		!isValidProtocolText(bleName, 48) ||
		!isValidLanguage(language) ||
		authorizedUserIds.length() == 0 ||
		authorizedUserIds.length() > 32)
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
	config.language = language;
	config.authorizedUserIds = authorizedUserIds;
	config.soundEnabled = soundEnabled;
	config.rfid = rfid;
	config.fingerprint = fingerprint;
	config.maxErrorCount = maxErrorCount;
	config.errorCountdownSeconds = errorCountdownSeconds;

	if (!m_storage.saveConfig(config))
	{
		return false;
	}

	m_display.setLanguage(config.language);

	return true;
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

bool Application::isValidLanguage(const String &language) const
{
	return language == "it" || language == "en";
}

bool Application::isEnglishLanguage() const
{
	return m_storage.getConfig().language == "en";
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
