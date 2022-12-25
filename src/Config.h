/**
 * Configuration file for the Escornalib library.
 * 
 * A collection of parameters to configure how the Escornalib libray operates.
 * More info about the project at roboteach.es/escornabot and escornabot.org.
 *
 * @file      Config.h
 * @author    mgesteiro
 * @date      20221216
 * @version   0.1.0-beta
 * @copyright OpenSource, LICENSE GPLv3
 */

// Escornabot geometry
#define WHEEL_DIAMETER 75.5f // mm Luci: 75.5 (theoretical)
#define WHEEL_DISTANCE 80.1f // mm (ground touching point to point) Luci: 80.1 (theoretical)

// Stepper motors
#define STEPPERMOTOR_STEPS_PER_SECOND 450.0f //475.0f // speed -> MAX=490, MIN=60
// from https://github.com/mgesteiro/steppers#stepping, using full-stepping mode
// theoretical value: 32 * 63,6840 = 2037,8864
// practical value: 2048 (slips, gear teeth engagement, etc.)
#define STEPPERMOTOR_FULLREVOLUTION_STEPS 2048.0f // number of steps for a full revolution of the axis

// Buzzer
#define BUZZER_PIN 2

// Keypad
// https://github.com/mgesteiro/escornakeypad
#define KEYPAD_PIN A0
#define KEYVALUE_FORWARD 230
#define KEYVALUE_TURNLEFT 368
#define KEYVALUE_GO 586
#define KEYVALUE_TURNRIGHT 532
#define KEYVALUE_BACKWARD 462
#define KEYVALUE_NONE 1002
// advanced
#define EB_KP_LP_MIN_DURATION 900L   // long press minimum duration, ms
#define EB_KP_DB_TIME 30L            // debouncing time, ms
#define EB_KP_CHECK_MIN_INTERVAL 5L  // checking minimum interval, ms


// Led
#define SIMPLELED_PIN 13

// NeoPixel
#define NEOPIXEL_PIN 12
#define BRIGHTNESS_LEVEL 50  // range 10-255

// Extra
//#define DEBUG_MODE
