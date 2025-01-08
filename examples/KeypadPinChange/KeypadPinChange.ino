/**
 * Escornabot-lib example on how to change the pin/port for the analog
 * keypad, from the default A0 to A1.
 * 
 * NOTES:
 * - If you want this change to be permanet, you could also edit the
 *   Config.h file that exists alongside the other library files of
 *   Escornabot-lib (in your Arduino IDE libraries folder).
 * - In the previous mentioned library configuration file (Config.h)
 *   you can also change a lot of general other Escornabot parameters.
 * - Beware also that if you update the library via the Arduino IDE
 *   library manager, your changes will be lost.
 */

#include <Escornabot-lib.h>

Escornabot luci; // create our robot object

void setup()
{
	// setup luci
	luci.init(); // this will use the default A0 pin
	// banner
	Serial.println("Escornalib change keypad pin/port to A1");

	// change the pin
	int16_t* kv = luci.getKeypadValues(); // current keypad keys values
	luci.configKeypad(A1, kv[0], kv[1], kv[2], kv[3], kv[4], kv[5]); // change only pin/port

	// start-up sequence: beep + Luci color
	luci.beep(EB_BEEP_DEFAULT, 100);
	luci.showColor(50, 0, 20); // purple
	delay(1000);
}  // setup()

void loop()
{
	// read current pressed key (from A1, as changed in setup())
	EB_T_KP_KEYS key = luci.getPressedKey();

	// do something with every key press
	switch (key)
	{
	case EB_KP_KEY_FW:
		luci.showKeyColor(EB_KP_KEY_FW);  // blue
		luci.beep(EB_BEEP_FORWARD, 100);
		delay(200); // give it some time to finish
		break;
	case EB_KP_KEY_GO:
		luci.showKeyColor(EB_KP_KEY_GO);  // white
		luci.beep(EB_BEEP_DEFAULT, 100);
		delay(200); // give it some time to finish
		break;
	case EB_KP_KEY_BW:
		luci.showKeyColor(EB_KP_KEY_BW);  // blue
		luci.beep(EB_BEEP_BACKWARD, 100);
		delay(200); // give it some time to finish
		break;
	default:
		// nothing
		;
	} // switch
}  // loop()
