/**
 * Escornabot-lib is a library for the Escornabot ROBOT.
 * 
 * A library with all the core functions and data to program an Escornabot
 * ROBOT. More info about the project at roboteach.es/escornabot and
 * escornabot.org.
 *
 * @file      Escornabot-lib.cpp
 * @author    mgesteiro einsua
 * @date      20250101
 * @version   1.1.0
 * @copyright OpenSource, LICENSE GPLv3
 */

#include <Arduino.h>
#include "Escornabot-lib.h"



////////////////////////////////////////
//
// Initialization
//
////////////////////////////////////////

/**
 * Constructor
 */
Escornabot::Escornabot()
{
	// moved to init() because of
	// https://forum.arduino.cc/t/analogread-seems-not-working-into-a-class-constructor/109081/8
}

/**
 * Destructor
 */
Escornabot::~Escornabot()
{
}

/**
 * Initialization function to be called in Arduino's setup().
 *
 * This function initializes the pins for the stepper motors, the buzzer, and
 * the onboard LED. It also initializes the NeoPixel and the Serial port (if
 * a Bluetooth dongle or any other device is used to communicate). Finally, the
 * auto configuration routine for the keypad is invoqued just before
 * configuring the keypad with the stored values (in the EEPROM), or the
 * default ones in case they are invalid.
 *
 * @see an init function is required due to Arduino's architecture and constructors should be avoided:
 *      https://forum.arduino.cc/t/analogread-seems-not-working-into-a-class-constructor/109081/8
 */
void Escornabot::init()
{
	// Stepper motors
	_initCoilsPins();
	// Buzzer
	pinMode(BUZZER_PIN, OUTPUT);
	// On-board LED
	pinMode(SIMPLELED_PIN, OUTPUT);
	// NeoPixel
	_initNeoPixel(NEOPIXEL_PIN);
	// Serial / Bluetooth
	Serial.begin(EB_BAUDRATE);
	// Keypad autoconfig: give a chance
	autoConfigKeypad();
	// Keypad with EEPROM values or default
	uint16_t *eeprom_index = EB_KP_EEPROM_VALUES_INDEX;
	uint16_t eeprom_values[5];
	for (uint8_t i = 0; i < 5; i ++)
	{
		eeprom_values[i] = eeprom_read_word(eeprom_index);
		eeprom_index++;
	}
	configKeypad(
		KEYPAD_PIN,
		EB_KP_VALUE_NN,
		eeprom_values[0], // FW
		eeprom_values[1], // TL
		eeprom_values[2], // GO
		eeprom_values[3], // TR
		eeprom_values[4]  // BW
	);
	// cleaning
	clearKeypad(0);
}  // init()



////////////////////////////////////////
//
// Stepper motors
//
////////////////////////////////////////

/**
 * Move the robot forward or backward.
 *
 * @param cms  number of centimenters to move. If positive, the robot
 *             moves forward, if negative, backward.
 * 
 * @note This method is blocking: it only returns after finishing the move.
 */
void Escornabot::move(float cms)
{
	// prepare action
	EB_T_COMMANDS command = EB_CMD_FW;
	if (cms < 0) command = EB_CMD_BW;
	prepareAction(command, cms);

	// execute action
	while (handleAction(millis(), command) != EB_CMD_R_FINISHED_ACTION);

	// finish
	disableStepperMotors();

}  // move()

/**
 * Turn the robot to the left or the right rotating over its central axis.
 *
 * @param degrees  number of degrees to rotate. If positive, the robot
 *                 turns to the right, if negative, to the left.
 * @note This method is blocking: it only returns after finishing the turn.
 */
void Escornabot::turn(float degrees)
{
	// prepare action
	EB_T_COMMANDS command = EB_CMD_TR;
	if (degrees < 0) command = EB_CMD_TL;
	prepareAction(command, degrees);

	// execute action
	while (handleAction(millis(), command) != EB_CMD_R_FINISHED_ACTION);

	// finish
	disableStepperMotors();

}  // turn()

/**
 * Disable the stepper motors (switching off the coils).
 */
void Escornabot::disableStepperMotors()
{
	_setCoils(0, 0);
}  // disableStepperMotors()

/**
 * Initializes the output pins for the stepper motor coils.
 */
