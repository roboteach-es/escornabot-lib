/**
 * Luci's FIRMWARE.
 * 
 * This is a preliminary version of Luci's firmware, with almost all
 * the same functionalities as the current Escornabot's firmware but
 * with less complexity and overengineering.
 *
 * @file      Firmware.ino
 * @author    mgesteiro einsua
 * @date      20250101
 * @version   1.1.0
 * @copyright OpenSource, LICENSE GPLv3
 */

#define FIRMWARE_VERSION "1.1.0"
//#define DEBUG_MODE

#include <Arduino.h>
#include <Escornabot-lib.h>

const float LUCI_MOVE_DISTANCE      = 10.0;  // cms

// NOTE: if you change the following values, you should also update the logic in processProgram()
const float LUCI_ROTATE_DEGREES     = 90.0;  // degrees
const float LUCI_ROTATE_DEGREES_ALT = 45.0;  // degrees
const float LUCI_DIAGONAL_DISTANCE  = sqrt(2 * square(LUCI_MOVE_DISTANCE)); // Pythagoras, valid for 90/45

// change this if your stepper motors are reverse wired.
#define STEPPERMOTOR_FIXED_REVERSED false // fix stepper motors with swapped cables

// Luci color = Purple (~ darkish magenta)
#define LUCI_COLOR_R BRIGHTNESS_LEVEL * 0.4
#define LUCI_COLOR_G 0
#define LUCI_COLOR_B BRIGHTNESS_LEVEL

// Diagonal warning color = Orange
#define DIAGONAL_COLOR_R BRIGHTNESS_LEVEL
#define DIAGONAL_COLOR_G BRIGHTNESS_LEVEL * 0.4
#define DIAGONAL_COLOR_B 0

#define BEEP_DURATION_SHORT 100  // ms
#define BEEP_DURATION_LONG  200  // ms
#define RTTTL_STARTUP ":d=16,o=6,b=140:c,p,e,p,g,"
#define RTTTL_FINISH  ":d=16,o=6,b=800:f,4p,f,4p,f,4p,f,4p,c,4p,c,4p,c,4p,c,"
#define RTTTL_PRESET  ":d=8,o=4,b=320:d#6,e6,f#6,d#6,"  // Program RESET

#define PROGRAMMING 0
#define EXECUTING   1
uint8_t status = PROGRAMMING;

EB_T_COMMANDS program[128];  // list of actions saved
uint8_t program_count = 0;   // number of commands in the program
uint8_t program_index = 0;   // current command
uint8_t num_alt_turns = 0;   // number of alternative turns, for diagonal moves

Escornabot luci;
uint32_t currentTime;

/*
 *   S E T U P   &   L O O P   F U N C T I O N S
 */

void setup()
{
	// banner
	Serial.begin(EB_BAUDRATE); // 9600 by default
	Serial.print("Luci's FIRMWARE (v");
	Serial.print(FIRMWARE_VERSION);
	Serial.println(")");
	Serial.flush();
	// setup and start luci
	luci.init();
	startUpShow();
	// purge serial queue
	while (Serial.read() != -1);
	// fix problem with swapped cables in steppers
	#if STEPPERMOTOR_FIXED_REVERSED
	luci.fixReversed();
	#endif
}  // setup()

void loop()
{
	// get time
	currentTime = millis();

	// watch standby
	luci.handleStandby(currentTime);

	// watch keypad
	uint8_t kp_code = luci.handleKeypad(currentTime);

	// watch serial/bluetooth
	uint8_t bt_code = luci.handleSerial();

	// conduct!
	switch (status)
	{
	case PROGRAMMING:
		// accept commands
		if (kp_code) processKeyStroke(kp_code);
		if (bt_code) processKeyStroke(bt_code);
		break;
	case EXECUTING:
		// continue sequence
		if (kp_code || bt_code) stop(currentTime); // abort if any input
		else processProgram();
	}
}  // loop()


/*
 *   S U P P O R T   F U N C T I O N S
 */

/**
 * Start-up light and sound sequence.
 */
void startUpShow()
{
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
	luci.showColor(LUCI_COLOR_R, LUCI_COLOR_G, LUCI_COLOR_B); // input color, purple
	// startup tune
	luci.playRTTTL(RTTTL_STARTUP);
}  // startUpShow()

/**
 * Show command color
 * 
 * @param cmd  Command from which to show the associated color.
 */
