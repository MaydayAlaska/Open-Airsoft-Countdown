#include "Storage.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

namespace
{
	const char *ConfigFilePath = "/config.json";
	const char *UsersFilePath = "/users.json";

	const char *DefaultAdminPin = "000000";
	const char *DefaultBleName = "Open Airsoft Countdown";
	const char *DefaultLanguage = "it";
	const char *DefaultAuthorizedUserIds = "1";
	const bool DefaultSoundEnabled = true;
	const bool DefaultRfid = false;
	const bool DefaultFingerprint = false;
	const uint32_t DefaultMaxErrorCount = 3;
	const uint32_t DefaultErrorCountdownSeconds = 10;
}

bool Storage::begin()
{
	Serial.println("Initializing storage...");

	if (!LittleFS.begin(true))
	{
		Serial.println("LittleFS initialization failed.");
		return false;
	}

	if (!LittleFS.exists(ConfigFilePath))
	{
		Serial.println("config.json not found. Creating default config...");

		if (!createDefaultConfig())
		{
			Serial.println("Failed to create config.json.");
			return false;
		}
	}

	if (!LittleFS.exists(UsersFilePath))
	{
		Serial.println("users.json not found. Creating default users file...");

		if (!createDefaultUsers())
		{
			Serial.println("Failed to create users.json.");
			return false;
		}
	}

	if (!loadConfig())
	{
		Serial.println("Failed to load config.json.");
		return false;
	}

	Serial.println("Storage initialized.");
	Serial.print("BLE name: ");
	Serial.println(m_config.bleName);
	Serial.print("UI language: ");
	Serial.println(m_config.language);
	Serial.print("Authorized user IDs: ");
	Serial.println(m_config.authorizedUserIds);
	Serial.print("Sound enabled: ");
	Serial.println(m_config.soundEnabled ? "yes" : "no");
	Serial.print("RFID enabled: ");
	Serial.println(m_config.rfid ? "yes" : "no");
	Serial.print("Fingerprint enabled: ");
	Serial.println(m_config.fingerprint ? "yes" : "no");
	Serial.print("Max error count: ");
	Serial.println(m_config.maxErrorCount);
	Serial.print("Error countdown seconds: ");
	Serial.println(m_config.errorCountdownSeconds);

	return true;
}

const AppConfig &Storage::getConfig() const
{
	return m_config;
}

bool Storage::saveConfig(const AppConfig &config)
{
	AppConfig normalizedConfig = config;
	normalizedConfig.language = sanitizeLanguage(config.language);
	normalizedConfig.authorizedUserIds = sanitizeAuthorizedUserIds(config.authorizedUserIds);

	File file = LittleFS.open(ConfigFilePath, "w");

	if (!file)
	{
		return false;
	}

	JsonDocument document;

	document["adminPin"] = normalizedConfig.adminPin;
	document["bleName"] = normalizedConfig.bleName;
	document["language"] = normalizedConfig.language;
	document["authorizedUserIds"] = normalizedConfig.authorizedUserIds;
	document["soundEnabled"] = normalizedConfig.soundEnabled;
	document["rfid"] = normalizedConfig.rfid;
	document["fingerprint"] = normalizedConfig.fingerprint;
	document["maxErrorCount"] = normalizedConfig.maxErrorCount;
	document["errorCountdownSeconds"] = normalizedConfig.errorCountdownSeconds;

	const size_t bytesWritten = serializeJsonPretty(document, file);

	file.close();

	if (bytesWritten == 0)
	{
		return false;
	}

	m_config = normalizedConfig;

	return true;
}

void Storage::printFileSystem() const
{
	Serial.println();
	Serial.println("LittleFS files:");
	Serial.println("--------------------------------");

	File root = LittleFS.open("/");

	if (!root)
	{
		Serial.println("Failed to open LittleFS root.");
		return;
	}

	File file = root.openNextFile();

	while (file)
	{
		Serial.print(file.name());
		Serial.print(" - ");
		Serial.print(file.size());
		Serial.println(" bytes");

		file = root.openNextFile();
	}

	Serial.println("--------------------------------");

	printFile(ConfigFilePath);
	printFile(UsersFilePath);

	Serial.println("--------------------------------");
	Serial.println();
}

