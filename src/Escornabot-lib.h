/**
 * Escornabot-lib is a library for the Escornabot ROBOT.
 * 
 * A library with all the core functions and data to program an Escornabot
 * ROBOT. More info about the project at roboteach.es/escornabot and
 * escornabot.org.
 *
 * @file      Escornabot-lib.h
 * @author    mgesteiro
 * @date      20230101
 * @version   1.0.0
 * @copyright OpenSource, LICENSE GPLv3
 */

#ifndef ESCORNABOT_LIB_H
#define ESCORNABOT_LIB_H

#define EB_VERSION "0.2.2-beta"

#include <Arduino.h>
#include <stdint.h>
#include <avr/eeprom.h>
#include "lib/NeoPixel.h"
#include "Config.h"



//
// STEPPER MOTORS                    //
//
#define WHEEL_CIRCUMFERENCE float(PI * WHEEL_DIAMETER)
#define ROTATION_CIRCUMFERENCE float(PI * WHEEL_DISTANCE) // rotation circunference determined by the wheel separation
#define STEPPERS_STEPS_MM float(STEPPERMOTOR_FULLREVOLUTION_STEPS / WHEEL_CIRCUMFERENCE) // how many steps to move 1 mm
#define STEPPERS_STEPS_DEG float((ROTATION_CIRCUMFERENCE/360) * STEPPERS_STEPS_MM) // how many steps to rotate the robot 1 degree

const uint8_t EB_SM_DRIVING_SEQUENCE[] = {B0011, B0110, B1100, B1001};  // full drive - stronger
// const uint8_t EB_SM_DRIVING_SEQUENCE[] = {B0001, B0010, B0100, B1000};  // wave drive - less consume
#define EB_SM_DRIVING_SEQUENCE_MAX sizeof(EB_SM_DRIVING_SEQUENCE) - 1



//
// BUZZER                            //
//
typedef enum
{
	EB_BEEP_DEFAULT   = 4699,  // D8
	EB_BEEP_FORWARD   = 2637,  // E7
	EB_BEEP_TURNLEFT  = 2217,  // C#7
	EB_BEEP_TURNRIGHT = 4434,  // C#8
	EB_BEEP_BACKWARD  = 3520   // A7
} EB_T_BEEPS;



//
// NEOPIXEL                          //
//
#define EB_LUCI_COLOR 10



//
// KEYPAD                            //
//

/**
 * Definition of all the possible KEYs in an Escornabot keypad.
 */
typedef enum
{
	EB_KP_KEY_NN = 0,
	EB_KP_KEY_FW = 1,
	EB_KP_KEY_TL = 2,
	EB_KP_KEY_GO = 3,
	EB_KP_KEY_TR = 4,
	EB_KP_KEY_BW = 5
} EB_T_KP_KEYS;
#define EB_T_KP_KEYS_SIZE 6
const String EB_KP_KEYS_LABELS[] = {"NONE", "FORWARD", "TURN LEFT", "GO", "TURN RIGHT", "BACKWARD"};

/**
 * Definition of all the possible EVENTs handling an Escornabot keypad.
 */
typedef enum
{
	EB_KP_EVT_NONE         = 0,
	EB_KP_EVT_PRESSED      = 1,
	EB_KP_EVT_RELEASED     = 2,
	EB_KP_EVT_LONGPRESSED  = 3,
	EB_KP_EVT_LONGRELEASED = 4
} EB_T_KP_EVENTS;

// for code clarity
#define OFF      0
#define ON       1
#define STALLED  2
#define CURRENT  0
#define PREVIOUS 1
#define SAVED    1

// auto keypad configuration
#define EB_KP_PULLUP_MARGIN 140
// Index to the last 5 uint16_t EEPROM positions;
// E2END = The last EEPROM address (bytes). 1023 for Arduino Nano 328
#define EB_KP_EEPROM_VALUES_INDEX (uint16_t *)(E2END - 2 * 5 + 1)



//
// COMMANDS                          //
//

