/*
 * Escornabot-lib basic example: move in squares!
 */

#include <Escornabot-lib.h>
Escornabot luci;


void setup(){
	// banner
	Serial.begin(9600);
	Serial.println("Escornalib basic test for Luci");
	// start-up sequence: beep + Luci color
	luci.beep(EB_BEEP_DEFAULT, 100);
	luci.showKeyColor(EB_LUCI_COLOR);
	delay(1000);
}

void loop() {
	// SQUARES!!

	// advance
	luci.showKeyColor(EB_KP_KEY_FW);  // blue
	luci.beep(EB_BEEP_FORWARD, 100);
	luci.move(10.0);

	// turn
	luci.showKeyColor(EB_KP_KEY_TR);  // green
	luci.beep(EB_BEEP_TURNRIGHT, 100);
	luci.turn(90);

}