void Escornabot::_initCoilsPins()
{
	// PORTB maps to Arduino digital pins 8 to 13. The two high bits (6 & 7) map to the crystal pins and are not usable.
	DDRB = DDRB | B00001111;  // pins x,x,x,x,11,10,9,8 as OUTPUT - Right motor
	// PORTD maps to Arduino digital pins 0 to 7. Pins 0 and 1 are TX and RX, manipulate with care.
	DDRD = DDRD | B11110000;  // pins 7,6,5,4,x,x,x,x as OUTPUT - Left motor
}  // _initCoilsPins()

/**
 * Sets the stepper motor coils.
 * 
 * This function uses low level PORTx access to set all the coils at the same
 * time and to be fast. That's also why the stepper motor pins are fixed now
 * and not configurable anymore.
 *
 * @param stateR  coils on/off pattern to be applied to the right stepper motor.
 *                Only the lower nibble is used (4 coils -> 4 bits), the rest is ignored.
 * @param stateL  coils on/off pattern to be applied to the left stepper motor.
 *                Only the lower nibble is used (4 coils -> 4 bits), the rest is ignored.
 */
void Escornabot::_setCoils(uint8_t stateR, uint8_t stateL)
{
	// PORTB maps to Arduino digital pins 8 to 13 The two high bits (6 & 7) map to the crystal pins and are not usable
	// RightMotor - pins 11,10,9,8 -> PORTB bits[3-0] = stateR bits[3-0]
	// (a & ~mask) | (b & mask)
	// a -> PORTB  b->stateR  mask->B00001111
	PORTB = (PORTB & B11110000) | (stateR & B00001111);

	// PORTD maps to Arduino digital pins 0 to 7
	// LeftMotor - pins 7,6,5,4 -> PORTD bits[7-4] = stateL bits[3-0]
	// (a & ~mask) | (b & mask)
	// a -> PORTD  b->(stateL << 4)  mask->B11110000
	PORTD = (PORTD & B00001111) | (stateL << 4); // implicit mask in second part
}  // _setCoils()



////////////////////////////////////////
//
// Buzzer
//
////////////////////////////////////////

/**
 * Plays the specified BEEP for the indicated duration.
 *
 * @param beepid     Which beep to play
 * @param duration   Duration, in milliseconds, of the sound
 *
 * @note this function is non-blocking and returns inmediatly.
 */
void Escornabot::beep(EB_T_BEEPS beepid, uint16_t duration)
{
	tone(BUZZER_PIN, EB_BEEP_FREQUENCIES[beepid], duration);
}  // beep()

/**
 * Plays the frequency provided during the indicated time and may wait until finished.
 *
 * @param frequency  Which frequency in Hz to play
 * @param duration   Duration, in milliseconds, of the sound
 * @param blocking   Indicate if the method is blocking or returns immediately
 */
void Escornabot::playTone(uint16_t frequency, uint16_t duration, bool blocking)
{
	tone(BUZZER_PIN, frequency, duration);
	if (blocking) delay(duration); // wait for it
}  // playTone()

const uint16_t EB_NOTES_FREQUENCIES[] = // 12 notes, 4 octaves (4 to 7)
{
	// C,   C#,    D,   D#,    E,    F,   F#,    G,   G#,    A,   A#,    B,
	 262,  277,  294,  311,  330,  349,  370,  392,  415,  440,  466,  494,
	 523,  554,  587,  622,  659,  698,  740,  784,  831,  880,  932,  987,
	1046, 1108, 1174, 1244, 1318, 1396, 1479, 1567, 1661, 1760, 1864, 1975,
	2093, 2217, 2349, 2489, 2637, 2793, 2959, 3135, 3322, 3520, 3729, 3951
};
/**
 * Plays an RTTTL tune.
 *
 * @param tune  A string with the tune in RTTTL format
 *
 * @note More info about the RTTTL format here: https://github.com/ArminJo/PlayRtttl/#rtttl-format
 */
