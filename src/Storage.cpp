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
	const bool DefaultSoundEnabled = true;
	const bool DefaultRfid = true;
	const bool DefaultFingerprint = false;
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
	Serial.print("Sound enabled: ");
	Serial.println(m_config.soundEnabled ? "yes" : "no");
	Serial.print("RFID enabled: ");
	Serial.println(m_config.rfid ? "yes" : "no");
	Serial.print("Fingerprint enabled: ");
	Serial.println(m_config.fingerprint ? "yes" : "no");

	return true;
}

const AppConfig &Storage::getConfig() const
{
	return m_config;
}

bool Storage::saveConfig(const AppConfig &config)
{
	File file = LittleFS.open(ConfigFilePath, "w");

	if (!file)
	{
		return false;
	}

	JsonDocument document;

	document["adminPin"] = config.adminPin;
	document["bleName"] = config.bleName;
	document["soundEnabled"] = config.soundEnabled;
	document["rfid"] = config.rfid;
	document["fingerprint"] = config.fingerprint;

	const size_t bytesWritten = serializeJsonPretty(document, file);

	file.close();

	if (bytesWritten == 0)
	{
		return false;
	}

	m_config = config;

	return true;
}

bool Storage::createDefaultConfig()
{
	AppConfig defaultConfig;

	defaultConfig.adminPin = DefaultAdminPin;
	defaultConfig.bleName = DefaultBleName;
	defaultConfig.soundEnabled = DefaultSoundEnabled;
	defaultConfig.rfid = DefaultRfid;
	defaultConfig.fingerprint = DefaultFingerprint;

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

	const char *adminPin = document["adminPin"] | DefaultAdminPin;
	const char *bleName = document["bleName"] | DefaultBleName;

	m_config.adminPin = adminPin;
	m_config.bleName = bleName;
	m_config.soundEnabled = document["soundEnabled"] | DefaultSoundEnabled;
	m_config.rfid = document["rfid"] | DefaultRfid;
	m_config.fingerprint = document["fingerprint"] | DefaultFingerprint;

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

	if (shouldSaveConfig)
	{
		saveConfig(m_config);
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