#pragma once

#include <Arduino.h>

struct AppConfig
{
	String adminPin;
	String bleName;
	bool soundEnabled;
};

class Storage
{
public:
	bool begin();

	const AppConfig &getConfig() const;
	bool saveConfig(const AppConfig &config);

private:
	bool createDefaultConfig();
	bool createDefaultUsers();
	bool loadConfig();

	bool isValidAdminPin(const String &pin) const;

	AppConfig m_config;
};