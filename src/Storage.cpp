#include "Storage.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

namespace
{
	const char *ConfigFilePath = "/config.json";
	const char *UsersFilePath = "/users.json";
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

	Serial.println("Storage initialized.");

	return true;
}

bool Storage::createDefaultConfig()
{
	File file = LittleFS.open(ConfigFilePath, "w");

	if (!file)
	{
		return false;
	}

	JsonDocument document;

	document["adminPassword"] = "admin";
	document["bleName"] = "Open Airsoft Countdown";
	document["soundEnabled"] = true;

	serializeJsonPretty(document, file);

	file.close();

	return true;
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

	serializeJsonPretty(document, file);

	file.close();

	return true;
}