void Escornabot::playRTTTL(const char* tune)
{
	// song name - discarded
	while (*tune && *tune != ':') tune++;
	tune++;

	// default tune parameters
	uint8_t default_octave = 5;
	uint16_t default_duration= 16, bpm = 320;
	while (*tune && (*tune != ':'))
	{
		switch (*tune)
		{
		case 'd': // note duration
			tune += 2; // skip 'd='
			default_duration = atoi(tune);
			while (*tune >= '0' && *tune <= '9') tune++; // discard used numbers
			break;

		case 'o': // octave
			tune += 2; // skip 'o='
			default_octave = atoi(tune);
			while (*tune >= '0' && *tune <= '9') tune++; // discard used numbers
			break;

		case 'b': // beats per minute
			tune += 2; // skip 'b='
			bpm = atoi(tune);
			while (*tune >= '0' && *tune <= '9') tune++; // discard used numbers
			break;

		default:
			tune++; // discard invalid character
		}
	}
	tune++; // discard ':'

	// list of notes
	uint16_t duration = default_duration;
	int8_t note = -1;
	uint8_t octave = default_octave;
	while (*tune)
	{
		if (*tune >= '0' && *tune <= '9')
		{
			// consume numbers
			if (note < 0) duration = atoi(tune);
			else octave = atoi(tune);
			while (*tune >= '0' && *tune <= '9') tune++; // discard used numbers
		}
		else
		{
			// consume notes
			switch (*tune)
			{
				// C C# D D# E F F# G G# A A# B <-- octave
				case 'p': note = 0; break;
				case 'c': note = 1; break;
				case 'd': note = 3; break;
				case 'e': note = 5; break;
				case 'f': note = 6; break;
				case 'g': note = 8; break;
				case 'a': note = 10; break;
				case 'b': note = 12; break;
				case '#': note++; break;
				case ',': case '\0':
					if (note != -1 && octave >= 4 && octave <= 8) {
						if (note > 0)
							tone(BUZZER_PIN, EB_NOTES_FREQUENCIES[((octave - 4) * 12) + note - 1]);
						delay(1000 * 60 / bpm / duration * 4); // BPM usually expresses the number of quarter notes per minute
						// see https://github.com/ArminJo/PlayRtttl/blob/master/src/PlayRtttl.hpp#L192
						noTone(BUZZER_PIN);
					}

					duration = default_duration;
					note = -1;
					octave = default_octave;
					break;
			}
			tune++; // next
		}
	}
}  // playRTTTL()



////////////////////////////////////////
//
// LED
//
////////////////////////////////////////

/**
 * Turns on or off the Escornabot LED.
 * 
 * @param state  Should be HIGH (on) or LOW (off)
 */
void Escornabot::turnLED(uint8_t state)
{
	digitalWrite(SIMPLELED_PIN, state);
}  // turnLED()

/**
 * Blinks the Escornabot LED a number o times.
 * 
 * @param times  Number of times to blink.
 */
void Escornabot::blinkLED(uint8_t times)
{
	while (times > 0)
	{
		digitalWrite(SIMPLELED_PIN, HIGH);
		delay(200);
		digitalWrite(SIMPLELED_PIN, LOW);
		delay(200);
		times--;
	}
}  // blinkLED()



////////////////////////////////////////
//
// NeoPixel
//
////////////////////////////////////////

/**
 * Shows the provided color in the Escornabot NeoPixel.
 * 
 * @param R  (0-255) Ammount of red
 * @param G  (0-255) Ammount of green
 * @param B  (0-255) Ammount of blue
 */
void Escornabot::showColor(uint8_t R, uint8_t G, uint8_t B)
{
	_neopixel->setPixelColor(0, _neopixel->Color(R, G, B));
	_neopixel->show();
}  // showColor()

/**
 * Shows the color associated with the selected key in the Escornabot NeoPixel.
 * 
 * @param key  Which key color to be shown.
 */
void Escornabot::showKeyColor(EB_T_KP_KEYS key)
{
	switch (key)
	{
	case EB_KP_KEY_NN:  // NONE
		showColor(0, 0, 0);  // off
		break;
	case EB_KP_KEY_FW:  // FW
		showColor(0, 0, BRIGHTNESS_LEVEL);  // blue
		break;
	case EB_KP_KEY_TL:  // TL
		showColor(BRIGHTNESS_LEVEL, 0, 0);  // red
		break;
	case EB_KP_KEY_GO:  // GO
		showColor(BRIGHTNESS_LEVEL, BRIGHTNESS_LEVEL, BRIGHTNESS_LEVEL);  // white
		break;
	case EB_KP_KEY_TR:  // TR
		showColor(0, BRIGHTNESS_LEVEL, 0);  // green
		break;
	case EB_KP_KEY_BW:  // BW
		showColor(BRIGHTNESS_LEVEL, BRIGHTNESS_LEVEL, 0);  // yellow
	}
}  // showKeyColor()

