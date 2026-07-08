#include "Storage.h"

#include <Arduino.h>

bool Storage::begin()
{
	Serial.println("Initializing storage...");

	// TODO: Initialize LittleFS

	Serial.println("Storage initialized.");

	return true;
}