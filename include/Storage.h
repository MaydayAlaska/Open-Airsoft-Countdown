#pragma once

class Storage
{
public:
	bool begin();

private:
	bool createDefaultConfig();
	bool createDefaultUsers();
};