/**
 * Escornabot NeoPixel initialization.
 */
void Escornabot::_initNeoPixel(int pin)
{
	// initializes our ONE pixel strip
	_neopixel = new NeoPixel(1, pin, NEO_GRB + NEO_KHZ800);
	_neopixel->begin();
}  // _initNeoPixel()



////////////////////////////////////////
//
// Keypad
//
////////////////////////////////////////

/**
 * If any key is pressed when calling this function, a configuration procedure
 * is started:
 *
 * 1. an alert of four beeps is sounded
 * 2. waits until no key is pressed anymore (if any, after the four beeps)
 * 3. waits for the user to press all the keys in the following order:
 *    FW, TL, GO, TR, BW (from top to bottom, from left to right)
 * 3. updates all five key values in the EEPROM (if different)
 */
void Escornabot::autoConfigKeypad()
{
	// detect key pressed to start
	pinMode(KEYPAD_PIN, INPUT_PULLUP); // if not "pullupable" works as normal INPUT
	bool press_detected = false;
	uint16_t port_read_value = analogRead(KEYPAD_PIN);
	if (
		(port_read_value > EB_KP_PULLUP_MARGIN)
		&& (port_read_value < (1023 - EB_KP_PULLUP_MARGIN))
	)
	{
		// something is pressed
		for (byte i=0; i < 4; i++)
		{
			beep(EB_BEEP_DEFAULT, 100);
			delay(500);
		}
		press_detected = true;
	}
	if (!press_detected) return; // exit

	// wait until no key is pressed anymore
	port_read_value = analogRead(KEYPAD_PIN);
	while (
			(port_read_value > EB_KP_PULLUP_MARGIN)
			&& (port_read_value < 1023 - EB_KP_PULLUP_MARGIN)
		) port_read_value = analogRead(KEYPAD_PIN); // wait for the release

	// read and save keys' values
	uint16_t keypad_values[5];
	for (byte i=0; i < 5; i++)
	{
		// read value
		port_read_value = analogRead(KEYPAD_PIN);
		while (
			(port_read_value < EB_KP_PULLUP_MARGIN)
			|| (port_read_value > 1023 - EB_KP_PULLUP_MARGIN)
		) port_read_value = analogRead(KEYPAD_PIN); // wait for a key press

		// store value for each key
		keypad_values[i] = port_read_value;

		// signal + delay
		beep(EB_BEEP_DEFAULT, 100);
		delay(350);
	} // for 5 keys

	// write/update values in eeprom
	uint16_t *eeprom_index = EB_KP_EEPROM_VALUES_INDEX;
	for (uint8_t i = 0; i < 5; i ++)
	{
		eeprom_update_word(eeprom_index, keypad_values[i]);
		eeprom_index ++;
	}
}  // autoConfigKeypad()

/**
 * Updates the keypad configuration values: analog input pin and
 * analog reading values for every key. If the key values are invalid
 * (0X00 or 0XFFFF), the default Config.h values are used.
 *
 * @param KeypadPin  the analog pin to which the keypad is connected
 * @param Key_NN  analog value when no key is pressed
 * @param Key_FW  analog value for forward key
 * @param Key_TL  analog value for turn left key
 * @param Key_GO  analog value for go key
 * @param Key_TR  analog value for turn right key
 * @param Key_BW  analog value for forward key
 */
void Escornabot::configKeypad(
	uint8_t KeypadPin,
	int16_t Key_NN,
	int16_t Key_FW,
	int16_t Key_TL,
	int16_t Key_GO,
	int16_t Key_TR,
	int16_t Key_BW)
{
	_keypad_pin = KeypadPin;
	pinMode(_keypad_pin, INPUT_PULLUP); // 2-wires, works if it's externally pulled-up too
	if (Key_NN == 0x00 || Key_NN == 0xFFFF) _keypad_values[0] = EB_KP_KEY_NN;  // default Config.h
	else _keypad_values[0] = Key_NN;
	if (Key_FW == 0x00 || Key_FW == 0xFFFF) _keypad_values[1] = EB_KP_KEY_FW;  // default Config.h
	else _keypad_values[1] = Key_FW;
	if (Key_TL == 0x00 || Key_TL == 0xFFFF) _keypad_values[2] = EB_KP_KEY_TL;  // default Config.h
	else _keypad_values[2] = Key_TL;
	if (Key_GO == 0x00 || Key_GO == 0xFFFF) _keypad_values[3] = EB_KP_KEY_GO;  // default Config.h
	else _keypad_values[3] = Key_GO;
	if (Key_TR == 0x00 || Key_TR == 0xFFFF) _keypad_values[4] = EB_KP_KEY_TR;  // default Config.h
	else _keypad_values[4] = Key_TR;
	if (Key_BW == 0x00 || Key_BW == 0xFFFF) _keypad_values[5] = EB_KP_KEY_BW;  // default Config.h
	else _keypad_values[5] = Key_BW;
}  // configKeypad()

