#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include <action.h>
#include <condition.h>


#include "uart.h"

#define  HZ   24000000UL    // 24MHz
#define MAX_QUEUE_SIZE 10
#define LEN_QUEUE_MEX 20

#define MAX_PLAYERS 8

// Distance sensor
#define DS_RECOGNITION_MODE 1
#define DS_MAX_DISTANCE_DETECT 20 // cm

// Step Motor
#define STEPS_360 512

/* Graphic library context */
Graphics_Context g_sContext;

/**
 * UART baudrate configuration.
 */
const uint32_t uart_baudrate = 115200;

/* Timer_A Compare Configuration Parameter  (PWM) */
Timer_A_CompareModeConfig compareConfig_PWM = {
TIMER_A_CAPTURECOMPARE_REGISTER_4,          // Use CCR3
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
        5000                                    // 10% Duty Cycle initially
        };

/* Timer_A Up Configuration Parameter */
const Timer_A_UpModeConfig upConfig = {
TIMER_A_CLOCKSOURCE_SMCLK,                      // SMCLK = 3 MhZ
        TIMER_A_CLOCKSOURCE_DIVIDER_12,         // SMCLK/12 = 250 KhZ
        20000,                                  // 40 ms tick period
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,    // Disable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
        };

/* Timer_A Continuous Mode Configuration Parameter */
const Timer_A_ContinuousModeConfig continuousModeConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,           // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,       // SMCLK/1 = 3MHz
        TIMER_A_TAIE_INTERRUPT_DISABLE,      // Disable Timer ISR
        TIMER_A_SKIP_CLEAR                   // Skip Clear Counter
};

/* Timer_A Capture Mode Configuration Parameter */
const Timer_A_CaptureModeConfig captureModeConfig =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_1,        // CC Register 1
        TIMER_A_CAPTUREMODE_RISING_AND_FALLING_EDGE, // Rising Edge and falling
        TIMER_A_CAPTURE_INPUTSELECT_CCIxA,        // CCIxA Input Select
        TIMER_A_CAPTURE_SYNCHRONOUS,              // Synchronized Capture
        TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE,  // Enable interrupt
        TIMER_A_OUTPUTMODE_OUTBITVALUE            // Output bit value
};

// GRAPHICS

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

}

void turnOnBuzzer(){
    Interrupt_disableInterrupt(INT_TA0_N);
    Timer_A_stopTimer(TIMER_A0_BASE);
    /* Configures P2.7 to PM_TA0.4 for using Timer PWM to control the buzzer */
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN7,
    GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring Timer_A0 for Up Mode and starting */
    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    Timer_A_initCompare(TIMER_A0_BASE, &compareConfig_PWM); // For P2.7
}

void turnOffBuzzer(){
    Timer_A_stopTimer(TIMER_A0_BASE);

    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN7);

    /* Configuring Capture Mode */
    Timer_A_initCapture(TIMER_A0_BASE, &captureModeConfig);


    /* Configuring Continuous Mode */
    Timer_A_configureContinuousMode(TIMER_A0_BASE, &continuousModeConfig);

    /* Starting the Timer_A0 in continuous mode */
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);

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

    /* Initializes Clock System */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_24);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_8);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_8);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_8);

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
    Timer_A_initCapture(TIMER_A0_BASE, &captureModeConfig);

    /* Configuring Continuous Mode */
    Timer_A_configureContinuousMode(TIMER_A0_BASE, &continuousModeConfig);

    /* Enabling interrupts */
    Interrupt_enableInterrupt(INT_TA0_N);
    Interrupt_enableMaster();

}

/***
 * Declaration of functions
 */
void vTaskStepperMotor(void *pvParameters);
void vTaskDistanceSensor(void *pvParameters);
void peopleDetection();


/***
 * Function for printing to the UART terminal. This function will be modified in Task 2.1
 * Without modification it works exactly like uart_println()
 */
void uart_println_mutex(const char* str, ...);

//m1_3_1s//
int counter = 0;

/* The handle to TaskA, which can be used to delete TaskA */
TaskHandle_t xTaskXHandle = NULL;
int meas1 = 0;
int meas2 = 0;
int meas1Count=0;
int takeVal=0;

xQueueHandle q1;

