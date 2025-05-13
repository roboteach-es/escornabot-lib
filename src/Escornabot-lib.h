/**
 * Escornabot-lib is a library for the Escornabot ROBOT.
 *
 * A library with all the core functions and data to program an Escornabot
 * ROBOT. More info about the project at roboteach.es/escornabot and
 * escornabot.org.
 *
 * @file      Escornabot-lib.h
 * @author    mgesteiro einsua
 * @date      20250513
 * @version   1.4.1
 * @copyright OpenSource, LICENSE GPLv3
 */

#ifndef ESCORNABOT_LIB_H
#define ESCORNABOT_LIB_H

#define EB_VERSION "1.4.1"

#include <Arduino.h>
#include <stdint.h>
#include <avr/eeprom.h>
#include "lib/NeoPixel.h"
#include "Config.h"



//
// STEPPER MOTORS                    //
//
/**
 * Definition of all the supported wiring schemes for the stepper motors.
 */
typedef enum: uint8_t
{
	EB_TYPE_LUCI   = 0,
	EB_TYPE_BRIVOI = 1
} EB_T_WIRINGTYPES;
#define EB_SM_DRIVING_SEQUENCE_MAX sizeof(EB_SM_DRIVING_SEQUENCE) - 1



//
// BUZZER                            //
//
/**
 * Definition of all the possible BEEPs in an Escornabot.
 */
typedef enum: uint8_t
{
	EB_BEEP_DEFAULT   = 0,
	EB_BEEP_FORWARD   = 1,
	EB_BEEP_TURNLEFT  = 2,
	EB_BEEP_TURNRIGHT = 3,
	EB_BEEP_BACKWARD  = 4
} EB_T_BEEPS;
const uint16_t EB_BEEP_FREQUENCIES[] =
{
	3135,  // EB_BEEP_DEFAULT   = G7 - Sol
	2349,  // EB_BEEP_FORWARD   = D7 - Re
	2093,  // EB_BEEP_TURNLEFT  = C7 - Do
	2637,  // EB_BEEP_TURNRIGHT = E7 - Mi
	2793   // EB_BEEP_BACKWARD  = F7 - Fa
};



//
// NEOPIXEL                          //
//



//
// KEYPAD                            //
//
/**
 * Definition of all the possible KEYs in an Escornabot keypad.
 */
typedef enum: uint8_t
{
	EB_KP_KEY_NN = 0,  // none
	EB_KP_KEY_FW = 1,  // forward - blue
	EB_KP_KEY_TL = 2,  // turn left - red
	EB_KP_KEY_GO = 3,  // go - white
	EB_KP_KEY_TR = 4,  // turn right - green
	EB_KP_KEY_BW = 5   // backward - yellow
} EB_T_KP_KEYS;
#define EB_T_KP_KEYS_SIZE 6
const String EB_KP_KEYS_LABELS[] =
{
	"NONE",
	"FORWARD",
	"TURN LEFT",
	"GO",
	"TURN RIGHT",
	"BACKWARD"
};

/**
 * Definition of all the possible EVENTs handling an Escornabot keypad.
 */
typedef enum: uint8_t
{
	EB_KP_EVT_NONE         = 0,
	EB_KP_EVT_PRESSED      = 1,
	EB_KP_EVT_RELEASED     = 2,
	EB_KP_EVT_LONGPRESSED  = 3,
	EB_KP_EVT_LONGRELEASED = 4
} EB_T_KP_EVENTS;

// for code readability
#define OFF      0
#define ON       1
#define STALLED  2
#define CURRENT  0
#define PREVIOUS 1
#define SAVED    1

// auto keypad configuration
#define EB_KP_PULLUP_MARGIN 50
// Index to the last 5 uint16_t EEPROM positions;
// E2END = The last EEPROM address (bytes). 1023 for Arduino Nano 328
#define EB_KP_EEPROM_VALUES_INDEX (uint16_t *)(E2END - 2 * 5 + 1)



//
// COMMANDS                          //
//
/**
 * Definition of all the possible COMMANDs that an Escornabot can process.
 */