/**
 * Scans the keypad port and return the current active key.
 * 
 * This is a low level raw function: no logic is performed.
 *
 * @return the current active (closed) key. It may be none [0].
 */
EB_T_KP_KEYS Escornabot::getPressedKey()
{
	EB_T_KP_KEYS result;
	int16_t value, diff, minor_diff;

	// defaults
	minor_diff = 1023; // MAX analog read
	result = EB_KP_KEY_NN; // <- no key

	// check buttons
	value = rawKeypad();
	for (uint8_t i = EB_KP_KEY_NN; i < EB_T_KP_KEYS_SIZE; i ++)
	{
		diff = abs(_keypad_values[i] - value);
		if (diff < minor_diff) {
			minor_diff = diff;
			result = i;
		}
	}
	if (result) _standby_previousTime = millis(); // avoid standby
	return result;
}  // getPressedKey()

/**
 * Processes the key strokes and converts them, using timing, into useful info.
 * It also performs key press/release debouncing. It has an autolimited
 * resolution of EB_KP_CHECK_MIN_INTERVAL milliseconds (5 by default), that can
 * be changed in the Config.h file.
 * 
 * This is a high level management function, valid for logic control.
 * This function should be called in the loop() as often as possible.
 *
 * @param currentTime  Current time in milliseconds (should be provided).
 *
 * @return  lo nibble -> key [EB_T_KP_KEYS],  hi nibble -> event [EB_T_KP_EVENTS]
 */
uint8_t Escornabot::handleKeypad(uint32_t currentTime)
{
	uint8_t result = 0;
	if ((currentTime - _keypad_previousTime) > EB_KP_CHECK_MIN_INTERVAL)
	{
		/* Debouncing and reading */
		if (
			( _keypad_key_state[CURRENT] == OFF && (currentTime - _keypad_pressed_time[CURRENT]) > EB_KP_DB_TIME) // FDB - Final Debouncing
			||
			(_keypad_key_state[CURRENT] == ON && (currentTime - _keypad_pressed_time[PREVIOUS]) > EB_KP_DB_TIME) // IDB - Initial Debouncing
		   )
		{
			// debouncing window correctly cleared -> let's read
			_keypad_key[CURRENT] = getPressedKey();
			// disallow key change
			if (_keypad_key[CURRENT] && _keypad_key[SAVED] && (_keypad_key[CURRENT] != _keypad_key[SAVED]))
				_keypad_key[CURRENT] = 0; // == NONE
			// store key state
			_keypad_key_state[CURRENT] = _keypad_key[CURRENT] ? ON : OFF; // if a key was detected, it is ON
		}
		/* Check state changes */
		// pressed
		if (_keypad_key_state[PREVIOUS] == OFF && _keypad_key_state[CURRENT] == ON)
		{
			_keypad_key[SAVED] = _keypad_key[CURRENT];
			_keypad_pressed_time[PREVIOUS] = currentTime;
			_keypad_key_state[PREVIOUS] = ON;
			// PRESSED event
			result = EB_KP_EVT_PRESSED << 4 | _keypad_key[SAVED];
		}
		else
		// long pressed
		if (_keypad_key_state[PREVIOUS] == ON && _keypad_key_state[CURRENT] == ON)
		{
			// update pressed time
			_keypad_pressed_time[CURRENT] = currentTime;
			if (_keypad_pressed_time[CURRENT] - _keypad_pressed_time[PREVIOUS] > EB_KP_LP_MIN_DURATION)
			{
				// button was long pressed
				_keypad_key_state[PREVIOUS] = STALLED;
				// LONGPRESSED event
				result = EB_KP_EVT_LONGPRESSED << 4 | _keypad_key[SAVED];
			}
		}
		else
		// released
		if (_keypad_key_state[PREVIOUS] == ON && _keypad_key_state[CURRENT] == OFF)
		{
			// button was released
			_keypad_pressed_time[CURRENT] = currentTime;
			_keypad_key_state[PREVIOUS] = OFF;
			// RELEASED event
			result = EB_KP_EVT_RELEASED << 4 | _keypad_key[SAVED];
			// save key
			_keypad_key[SAVED] = 0; // == NONE == _keypad_key[CURRENT];
		}
		else
		// longreleased
		if (_keypad_key_state[PREVIOUS] == STALLED && _keypad_key_state[CURRENT] == OFF)
		{
			// button was released after a long press
			_keypad_pressed_time[CURRENT] = currentTime;
			_keypad_key_state[PREVIOUS] = OFF;
			// LONGRELEASED event");
			result = EB_KP_EVT_LONGRELEASED << 4 | _keypad_key[SAVED];
			// save key
			_keypad_key[SAVED] = 0; // == NONE == _keypad_key[CURRENT];
		}
		_keypad_previousTime = currentTime;
	}
	return result;
}  // handleKeypad()

