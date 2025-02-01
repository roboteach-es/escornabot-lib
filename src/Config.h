/**
 * Configuration file for the Escornabot-lib library.
 *
 * A collection of parameters to configure how the Escornabot-lib library
 * operates. More info about the project at roboteach.es/escornabot and
 * escornabot.org.
 *
 * @file      Config.h
 * @author    mgesteiro einsua
 * @date      20250201
 * @version   1.3.0
 * @copyright OpenSource, LICENSE GPLv3
 */

// Escornabot geometry (default values)
#define WHEEL_DIAMETER   75.5f // mm - Brivoi=Luci=75.5 (theoretical)
#define WHEEL_SEPARATION 80.1f // mm - Brivoi=75.5 Luci=80.1 (theoretical)
#define WHEEL_CIRCUMFERENCE float(PI * WHEEL_DIAMETER) // wheel circumference
#define ROTATION_CIRCUMFERENCE float(PI * WHEEL_SEPARATION) // rotation circunference on the floor

// Stepper motors
#define STEPPERMOTOR_STEPS_PER_SECOND 450.0f // speed -> MAX~490, MIN~60
// from https://github.com/mgesteiro/steppers#stepping, using full-stepping mode
// theoretical value: 32 * 63,6840 = 2037,8864
// practical value: 2048 (slips, gear teeth engagement, etc.)
#define STEPPERMOTOR_FULLREVOLUTION_STEPS 2048.0f // number of steps for a full revolution of the axis
#define STEPPERS_STEPS_MM float(STEPPERMOTOR_FULLREVOLUTION_STEPS / WHEEL_CIRCUMFERENCE) // how many steps to move 1 mm
#define STEPPERS_STEPS_DEG float((ROTATION_CIRCUMFERENCE/360) * STEPPERS_STEPS_MM) // how many steps to rotate 1 degree

// Buzzer
#define BUZZER_PIN 2

// Keypad  (default values, based on https://github.com/mgesteiro/escornakeypad)
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

// SERIAL / BLUETOOTH
#define EB_BAUDRATE 9600

// LED
#define SIMPLELED_PIN 13

// NeoPixel
#define NEOPIXEL_PIN 12
#define BRIGHTNESS_LEVEL 50  // range 10-255

// Stand-by (default values)
#define POWERBANK_TIMEOUT 2000    // max time without any high current demand to the powerbank
#define INACTIVITY_TIMEOUT 30000  // max time without any Escornabot activity before "Still ON!" alert
