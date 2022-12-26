/**
 * Luci's FIRMWARE.
 * 
 * This is a preliminary version of Luci's firmware, with almost all 
 * the same functionalities as the current Escornabot firmware but 
 * with less complexity and overengineering.
 *
 * @file      Config.h
 * @author    mgesteiro
 * @date      20221216
 * @version   0.1.1-beta
 * @copyright OpenSource, LICENSE GPLv3
 */

#define VERSION "0.1.1-beta"

#include <Arduino.h>
#include <Wire.h>
#include <Escornabot-lib.h>

Escornabot luci;
uint32_t currentTime;

#define PROGRAMMING 0
#define EXECUTING   1
uint8_t status = PROGRAMMING;

EB_T_COMMANDS program[128];  // list of actions saved
uint8_t program_count = 0;   // number of commands in the program
uint8_t program_index = 0;   // current command


/*
 *   S E T U P   &   L O O P   F U N C T I O N S
 */

void setup(){
	// banner
	Serial.begin(9600);
	Serial.print("Luci's FIRMWARE (v");
	Serial.print(VERSION);
	Serial.println(")");
	startUpShow();
}

void loop() {
	// get time
	currentTime = millis();

	// watch standby
	luci.handleStandby(currentTime);

	// watch keypad
	uint8_t kp_code = luci.handleKeypad(currentTime);

	// conduct!
	switch (status)
	{
	case PROGRAMMING:
		// accept commands
		if (kp_code) processKeyStroke(kp_code);
		break;
	case EXECUTING:
		// continue sequence
		if (kp_code) stop(currentTime); // abort if any key is pressed
		else processProgram();
		break;
	}

}


/*
 *   S U P P O R T   F U N C T I O N S
 */

/**
 * Start-up light and sound sequence.
 */
void startUpShow() {
	// start-up sequence: beep + all button colors + purple
	luci.beep(EB_BEEP_DEFAULT, 100);
	uint8_t tshow = 150;
	luci.showKeyColor(EB_KP_KEY_FW); // blue
	delay(tshow);
	luci.showKeyColor(EB_KP_KEY_BW); // yellow
	delay(tshow);
	luci.showKeyColor(EB_KP_KEY_TL); // red
	delay(tshow);
	luci.showKeyColor(EB_KP_KEY_TR); // green
	delay(tshow);
	luci.showKeyColor(EB_KP_KEY_GO); // white
	delay(tshow);
	// finish with Luci color for a while
	luci.showKeyColor(EB_LUCI_COLOR); // purple
} // startUpShow()

/**
 * Add a command to our program/list.
 */
void addCommand(EB_T_COMMANDS command)
{
	if (program_count >= 128)
	{
		status = EXECUTING; // GO!
		return;
	}
	program[program_count] = command;
	program_count ++;

	#ifdef DEBUG_MODE
	Serial.print("ADDED ");
	Serial.println(EB_CMD_LABELS[command]);
	#endif
}

/**
 * Stops current execution and clears everything.
 */
void stop(uint32_t currentTime)
{
	// shutdown execution
	luci.stopAction(currentTime);
	luci.disableSM();
	luci.clearKeypad(currentTime);
	luci.beep(EB_BEEP_DEFAULT, 100);
	luci.showKeyColor(EB_LUCI_COLOR); // purple
	program_count = 0;  // reset program
	program_index = 0;  // and index
	status = PROGRAMMING;  // back to user input

	#ifdef DEBUG_MODE
	Serial.println("STOP!");
	#endif

	delay(500); // allow some time for key stroke clearance
}

/**
 * Takes care of the keypad and what to do when some key is used.
 * 
 * This function is responsible for the "PROGRAMMING" state when user input is
 * being attended and knows what to do with that input (like adding commands to
 * the list, launching the execution, etc.)
 */
