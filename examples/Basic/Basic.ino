/**
 * Escornabot-lib basic example: move in squares!
 */

#include <Escornabot-lib.h>
Escornabot luci; // create Escornabot object

void setup()
{
	// setup luci
	luci.init(); // 9600 baudrate
	// banner
	Serial.println("Escornalib basic test for Luci");
	// start-up sequence: beep + Luci color
	luci.beep(EB_BEEP_DEFAULT, 100);
	luci.showColor(50, 0, 20); // purple
	delay(1000);
}  // setup()

void loop()
{
	// advance
	luci.showKeyColor(EB_KP_KEY_FW);  // blue
	luci.beep(EB_BEEP_FORWARD, 100);
	luci.move(10.0);

	// turn
	luci.showKeyColor(EB_KP_KEY_TR);  // green
	luci.beep(EB_BEEP_TURNRIGHT, 100);
	luci.turn(90);
}  // loop()
