/**
 * Brivoi's FIRMWARE.
 *
 * This is a firmware for the Brivoi family of the Escornabot robot, more
 * functional, consistent and with less complexity & overengineering.
 *
 * Pay attention to the init function, which is called with configuration params.
 * Also:
 *   1. Use of the on-board LED instead of the RGB-LED (Brivoi hasn't one!).
 *   2. Geometry is slightly different from the default (Luci).
 *
 * Additional info and guides at roboteach.es/escornabot (spanish/galician).
 *
 * @file      Firmware-Brivoi.ino
 * @author    mgesteiro einsua
 * @date      20250420
 * @version   1.2.3
 * @copyright OpenSource, LICENSE GPLv3
 */

#define FIRMWARE_VERSION "1.2.3"
// change this to true if your stepper motors work backwards.
#define STEPPERMOTOR_FIXED_REVERSED false
// uncomment the following line if you want to see debug information (via serial)
//#define DEBUG_MODE

#include <Arduino.h>
#include <Escornabot-lib.h>

const float BRIVOI_MOVE_DISTANCE      = 10.0;  // cms
// NOTE: if you change the following values, you should also update the logic in processProgram()
const float BRIVOI_ROTATE_DEGREES     = 90.0;  // degrees
const float BRIVOI_ROTATE_DEGREES_ALT = 45.0;  // degrees
const float BRIVOI_DIAGONAL_DISTANCE  = sqrt(2 * square(BRIVOI_MOVE_DISTANCE)); // Pythagoras, valid for 90/45

#define BEEP_DURATION_SHORT 100  // ms
#define BEEP_DURATION_LONG  200  // ms
#define RTTTL_STARTUP ":d=16,o=6,b=140:c,p,e,p,g,"
#define RTTTL_FINISH  ":d=16,o=6,b=800:f,4p,f,4p,f,4p,f,4p,c,4p,c,4p,c,4p,c,"
#define RTTTL_PRESET  ":d=16,o=7,b=160:d#,e,f#,d#,"  // Program RESET
#define RTTTL_MODECHG ":d=16,o=7,b=140:f,p,d,2p,"    // Mode change

#define PROGRAMMING 0
#define EXECUTING   1
uint8_t status = PROGRAMMING;

#define STANDARD  1
#define DONTRESET 2
uint8_t mode = STANDARD;

EB_T_COMMANDS program[128];  // list of actions saved
uint8_t program_count = 0;   // number of commands in the program
uint8_t program_index = 0;   // current command
bool    is_diagonal = false; // indicates whether the next move is a diagonal

Escornabot brivoi;
uint32_t currentTime;

/*
 *   S E T U P   &   L O O P   F U N C T I O N S
 */

