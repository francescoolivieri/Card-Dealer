/*
 * interfaces.c
 *
 *  Created on: 5 gen 2023
 *      Author: Francesco Olivieri
 */
#include "interfaces.h"

void initLibInterface(){
    // Initialize Queuexcx
    int msg;
    q1 = xQueueCreate(MAX_QUEUE_SIZE, sizeof(msg));
    if (!q1)
        printf("Problems opening queue\n");

    // Initialize UART
    uart_init(uart_baudrate);
    contPeople = 0;

    initCardDealer();
}

int peopleDetection(){
    BaseType_t result = pdPASS;

    // set-up display
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Scanning people",AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);

    // SETUP TASK STEPPER MOTOR
    int rotationDegrees = 360;

    result = xTaskCreate(vTaskStepperMotor, "TaskStepperMotor", 1000, (void*) &rotationDegrees, 1, NULL);
    if (result != pdPASS)
    {
        uart_println("Error creating TaskStepperMotor task.");
        return -1;
    }


    // SETUP TASK DISTANCE SENSOR
    int pcParameters2 = 1;
    result = xTaskCreate(vTaskDistanceSensor, "TaskDistanceSensor", 1000, (void*) &pcParameters2, 1, &xTaskXHandle);
    if (result != pdPASS)
    {
        uart_println("Error creating TaskDistanceSensor task.");
        return -1;
    }

    // Start the tasks
    vTaskStartScheduler();

    // NOTIFY END

    return contPeople;
}

void gameSelection(){

    while(getEvent() != BUTTON1_PRESSED){
        if(getEvent() == JOYSTICK_UP){
            numCards++;
            clearEvent();
        }else if(getEvent() == JOYSTICK_DOWN){
            numCards++;
            clearEvent();
        }

        char string[10];
        sprintf(string, "%d", numCards);
        Graphics_drawStringCentered(&g_sContext,(int8_t *)string, 8, 64, 70, OPAQUE_TEXT);
    }
}


/* ----- FreeRTOS Tasks ----- */

/**
 * TaskDistanceSensor
 * Reads distance
 * @param mode
 */
void vTaskDistanceSensor(void *pvParameters)
{
    uart_println("Start TaskDistanceSensor.");
    int count = 0;

    initTriggerDS();
    startDelayCaptureDS();

    while (1) {
        sendTrigPulseDS();
        //vTaskDelay(HZ / 4);     // delay

        vTaskDelay(pdMS_TO_TICKS(500));
        if (DS_isValueReady() == true)
        {
            int distCM = DS_getDistCM();
            DS_valueRead();
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

/**
 * TaskStepperMotor
 * Make the motor turn
 * @param Degrees of rotation
 */
//m1_1_1s//
void vTaskStepperMotor(void *pvParameters)
{
    int cont = 0;
    uart_println("Start TaskStepperMotor.");

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
            if(getPeopleNumber() < 8){
                setNewPersonPosition(cont-1);

                vTaskSuspend(xTaskXHandle);
                turnOnBuzzer();
                vTaskDelay(pdMS_TO_TICKS(30000));
                turnOffBuzzer();
                vTaskResume(xTaskXHandle);
            }
        }
    }

    vTaskDelete(xTaskXHandle);
}

int getPeopleNumber(){
    return contPeople;
}

int* getPeoplePositions(){
    return peoplePos;
}

void incPeopleNumber(){
    if(contPeople < MAX_PLAYERS)
        contPeople++;
}
void setNewPersonPosition(int steps){
    if(contPeople < MAX_PLAYERS){
        peoplePos[contPeople] = steps;
        incPeopleNumber();
    }

}
