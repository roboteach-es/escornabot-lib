/**
 * Luci's FIRMWARE.
 * 
 * This is a preliminary version of Luci's firmware, with almost all
 * the same functionalities as the current Escornabot's firmware but
 * with less complexity and overengineering.
 *
 * @file      Firmware.ino
 * @author    mgesteiro
 * @date      20221230
 * @version   0.2.2-beta
 * @copyright OpenSource, LICENSE GPLv3
 */

#define FIRMWARE_VERSION "0.2.2-beta"
//#define DEBUG_MODE

#include <Arduino.h>
#include <Escornabot-lib.h>
#include "Config.h"

Escornabot luci;
uint32_t currentTime;

const float EB_ADVANCE    = 10.0;  // cms
const float EB_ROTATE     = 90.0;  // degrees
const float EB_ROTATE_ALT = 45.0;  // degrees
const float EB_ADVANCE_DIAGONAL = sqrt(2 * square(EB_ADVANCE));



#define RTTTL_STARTUP ":d=16,o=6,b=140:c,p,e,p,g,"
#define RTTTL_FINISH  ":d=16,o=6,b=800:f,4p,f,4p,f,4p,f,4p,c,4p,c,4p,c,4p,c,"
#define RTTTL_PRESET  ":d=16,o=6,b=140:g,p,e,p,c,"
#define RTTTL_TEST    ":d=16,o=4,b=200:c,c#,d,d#,e,f,f#,g,g#,a,a#,b,c5,c#5,d5,d#5,e5,f5,f#5,g5,g#5,a5,a#5,b5,c6,c#6,d6,d#6,e6,f6,f#6,g6,g#6,a6,a#6,b6,c7,c#7,d7,d#7,e7,f7,f#7,g7,g#7,a7,a#7,b7,"

#define PROGRAMMING 0
#define EXECUTING   1
uint8_t status = PROGRAMMING;

EB_T_COMMANDS program[128];  // list of actions saved
uint8_t program_count = 0;   // number of commands in the program
uint8_t program_index = 0;   // current command
uint8_t num_alt_turns = 0;   // number of alternative turns, for diagonal moves

/*
 *   S E T U P   &   L O O P   F U N C T I O N S
 */

void setup(){
	// banner
	Serial.begin(9600);
	Serial.print("Luci's FIRMWARE (v");
	Serial.print(FIRMWARE_VERSION);
	Serial.println(")");
	// setup and start luci
	luci.init();
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
	// start-up sequence: all button colors + purple + tune
	uint8_t tshow = 50;
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
	// finish with Luci color
	luci.showKeyColor(EB_LUCI_COLOR); // input color, purple
	// startup tune
	luci.playRTTTL(RTTTL_STARTUP);
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
	if (num_alt_turns % 2 == 0) luci.showKeyColor(EB_LUCI_COLOR); // input color, purple
	else luci.showColor(BRIGHTNESS_LEVEL, BRIGHTNESS_LEVEL * 0.4, 0); // orange, diagonal!
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
			if (program_count < 1) break;

			luci.beep(EB_BEEP_DEFAULT, 100);
			luci.showKeyColor(key);
			status = EXECUTING;

			#ifdef DEBUG_MODE
			Serial.println("GO!");
			#endif

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
		default:
			// this case should not be possible
			return;
		}
		// go back to "input color" after a moment
		delay(150);
		luci.showKeyColor(EB_LUCI_COLOR);
	}
	// LONG key presses
	else if (event == EB_KP_EVT_LONGPRESSED)
	{
		switch (key)
		{
		case EB_KP_KEY_FW:
			if
			(
				(program_count < 1)  // no program
				&& !(num_alt_turns % 2)  // diagonal angle
			) break;
			// reset!!
			luci.showKeyColor(key);
			luci.beep(EB_BEEP_DEFAULT, 100);
			delay(100);
			luci.playRTTTL(RTTTL_PRESET);
			program_count = 0;  // reset program
			program_index = 0;  // and index
			num_alt_turns = 0;  // and alternate turns!
			break;
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
		default:
			return; // unhandled case, avoid any further action
		}
		// go back to "input color" after a moment
		delay(150);
		luci.showKeyColor(EB_LUCI_COLOR);
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
			if (program_index == 0) delay(600); // small pause before starting

			switch (program[program_index])
			{
			case EB_CMD_FW:
				luci.showKeyColor(EB_KP_KEY_FW);
				luci.beep(EB_BEEP_FORWARD, 100);
				if (num_alt_turns % 2 == 0) luci.prepareAction(EB_CMD_FW, EB_ADVANCE);
				else luci.prepareAction(EB_CMD_FW, EB_ADVANCE_DIAGONAL);
				break;
			case EB_CMD_TL:
				luci.showKeyColor(EB_KP_KEY_TL);
				luci.beep(EB_BEEP_TURNLEFT, 100);
				luci.prepareAction(EB_CMD_TL, EB_ROTATE);
				break;
			case EB_CMD_TR:
				luci.showKeyColor(EB_KP_KEY_TR);
				luci.beep(EB_BEEP_TURNRIGHT, 100);
				luci.prepareAction(EB_CMD_TR, EB_ROTATE);
				break;
			case EB_CMD_BW:
				luci.showKeyColor(EB_KP_KEY_BW);
				luci.beep(EB_BEEP_BACKWARD, 100);
				if (num_alt_turns % 2 == 0) luci.prepareAction(EB_CMD_BW, EB_ADVANCE);
				else luci.prepareAction(EB_CMD_BW, EB_ADVANCE_DIAGONAL);
				break;
			case EB_CMD_PA:
				luci.showKeyColor(EB_KP_KEY_BW);
				luci.beep(EB_BEEP_BACKWARD, 100);
				luci.prepareAction(EB_CMD_BW, EB_ADVANCE); // same time as the movement
				break;
			case EB_CMD_TL_ALT:
				luci.showKeyColor(EB_KP_KEY_TL);
				luci.beep(EB_BEEP_TURNLEFT, 100);
				luci.prepareAction(EB_CMD_TL_ALT, EB_ROTATE_ALT); // half degrees
				num_alt_turns ++;
				break;
			case EB_CMD_TR_ALT:
				luci.showKeyColor(EB_KP_KEY_TR);
				luci.beep(EB_BEEP_TURNRIGHT, 100);
				luci.prepareAction(EB_CMD_TR_ALT, EB_ROTATE_ALT); // half degrees
				num_alt_turns ++;
				break;
			}
		}
		else
		{
			// execution finished
			luci.disableSM();
			luci.playRTTTL(RTTTL_FINISH);
			if (num_alt_turns % 2 == 0) luci.showKeyColor(EB_LUCI_COLOR); // input color, purple
			else luci.showColor(BRIGHTNESS_LEVEL, BRIGHTNESS_LEVEL * 0.4, 0); // orange, diagonal!
			program_count = 0;  // reset program
			program_index = 0;  // and index
			status = PROGRAMMING;  // back to user input

			#ifdef DEBUG_MODE
			Serial.println("FINISHED");
			#endif
		}
	} // switch
} // processProgram()