void showCmdColor(EB_T_COMMANDS cmd)
{
	switch (cmd)
	{
	case EB_CMD_NN:  // NONE
		luci.showColor(0, 0, 0);  // off
		break;
	case EB_CMD_FW:  // FW
		luci.showColor(0, 0, BRIGHTNESS_LEVEL);  // blue
		break;
	case EB_CMD_TL:  // TL
		luci.showColor(BRIGHTNESS_LEVEL, 0, 0);  // red
		break;
	case EB_CMD_TR:  // TR
		luci.showColor(0, BRIGHTNESS_LEVEL, 0);  // green
		break;
	case EB_CMD_BW:  // BW
		luci.showColor(BRIGHTNESS_LEVEL, BRIGHTNESS_LEVEL, 0);  // yellow
		break;
	case EB_CMD_PA:  // Pause
		luci.showColor(BRIGHTNESS_LEVEL, BRIGHTNESS_LEVEL, 0);  // yellow
		break;
	case EB_CMD_TL_ALT:  // TL alternative
		luci.showColor(BRIGHTNESS_LEVEL, 0, BRIGHTNESS_LEVEL);  // red + blue = magenta
		break;
	case EB_CMD_TR_ALT: // TR alternative
		luci.showColor(0, BRIGHTNESS_LEVEL, BRIGHTNESS_LEVEL);  // blue + green = cyan
	}
}  // showCmdColor()

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
}  // addCommand()

/**
 * Stops current execution and clears everything.
 */
