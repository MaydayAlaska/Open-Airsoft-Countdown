#pragma once

#include <Arduino.h>

struct AppConfig
{
	String adminPin;
	String bleName;
	String language;
	String authorizedUserIds;
	bool soundEnabled;
	bool rfid;
	bool fingerprint;
	uint32_t maxErrorCount;
	uint32_t errorCountdownSeconds;
};

class Storage
{
public:
	bool begin();

	const AppConfig &getConfig() const;
	bool saveConfig(const AppConfig &config);

	void printFileSystem() const;

private:
	bool createDefaultConfig();
	bool createDefaultUsers();
	bool loadConfig();

	bool isValidAdminPin(const String &pin) const;
	String sanitizeLanguage(const String &language) const;
	String sanitizeAuthorizedUserIds(const String &value) const;
	uint32_t sanitizeMaxErrorCount(uint32_t count) const;
	uint32_t sanitizeErrorCountdownSeconds(uint32_t seconds) const;

	void printFile(const char *path) const;

	AppConfig m_config;
};
