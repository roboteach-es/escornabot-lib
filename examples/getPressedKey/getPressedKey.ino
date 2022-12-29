/**
 * Escornabot-lib getPressedKey() test.
 * 
 * getPressedKey() is is a low level function: no logic, timing or debouncing is done.
 * 
 */

#include <Escornabot-lib.h>
Escornabot luci;


void setup() {
	Serial.begin(9600);
	Serial.println("Luci's getPressedKey() test");
	luci.init();
	luci.beep(EB_BEEP_DEFAULT, 100);
	luci.showKeyColor(EB_LUCI_COLOR);
}

void loop() {
	EB_T_KP_KEYS key = luci.getPressedKey();
	Serial.println(EB_KP_KEYS_LABELS[key]);
	delay(200);
}