bool Storage::createDefaultConfig()
{
	AppConfig defaultConfig;

	defaultConfig.adminPin = DefaultAdminPin;
	defaultConfig.bleName = DefaultBleName;
	defaultConfig.language = DefaultLanguage;
	defaultConfig.authorizedUserIds = DefaultAuthorizedUserIds;
	defaultConfig.soundEnabled = DefaultSoundEnabled;
	defaultConfig.rfid = DefaultRfid;
	defaultConfig.fingerprint = DefaultFingerprint;
	defaultConfig.maxErrorCount = DefaultMaxErrorCount;
	defaultConfig.errorCountdownSeconds = DefaultErrorCountdownSeconds;

	return saveConfig(defaultConfig);
}

bool Storage::createDefaultUsers()
{
	File file = LittleFS.open(UsersFilePath, "w");

	if (!file)
	{
		return false;
	}

	JsonDocument document;
	document.to<JsonArray>();

	const size_t bytesWritten = serializeJsonPretty(document, file);

	file.close();

	return bytesWritten > 0;
}

bool Storage::loadConfig()
{
	File file = LittleFS.open(ConfigFilePath, "r");

	if (!file)
	{
		return false;
	}

	JsonDocument document;
	DeserializationError error = deserializeJson(document, file);

	file.close();

	if (error)
	{
		Serial.println("config.json is invalid. Recreating default config...");
		return createDefaultConfig();
	}

	bool shouldSaveConfig = false;

	if (!document["adminPin"].is<const char *>())
	{
		shouldSaveConfig = true;
	}

	if (!document["bleName"].is<const char *>())
	{
		shouldSaveConfig = true;
	}

	if (!document["language"].is<const char *>())
	{
		shouldSaveConfig = true;
	}

	if (!document["authorizedUserIds"].is<const char *>())
	{
		shouldSaveConfig = true;
	}

	if (!document["soundEnabled"].is<bool>())
	{
		shouldSaveConfig = true;
	}

	if (!document["rfid"].is<bool>())
	{
		shouldSaveConfig = true;
	}

	if (!document["fingerprint"].is<bool>())
	{
		shouldSaveConfig = true;
	}

	if (!document["maxErrorCount"].is<uint32_t>())
	{
		shouldSaveConfig = true;
	}

	if (!document["errorCountdownSeconds"].is<uint32_t>())
	{
		shouldSaveConfig = true;
	}

	const char *adminPin = document["adminPin"] | DefaultAdminPin;
	const char *bleName = document["bleName"] | DefaultBleName;
	const char *language = document["language"] | DefaultLanguage;
	const char *authorizedUserIds = document["authorizedUserIds"] | DefaultAuthorizedUserIds;

	m_config.adminPin = adminPin;
	m_config.bleName = bleName;
	m_config.language = language;
	m_config.authorizedUserIds = authorizedUserIds;
	m_config.soundEnabled = document["soundEnabled"] | DefaultSoundEnabled;
	m_config.rfid = document["rfid"] | DefaultRfid;
	m_config.fingerprint = document["fingerprint"] | DefaultFingerprint;
	m_config.maxErrorCount = document["maxErrorCount"] | DefaultMaxErrorCount;
	m_config.errorCountdownSeconds = document["errorCountdownSeconds"] | DefaultErrorCountdownSeconds;

	if (!isValidAdminPin(m_config.adminPin))
	{
		m_config.adminPin = DefaultAdminPin;
		shouldSaveConfig = true;
	}

	if (m_config.bleName.length() == 0)
	{
		m_config.bleName = DefaultBleName;
		shouldSaveConfig = true;
	}

	const String sanitizedLanguage = sanitizeLanguage(m_config.language);

	if (sanitizedLanguage != m_config.language)
	{
		m_config.language = sanitizedLanguage;
		shouldSaveConfig = true;
	}

	const String sanitizedAuthorizedUserIds =
		sanitizeAuthorizedUserIds(m_config.authorizedUserIds);

	if (sanitizedAuthorizedUserIds != m_config.authorizedUserIds)
	{
		m_config.authorizedUserIds = sanitizedAuthorizedUserIds;
		shouldSaveConfig = true;
	}

	const uint32_t sanitizedMaxErrorCount = sanitizeMaxErrorCount(m_config.maxErrorCount);

	if (sanitizedMaxErrorCount != m_config.maxErrorCount)
	{
		m_config.maxErrorCount = sanitizedMaxErrorCount;
		shouldSaveConfig = true;
	}

	const uint32_t sanitizedErrorCountdownSeconds = sanitizeErrorCountdownSeconds(m_config.errorCountdownSeconds);

	if (sanitizedErrorCountdownSeconds != m_config.errorCountdownSeconds)
	{
		m_config.errorCountdownSeconds = sanitizedErrorCountdownSeconds;
		shouldSaveConfig = true;
	}

	if (shouldSaveConfig)
	{
		return saveConfig(m_config);
	}

	return true;
}

