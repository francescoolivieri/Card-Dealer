#include "interfaces.h"

void fn_IDLE();
void fn_WAITING();
void fn_SCAN();
void fn_GAME_SELECTION();
void fn_DISTRIBUTION();
void fn_GAME();
void fn_EMERGENCY_STOP();

typedef enum{
    IDLE,
    WAITING,
    SCAN,
    GAME_SELECTION,
    CARD_DISTRIBUTION,
    GAME,
    EMERGENCY_STOP
}State_t;

typedef struct{
    State_t state;
    void (*state_function)(void);
} StateMachine_t;

StateMachine_t fsm[]={
                      {IDLE, fn_IDLE},
                      {WAITING, fn_WAITING},
                      {SCAN, fn_SCAN},
                      {GAME_SELECTION, fn_GAME_SELECTION},
                      {CARD_DISTRIBUTION, fn_DISTRIBUTION},
                      {GAME, fn_GAME},
                      {EMERGENCY_STOP, fn_EMERGENCY_STOP}
};

State_t current_state;
bool state_transition = false;

Graphics_Context g_sContext;

/***
 * Function for printing to the UART terminal. This function will be modified in Task 2.1
 * Without modification it works exactly like uart_println()
 */
void uart_println_mutex(const char* str, ...);
void vTaskMain(void *pvParameters);


/*-----------------------------------------------------------*/
int main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer


    current_state = IDLE;   // set fsm state to IDLE

    BaseType_t result = pdPASS;

    result = xTaskCreate(vTaskMain, "TaskMain", 1000, NULL, 1, NULL);
    if (result != pdPASS)   printf("Can't run main task\n");

    // Start the tasks
    vTaskStartScheduler();

    /* The following line should never be reached.
     * Otherwise there was insufficient FreeRTOS heap
     * memory available for the task to be created.
     */
    return 0;
}
/*-----------------------------------------------------------*/

void vTaskMain(void *pvParameters)
{
    current_state = IDLE;   // set fsm state to IDLE

    while (1)
    {
        (*fsm[current_state].state_function)();
    }
}

void fn_IDLE(){
    initLibInterface();  // initialize peripherals, clocks, graphics, timers and adc
    g_sContext = getGraphicsContext();

    //Interrupt_disableInterrupt(ADC14_IRQHandler);
    Interrupt_disableInterrupt(INT_ADC14);

    current_state = WAITING;
    state_transition = true;
}

void fn_WAITING(){
    if(state_transition){
        Graphics_drawStringCentered(&g_sContext,(int8_t *)"Waiting State,\n press Joystick", AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);
        clearEvent();
        state_transition = false;
    }

    if(getEvent() == BUTTON1_PRESSED){
        Graphics_clearDisplay(&g_sContext);
        current_state=SCAN;
        state_transition = true;
    }
}

void fn_SCAN(){
    peopleDetection();

    if(getEvent() == BUTTON2_PRESSED)
        current_state = EMERGENCY_STOP;
    else
        current_state = GAME_SELECTION;
}

void fn_GAME_SELECTION(){
    /*
    if(state_transition){
        // implement num card sel in interfaces lib
        state_transition = false;
    }


    if(getEvent() == BUTTON1_PRESSED){
        current_state = CARD_DISTRIBUTION;
        state_transition = true;
    }*/
    Interrupt_enableInterrupt(INT_ADC14);
    gameSelection();


    if(getEvent() == BUTTON2_PRESSED)
            current_state = EMERGENCY_STOP;
    else
            current_state = CARD_DISTRIBUTION;
    state_transition = true;
}

// EXPAND WORK
void fn_DISTRIBUTION(){
    clearEvent();
    distributeCards();

    if(getEvent()==BUTTON2_PRESSED){
        current_state=EMERGENCY_STOP;
    }else{
        current_state = GAME;
    }
}

void fn_GAME(){
    startGame();

    if(getEvent()==BUTTON2_PRESSED){
        current_state=EMERGENCY_STOP;
    }else{
        current_state=IDLE;
    }
}

void fn_EMERGENCY_STOP(){
    Graphics_clearDisplay(&g_sContext);
    resetPosition();

    current_state = IDLE;
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
