#include <Arduino.h>
#include "Application.h"

Application app;

void setup()
{
	Serial.begin(115200);

	Serial.println();
	Serial.println("=========================================");
	Serial.println("Open Airsoft Countdown - GitHub");
	Serial.println("v1.9");
	Serial.println("=========================================");

	if (!app.begin())
	{
		Serial.println("Application initialization failed.");

		while (true)
		{
			delay(1000);
		}
	}

	Serial.println("System ready.");
}

void loop()
{
	app.update();
}