/**
 * Definition of all the possible COMMANDs that an Escornabot can process.
 */
typedef enum
{
	EB_CMD_NN     = 0,
	EB_CMD_FW     = 1,
	EB_CMD_TL     = 2,
	EB_CMD_TR     = 3,
	EB_CMD_BW     = 4,
	EB_CMD_PA     = 5,
	EB_CMD_TL_ALT = 6,
	EB_CMD_TR_ALT = 7
} EB_T_COMMANDS;

const String EB_CMD_LABELS[] = 
{
	"NONE",
	"MOVE FORWARD",
	"TURN LEFT",
	"TURN RIGHT",
	"MOVE BACKWARD",
	"PAUSE",
	"TURN LEFT ALT",
	"TURN RIGHT ALT"
};

#define EB_CMD_R_NOTHING_TO_DO   0
#define EB_CMD_R_PENDING_ACTION  1
#define EB_CMD_R_FINISHED_ACTION 2


/**
 * Main class with the core functions and data to program an Escornabot ROBOT.
 */
class Escornabot {
public:
	// constructor and destructor
	Escornabot();
	virtual ~Escornabot();
	void init();

	// Stepper motors
	void move(float cms);
	void turn(float degrees);
	void disableSM();

	// Buzzer
	void beep(EB_T_BEEPS frequency, uint16_t duration);
	void playNote(uint16_t frequency, uint16_t duration);
	void playRTTTL(const char* tune);

	// LED
	void turnLED(uint8_t state);
	void blinkLED(uint8_t times);

	// NeoPixel
	void showColor(uint8_t R, uint8_t G, uint8_t B);
	void showKeyColor(uint8_t key);

	// Keypad
	void autoConfigKeypad();
	void configKeypad(
		uint8_t KeypadPin,
		int16_t KeypadValue_NN,
		int16_t KeypadValue_FW,
		int16_t KeypadValue_TL,
		int16_t KeypadValue_GO,
		int16_t KeypadValue_TR,
		int16_t KeypadValue_BW);
	EB_T_KP_KEYS getPressedKey();
	uint8_t handleKeypad(uint32_t currentTime);
	void clearKeypad(uint32_t currentTime);
	bool isButtonPressed(String button);
	int16_t rawKeypad();

	// Serial / Blueetooth
	uint8_t handleSerial();

	// Commands
	void prepareAction(EB_T_COMMANDS command, float value);
	uint8_t handleAction(uint32_t currentTime, EB_T_COMMANDS command);
	void stopAction(uint32_t currentTime);

	// Extra
	void fixReversed();
	void handleStandby(uint32_t currentTime);
	void debug();

private:
	// Stepper motors
	void _initCoilsPins();
	void _setCoils(uint8_t stateR, uint8_t stateL);

	// Neopixel
	NeoPixel *_neopixel;
	void _initNeoPixel(int pin);

	// Keypad
	uint8_t _keypad_pin;                        // pin in use
	int16_t _keypad_values[EB_T_KP_KEYS_SIZE];  // analog reading for each key
	uint8_t _keypad_key[2];                     // current and saved key in process
	uint8_t _keypad_key_state[2];               // current and previous
	uint32_t _keypad_pressed_time[2];           // current and previous
	uint32_t _keypad_previousTime;              // previous time

	// Command execution
	uint16_t _exec_steps;   // # steps for the current action
	uint16_t _exec_wait;    // delay between steps, microseconds
	uint16_t _exec_ap;      // acceleration point: #steps up to accelerate
	uint16_t _exec_dp;      // deceleration point: #steps to start deceleration
	uint8_t  _exec_drinit;  // initial driving sequence index
	int8_t   _exec_drinc;   // driving sequence index groth sign
	uint16_t _exec_drindex; // driving sequence index
	uint32_t _exec_ptime;   // previous execution time

	// Extra
	bool _isReversed = false;
	uint32_t _powerbank_previousTime = 0;
	uint32_t _standby_previousTime = 0;

};

#endif  //library