void processKeyStroke(uint8_t kp_code)
{
	uint8_t key, event;
	key = kp_code & B1111; // lower nibble
	event = kp_code >> 4;  // upper nibble

	// SHORT key presses
	if (event == EB_KP_EVT_RELEASED)
	{
		switch (key)
		{
		case EB_KP_KEY_FW:
			luci.beep(EB_BEEP_FORWARD, 100);
			luci.showKeyColor(key);
			addCommand(EB_CMD_FW);
			break;
		case EB_KP_KEY_TL:
			luci.beep(EB_BEEP_TURNLEFT, 100);
			luci.showKeyColor(key);
			addCommand(EB_CMD_TL);
			break;
		case EB_KP_KEY_GO:
			luci.beep(EB_BEEP_DEFAULT, 100);
			luci.showKeyColor(key);
			status = EXECUTING;

			#ifdef DEBUG_MODE
			Serial.println("GO!");
			#endif

			delay(900); // small pause before starting
			break;
		case EB_KP_KEY_TR:
			luci.beep(EB_BEEP_TURNRIGHT, 100);
			luci.showKeyColor(key);
			addCommand(EB_CMD_TR);
			break;
		case EB_KP_KEY_BW:
			luci.beep(EB_BEEP_BACKWARD, 100);
			luci.showKeyColor(key);
			addCommand(EB_CMD_BW);
			break;
		}
		// remove color after a moment
		delay(150);
		luci.showKeyColor(0);
	}
	// LONG key presses
	else if (event == EB_KP_EVT_LONGPRESSED)
	{
		switch (key)
		{
		case EB_KP_KEY_TL:
			luci.beep(EB_BEEP_FORWARD, 100);
			luci.showKeyColor(key);
			addCommand(EB_CMD_TL_ALT);
			break;
		case EB_KP_KEY_TR:
			luci.beep(EB_BEEP_FORWARD, 100);
			luci.showKeyColor(key);
			addCommand(EB_CMD_TR_ALT);
			break;
		case EB_KP_KEY_BW:
			luci.beep(EB_BEEP_BACKWARD, 100);
			luci.showKeyColor(key);
			addCommand(EB_CMD_PA);
			break;
		}
		// remove color after a moment
		delay(150);
		luci.showKeyColor(0);
	}
} // processKeyStroke()


/**
 * Takes care of the program execution.
 * 
 * This function is responsible for the "EXECUTING" state when the program/list
 * of commands is being processed. It takes care of both, actions execution
 * themselves, and program/commands processing (reading and interpreting commands).
 */
void processProgram()
{
	// continue executing pending action (if any)
	switch (luci.handleAction(currentTime, program[program_index]))
	{
	case EB_CMD_R_PENDING_ACTION:
		return;  // still pending execution -> exit to the next loop

	case EB_CMD_R_FINISHED_ACTION:  // next command
		program_index ++;  // <- no "break"!!

	case EB_CMD_R_NOTHING_TO_DO:
	default:
		// process next command
		if (program_index < program_count)
		{
			// process command
			switch (program[program_index])
			{
			case EB_CMD_FW:
				luci.showKeyColor(EB_KP_KEY_FW);
				luci.beep(EB_BEEP_FORWARD, 100);
				luci.prepareAction(EB_CMD_FW, 10);
				break;
			case EB_CMD_TL:
				luci.showKeyColor(EB_KP_KEY_TL);
				luci.beep(EB_BEEP_TURNLEFT, 100);
				luci.prepareAction(EB_CMD_TL, 90);
				break;
			case EB_CMD_TR:
				luci.showKeyColor(EB_KP_KEY_TR);
				luci.beep(EB_BEEP_TURNRIGHT, 100);
				luci.prepareAction(EB_CMD_TR, 90);
				break;
			case EB_CMD_BW:
				luci.showKeyColor(EB_KP_KEY_BW);
				luci.beep(EB_BEEP_BACKWARD, 100);
				luci.prepareAction(EB_CMD_BW, 10);
				break;
			case EB_CMD_PA:
				luci.showKeyColor(EB_KP_KEY_BW);
				luci.beep(EB_BEEP_BACKWARD, 100);
				luci.prepareAction(EB_CMD_BW, 10); // same time as the movement
				break;
			case EB_CMD_TL_ALT:
				luci.showKeyColor(EB_KP_KEY_TL);
				luci.beep(EB_BEEP_TURNLEFT, 100);
				luci.prepareAction(EB_CMD_TL_ALT, 45); // half degrees
				break;
			case EB_CMD_TR_ALT:
				luci.showKeyColor(EB_KP_KEY_TR);
				luci.beep(EB_BEEP_TURNRIGHT, 100);
				luci.prepareAction(EB_CMD_TR_ALT, 45); // half degrees
				break;
			}
		}
		else
		{
			// execution finished
			luci.disableSM();
			// play final melody()
			luci.showKeyColor(EB_LUCI_COLOR); // purple
			program_count = 0;  // reset program
			program_index = 0;  // and index
			status = PROGRAMMING;  // back to user input

			#ifdef DEBUG_MODE
			Serial.println("FINISHED");
			#endif
		}
	} // switch
} // processProgram()
