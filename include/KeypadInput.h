#pragma once

#include <Arduino.h>

class KeypadInput
{
public:
	bool begin();
	void update();

	char getKey();

private:
	char m_pendingKey = '\0';
};