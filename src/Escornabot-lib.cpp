/**
 * Escornabot-lib is a library for the Escornabot ROBOT.
 * 
 * A library with all the core functions and data to program an Escornabot
 * ROBOT. More info about the project at roboteach.es/escornabot and
 * escornabot.org.
 *
 * @file      Escornabot-lib.cpp
 * @author    mgesteiro
 * @date      20221216
 * @version   0.1.1-beta
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
Escornabot::Escornabot() {
	// Stepper motors
	_initCoilsPins();
	// Buzzer
	pinMode(BUZZER_PIN, OUTPUT);
	// On-board LED
	pinMode(SIMPLELED_PIN, OUTPUT);
	// NeoPixel
	_initNeoPixel(NEOPIXEL_PIN);
	// Keypad with default (Config) values
	configKeypad(
		// with values from Config.h
		KEYPAD_PIN,
		KEYVALUE_NONE,
		KEYVALUE_FORWARD,
		KEYVALUE_TURNLEFT,
		KEYVALUE_GO,
		KEYVALUE_TURNRIGHT,
		KEYVALUE_BACKWARD
	);
	clearKeypad(0);
}

/**
 * Destructor
 */
Escornabot::~Escornabot() {
}



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
	// fixReversed - stepper motors with swapped cables
	if (_isReversed) cms = -cms;

	// prepare action
	EB_T_COMMANDS command = EB_CMD_FW;
	if (cms < 0) command = EB_CMD_BW;
	prepareAction(command, cms);

	// execute action
	while (handleAction(millis(), command) != EB_CMD_R_FINISHED_ACTION);

	// finish
	disableSM();

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
	// fixReversed - stepper motors with swapped cables
	if (_isReversed) degrees = -degrees;

	// prepare action
	EB_T_COMMANDS command = EB_CMD_TR;
	if (degrees < 0) command = EB_CMD_TL;
	prepareAction(command, degrees);

	// execute action
	while (handleAction(millis(), command) != EB_CMD_R_FINISHED_ACTION);

	// finish
	disableSM();

}  // turn()

/**
 * Disable the stepper motors (switching off the coils).
 */
void Escornabot::disableSM()
{
	_setCoils(0, 0);
}

/**
 * Initializes the output pins for the stepper motor coils.
 */
void Escornabot::_initCoilsPins()
{
	// PORTB maps to Arduino digital pins 8 to 13. The two high bits (6 & 7) map to the crystal pins and are not usable.
	DDRB = DDRB | B00001111;  // pins x,x,x,x,11,10,9,8 as OUTPUT - Right motor
	// PORTD maps to Arduino digital pins 0 to 7. Pins 0 and 1 are TX and RX, manipulate with care.
	DDRD = DDRD | B11110000;  // pins 7,6,5,4,x,x,x,x as OUTPUT - Left motor
}

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
	// a -> PINB  b->stateR  mask->B00001111
	PORTB = (PORTB & B11110000) | (stateR & B00001111);

	// PORTD maps to Arduino digital pins 0 to 7
	// LeftMotor - pins 7,6,5,4 -> PORTD bits[7-4] = stateL bits[3-0]
	// (a & ~mask) | (b & mask)
	// a -> PIND  b->(stateL << 4)  mask->B11110000
	PORTD = (PORTD & B00001111) | (stateL << 4); // implicit mask in second part
}



////////////////////////////////////////
//
// Buzzer
//
////////////////////////////////////////

/**
 * Plays the frequency provided during the time indicated.
 *
 * @param frequency  Which frequency in Hz to play
 * @param duration   Duration, in milliseconds, of the sound
 *
 * @note this function is non-blocking and returns inmediatly.
 */
void Escornabot::beep(EB_T_BEEPS frequency, uint16_t duration)
{
	tone(BUZZER_PIN, frequency, duration);
}

/**
 * Plays the frequency provided during the time indicated.
 *
 * @param frequency  Which frequency in Hz to play
 * @param duration   Duration, in milliseconds, of the sound
 *
 * @note this function is blocking (wait for the note to be finished).
 */
void Escornabot::playNote(uint16_t frequency, uint16_t duration)
{
	beep(frequency, duration);
	delay(duration);
}



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
}

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
}



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
	//_neopixel->clear(); // Set all pixel colors to 'off'
	_neopixel->setPixelColor(0, _neopixel->Color(R, G, B));
	_neopixel->show();
}

/**
 * Shows the color associated with the selected key in the Escornabot NeoPixel.
 * 
 * @param key  Which key color to be shown.
 */
