/*
 * card_dealer.c
 *
 *  Created on: 5 gen 2023
 *      Author: Francesco Olivieri
 */

#include "card_dealer.h"

/* ------ Init Functions ------  */

void initCardDealer(){
    _hwInit();
}

//adc initialization
void _adcInit(){
        GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION);
        GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);

        ADC14_enableModule();
        ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_64, ADC_DIVIDER_8, 0);

        ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);
        ADC14_configureConversionMemory(ADC_MEM0,
                ADC_VREFPOS_AVCC_VREFNEG_VSS,
                ADC_INPUT_A15, ADC_NONDIFFERENTIAL_INPUTS);

        ADC14_configureConversionMemory(ADC_MEM1,
                ADC_VREFPOS_AVCC_VREFNEG_VSS,
                ADC_INPUT_A9, ADC_NONDIFFERENTIAL_INPUTS);

        ADC14_enableInterrupt(ADC_INT1);

        Interrupt_enableInterrupt(INT_ADC14);
        Interrupt_enableMaster();

        ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

        ADC14_enableConversion();
        ADC14_toggleConversionTrigger();
}

void _graphicsInit()
{
    /* Initializes display */
    Crystalfontz128x128_Init();

    /* Set default screen orientation */
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128, &g_sCrystalfontz128x128_funcs);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext,
                                            (int8_t *)"Waiting to start :",
                                            AUTO_STRING_LENGTH,
                                            64,
                                            30,
                                            OPAQUE_TEXT);

}

void _hwInit()
{
    /* Halting WDT and disabling master interrupts */
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    /* Set the core voltage level to VCORE1 */
    PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set 2 flash wait states for Flash bank 0 and 1*/
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_24);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_8);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_8);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_8);

    _adcInit();
    _graphicsInit();

    // set pins for Stepper Motor
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN2);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN6);

    // set pins for Distance Sensor
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2, GPIO_PIN4,
                                               GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN6);

    /* Configuring Capture Mode */
    Timer_A_initCapture(TIMER_A0_BASE, &DS_captureModeConfig);

    /* Configuring Continuous Mode */
    Timer_A_configureContinuousMode(TIMER_A0_BASE, &DS_continuousModeConfig);

    /* Enabling interrupts */
    Interrupt_enableInterrupt(INT_TA0_N);
    Interrupt_enableMaster();

}


/* ------- Interrupt Handlers ------- */

int meas1 = 0;
int meas2 = 0;
int meas1Count=0;
void TA0_N_IRQHandler(void)  // Timer Interrupt for distance sensor
{
    int rising  = 0;

    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_1);

    if(P2IN&0x10) rising=1; else rising=0;

    if(rising) // Start
    {
        meas1 = Timer_A_getCaptureCompareCount(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);

        meas1Count=1;
    }
    else
    {
        meas2 = Timer_A_getCaptureCompareCount(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);

        if (meas1Count==1){ //if meas1 has been collected
            meas1Count=0; //reset meas1 count
            DS_valueReady(meas1, meas2);
        }
    }
}


static uint16_t resultsBuffer[2];
bool joystick_semaphore=true;
void ADC14_IRQHandler(void)    // Handler for joystick
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if(status & ADC_INT1) // BOH
    {
        resultsBuffer[1] = ADC14_getResult(ADC_MEM1);

        if(resultsBuffer[1]>14000 && joystick_semaphore==true){
            joystick_up();
            joystick_semaphore=false;
        }

        if(resultsBuffer[1]<10000 && resultsBuffer[1]>4000 && joystick_semaphore==false){
            joystick_semaphore=true;
        }


        if(resultsBuffer[1]<2000 && joystick_semaphore==true){
            joystick_down();
            joystick_semaphore=false;
            }

    }

    if(status & ADC_INT1){
            resultsBuffer[1] = ADC14_getResult(ADC_MEM1);
            if (!(P4IN & GPIO_PIN1)){
                joystick_press();
            }

        }

}




void initTriggerDS(){
    /* Starting the Timer32 */
    Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT, TIMER32_PERIODIC_MODE); // uses MCLK
    Timer32_disableInterrupt(TIMER32_0_BASE);
    Timer32_setCount(TIMER32_0_BASE, 1);
    Timer32_startTimer(TIMER32_0_BASE, true);
}

void sendTrigPulseDS(){ // triggers Pin 5.6 for 1 us
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN6); // 5.6 trigger pin of distance sensor
    Timer32_setCount(TIMER32_0_BASE, 24 * 10); // multiply by 24 -> 1 us * 10 -> 1 us
    while (Timer32_getValue(TIMER32_0_BASE) > 0) // Wait 10us
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN6);
}

void startDelayCaptureDS(){
    /* Starting the Timer_A0 in continuous mode for DS measuring*/
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);
}

/* ----- Service Functions ----- */

void DS_valueReady(int t1, int t2){
    int diff = ((t2 - t1) & 0xFFFF);
    distCM = (diff / 3) / 58; // diff is time in ticks (1 tick is 1/3us) so by dividing diff/3 I find how much time has passed, then I know that the sound makes 1cm every 58us so I divide (timePassed)/58


    DS_takeVal = true;
}

bool DS_isValueReady(){
    return DS_takeVal;
}

int DS_getDistCM(){
    return distCM;
}

void DS_valueRead(){
    DS_takeVal = false;
}

Graphics_Context getGraphicsContext(){
    return g_sContext;
}

/* ----- Extra Features ----- */

void turnOnBuzzer(){
    Interrupt_disableInterrupt(INT_TA0_N);
    Timer_A_stopTimer(TIMER_A0_BASE);
    /* Configures P2.7 to PM_TA0.4 for using Timer PWM to control the buzzer */
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN7,
    GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring Timer_A0 for Up Mode and starting */
    Timer_A_configureUpMode(TIMER_A0_BASE, &BZ_upConfig);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    Timer_A_initCompare(TIMER_A0_BASE, &BZ_compareConfig_PWM); // For P2.7
}

void turnOffBuzzer(){
    Timer_A_stopTimer(TIMER_A0_BASE);

    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN7);

    /* Configuring Capture Mode */
    Timer_A_initCapture(TIMER_A0_BASE, &DS_captureModeConfig);


    /* Configuring Continuous Mode */
    Timer_A_configureContinuousMode(TIMER_A0_BASE, &DS_continuousModeConfig);

    /* Starting the Timer_A0 in continuous mode */
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);

    Interrupt_enableInterrupt(INT_TA0_N);

}
