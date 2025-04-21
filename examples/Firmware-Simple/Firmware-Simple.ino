/**
 * A simplified FIRMWARE.
 *
 * This is a simplified firmware for the Escornabot, created initially for the
 * Saladino School (especial education) where only basic controls were required.
 * Based on Luci's firmware, with only four main moves: FW-BW (10cms) & TL-TR (90°).
 *
 * Additional info at roboteach.es/escornabot (spanish/galician)
 *
 * @file      Firmware-Simple.ino
 * @author    mgesteiro einsua
 * @date      20250420
 * @version   1.2.3-Simple
 * @copyright OpenSource, LICENSE GPLv3
 * @notes     Stripped down from Luci's v1.2.3
 */

#define FIRMWARE_VERSION "1.2.3-Simple"
// change this to true if your stepper motors work backwards.
#define STEPPERMOTOR_FIXED_REVERSED false

#include <Arduino.h>
#include <Escornabot-lib.h>

const float ROBOT_MOVE_DISTANCE      = 10.0;  // cms
// NOTE: if you change the following values, you should also update the logic in processProgram()
const float ROBOT_ROTATE_DEGREES     = 90.0;  // degrees

// Luci color = Purple (~ darkish magenta)
#define ROBOT_COLOR_R BRIGHTNESS_LEVEL * 0.4
#define ROBOT_COLOR_G 0
#define ROBOT_COLOR_B BRIGHTNESS_LEVEL

#define BEEP_DURATION_SHORT 100  // ms
#define RTTTL_STARTUP ":d=16,o=6,b=140:c,p,e,p,g,"
#define RTTTL_FINISH  ":d=16,o=6,b=800:f,4p,f,4p,f,4p,f,4p,c,4p,c,4p,c,4p,c,"

#define PROGRAMMING 0
#define EXECUTING   1
uint8_t status = PROGRAMMING;

EB_T_COMMANDS program[128];  // list of actions saved
uint8_t program_count = 0;   // number of commands in the program
uint8_t program_index = 0;   // current command

Escornabot robot;
uint32_t currentTime;

/*
 *   S E T U P   &   L O O P   F U N C T I O N S
 */

void setup()
{
	// setup and start the Escornabot
	robot.init(A4, 10, 12, EB_TYPE_BRIVOI); // Brivoi's Keypad, Buzzer, Neopixel & Wiring
	// banner
	Serial.print(F("Simple's FIRMWARE v"));
	Serial.println(F(FIRMWARE_VERSION));
	// show keypad values
	int16_t* kv = robot.getKeypadValues();
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
	robot.fixReversed();
	#endif
	// uncomment one of the following line to disable different timeouts
	robot.setStandbyTimeouts(0, INACTIVITY_TIMEOUT); // disable powerbank timeout
	//robot.setStandbyTimeouts(POWERBANK_TIMEOUT, 0);  // disable inactivity timeout
	//robot.setStandbyTimeouts(0, 0); // disable both timeouts
}  // setup()

void loop()
{
	// get time
	currentTime = millis();

	// watch standby
	robot.handleStandby(currentTime);

	// watch keypad
	uint8_t kp_code = robot.handleKeypad(currentTime);

	// watch serial/bluetooth
	uint8_t bt_code = robot.handleSerial();

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
	robot.showKeyColor(EB_KP_KEY_FW); // blue
	delay(tshow);
	robot.showKeyColor(EB_KP_KEY_BW); // yellow
	delay(tshow);
	robot.showKeyColor(EB_KP_KEY_TL); // red
	delay(tshow);
	robot.showKeyColor(EB_KP_KEY_TR); // green
	delay(tshow);
	robot.showKeyColor(EB_KP_KEY_GO); // white
	delay(tshow);
	// finish with Luci color
	robot.showColor(ROBOT_COLOR_R, ROBOT_COLOR_G, ROBOT_COLOR_B); // input color, purple
	// startup tune
	robot.playRTTTL(RTTTL_STARTUP);
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
		robot.showColor(0, 0, 0);  // off
		break;
	case EB_CMD_FW:  // FW
		robot.showColor(0, 0, BRIGHTNESS_LEVEL);  // blue
		break;
	case EB_CMD_TL:  // TL
		robot.showColor(BRIGHTNESS_LEVEL, 0, 0);  // red
		break;
	case EB_CMD_TR:  // TR
		robot.showColor(0, BRIGHTNESS_LEVEL, 0);  // green
		break;
	case EB_CMD_BW:  // BW
		robot.showColor(BRIGHTNESS_LEVEL, BRIGHTNESS_LEVEL, 0);  // yellow
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
}  // addCommand()

