/**
 * Escornabot-lib getPressedKey() test.
 * 
 * getPressedKey() is is a low featured function: no logic, timing or debouncing is done.
 * 
 */

#include <Escornabot-lib.h>
Escornabot luci; // create Escornabot object

void setup()
{
	// setup luci
	luci.init(); // 9600 baudrate
	// banner
	Serial.println("Luci's getPressedKey() test");
	// start-up sequence: beep + Luci color
	luci.beep(EB_BEEP_DEFAULT, 100);
	luci.showColor(50, 0, 20); // purple
}  // setup()

void loop()
{
	// get pressed key
	EB_T_KP_KEYS key = luci.getPressedKey();
	// print name
	Serial.println(EB_KP_KEYS_LABELS[key]);
	// give it some room (as "debouncing")
	delay(200);
}  // loop()
