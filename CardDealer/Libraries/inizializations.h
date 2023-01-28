#ifndef LIBRARIES_INIZIALIZATIONS_H_
#define LIBRARIES_INIZIALIZATIONS_H_

#include <stdio.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "events.h"

#define  HZ   24000000UL    // 24MHz


/* -------- Timers -------- */

/* Timer_A Compare Configuration Parameter  (PWM) */
static const Timer_A_CompareModeConfig BZ_compareConfig_PWM = {
TIMER_A_CAPTURECOMPARE_REGISTER_4,          // Use CCR3
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
        5000                                    // 10% Duty Cycle initially
        };

/* Timer_A Up Configuration Parameter */
static const Timer_A_UpModeConfig BZ_upConfig = {
TIMER_A_CLOCKSOURCE_SMCLK,                      // SMCLK = 3 MhZ
        TIMER_A_CLOCKSOURCE_DIVIDER_12,         // SMCLK/12 = 250 KhZ
        20000,                                  // 40 ms tick period
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,    // Disable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
        };

/* Timer_A Continuous Mode Configuration Parameter */
static const Timer_A_ContinuousModeConfig DS_continuousModeConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,           // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,       // SMCLK/1 = 3MHz
        TIMER_A_TAIE_INTERRUPT_DISABLE,      // Disable Timer ISR
        TIMER_A_SKIP_CLEAR                   // Skip Clear Counter
};

/* Timer_A Capture Mode Configuration Parameter */
static const Timer_A_CaptureModeConfig DS_captureModeConfig =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_1,        // CC Register 1
        TIMER_A_CAPTUREMODE_RISING_AND_FALLING_EDGE, // Rising Edge and falling
        TIMER_A_CAPTURE_INPUTSELECT_CCIxA,        // CCIxA Input Select
        TIMER_A_CAPTURE_SYNCHRONOUS,              // Synchronized Capture
        TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE,  // Enable interrupt
        TIMER_A_OUTPUTMODE_OUTBITVALUE            // Output bit value
};


/* Graphic library context */
Graphics_Context g_sContext;
Graphics_Context getGraphicsContext();

/* -------- Init Functions -------- */

/*
 * Initializes pins and peripherals to
 * handle the joystick
 */
void _adcInit();

/*
 * Initializes display and graphics content
 */
void _graphicsInit();

/*
 * Initializes buttons B1 and B2 of MKII
 */
void _buttonsInit();

/*
 * Sets all the pins and timers needed to
 *   manage the DC motor
 */
void _DCMotorInit();

/*
 * Initializes step motor pins
 */
void _stepMotorInit();

/*
 * Initializes step motor pins and timers
 */
void _distanceSensorInit();

/*
 * Calls all the others initializations
 * */
void _hwInit();


/* -------- Handlers -------- */

void TA0_N_IRQHandler(void);  // Handler for distance sensor measure

void ADC14_IRQHandler(void);  // Joystick handler

void PORT3_IRQHandler(void);  // B1 handler

void PORT5_IRQHandler(void);  // B2 handler


/* -------- Handlers Utilities -------- */

/*
 * Notifies that the distance sensor has
 * taken a new measure
 *   t1 -> numbers of ticks on rising edge (trigger sended to sensor)
 *   t2 -> numbers of ticks on falling edge (reply received from the sensor)
 */
static void DS_measureReady(int t1, int t2);

/*
 * Notify if there is available a measure
 * that has never been read before
 */
bool DS_hasNewMeasure();

/*
 * Returns the last taken measure
 * of the distance sensor
 */
int DS_getMeasure();


/* -------- Buzzer -------- */

/*
 * Initialize timer TA0 and start the buzzer
 */
void turnOnBuzzer();

/*
 * Stops and initialize the timer TA0 for the
 * distance sensor
 *  (we "deinitialize" and initialize every time the
 *  timer TA0 since both buzzer and distance sensor use it)
 */
void turnOffBuzzer();

#endif /* LIBRARIES_INIZIALIZATIONS_H_ */
