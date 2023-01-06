/**
 * Escornabot-lib basic asynchronous example: move while processing key strokes
 *
 * Asynchronous actions take two steps:
 *   1. Prepare the action beforehand (to set-up some internal parameters)
 *   2. Call the action handler repeatedly (in the loop())
 *
 * The action handler executes, when the time is correct, very small (short)
 * tasks like one step in the stepper(s) motor(s).
 */

#include <Escornabot-lib.h>
Escornabot luci;

void setup() {
	// banner
	Serial.begin(9600);
	Serial.println("Escornalib basic asynchronous test for Luci");
	// setup luci
	luci.init();
	// start-up sequence: beep + Luci color
	luci.beep(EB_BEEP_DEFAULT, 100);
	luci.showKeyColor(EB_LUCI_COLOR);
	delay(1000);

	// prepare forward movement
	luci.prepareAction(EB_CMD_FW, 20.0);
}

void loop() {
	uint32_t currentTime = millis();

	// attend current movement
	uint8_t result = luci.handleAction(currentTime, EB_CMD_FW);

	if (result == EB_CMD_R_FINISHED_ACTION)
	{
		luci.beep(EB_BEEP_DEFAULT, 100);
		delay(1000);  // <-- NOTE: during this period, everything is blocked!
		// reschedule new forward move
		luci.prepareAction(EB_CMD_FW, 20.0);
	}

	// do other stuff while the robot is moving
	uint8_t code = luci.handleKeypad(currentTime);
	uint8_t event = code >> 4;  // upper nibble
	if (event == EB_KP_EVT_PRESSED)
	{
		// some key was pressed
		luci.beep(440, 100);
	}
}