/**
 * Stops current execution and clears everything.
 */
void stop(uint32_t currentTime)
{
	// shutdown execution
	robot.stopAction(currentTime);
	robot.disableStepperMotors();
	robot.clearKeypad(currentTime);
	robot.beep(EB_BEEP_DEFAULT, BEEP_DURATION_SHORT);
	program_count = 0;  // reset program
	program_index = 0;  // reset execution pointer
	robot.showColor(ROBOT_COLOR_R, ROBOT_COLOR_G, ROBOT_COLOR_B); // input color, purple
	status = PROGRAMMING;  // back to user input
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
			robot.showKeyColor(key);
			robot.beep(EB_BEEP_FORWARD, BEEP_DURATION_SHORT);
			addCommand(EB_CMD_FW);
			break;
		case EB_KP_KEY_TL:
			robot.showKeyColor(key);
			robot.beep(EB_BEEP_TURNLEFT, BEEP_DURATION_SHORT);
			addCommand(EB_CMD_TL);
			break;
		case EB_KP_KEY_GO:
			robot.showKeyColor(key);
			robot.beep(EB_BEEP_DEFAULT, BEEP_DURATION_SHORT);
			if (program_count < 1) break;
			status = EXECUTING;
			break;
		case EB_KP_KEY_TR:
			robot.showKeyColor(key);
			robot.beep(EB_BEEP_TURNRIGHT, BEEP_DURATION_SHORT);
			addCommand(EB_CMD_TR);
			break;
		case EB_KP_KEY_BW:
			robot.showKeyColor(key);
			robot.beep(EB_BEEP_BACKWARD, BEEP_DURATION_SHORT);
			addCommand(EB_CMD_BW);
			break;
		default:
			// this case should not be possible
			return;
		}
		// go back to "input color" after a moment
		delay(BEEP_DURATION_SHORT + 50);
		robot.showColor(ROBOT_COLOR_R, ROBOT_COLOR_G, ROBOT_COLOR_B); // input color, purple
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
	switch (robot.handleAction(currentTime, program[program_index]))
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
				robot.beep(EB_BEEP_FORWARD, BEEP_DURATION_SHORT);
				robot.prepareAction(EB_CMD_FW, ROBOT_MOVE_DISTANCE);
				break;
			case EB_CMD_TL:
				showCmdColor(program[program_index]);
				robot.beep(EB_BEEP_TURNLEFT, BEEP_DURATION_SHORT);
				robot.prepareAction(EB_CMD_TL, ROBOT_ROTATE_DEGREES);
				break;
			case EB_CMD_TR:
				showCmdColor(program[program_index]);
				robot.beep(EB_BEEP_TURNRIGHT, BEEP_DURATION_SHORT);
				robot.prepareAction(EB_CMD_TR, ROBOT_ROTATE_DEGREES);
				break;
			case EB_CMD_BW:
				showCmdColor(program[program_index]);
				robot.beep(EB_BEEP_BACKWARD, BEEP_DURATION_SHORT);
				robot.prepareAction(EB_CMD_BW, ROBOT_MOVE_DISTANCE);
			}
		}
		else
		{
			// execution finished
			program_count = 0;  // reset program
			program_index = 0;  // reset execution pointer
			robot.disableStepperMotors();
			robot.playRTTTL(RTTTL_FINISH);
			robot.showColor(ROBOT_COLOR_R, ROBOT_COLOR_G, ROBOT_COLOR_B); // input color, purple
			status = PROGRAMMING; // back to user input
		}
	} // switch
}  // processProgram()