typedef enum{
    IDLE,
    GAME_SELECTION,
    RECOGNITION,
    IN_GAME,
}State_t;

State_t current_state = IDLE;

/*-----------------------------------------------------------*/
int main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    /* P2.7 buzzer */
    _hwInit();


    //Graphics_drawStringCentered(&g_sContext, (int8_t *) "Buzzer Demo", AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);


    // Initialize Queue
    int msg;
    q1 = xQueueCreate(10, sizeof(msg));

    if(!q1) printf("Problems opening queue\n");

    // Initialize UART
    uart_init(uart_baudrate);

    // Start people detection
    peopleDetection();

    while(1){

        switch(current_state){
            case IDLE:
                current_state = RECOGNITION;

                break;
            case RECOGNITION:
                peopleDetection();

                break;
        }

    }

    /* The following line should never be reached.
     * Otherwise there was insufficient FreeRTOS heap
     * memory available for the task to be created.
     */
    return 0;
}
/*-----------------------------------------------------------*/


int peoplePos[MAX_PLAYERS];
int contPeople = 0;

void peopleDetection(){
    BaseType_t result = pdPASS;

    // SETUP TASK STEPPER MOTOR
    int rotationDegrees = 360;

    result = xTaskCreate(vTaskStepperMotor, "TaskStepperMotor", 1000, (void*) &rotationDegrees, 1, NULL);
    if (result != pdPASS)
    {
        uart_println("Error creating TaskStepperMotor task.");
        return ;
    }


    // SETUP TASK DISTANCE SENSOR
    int pcParameters2 = 1;
    result = xTaskCreate(vTaskDistanceSensor, "TaskDistanceSensor", 1000, (void*) &pcParameters2, 1, &xTaskXHandle);
    if (result != pdPASS)
    {
        uart_println("Error creating TaskDistanceSensor task.");
        return ;
    }

    // Start the tasks
    vTaskStartScheduler();
}

/**
 * TaskStepperMotor
 * Make the motor turn
 * @param Degrees of rotation
 */
//m1_1_1s//

void vTaskStepperMotor(void *pvParameters)
{
    int cont = 0;
    uart_println_mutex("Start TaskStepperMotor.");

    int degreesToSteps = STEPS_360/(360 / (*((int *)pvParameters)));

    int velocity = 80000;
    while (cont < degreesToSteps)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN7);
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN2);
        GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6);
        vTaskDelay(HZ / velocity);     // delay

        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN7);
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN2);
        GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6);
        vTaskDelay(HZ / velocity);     // delay

        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN7);
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN0);
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN2);
        GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
        vTaskDelay(HZ / velocity);     // delay

        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN7);
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0);
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN2);
        GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
        vTaskDelay(HZ / velocity);     // delay
        cont++;

        int msg;
        if(xQueueReceive(q1, &(msg), 0)){
            if(contPeople < 8){
                peoplePos[contPeople] = cont-1;
                contPeople++;

                vTaskSuspend(xTaskXHandle);
                turnOnBuzzer();
                vTaskDelay(pdMS_TO_TICKS(30000));
                turnOffBuzzer();
                vTaskResume(xTaskXHandle);
            }
        }
    }

    vTaskDelete(xTaskXHandle);


    int i;
    printf("STAMPAAAAA \n");
    for(i=0 ; i<contPeople ; i++){
        printf("%d ", peoplePos[i]);
    }
    printf("\n");
}
//m1_1_1e//

/**
 * TaskDistanceSensor
 * Reads distance
 * @param mode
 */
