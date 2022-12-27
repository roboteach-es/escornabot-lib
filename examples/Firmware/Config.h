/**
 * Configuration file for the Luci's Firmware.
 * 
 * A collection of parameters to configure Luci's behaviour.
 * Based on the Escornabot-lib library config file: should be included after Escornabot-lib.h. 
 *
 * @file      Config.h
 * @author    mgesteiro
 * @date      20221225
 * @version   0.1.2-beta
 * @copyright OpenSource, LICENSE GPLv3
 */

// Escornabot geometry
#define WHEEL_DIAMETER 75.5f // mm
#define WHEEL_DISTANCE 80.1f // mm (ground touching point to point)

// Stepper motors
#define STEPPERMOTOR_STEPS_PER_SECOND 450.0f // speed -> MAX=490, MIN=60
// from https://github.com/mgesteiro/steppers#stepping, using full-stepping mode
#define STEPPERMOTOR_FULLREVOLUTION_STEPS 2048.0f // number of steps for a full revolution of the axis

// Buzzer
#define BUZZER_PIN 2

// Keypad  --> based on https://github.com/mgesteiro/escornakeypad
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

// LED
#define SIMPLELED_PIN 13

// NeoPixel
#define NEOPIXEL_PIN 12
#define BRIGHTNESS_LEVEL 50  // range 10-255

// Extra
//#define DEBUG_MODE