/**
 * Clear all internal states of the keypad management process.
 * 
 * @param currentTime  Current time in milliseconds (should be provided).
 */
void Escornabot::clearKeypad(uint32_t currentTime)
{
	memset(_keypad_key, 0, sizeof(_keypad_key));
	memset(_keypad_key_state, 0, sizeof(_keypad_key_state));
	memset(_keypad_pressed_time, 0, sizeof(_keypad_pressed_time));
	_keypad_previousTime = currentTime;
}  // clearKeypad()

/**
 * Checks if the named button is being pressed.
 * 
 * Helper function for mBlock3 extension.
 * 
 * @param label  Name of the button to check.
 * @return true if the button is active, false otherwise.
 */
bool Escornabot::isButtonPressed(String label)
{
	int8_t keyPressed = getPressedKey();
	if (label == EB_KP_KEYS_LABELS[keyPressed]) return true;
	return false;
}  // isButtonPressed()

/**
 * Lowest level reading function of the keypad input pin.
 * 
 * @return Analog reading output of the keypad pin.
 */
int16_t Escornabot::rawKeypad()
{
	return analogRead(_keypad_pin);
}  // rawKeypad()



////////////////////////////////////////
//
// Serial / Blueetooth
//
////////////////////////////////////////

/**
 * Processes data comming through the Serial port and converts it into useful
 * info, with the same format as handleKeypad().
 *
 * This is a high level management function, valid for logic control.
 * This function should be called in the loop() as often as possible.
 *
 * @return  lo nibble -> key [EB_T_KP_KEYS],  hi nibble -> event [EB_T_KP_EVENTS]
 */
uint8_t Escornabot::handleSerial()
{
	char key = Serial.read();
	switch (key)
	{
	case 'n':
		return ((EB_KP_EVT_RELEASED << 4) | EB_KP_KEY_FW);
		break;
	case 'w':
		return ((EB_KP_EVT_RELEASED << 4) | EB_KP_KEY_TL);
		break;
	case 'g':
		return ((EB_KP_EVT_RELEASED << 4) | EB_KP_KEY_GO);
		break;
	case 'e':
		return ((EB_KP_EVT_RELEASED << 4) | EB_KP_KEY_TR);
		break;
	case 's':
		return ((EB_KP_EVT_RELEASED << 4) | EB_KP_KEY_BW);
		break;
	case 'N':
		return ((EB_KP_EVT_LONGPRESSED << 4) | EB_KP_KEY_FW);
		break;
	case 'W':
		return ((EB_KP_EVT_LONGPRESSED << 4) | EB_KP_KEY_TL);
		break;
	case 'G':
		return ((EB_KP_EVT_LONGPRESSED << 4) | EB_KP_KEY_GO);
		break;
	case 'E':
		return ((EB_KP_EVT_LONGPRESSED << 4) | EB_KP_KEY_TR);
		break;
	case 'S':
		return ((EB_KP_EVT_LONGPRESSED << 4) | EB_KP_KEY_BW);
		break;
	default:
		return 0; // just ignore it, even CR & LF
	}
}  // handleSerial()



////////////////////////////////////////
//
// Commands
//
////////////////////////////////////////

