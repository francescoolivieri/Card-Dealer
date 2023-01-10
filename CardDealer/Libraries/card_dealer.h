/*
 * card_dealer.h
 *
 *  Created on: 5 gen 2023
 *      Author: Francesco Olivieri
 */

#ifndef LIBRARIES_CARD_DEALER_H_
#define LIBRARIES_CARD_DEALER_H_

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

// Init Functions
void initCardDealer();  // calls all the others
void _adcInit();
void _graphicsInit();
void button_init();
void initDispenserMotor();
void giveOneCard();
void _hwInit();


// handlers
static int meas1 = 0;
static int meas2 = 0;
static int meas1Count=0;
void TA0_N_IRQHandler(void);
void ADC14_IRQHandler(void);
void PORT3_IRQHandler(void);
void PORT5_IRQHandler(void);


// Step Motor
static int miglior_variabile_del_mondo_in_assoluto_best_in_town_bro_to_the_top_never_stop = 0;
void makeStep(bool move_forward);


// Distance sensor
void initTriggerDS();
void sendTrigPulseDS();
void startDelayCaptureDS();

// Dispenser Motor
void turnOnDispenserForward();
void turnOnDispenserBackward();
void turnOffDispenser();

void turnOnBuzzer();
void turnOffBuzzer();

static bool DS_takeVal = false;
static int distCM = 0;
void DS_valueReady(int t1, int t2);
bool DS_isValueReady();
int DS_getDistCM();
void DS_valueRead();

#endif /* LIBRARIES_CARD_DEALER_H_ */