bool Storage::isValidAdminPin(const String &pin) const
{
	if (pin.length() != 6)
	{
		return false;
	}

	for (uint8_t i = 0; i < 6; i++)
	{
		if (pin[i] < '0' || pin[i] > '9')
		{
			return false;
		}
	}

	return true;
}

String Storage::sanitizeLanguage(const String &language) const
{
	String normalized = language;
	normalized.trim();
	normalized.toLowerCase();

	if (normalized == "it" || normalized == "en")
	{
		return normalized;
	}

	return String(DefaultLanguage);
}

String Storage::sanitizeAuthorizedUserIds(const String &value) const
{
	String normalized = value;
	normalized.trim();
	normalized.replace(" ", "");

	if (normalized.length() == 0)
	{
		return String(DefaultAuthorizedUserIds);
	}

	const bool hasComma = normalized.indexOf(',') >= 0;
	const bool hasSemicolon = normalized.indexOf(';') >= 0;

	if (hasComma && hasSemicolon)
	{
		return String(DefaultAuthorizedUserIds);
	}

	uint8_t idCount = 0;
	uint16_t start = 0;

	for (uint16_t i = 0; i <= normalized.length(); i++)
	{
		const bool atEnd = i == normalized.length();
		const bool atSeparator = !atEnd && (normalized[i] == ',' || normalized[i] == ';');

		if (!atEnd && !atSeparator)
		{
			if (normalized[i] < '0' || normalized[i] > '9')
			{
				return String(DefaultAuthorizedUserIds);
			}

			continue;
		}

		if (i == start)
		{
			return String(DefaultAuthorizedUserIds);
		}

		const uint32_t id = static_cast<uint32_t>(normalized.substring(start, i).toInt());

		if (id == 0 || id > 65535)
		{
			return String(DefaultAuthorizedUserIds);
		}

		idCount++;

		if (hasComma && idCount > 4)
		{
			return String(DefaultAuthorizedUserIds);
		}

		start = i + 1;
	}

	return normalized;
}

uint32_t Storage::sanitizeMaxErrorCount(uint32_t count) const
{
	if (count < 1 || count > 10)
	{
		return DefaultMaxErrorCount;
	}

	return count;
}

uint32_t Storage::sanitizeErrorCountdownSeconds(uint32_t seconds) const
{
	if (seconds > 3600)
	{
		return DefaultErrorCountdownSeconds;
	}

	return seconds;
}

void Storage::printFile(const char *path) const
{
	Serial.print("Content of ");
	Serial.println(path);
	Serial.println("--------------------------------");

	File file = LittleFS.open(path, "r");

	if (!file)
	{
		Serial.println("File not found.");
		return;
	}

	while (file.available())
	{
		Serial.write(file.read());
	}

	file.close();

	Serial.println();
	Serial.println("--------------------------------");
}