void Escornabot::showKeyColor(uint8_t key)
{
	switch (key) {
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
	case EB_KP_KEY_BW:
		showColor(BRIGHTNESS_LEVEL, BRIGHTNESS_LEVEL, 0);  // yellow
		break;
	case EB_LUCI_COLOR:
		showColor(BRIGHTNESS_LEVEL, 0, BRIGHTNESS_LEVEL);  // purple
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
}



////////////////////////////////////////
//
// Keypad
//
////////////////////////////////////////

/**
 * Updates the keypad configuration values: analog input pin and
 * analog reading values for every key.
 *
 * @param KeypadPin  the analog pin to which the keypad is connected
 * @param KeypadValue_NN  analog value when no key is pressed
 * @param KeypadValue_FW  analog value for forward key
 * @param KeypadValue_TL  analog value for turn left key
 * @param KeypadValue_GO  analog value for go key
 * @param KeypadValue_TR  analog value for turn right key
 * @param KeypadValue_BW  analog value for forward key
 */
void Escornabot::configKeypad(
	uint8_t KeypadPin,
	int16_t KeypadValue_NN,
	int16_t KeypadValue_FW,
	int16_t KeypadValue_TL,
	int16_t KeypadValue_GO,
	int16_t KeypadValue_TR,
	int16_t KeypadValue_BW)
{
	_keypad_pin = KeypadPin;
	pinMode(_keypad_pin, INPUT_PULLUP); // 2-wires, works if it's externally pulled-up too
	_keypad_values[0] = KeypadValue_NN;
	_keypad_values[1] = KeypadValue_FW;
	_keypad_values[2] = KeypadValue_TL;
	_keypad_values[3] = KeypadValue_GO;
	_keypad_values[4] = KeypadValue_TR;
	_keypad_values[5] = KeypadValue_BW;
}

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
} // handleKeypad()

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
}

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
	if (label == EB_KEYS_LABELS[keyPressed]) return true;
	return false;
}

/**
 * Lowest level reading function of the keypad input pin.
 * 
 * @return Analog reading output of the keypad pin.
 */
int16_t Escornabot::rawKeypad()
{
	return analogRead(_keypad_pin);
}



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
	value = abs(value);
	switch (command)
	{
	case EB_CMD_FW : // FORWARD
		_exec_steps = value * 10 * STEPPERS_STEPS_MM;  // # steps, implicit trunc()
		_exec_drinit = 0;  // initial driving state index
		_exec_drinc = 1;  // driving index groth direction
		break;
	case EB_CMD_BW : // BACKWARD
		_exec_steps = value * 10 * STEPPERS_STEPS_MM;  // # steps, implicit trunc()
		_exec_drinit = EB_SM_DRIVING_SEQUENCE_MAX;  // initial driving state index
		_exec_drinc = -1;  // driving index groth direction
		break;
	case EB_CMD_TL : // TURN LEFT
	case EB_CMD_TL_ALT : // TURN LEFT ALTERNATE
		_exec_steps = value * STEPPERS_STEPS_DEG;  // # steps, implicit trunc()
		_exec_drinit = EB_SM_DRIVING_SEQUENCE_MAX;  // initial driving state index
		_exec_drinc = -1;  // driving index groth direction
		break;
	case EB_CMD_TR : // TURN RIGHT
	case EB_CMD_TR_ALT : // TURN RIGHT ALTERNATE
		_exec_steps = value * STEPPERS_STEPS_DEG;  // # steps, implicit trunc()
		_exec_drinit = 0;  // initial driving state index
		_exec_drinc = 1;  // driving index groth direction
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

	#ifdef DEBUG_MODE
	Serial.print("PREPARING ");
	Serial.println(EB_CMD_LABELS[command]);
	Serial.print("Total STEPS: ");
	Serial.println(_exec_steps);
	Serial.print("Acceleration point: ");
	Serial.println(_exec_ap);
	Serial.print("Deceleration point: ");
	Serial.println(_exec_dp);
	#endif
} // prepareAction()

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

	#ifdef DEBUG_MODE
		//if (_microdelays_index < 200) _microdelays[_microdelays_index] = cTime - _exec_ptime;
		//_microdelays_index ++;
	#endif

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
} // handleAction()

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
}



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
}

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
	if (currentTime - _standby_previousTime > 30000) {
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

#ifdef DEBUG_MODE
/**
 * Provides debug information about the library via serial port.
 */
void Escornabot::debug()
{
	// debug info
	Serial.print("Escornabot-lib v");
	Serial.println(EB_VERSION);
}
#endif
