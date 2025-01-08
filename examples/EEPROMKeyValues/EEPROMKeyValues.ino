/**
 * Escornabot-lib additional tool: read EEPROM stored keypad keys values (if any).
 *
 * This is a low level tool with code extracted from the Escornabot-lib itself. You can
 * use the library method getKeypadValues() if you really want to know which values are
 * in use after the library initialization (init()), as it's done in Luci's firmware
 * (firmware.ino).
 */

#include <avr/eeprom.h>

void setup()
{
	// serial port
	Serial.begin(9600);

	// banner
	Serial.println("Read Escornabot-lib EEPROM stored keypad keys values");

	// Index to the last 5 uint16_t EEPROM positions;
	// E2END = The last EEPROM address (bytes). 1023 for Arduino Nano 328
	#define EB_KP_EEPROM_VALUES_INDEX (uint16_t *)(E2END - 2 * 5 + 1)

	// Read and show keypad EEPROM values
	uint16_t *eeprom_index = EB_KP_EEPROM_VALUES_INDEX;
	int16_t values[5];
	for (uint8_t i = 0; i < 5; i ++)
	{
		values[i] = eeprom_read_word(eeprom_index);
		eeprom_index++;
	}
	Serial.print("   FORWARD ");  Serial.print(values[0]); Serial.print(" - 0x"); Serial.println(uint16_t(values[0]), HEX);
	Serial.print(" TURN LEFT ");  Serial.print(values[1]); Serial.print(" - 0x"); Serial.println(uint16_t(values[1]), HEX);
	Serial.print("        GO ");  Serial.print(values[2]); Serial.print(" - 0x"); Serial.println(uint16_t(values[2]), HEX);
	Serial.print("TURN RIGHT ");  Serial.print(values[3]); Serial.print(" - 0x"); Serial.println(uint16_t(values[3]), HEX);
	Serial.print("  BACKWARD ");  Serial.print(values[4]); Serial.print(" - 0x"); Serial.println(uint16_t(values[4]), HEX);
}  // setup()

void loop()
{
	// nothing here
}  // loop()
