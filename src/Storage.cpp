#include "Storage.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

namespace
{
	const char *ConfigFilePath = "/config.json";
	const char *UsersFilePath = "/users.json";

	const char *DefaultAdminPassword = "admin";
	const char *DefaultBleName = "Open Airsoft Countdown";
	const bool DefaultSoundEnabled = true;
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

	document["adminPassword"] = config.adminPassword;
	document["bleName"] = config.bleName;
	document["soundEnabled"] = config.soundEnabled;

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

	defaultConfig.adminPassword = DefaultAdminPassword;
	defaultConfig.bleName = DefaultBleName;
	defaultConfig.soundEnabled = DefaultSoundEnabled;

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

		if (!createDefaultConfig())
		{
			return false;
		}

		return true;
	}

	m_config.adminPassword = document["adminPassword"] | DefaultAdminPassword;
	m_config.bleName = document["bleName"] | DefaultBleName;
	m_config.soundEnabled = document["soundEnabled"] | DefaultSoundEnabled;

	return true;
}