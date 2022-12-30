/**
 * Configuration file for the Luci's Firmware.
 * 
 * A collection of parameters to configure Luci's behaviour.
 * Based on the Escornabot-lib library config file: should be included after Escornabot-lib.h. 
 *
 * @file      Config.h
 * @author    mgesteiro
 * @date      20221230
 * @version   0.2.2-beta
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
#define EB_KP_VALUE_FW 230
#define EB_KP_VALUE_TL 368
#define EB_KP_VALUE_GO 586
#define EB_KP_VALUE_TR 532
#define EB_KP_VALUE_BW 462
#define EB_KP_VALUE_NN 1002
// advanced
#define EB_KP_LP_MIN_DURATION 900L   // long press minimum duration, ms
#define EB_KP_DB_TIME 30L            // debouncing time, ms
#define EB_KP_CHECK_MIN_INTERVAL 5L  // checking minimum interval, ms

// LED
#define SIMPLELED_PIN 13

// NeoPixel
#define NEOPIXEL_PIN 12
#define BRIGHTNESS_LEVEL 50  // range 10-255