void setup()
{
	// setup and start brivoi
	brivoi.init(A4, 10, 12, EB_TYPE_BRIVOI); // Keypad, Buzzer, Neopixel, Wiring
	// change geometry
	#define WHEEL_DIAMETER   75.5f // mm
	#define WHEEL_SEPARATION 75.5f // mm
	#define WHEEL_CIRCUMFERENCE float(PI * WHEEL_DIAMETER) // wheel circumference
	#define ROTATION_CIRCUMFERENCE float(PI * WHEEL_SEPARATION) // rotation circunference on the floor
	#define STEPPERMOTOR_FULLREVOLUTION_STEPS 2048.0f // number of steps for a full revolution of the axis
	#define STEPPERS_STEPS_MM float(STEPPERMOTOR_FULLREVOLUTION_STEPS / WHEEL_CIRCUMFERENCE) // how many steps to move 1 mm
	#define STEPPERS_STEPS_DEG float((ROTATION_CIRCUMFERENCE/360) * STEPPERS_STEPS_MM) // how many steps to rotate 1 degree
	brivoi.setStepsPerMilimiter(STEPPERS_STEPS_MM);
	brivoi.setStepsPerDegree(STEPPERS_STEPS_DEG);
	// banner
	Serial.print(F("Brivoi's FIRMWARE v"));
	Serial.println(F(FIRMWARE_VERSION));
	// show keypad values
	int16_t* kv = brivoi.getKeypadValues();
	Serial.println(F("\nKeypad current values:"));
	Serial.print(F("      NONE "));  Serial.println(kv[0]);
	Serial.print(F("   FORWARD "));  Serial.println(kv[1]);
	Serial.print(F(" TURN LEFT "));  Serial.println(kv[2]);
	Serial.print(F("        GO "));  Serial.println(kv[3]);
	Serial.print(F("TURN RIGHT "));  Serial.println(kv[4]);
	Serial.print(F("  BACKWARD "));  Serial.println(kv[5]);
	Serial.println(F("\nGeometry values:"));
	Serial.print(F("  Wheel diameter   = "));  Serial.println(WHEEL_DIAMETER);
	Serial.print(F("  Wheel separation = "));  Serial.println(WHEEL_SEPARATION);
	Serial.print(F("  Steps/mm = "));  Serial.println(STEPPERS_STEPS_MM);
	Serial.print(F("  Steps/°  = "));  Serial.println(STEPPERS_STEPS_DEG);
	Serial.flush();
	// do initial show
	startUpShow();
	// purge serial queue
	while (Serial.read() != -1);
	// fix problem with swapped cables in steppers
	#if STEPPERMOTOR_FIXED_REVERSED
	brivoi.fixReversed();
	#endif
	// uncomment one of the following line to disable different timeouts
	brivoi.setStandbyTimeouts(0, INACTIVITY_TIMEOUT); // disable powerbank timeout
	//brivoi.setStandbyTimeouts(POWERBANK_TIMEOUT, 0);  // disable inactivity timeout
	//brivoi.setStandbyTimeouts(0, 0); // disable both timeouts
}  // setup()

void loop()
{
	// get time
	currentTime = millis();

	// watch standby
	brivoi.handleStandby(currentTime);

	// watch keypad
	uint8_t kp_code = brivoi.handleKeypad(currentTime);

	// watch serial/bluetooth
	uint8_t bt_code = brivoi.handleSerial();

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
	// start-up sequence: flash 3 times + tune
	brivoi.blinkLED(3);
	brivoi.playRTTTL(RTTTL_STARTUP);
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
		brivoi.turnLED(OFF);  // off
		break;
	case EB_CMD_FW:  // FW
	case EB_CMD_TL:  // TL
	case EB_CMD_TR:  // TR
	case EB_CMD_BW:  // BW
	case EB_CMD_PA:  // Pause
	case EB_CMD_TL_ALT:  // TL alternative
	case EB_CMD_TR_ALT:  // TR alternative
		brivoi.turnLED(ON);
	}
}  // showCmdColor()

/**
 * Add a command to our program/list.
 */
void addCommand(EB_T_COMMANDS command)
{
	if (program_count >= sizeof(program))
	{
		status = EXECUTING; // GO!
		return;
	}
	program[program_count] = command;
	program_count ++;

	#ifdef DEBUG_MODE
	Serial.print(F("ADDED "));
	Serial.println(EB_CMD_LABELS[command]);
	#endif
}  // addCommand()

/**
 * Stops current execution and clears everything.
 */