/**
 * Sets up the necessary params to be able to execute the command [asynchronously, via handleAction()].
 * This method should be called once, just before invoquing handleAction() in the main loop().
 *
 * @param command  Which command/action is going to be executed
 * @param value  cms or degrees (for PAUSE use cms). You should provide de final value,
 *               no logic/calculation is done in this method.
 */
void Escornabot::prepareAction(EB_T_COMMANDS command, float value)
{
	// fixReversed - stepper motors with swapped cables
	if (_isReversed)
		switch (command)
		{
			case EB_CMD_FW: command = EB_CMD_BW; break;
			case EB_CMD_TL: command = EB_CMD_TR; break;
			case EB_CMD_TR: command = EB_CMD_TL; break;
			case EB_CMD_BW: command = EB_CMD_FW; break;
			case EB_CMD_TL_ALT: command = EB_CMD_TR_ALT; break;
			case EB_CMD_TR_ALT: command = EB_CMD_TL_ALT; break;
		}
	// continue preparation
	value = abs(value);
	switch (command)
	{
	case EB_CMD_FW : // FORWARD
		_exec_steps = value * 10 * STEPPERS_STEPS_MM;  // # steps, implicit trunc()
		_exec_drinit = 0;  // initial driving state index
		_exec_drinc = 1;  // driving index growth direction
		break;
	case EB_CMD_BW : // BACKWARD
		_exec_steps = value * 10 * STEPPERS_STEPS_MM;  // # steps, implicit trunc()
		_exec_drinit = EB_SM_DRIVING_SEQUENCE_MAX;  // initial driving state index
		_exec_drinc = -1;  // driving index growth direction
		break;
	case EB_CMD_TL : // TURN LEFT
	case EB_CMD_TL_ALT : // TURN LEFT ALTERNATE
		_exec_steps = value * STEPPERS_STEPS_DEG;  // # steps, implicit trunc()
		_exec_drinit = EB_SM_DRIVING_SEQUENCE_MAX;  // initial driving state index
		_exec_drinc = -1;  // driving index growth direction
		break;
	case EB_CMD_TR : // TURN RIGHT
	case EB_CMD_TR_ALT : // TURN RIGHT ALTERNATE
		_exec_steps = value * STEPPERS_STEPS_DEG;  // # steps, implicit trunc()
		_exec_drinit = 0;  // initial driving state index
		_exec_drinc = 1;  // driving index growth direction
		break;
	case EB_CMD_PA : // PAUSE <-- same time/steps as advance
		_exec_steps = value * 10 * STEPPERS_STEPS_MM;  // # steps, implicit trunc()
		break;
	default: // should never happen ??
		_exec_steps = 0;
	}
	_exec_wait = 1000000 / STEPPERMOTOR_STEPS_PER_SECOND;  // microseconds, implicit trunc()
	//_exec_drindex = _exec_drinit;  <- commented out: continuous flow, we peek where we left
	_exec_ptime = micros(); // start after window (i.e. we do wait for the step BEFOREHAND)

	// acceleration point, accelerate during initial 40% of steps (limit of 345)
	_exec_ap = _exec_steps * 40 / 100;  // 40% of total steps
	if (_exec_ap > 345) _exec_ap = 345;  // limit to 345
	_exec_ap = _exec_steps - _exec_ap;  // point "up-to", as _exec_steps is decreasing

	// deceleration point, last 27% of steps (limit of 230)
	_exec_dp = _exec_steps * 27 / 100;  // deceleration point: 27% (final stretch)
	if (_exec_dp > 230) _exec_dp = 230;

	#ifdef EB_DEBUG_MODE
	Serial.print("PREPARING ");
	Serial.println(EB_CMD_LABELS[command]);
	Serial.print("Total STEPS: ");
	Serial.println(_exec_steps);
	Serial.print("Acceleration point: ");
	Serial.println(_exec_ap);
	Serial.print("Deceleration point: ");
	Serial.println(_exec_dp);
	#endif
}  // prepareAction()

/**
 * Function responsible for executing actions/movement. This function keeps its own
 * internal state and should be called in the loop() as frequently as possible.
 *
 * @param currentTime  Current time in milliseconds (should be provided).
 * @param command  Which command is being executed (for steppers direction)
 *
 * @return  EB_CMD_R_NOTHING_TO_DO if no pending movement/action
 *          EB_CMD_R_PENDING_ACTION if there is still pending movement/action
 *          EB_CMD_R_FINISHED_ACTION if just has finished the last movement/action
 */