typedef enum: uint8_t
{
	EB_CMD_NN     = 0,  // none
	EB_CMD_FW     = 1,  // move forward
	EB_CMD_TL     = 2,  // turn left
	EB_CMD_TR     = 3,  // turn right
	EB_CMD_BW     = 4,  // move backward
	EB_CMD_PA     = 5,  // pause
	EB_CMD_TL_ALT = 6,  // turn left alternative
	EB_CMD_TR_ALT = 7   // turn right alternative
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

// Return codes for the command handling routine
#define EB_CMD_R_NOTHING_TO_DO   0
#define EB_CMD_R_PENDING_ACTION  1
#define EB_CMD_R_FINISHED_ACTION 2


/**
 * Main class with the core functions and data to program an Escornabot ROBOT.
 */
class Escornabot
{
public:
	// constructor and destructor
	Escornabot();
	virtual ~Escornabot();
	void init(
		uint8_t keypadPin   = KEYPAD_PIN,    // config.h default
		uint8_t buzzerPin   = BUZZER_PIN,    // config.h default
		uint8_t neopixelPin = NEOPIXEL_PIN,  // config.h default
		EB_T_WIRINGTYPES wiringType = EB_TYPE_LUCI  // Luci default
	);

	// Stepper motors
	void move(float cms);
	void turn(float degrees);
	void disableStepperMotors();
	void setStepsPerMilimiter(float steps);
	void setStepsPerDegree(float steps);

	// Buzzer
	void beep(EB_T_BEEPS beepId, uint16_t duration);
	void playTone(uint16_t frequency, uint16_t duration, bool blocking);
	void playRTTTL(const char* tune);

	// LED
	void turnLED(uint8_t state);
	void blinkLED(uint8_t times, bool reversed = false);

	// NeoPixel
	void showColor(uint8_t R, uint8_t G, uint8_t B);
	void showKeyColor(EB_T_KP_KEYS key);

	// Keypad
	void autoConfigKeypad(uint8_t keypadPin);
	void configKeypad(
		uint8_t keypadPin,
		int16_t keypadValue_NN,
		int16_t keypadValue_FW,
		int16_t keypadValue_TL,
		int16_t keypadValue_GO,
		int16_t keypadValue_TR,
		int16_t keypadValue_BW);
	EB_T_KP_KEYS getPressedKey();
	uint8_t handleKeypad(uint32_t currentTime);
	void clearKeypad(uint32_t currentTime);
	bool isButtonPressed(String button);
	int16_t rawKeypad();
	int16_t* getKeypadValues();

	// Serial / Blueetooth
	uint8_t handleSerial();

	// Commands
	void prepareAction(EB_T_COMMANDS command, float value);
	uint8_t handleAction(uint32_t currentTime, EB_T_COMMANDS command);
	void stopAction(uint32_t currentTime);

	// Stand-by
	void handleStandby(uint32_t currentTime);
	void setStandbyTimeouts(uint32_t powerBank, uint32_t inactivity);

	// Extra
	void fixReversed();
	void debug();

private:
	// Stepper motors
	void _initCoilsPins_Luci();
	void _initCoilsPins_Brivoi();
	void _setCoils_Luci(uint8_t stateR, uint8_t stateL);
	void _setCoils_Brivoi(uint8_t stateR, uint8_t stateL);
	void _setSteppersWiring(EB_T_WIRINGTYPES type);

	// pointer functions for base actions with the coils
	// configured during init() with the _setSteppersWiring() method
	void (Escornabot::*_initCoilsPins)();
	void (Escornabot::*_setCoils)(uint8_t stateR, uint8_t stateL);

	float _steppers_steps_mm = STEPPERS_STEPS_MM;   // default from Config.h
	float _steppers_steps_deg = STEPPERS_STEPS_DEG; // default from Config.h

	// Buzzer
	uint8_t _buzzer_pin; // pin in use

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
	uint32_t _exec_steps;   // # steps for the current action
	uint32_t _exec_wait;    // delay between steps, microseconds
	uint32_t _exec_ap;      // acceleration point: #steps until stop accelerating
	uint32_t _exec_dp;      // deceleration point: #steps to start deceleration
	uint8_t  _exec_drinit;  // initial driving sequence index
	int8_t   _exec_drinc;   // driving sequence index growth sign
	uint32_t _exec_drindex; // driving sequence index
	uint32_t _exec_ptime;   // previous execution time

	// Stand-by
	uint32_t _powerbank_timeout       = POWERBANK_TIMEOUT;
	uint32_t _powerbank_previousTime  = 0;
	uint32_t _inactivity_timeout      = INACTIVITY_TIMEOUT;
	uint32_t _inactivity_previousTime = 0;

	// Extra
	bool _isReversed = false;

};

#endif  //library