//m1_3_3s//
void vTaskDistanceSensor(void *pvParameters)
{
    uart_println_mutex("Start TaskDistanceSensor.");
    int count = 0;

    /* Starting the Timer32 */
    Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,TIMER32_PERIODIC_MODE); // uses MCLK
    Timer32_disableInterrupt(TIMER32_0_BASE);
    Timer32_setCount(TIMER32_0_BASE, 1);
    Timer32_startTimer(TIMER32_0_BASE, true);

    /* Starting the Timer_A0 in continuous mode */
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);

    while (1) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN6);
        Timer32_setCount(TIMER32_0_BASE, 24 * 10); // multiply by 24 -> 1 us * 10 -> 1 us
        while (Timer32_getValue(TIMER32_0_BASE) > 0) // Wait 10us
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN6);
        //vTaskDelay(HZ / 4);     // delay

        vTaskDelay(pdMS_TO_TICKS(500));
        if (takeVal == 1)
        {
            takeVal = 0;
            int diff = ((meas2 - meas1) & 0xFFFF);
            int distCM = (diff / 3) / 58; // diff is time in ticks (1 tick is 1/3us) so by dividing diff/3 I find how much time has passed, then I know that the sound makes 1cm every 58us so I divide (timePassed)/58
            printf("Distance: %i cm \n", distCM);

            if((*((int *)pvParameters)) == DS_RECOGNITION_MODE){ // recognition mode
                if(distCM<DS_MAX_DISTANCE_DETECT){
                    count++;

                    if(count == 4){
                        int msg = 1;
                        xQueueSend(q1, (void *)msg, (portTickType)0);

                        vTaskDelay(pdMS_TO_TICKS(500));
                    }
                }else{
                    count = 0;
                }
            }
        }
    }
}



void TA0_N_IRQHandler(void)
{
    int rising  = 0;

    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_1);

    if(P2IN&0x10) rising=1; else rising=0;

    if(rising) // Start
    {
        meas1 = Timer_A_getCaptureCompareCount(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);
       // printf("Measure 1 %i \n",meas1);
        meas1Count=1;
    }
    else
    {
        meas2 = Timer_A_getCaptureCompareCount(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);
       // printf("Measure 2 %i \n",meas2);
        if (meas1Count==1){ //if meas1 has been collected
            meas1Count=0; //reset meas1 count
            takeVal=1; //flag for conversion
        }
    }
}

/***
 * A custom print function, which shall be extended in Task 2.1.
 * @param str Pointer to the message to send
 * @param ... printf-style parameters
 */
void uart_println_mutex(const char* str, ...)
{
    // TODO: You can insert your code here

    // ======= DO NOT EDIT BELOW THIS LINE =======
    static char print_buffer[256];

    va_list lst;
    va_start(lst, str);
    vsnprintf(print_buffer, 256, str, lst);
    str = print_buffer;
    while (*str)
    {
        while (!(EUSCI_A_CMSIS(UART_INTERFACE)->IFG & EUSCI_A_IFG_TXIFG));
        EUSCI_A_CMSIS(UART_INTERFACE)->TXBUF = *str;
        str++;
    }
    while (!(EUSCI_A_CMSIS(UART_INTERFACE)->IFG & EUSCI_A_IFG_TXIFG));
    EUSCI_A_CMSIS(UART_INTERFACE)->TXBUF = '\r';
    while (!(EUSCI_A_CMSIS(UART_INTERFACE)->IFG & EUSCI_A_IFG_TXIFG));
    EUSCI_A_CMSIS(UART_INTERFACE)->TXBUF = '\n';
    // ======= DO NOT EDIT ABOVE THIS LINE =======

    // TODO: You can insert your code here
}


/* -------------------------------------------------------------------------
 * Default FreeRTOS hooks
 * ------------------------------------------------------------------------- */

void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/* ------------------------------------------------------------------------- */

void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
}
/* ------------------------------------------------------------------------- */

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/* ------------------------------------------------------------------------- */

void *malloc( size_t xSize )
{
    /* There should not be a heap defined, so trap any attempts to call
    malloc. */
    Interrupt_disableMaster();
    for( ;; );
}
/* ------------------------------------------------------------------------- */


/*-----------------------------------------------------------*/

void vPreSleepProcessing(uint32_t ulExpectedIdleTime)
{

}
/*-----------------------------------------------------------*/

void vApplicationTickHook(void)
{
    /* This function will be called by each tick interrupt if
     configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
     added here, but the tick hook is called from an interrupt context, so
     code must not attempt to block, and only the interrupt safe FreeRTOS API
     functions can be used (those that end in FromISR()). */

    /* The full demo includes a software timer demo/test that requires
     prodding periodically from the tick interrupt. */
    //vTimerPeriodicISRTests();
    /* Call the periodic event group from ISR demo. */
    //vPeriodicEventGroupsProcessing();
    /* Use task notifications from an interrupt. */
    //xNotifyTaskFromISR();
    /* Use mutexes from interrupts. */
    //vInterruptSemaphorePeriodicTest();
}
/*-----------------------------------------------------------*/