void stop(uint32_t currentTime)
{
	// shutdown execution
	brivoi.stopAction(currentTime);
	brivoi.disableStepperMotors();
	brivoi.clearKeypad(currentTime);
	brivoi.beep(EB_BEEP_DEFAULT, BEEP_DURATION_SHORT);
	if (mode == STANDARD)
		program_count = 0;  // reset program
	else
		is_diagonal = false;  // reset diagonal status
	program_index = 0;  // reset execution pointer
	if (! is_diagonal) brivoi.turnLED(OFF); // input status
	else brivoi.blinkLED(3, true); // diagonal!
	status = PROGRAMMING; // back to user input

	#ifdef DEBUG_MODE
	Serial.println(F("STOP!"));
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
			brivoi.turnLED(ON);
			brivoi.beep(EB_BEEP_FORWARD, BEEP_DURATION_SHORT);
			addCommand(EB_CMD_FW);
			break;
		case EB_KP_KEY_TL:
			brivoi.turnLED(ON);
			brivoi.beep(EB_BEEP_TURNLEFT, BEEP_DURATION_SHORT);
			addCommand(EB_CMD_TL);
			break;
		case EB_KP_KEY_GO:
			brivoi.turnLED(ON);
			brivoi.beep(EB_BEEP_DEFAULT, BEEP_DURATION_SHORT);

			if (program_count < 1) break;

			status = EXECUTING;

			#ifdef DEBUG_MODE
			Serial.println(F("GO!"));
			#endif

			break;
		case EB_KP_KEY_TR:
			brivoi.turnLED(ON);
			brivoi.beep(EB_BEEP_TURNRIGHT, BEEP_DURATION_SHORT);
			addCommand(EB_CMD_TR);
			break;
		case EB_KP_KEY_BW:
			brivoi.turnLED(ON);
			brivoi.beep(EB_BEEP_BACKWARD, BEEP_DURATION_SHORT);
			addCommand(EB_CMD_BW);
			break;
		default:
			// this case should not be possible
			return;
		}
		// go back to "input color" after a moment
		delay(BEEP_DURATION_SHORT + 50);
		if (! is_diagonal) brivoi.turnLED(OFF); // input status
		else brivoi.blinkLED(3, true); // diagonal!
	}
	// LONG key presses
	else if (event == EB_KP_EVT_LONGPRESSED)
	{
		switch (key)
		{
		case EB_KP_KEY_FW:
			// PAUSE action
			brivoi.turnLED(ON);
			brivoi.beep(EB_BEEP_FORWARD, BEEP_DURATION_LONG);
			addCommand(EB_CMD_PA);
			break;
		case EB_KP_KEY_TL:
			brivoi.turnLED(ON);
			brivoi.beep(EB_BEEP_TURNLEFT, BEEP_DURATION_LONG);
			addCommand(EB_CMD_TL_ALT);
			break;
		case EB_KP_KEY_GO:
			brivoi.turnLED(ON);
			brivoi.beep(EB_BEEP_DEFAULT, BEEP_DURATION_LONG);
			delay(BEEP_DURATION_LONG * 5);
			brivoi.playRTTTL(RTTTL_MODECHG);
			if (mode == STANDARD) mode = DONTRESET;
			else mode = STANDARD;
			// notify which mode is selected via the number of beeps
			for (uint8_t i = 0; i < mode; i ++)
			{
				delay(BEEP_DURATION_SHORT + 100);
				brivoi.beep(EB_BEEP_DEFAULT, BEEP_DURATION_SHORT);
			}
			break;
		case EB_KP_KEY_TR:
			brivoi.turnLED(ON);
			brivoi.beep(EB_BEEP_TURNRIGHT, BEEP_DURATION_LONG);
			addCommand(EB_CMD_TR_ALT);
			break;
		case EB_KP_KEY_BW:
			// RESET action
			brivoi.turnLED(ON);
			brivoi.beep(EB_BEEP_BACKWARD, BEEP_DURATION_LONG);
			// check if there is something to reset
			if
			(
				(program_count < 1) // no program
				&& (! is_diagonal)  // no diagonal angle
			) break; // nothing to do here
			// program reset!!
			delay(BEEP_DURATION_LONG + 50); // necessary to show key color = input feedback
			brivoi.turnLED(OFF);  // RESET = Off
			delay(BEEP_DURATION_LONG * 5);
			brivoi.playRTTTL(RTTTL_PRESET);
			program_count = 0;   // reset program
			program_index = 0;   // reset execution pointer
			is_diagonal = false; // reset diagonal status
			break;
		default:
			return; // unhandled case, avoid any further action
		}
		// go back to "input color" after a moment
		delay(BEEP_DURATION_LONG + 50);
		if (! is_diagonal) brivoi.turnLED(OFF); // input status
		else brivoi.blinkLED(3, true); // diagonal!
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
	switch (brivoi.handleAction(currentTime, program[program_index]))
	{
	case EB_CMD_R_PENDING_ACTION:
		return;  // still pending execution -> exit to the next loop

	case EB_CMD_R_FINISHED_ACTION:  // next command
		program_index ++;  // <- no "break" necessary

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
				brivoi.beep(EB_BEEP_FORWARD, BEEP_DURATION_SHORT);
				if (! is_diagonal) brivoi.prepareAction(EB_CMD_FW, BRIVOI_MOVE_DISTANCE);
				else brivoi.prepareAction(EB_CMD_FW, BRIVOI_DIAGONAL_DISTANCE);
				break;
			case EB_CMD_TL:
				showCmdColor(program[program_index]);
				brivoi.beep(EB_BEEP_TURNLEFT, BEEP_DURATION_SHORT);
				brivoi.prepareAction(EB_CMD_TL, BRIVOI_ROTATE_DEGREES);
				break;
			case EB_CMD_TR:
				showCmdColor(program[program_index]);
				brivoi.beep(EB_BEEP_TURNRIGHT, BEEP_DURATION_SHORT);
				brivoi.prepareAction(EB_CMD_TR, BRIVOI_ROTATE_DEGREES);
				break;
			case EB_CMD_BW:
				showCmdColor(program[program_index]);
				brivoi.beep(EB_BEEP_BACKWARD, BEEP_DURATION_SHORT);
				if (! is_diagonal) brivoi.prepareAction(EB_CMD_BW, BRIVOI_MOVE_DISTANCE);
				else brivoi.prepareAction(EB_CMD_BW, BRIVOI_DIAGONAL_DISTANCE);
				break;
			case EB_CMD_PA:
				showCmdColor(program[program_index]);
				brivoi.beep(EB_BEEP_BACKWARD, BEEP_DURATION_SHORT);
				brivoi.prepareAction(EB_CMD_BW, BRIVOI_MOVE_DISTANCE); // same time as the movement
				break;
			case EB_CMD_TL_ALT:
				showCmdColor(program[program_index]);
				// Note = C#7, between C (TL) & D (FW)
				brivoi.playTone(2217, BEEP_DURATION_SHORT, false);
				brivoi.prepareAction(EB_CMD_TL_ALT, BRIVOI_ROTATE_DEGREES_ALT); // half degrees
				is_diagonal = ! is_diagonal;
				break;
			case EB_CMD_TR_ALT:
				showCmdColor(program[program_index]);
				// Note = D#7, between D (FW) & E (TR)
				brivoi.playTone(2489, BEEP_DURATION_SHORT, false);
				brivoi.prepareAction(EB_CMD_TR_ALT, BRIVOI_ROTATE_DEGREES_ALT); // half degrees
				is_diagonal = ! is_diagonal;
				break;
			}
		}
		else
		{
			// execution finished
			if (mode == STANDARD)
				program_count = 0;   // reset program
			else
				is_diagonal = false; // reset diagonal status
			program_index = 0;       // reset execution pointer
			brivoi.disableStepperMotors();
			brivoi.playRTTTL(RTTTL_FINISH);
			if (! is_diagonal) brivoi.turnLED(OFF); // input status
			else brivoi.blinkLED(3, true); // diagonal!
			status = PROGRAMMING; // back to user input

			#ifdef DEBUG_MODE
			Serial.println(F("FINISHED"));
			#endif
		}
	} // switch
}  // processProgram()
