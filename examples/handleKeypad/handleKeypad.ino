/**
 * Escornabot-lib handleKeypad() test.
 * 
 * handleKeypad() is a high level management function that processes 
 * the key strokes and converts them, using timing, debouncing and 
 * logic into useful information.
 * 
 */

#include <Escornabot-lib.h>
Escornabot luci; // create Escornabot object

void setup()
{
	// setup luci
	luci.init(); // 9600 baudrate
	// banner
	Serial.println("handleKeypad test for Luci");
	// start-up sequence: beep + Luci color
	luci.beep(EB_BEEP_DEFAULT, 100);
	luci.showColor(50, 0, 20); // purple
}  // setup()

void loop()
{
	uint32_t currentTime = millis(); // get time
	uint8_t code = luci.handleKeypad(currentTime);

	if (! code) return;  // nothing to process

	uint8_t key = code & B1111; // low nibble
	uint8_t event = code >> 4;  // high nibble

	Serial.print("KEY: [");
	Serial.print(EB_KP_KEYS_LABELS[key]);
	Serial.print("]  EVENT: ");

	switch (event) {
	case 1:
		Serial.println("PRESSED");
		break;
	case 2:
		Serial.println("RELEASED");
		break;
	case 3:
		Serial.println("LONGPRESSED");
		break;
	case 4:
		Serial.println("LONGRELEASED");
		break;
	default:
		Serial.println("UNKNOWN"); // shouldn't happen!
	}  // switch()
}  // loop()
