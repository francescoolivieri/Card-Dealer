/*
 * card_dealer.c
 *
 *  Created on: 5 gen 2023
 *      Author: Francesco Olivieri
 */

#include <inizializations.h>

/* ------ Init Functions ------  */

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
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);

    GrContextFontSet(&g_sContext, &g_sFontCmsc12);


    Graphics_clearDisplay(&g_sContext);

}

void _buttonsInit(){
    //initialization button1 MKII
        GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN5);
        GPIO_enableInterrupt(GPIO_PORT_P3, GPIO_PIN5);

        //initialization button1 MKII
        GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN1);
        GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN1);

        //interrupt enable
        Interrupt_enableInterrupt(INT_PORT3);
        Interrupt_enableInterrupt(INT_PORT5);

}

void _DCMotorInit(){

    /* Configuring output */
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);

    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1);

    /* Starting the Timer32 */
       Timer32_initModule(TIMER32_1_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,TIMER32_PERIODIC_MODE); // uses MCLK
       Timer32_disableInterrupt(TIMER32_1_BASE);
       Timer32_setCount(TIMER32_1_BASE, 1);
       Timer32_startTimer(TIMER32_1_BASE, true);

    /* Starting the Timer_A0 in continuous mode */
       Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_CONTINUOUS_MODE);

}

void _stepMotorInit()
{
    // set pins for Stepper Motor
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN2);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN6);
}

void _distanceSensorInit(){
    // set pins for Distance Sensor
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2, GPIO_PIN4,
    GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN6);

    /* Starting the Timer32 needed to send trigger */
    Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
                       TIMER32_PERIODIC_MODE); // uses MCLK
    Timer32_disableInterrupt(TIMER32_0_BASE);
    Timer32_setCount(TIMER32_0_BASE, 1);
    Timer32_startTimer(TIMER32_0_BASE, true);

    /* Configuring Capture Mode */
    Timer_A_initCapture(TIMER_A0_BASE, &DS_captureModeConfig);

    /* Configuring Continuous Mode */
    Timer_A_configureContinuousMode(TIMER_A0_BASE, &DS_continuousModeConfig);

    /* Starting the Timer_A0 in continuous mode */
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);

    /* Enabling interrupts */
    Interrupt_enableInterrupt(INT_TA0_N);
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

    _adcInit();
    _graphicsInit();
    _buttonsInit();
    _DCMotorInit();
    _stepMotorInit();
    _distanceSensorInit();

    Interrupt_enableMaster();

}


/* ------- Interrupt Handlers ------- */

static int meas1 = 0;
static int meas2 = 0;
static int meas1Count=0;
static int rising;
void TA0_N_IRQHandler(void)  // Timer Interrupt for distance sensor
{
    rising  = 0;

    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_1);

    if(P2IN&0x10) rising=1; else rising=0;

    if(rising) // Trigger sent
    {
        meas1 = Timer_A_getCaptureCompareCount(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);

        meas1Count=1;
    }
    else      // Echo received
    {
        meas2 = Timer_A_getCaptureCompareCount(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);

        if (meas1Count==1){ //if meas1 has been collected
            meas1Count=0; //reset meas1 count
            DS_measureReady(meas1, meas2);
        }
    }
}


static uint16_t resultsBuffer[2];
static bool joystick_semaphore=true; // manage
void ADC14_IRQHandler(void)    // Handler for joystick
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if(status & ADC_INT1)
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

}

//handler button1
void PORT3_IRQHandler(void)
{

    uint_fast16_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P3);
    GPIO_clearInterruptFlag(GPIO_PORT_P3, status);

    if((status & GPIO_PIN5)){
        button1_press();
    }
}

//handler button2
void PORT5_IRQHandler(void)
{

    uint_fast16_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P5);
    GPIO_clearInterruptFlag(GPIO_PORT_P5, status);

    if((status & GPIO_PIN1)){
        button2_press();
    }
}

/* ----- Distance Sensor Service Functions ----- */
static bool DS_newVal = false;
static int distCM = 0;

void DS_measureReady(int t1, int t2){
    int diff = ((t2 - t1) & 0xFFFF);
    distCM = (diff / 3) / 58; // diff is time in ticks (1 tick is 1/3us) so by dividing diff/3 I find how much time has passed, then I know that the sound makes 1cm every 58us so I divide (timePassed)/58

    DS_newVal = true;
}

bool DS_hasNewMeasure(){
    return DS_newVal;
}

int DS_getMeasure(){
    if(DS_newVal)  // become a "used measure"
        DS_newVal = false;
    return distCM;
}

/* ----- Buzzer Functions ----- */

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

Graphics_Context getGraphicsContext(){
    return g_sContext;
}
