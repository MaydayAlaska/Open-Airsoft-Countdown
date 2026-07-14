#include "KeypadInput.h"

#include <Keypad.h>

namespace
{
	constexpr byte RowCount = 4;
	constexpr byte ColumnCount = 4;

	char KeyMap[RowCount][ColumnCount] =
	{
		{ '1', '2', '3', 'A' },
		{ '4', '5', '6', 'B' },
		{ '7', '8', '9', 'C' },
		{ '*', '0', '#', 'D' }
	};

	byte RowPins[RowCount] =
	{
		18,
		17,
		16,
		15
	};

	byte ColumnPins[ColumnCount] =
	{
		7,
		6,
		5,
		4
	};

	Keypad MatrixKeypad = Keypad(
		makeKeymap(KeyMap),
		RowPins,
		ColumnPins,
		RowCount,
		ColumnCount
	);
}

bool KeypadInput::begin()
{
	Serial.println("Initializing matrix keypad...");
	Serial.println("Matrix keypad initialized.");
	Serial.println("Keypad pin mapping:");
	Serial.println("Rows: R1=GPIO18, R2=GPIO17, R3=GPIO16, R4=GPIO15");
	Serial.println("Cols: C1=GPIO7, C2=GPIO6, C3=GPIO5, C4=GPIO4");

	return true;
}

void KeypadInput::update()
{
	const char key = MatrixKeypad.getKey();

	if (key == NO_KEY)
	{
		return;
	}

	m_pendingKey = key;

	Serial.print("Key pressed: ");
	Serial.println(key);
}

char KeypadInput::getKey()
{
	const char key = m_pendingKey;

	m_pendingKey = '\0';

	return key;
}