uint8_t Escornabot::handleAction(uint32_t currentTime, EB_T_COMMANDS command)
{
	if (_exec_steps == 0) return 0; // nothing to do

	uint32_t cTime = micros();
	if (cTime - _exec_ptime < _exec_wait) return 1; // still pending steps

	// acceleration <-- update _exec_wait
	if (_exec_steps > _exec_ap) _exec_wait -= 2; // acceleration
	if (_exec_steps < _exec_dp) _exec_wait += 3; // deceleration
	
	// what command?
	switch (command)
	{
	case EB_CMD_FW : // move
	case EB_CMD_BW :
		// motors should turn in the same direction --> sequence inverted for each stepper
		_setCoils(
			EB_SM_DRIVING_SEQUENCE[EB_SM_DRIVING_SEQUENCE_MAX - _exec_drindex],
			EB_SM_DRIVING_SEQUENCE[_exec_drindex]
		);
		_powerbank_previousTime = currentTime; // avoid powerbank refresh
		break;
	case EB_CMD_TL : // turn
	case EB_CMD_TR :
	case EB_CMD_TL_ALT :
	case EB_CMD_TR_ALT :
		// motors should turn in opposite direction --> sequence equal for both steppers
		_setCoils(
			EB_SM_DRIVING_SEQUENCE[_exec_drindex],
			EB_SM_DRIVING_SEQUENCE[_exec_drindex]
		);
		_powerbank_previousTime = currentTime; // avoid powerbank refresh
		break;
	case EB_CMD_PA:
	default :
		; // nothing, just pass the time
	}

	// rotate driving index
	_exec_drindex += _exec_drinc;  // will overflow to 255 if inc is negative and value is 0
	if (_exec_drindex > EB_SM_DRIVING_SEQUENCE_MAX) _exec_drindex = _exec_drinit;  // see previous comment

	// update counters and timers
	_exec_steps --;
	_exec_ptime = cTime;
	_standby_previousTime = currentTime; // avoid standby alert

	// next command?
	if (_exec_steps > 0) return 1;  // still pending steps
	return 2;  // finished movement, time for next
}  // handleAction()

/**
 * Stop current Action (if any) execution.
 *
 * @param currentTime  Current time in milliseconds (should be provided).
 *  
 */
void Escornabot::stopAction(uint32_t currentTime)
{
	// shutdown execution
	_exec_steps = 0;
}  // stopAction()



////////////////////////////////////////
//
// Extra
//
////////////////////////////////////////

/**
 * Activates internal flag to reverse the stepper motor rotation.
 * 
 * This is an "easy fix" to deal with stepper motors that have swapped
 * wires and instead of blue-pink-yellow-orange come with pink-blue-orange-yellow.
 */
void Escornabot::fixReversed()
{
	_isReversed = true;
}  // fixReversed()

/**
 * This function takes care of the idle state of the Escornabot.
 * At this moment: avoid powerBank shutdown and alerts of inactivity.
 * 
 * @param currentTime  Current time in milliseconds (should be provided).
 */
void Escornabot::handleStandby(uint32_t currentTime)
{
	// avoid powerbank shutdown
	if (_powerbank_previousTime == 0) // first time only
	{
		// energize powerbank
		_setCoils(B000, B0001);
		delay(550);
		_setCoils(B0000, B0000);
		_powerbank_previousTime = currentTime;
	}
	if (currentTime - _powerbank_previousTime > 4000) // recurrent
	{
		// energize one coil for 5 ms
		_setCoils(B0000, B0001);
		delay(5);
		_setCoils(B0000, B0000);
		_powerbank_previousTime = currentTime;
	}

	// alert: "still ON" every 30 s of inactivity
	if (currentTime - _standby_previousTime > 30000)
	{
		beep(EB_BEEP_DEFAULT, 25);
		delay(50);
		beep(EB_BEEP_DEFAULT, 25);
		_standby_previousTime = currentTime;
	}
}  // handleStandby()



////////////////////////////////////////
//
// Debug
//
////////////////////////////////////////

#ifdef EB_DEBUG_MODE
/**
 * Provides debug information about the library via serial port.
 */
void Escornabot::debug()
{
	// debug info
	Serial.print("Escornabot-lib v");
	Serial.println(EB_VERSION);
}  // debug()
#endif