void stop(uint32_t currentTime)
{
	// shutdown execution
	luci.stopAction(currentTime);
	luci.disableStepperMotors();
	luci.clearKeypad(currentTime);
	luci.beep(EB_BEEP_DEFAULT, BEEP_DURATION_SHORT);
	if (num_alt_turns % 2 == 0) luci.showColor(LUCI_COLOR_R, LUCI_COLOR_G, LUCI_COLOR_B); // input color, purple
	else luci.showColor(DIAGONAL_COLOR_R, DIAGONAL_COLOR_G, DIAGONAL_COLOR_B); // diagonal!
	program_count = 0;  // reset program
	program_index = 0;  // and index
	status = PROGRAMMING;  // back to user input

	#ifdef DEBUG_MODE
	Serial.println("STOP!");
	#endif

	delay(500); // allow some time for sound and key stroke clearance
}  // stop()

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
			luci.showKeyColor(key);
			luci.beep(EB_BEEP_FORWARD, BEEP_DURATION_SHORT);
			addCommand(EB_CMD_FW);
			break;
		case EB_KP_KEY_TL:
			luci.showKeyColor(key);
			luci.beep(EB_BEEP_TURNLEFT, BEEP_DURATION_SHORT);
			addCommand(EB_CMD_TL);
			break;
		case EB_KP_KEY_GO:
			if (program_count < 1) break;

			luci.showKeyColor(key);
			luci.beep(EB_BEEP_DEFAULT, BEEP_DURATION_SHORT);
			status = EXECUTING;

			#ifdef DEBUG_MODE
			Serial.println("GO!");
			#endif

			break;
		case EB_KP_KEY_TR:
			luci.showKeyColor(key);
			luci.beep(EB_BEEP_TURNRIGHT, BEEP_DURATION_SHORT);
			addCommand(EB_CMD_TR);
			break;
		case EB_KP_KEY_BW:
			luci.showKeyColor(key);
			luci.beep(EB_BEEP_BACKWARD, BEEP_DURATION_SHORT);
			addCommand(EB_CMD_BW);
			break;
		default:
			// this case should not be possible
			return;
		}
		// go back to "input color" after a moment
		delay(BEEP_DURATION_SHORT + 50);
		if (num_alt_turns % 2 == 0) luci.showColor(LUCI_COLOR_R, LUCI_COLOR_G, LUCI_COLOR_B); // input color, purple
		else luci.showColor(DIAGONAL_COLOR_R, DIAGONAL_COLOR_G, DIAGONAL_COLOR_B); // diagonal!
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
			// program reset!!
			luci.showKeyColor(key);
			luci.beep(EB_BEEP_FORWARD, BEEP_DURATION_LONG);
			delay(400);
			luci.playRTTTL(RTTTL_PRESET);
			program_count = 0;  // reset program
			program_index = 0;  // and index
			num_alt_turns = 0;  // and alternate turns!
			break;
		case EB_KP_KEY_TL:
			luci.showKeyColor(key);
			luci.beep(EB_BEEP_TURNLEFT, BEEP_DURATION_LONG);
			addCommand(EB_CMD_TL_ALT);
			break;
		case EB_KP_KEY_TR:
			luci.showKeyColor(key);
			luci.beep(EB_BEEP_TURNRIGHT, BEEP_DURATION_LONG);
			addCommand(EB_CMD_TR_ALT);
			break;
		case EB_KP_KEY_BW:
			luci.showKeyColor(key);
			luci.beep(EB_BEEP_BACKWARD, BEEP_DURATION_LONG);
			addCommand(EB_CMD_PA);
			break;
		default:
			return; // unhandled case, avoid any further action
		}
		// go back to "input color" after a moment
		delay(BEEP_DURATION_LONG + 50);
		if (num_alt_turns % 2 == 0) luci.showColor(LUCI_COLOR_R, LUCI_COLOR_G, LUCI_COLOR_B); // input color, purple
		else luci.showColor(DIAGONAL_COLOR_R, DIAGONAL_COLOR_G, DIAGONAL_COLOR_B); // diagonal!
	}
}  // processKeyStroke()


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
				showCmdColor(program[program_index]);
				luci.beep(EB_BEEP_FORWARD, BEEP_DURATION_SHORT);
				if (num_alt_turns % 2 == 0) luci.prepareAction(EB_CMD_FW, LUCI_MOVE_DISTANCE);
				else luci.prepareAction(EB_CMD_FW, LUCI_DIAGONAL_DISTANCE);
				break;
			case EB_CMD_TL:
				showCmdColor(program[program_index]);
				luci.beep(EB_BEEP_TURNLEFT, BEEP_DURATION_SHORT);
				luci.prepareAction(EB_CMD_TL, LUCI_ROTATE_DEGREES);
				break;
			case EB_CMD_TR:
				showCmdColor(program[program_index]);
				luci.beep(EB_BEEP_TURNRIGHT, BEEP_DURATION_SHORT);
				luci.prepareAction(EB_CMD_TR, LUCI_ROTATE_DEGREES);
				break;
			case EB_CMD_BW:
				showCmdColor(program[program_index]);
				luci.beep(EB_BEEP_BACKWARD, BEEP_DURATION_SHORT);
				if (num_alt_turns % 2 == 0) luci.prepareAction(EB_CMD_BW, LUCI_MOVE_DISTANCE);
				else luci.prepareAction(EB_CMD_BW, LUCI_DIAGONAL_DISTANCE);
				break;
			case EB_CMD_PA:
				showCmdColor(program[program_index]);
				luci.beep(EB_BEEP_BACKWARD, BEEP_DURATION_SHORT);
				luci.prepareAction(EB_CMD_BW, LUCI_MOVE_DISTANCE); // same time as the movement
				break;
			case EB_CMD_TL_ALT:
				showCmdColor(program[program_index]);
				// Note = C#7, between C (TL) & D (FW)
				luci.playTone(2217, BEEP_DURATION_SHORT, false);
				luci.prepareAction(EB_CMD_TL_ALT, LUCI_ROTATE_DEGREES_ALT); // half degrees
				num_alt_turns ++;
				break;
			case EB_CMD_TR_ALT:
				showCmdColor(program[program_index]);
				// Note = D#7, between D (FW) & E (TR)
				luci.playTone(2489, BEEP_DURATION_SHORT, false);
				luci.prepareAction(EB_CMD_TR_ALT, LUCI_ROTATE_DEGREES_ALT); // half degrees
				num_alt_turns ++;
				break;
			}
		}
		else
		{
			// execution finished
			luci.disableStepperMotors();
			luci.playRTTTL(RTTTL_FINISH);
			if (num_alt_turns % 2 == 0) luci.showColor(LUCI_COLOR_R, LUCI_COLOR_G, LUCI_COLOR_B); // input color, purple
			else luci.showColor(DIAGONAL_COLOR_R, DIAGONAL_COLOR_G, DIAGONAL_COLOR_B); // diagonal!
			program_count = 0;  // reset program
			program_index = 0;  // and index
			status = PROGRAMMING;  // back to user input

			#ifdef DEBUG_MODE
			Serial.println("FINISHED");
			#endif
		}
	} // switch
}  // processProgram()
