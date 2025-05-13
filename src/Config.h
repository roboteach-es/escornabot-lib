/**
 * Configuration file for the Escornabot-lib library.
 *
 * A collection of parameters to configure how the Escornabot-lib library
 * operates. More info about the project at roboteach.es/escornabot and
 * escornabot.org.
 *
 * @file      Config.h
 * @author    mgesteiro einsua
 * @date      20250513
 * @version   1.4.1
 * @copyright OpenSource, LICENSE GPLv3
 */

// Escornabot geometry (default values)
#define WHEEL_DIAMETER   75.5f // mm - Brivoi=Luci=75.5 (theoretical)
#define WHEEL_SEPARATION 80.1f // mm - Brivoi=75.5 Luci=80.1 (theoretical)
#define WHEEL_CIRCUMFERENCE float(PI * WHEEL_DIAMETER) // wheel circumference
#define ROTATION_CIRCUMFERENCE float(PI * WHEEL_SEPARATION) // rotation circunference on the floor

// Stepper motors
// OPTION A: only 4 stages
const uint8_t EB_SM_DRIVING_SEQUENCE[] = {B0011, B0110, B1100, B1001};  // full drive - stronger
//const uint8_t EB_SM_DRIVING_SEQUENCE[] = {B0001, B0010, B0100, B1000};  // wave drive - less consumption
// from https://github.com/mgesteiro/steppers#stepping, using full-stepping mode
// theoretical value: 32 * 63,6840 = 2037,8864
// practical value: 2048 (slips, gear teeth engagement, etc.)
#define STEPPERMOTOR_FULLREVOLUTION_STEPS 2048.0f // number of steps for a full revolution of the axis
#define STEPPERMOTOR_STEPS_PER_SECOND 420.0f // speed -> MAX~490, MIN~60

// OPTION B: 8 stages -> more resolution
//const uint8_t EB_SM_DRIVING_SEQUENCE[] = {B0001, B0011, B0010, B0110, B0100, B1100, B1000, B1001};  // half drive - resolution & strength
//#define STEPPERMOTOR_FULLREVOLUTION_STEPS 4096.0f // number of steps for a full revolution of the axis
//#define STEPPERMOTOR_STEPS_PER_SECOND 800.0f // speed -> MAX~??, MIN~??

#define STEPPERS_STEPS_MM float(STEPPERMOTOR_FULLREVOLUTION_STEPS / WHEEL_CIRCUMFERENCE) // how many steps to move 1 mm
#define STEPPERS_STEPS_DEG float((ROTATION_CIRCUMFERENCE/360) * STEPPERS_STEPS_MM) // how many steps to rotate 1 degree

// Buzzer
#define BUZZER_PIN 2 // 10 for the Brivoi

// Keypad  (default values, based on https://github.com/mgesteiro/escornakeypad)
#define KEYPAD_PIN A0 // A4 or A7 for the Brivoi (depends on the version)
#define EB_KP_VALUE_FW 230
#define EB_KP_VALUE_TL 368
#define EB_KP_VALUE_GO 586
#define EB_KP_VALUE_TR 532
#define EB_KP_VALUE_BW 462
#define EB_KP_VALUE_NN 1023
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
