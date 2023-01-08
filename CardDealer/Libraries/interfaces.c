/*
 * interfaces.c
 *
 *  Created on: 5 gen 2023
 *      Author: Francesco Olivieri
 */
#include "interfaces.h"

void initLibInterface(){
    // Initialize Queuexcx
    /*
    int msg;
    q1 = xQueueCreate(MAX_QUEUE_SIZE, sizeof(msg));
    if (!q1)
        printf("Problems opening queue\n");
        */

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
    stepParameter pv;
    pv.forward = true;
    pv.steps = degreesToSteps(360);

    result = xTaskCreate(vTaskStepperMotor, "TaskStepperMotor", 1000, (void*) &pv, 1, NULL);
    if (result != pdPASS)
    {
        uart_println("Error creating TaskStepperMotor task.");
        return -1;
    }


    // SETUP TASK DISTANCE SENSOR
    int pcParameters2 = DS_RECOGNITION_MODE;
    result = xTaskCreate(vTaskDistanceSensor, "TaskDistanceSensor", 1000, (void*) &pcParameters2, 1, &xTaskXHandle);
    if (result != pdPASS)
    {
        uart_println("Error creating TaskDistanceSensor task.");
        return -1;
    }

    // Start the tasks
    vTaskStartScheduler();

    // NOTIFY END is ugly

    return contPeople;
}

void gameSelection(){

    while(getEvent() != BUTTON1_PRESSED && getEvent() != BUTTON2_PRESSED){
        if(getEvent() == JOYSTICK_UP){
            numStartingCards++;
            clearEvent();
        }else if(getEvent() == JOYSTICK_DOWN){
            numStartingCards++;
            clearEvent();
        }

        char string[10];
        sprintf(string, "%d", numStartingCards);
        Graphics_drawStringCentered(&g_sContext,(int8_t *)string, 8, 64, 70, OPAQUE_TEXT);
    }
}

void distributeCards(){
    int i;

    for(i=getPeopleNumber()-1 ; i>=0 && getEvent()!=BUTTON2_PRESSED ; i--){
        stepParameter pv;
        pv.steps = getPeoplePosition(i);
        pv.forward = false;
        vTaskStepperMotor((void*)&pv);

        int j;
        for(j=0 ; j<numStartingCards ; j++){
            //give card funct
        }
    }

    resetPosition();
}

void startGame(){
    int i;

    while(getCardsLeft()>0 && getEvent()!=BUTTON2_PRESSED){
        for(i=0 ; i<getPeopleNumber() && getEvent()!=BUTTON2_PRESSED ; i++){
            stepParameter pv;
            pv.steps = getPeoplePosition(i);
            pv.forward = false;
            vTaskStepperMotor((void*)&pv);

            int DS_mode = DS_GAME_MODE;
            vTaskDistanceSensor((void*)&DS_mode);
        }

        resetPosition();
    }

    if(getCardsLeft() == 0){
        char string[20] = "Finie le carte caro";
        Graphics_drawStringCentered(&g_sContext, (int8_t*) string, 15, 64, 70, OPAQUE_TEXT);
    }
}

void resetPosition(){

    stepParameter pv;
     pv.steps = getHomePosition();
     pv.forward = false;

    vTaskStepperMotor( (void*) &pv );

    clearHomePosition();
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
    int DS_mode = (*((int *)pvParameters));

    initTriggerDS();
    startDelayCaptureDS();

    while (1 && !(DS_mode!=DS_GAME_MODE && getEvent()!=SKIP)) {
        sendTrigPulseDS();
        vTaskDelay(HZ / 8);     // delay

        //vTaskDelay(pdMS_TO_TICKS(500));
        if (DS_isValueReady() == true)
        {
            int distCM = DS_getDistCM();
            DS_valueRead();
            printf("Distance: %i cm \n", distCM);

            if(DS_mode == DS_RECOGNITION_MODE){ // recognition mode
                if(distCM<DS_MAX_DISTANCE_DETECT){
                    count++;

                    if(count == 4){
                        /*
                        int msg = 1;
                        xQueueSend(q1, (void *)msg, (portTickType)0); // EVENT?!
                        */
                        person_detected();

                        vTaskDelay(pdMS_TO_TICKS(500));
                    }
                }else{
                    count = 0;
                }
            }else if(DS_mode == DS_GAME_MODE){
                if(distCM<DS_MAX_DISTANCE_DETECT){
                    count++;

                    if(count == 24){
                        skip();
                    }
                }else{
                    if(count > 12){
                        give_card();
                    }

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
    stepParameter pv = (*((stepParameter *)pvParameters));
    int cont = 0;
    uart_println("Start TaskStepperMotor.");

    int velocity = 80000;
    while (cont < pv.steps && getEvent()!=BUTTON2_PRESSED)
    {
        makeStep(pv.forward);
        vTaskDelay(HZ / velocity);     // delay
        cont++;

        if(pv.forward)
            updateHomePosition(1);
        else
            updateHomePosition(-1);

        //int msg;
        //if(xQueueReceive(q1, &(msg), 0)){  // EVENT
        if(getEvent() == PERSON_DETECTED){
            clearEvent();
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
    vTaskEndScheduler();
}

/* ----- Service Routines ----- */

int getCardsLeft(){
    return cards_left;
}

int degreesToSteps(int degrees){
    return (STEPS_360/(360 / degrees));
}

int getHomePosition(){
    return homePosition;
}

void updateHomePosition(int steps){
    homePosition += steps;

    homePosition = homePosition % STEPS_360;
}

void clearHomePosition(){
    homePosition = 0;
}

int getPeopleNumber(){
    return contPeople;
}

int getPeoplePosition(int i){
    if(i < getPeopleNumber())
        return peoplePos[i];
    else
        return -1;
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
