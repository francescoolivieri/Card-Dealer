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

void peopleDetection(){
    BaseType_t result = pdPASS;

    // set-up display
    screen_center_string(g_sContext, "Scanning...");

    // SETUP TASK STEPPER MOTOR
    stepParameter SM_param;
    SM_param.forward = true;
    SM_param.steps = degreesToSteps(360);
    SM_param.mode = RECOGNITION_MODE;

    result = xTaskCreate(vTaskStepperMotor, "TaskStepperMotor", 1000, (void*) &SM_param, 1, NULL);
    if (result != pdPASS)
    {
        uart_println("Error creating TaskStepperMotor task.");
    }


    // SETUP TASK DISTANCE SENSOR
    task_mode DS_param = RECOGNITION_MODE;
    result = xTaskCreate(vTaskDistanceSensor, "TaskDistanceSensor", 1000, (void*) &DS_param, 1, &xTaskXHandle);
    if (result != pdPASS)
    {
        uart_println("Error creating TaskDistanceSensor task.");
    }

    while(getEvent() != END_ARRIVED && getEvent() != BUTTON2_PRESSED);

    /*
    char string[20];
    sprintf(string, "Players : %d \n PINUS", getPeopleNumber());
    Graphics_drawStringCentered(&g_sContext,(int8_t *)string, 8, 64, 70, OPAQUE_TEXT);
    */



    vTaskDelay(pdMS_TO_TICKS(50000));


}

void gameSelection(){
    Graphics_clearDisplay(&g_sContext);

    while(getEvent() != BUTTON1_PRESSED && getEvent() != BUTTON2_PRESSED){
        if(getEvent() == JOYSTICK_UP && (numStartingCards*contPeople)<cards_left){
            numStartingCards++;
            clearEvent();
        }else if(getEvent() == JOYSTICK_DOWN){
            if(numStartingCards > 0)
                numStartingCards--;
            clearEvent();
        }


        screen_selecting_cards(g_sContext, numStartingCards, getPeopleNumber());
    }
}

void distributeCards(){
    int i;
    Graphics_clearDisplay(&g_sContext);

    for(i=getPeopleNumber()-1 ; i>=0 && getEvent()!=BUTTON2_PRESSED ; i--){

        //screen_card_distribution(g_sContext, i, 3); //we need to do a function to make count cards
        screen_center_string(g_sContext, "distributing...");

        stepParameter SM_param;
        SM_param.steps = getHomePosition() - getPeoplePosition(i);
        SM_param.forward = false;
        SM_param.mode = GAME_MODE;

        vTaskStepperMotor((void *)&SM_param);

        int j;
        for(j=0 ; j<numStartingCards && getEvent() != BUTTON2_PRESSED; j++){
            giveOneCard();
        }

    }

    //resetPosition();
}

void startGame(){
    int i;
    Graphics_clearDisplay(&g_sContext);

    while(getCardsLeft()>0 && getEvent()!=BUTTON2_PRESSED){
        for(i=0 ; i<getPeopleNumber() && getEvent()!=BUTTON2_PRESSED ; i++){

            screen_card_distribution(g_sContext, i, 3); //we need to do a function to make count cards

            stepParameter pv;
            pv.steps = getPeoplePosition(i) - getHomePosition();
            pv.forward = true;
            pv.mode = GAME_MODE;

            vTaskStepperMotor((void *)&pv);

            task_mode DS_mode = GAME_MODE;
            vTaskDistanceSensor((void *)&DS_mode);

        }

        resetPosition();
    }

    if(getCardsLeft() == 0){
        screen_center_string(g_sContext, "cards finished!");
    }
}

void giveOneCard(){

    if(getCardsLeft()>0){
        float scaleFactor = 1000000;
        float spinForwardTime = 0.015;
        float spinBackwardsTime = 0.008;

        /*wheel spins forward to give the card*/
        Timer32_setCount(TIMER32_1_BASE, 24 * scaleFactor*spinForwardTime); // multiply by 24 -> 1 us *
        turnOnDispenserForward();
        while (Timer32_getValue(TIMER32_1_BASE) > 0); // Wait 0.015sec

        /*wheel spins backwards to align moved cards*/
        turnOnDispenserBackward();
        Timer32_setCount(TIMER32_1_BASE, 24 * scaleFactor*spinBackwardsTime);
        while (Timer32_getValue(TIMER32_1_BASE) > 0); // Wait 0.008sec

        /*motor is turned off*/
        turnOffDispenser();

        cardRemoved();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }else{
        button2_press();
    }

}

void resetPosition(){

    stepParameter pv;
     pv.steps = getHomePosition();
     pv.forward = false;
     pv.mode = STOP_MODE;


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
    task_mode DS_mode = (*((task_mode *)pvParameters));

    initTriggerDS();
    startDelayCaptureDS();

    while (1) {
        if((DS_mode==GAME_MODE && getEvent()==SKIP) || (DS_mode==GAME_MODE && getEvent()==BUTTON2_PRESSED))
           break;

        sendTrigPulseDS();
        //vTaskDelay(HZ / 8);     // delay
        vTaskDelay(pdMS_TO_TICKS(500));

        fflush(stdout);
        //vTaskDelay(pdMS_TO_TICKS(500));
        if (DS_isValueReady() == true)
        {
            fflush(stdout);
            int distCM = DS_getDistCM();
            DS_valueRead();
            printf("Distance: %i cm \n", distCM);


            if(DS_mode == RECOGNITION_MODE){ // recognition mode
                if(distCM<DS_MAX_DISTANCE_DETECT){
                    count++;

                    if(count == 4){
                        person_detected();

                        vTaskDelay(pdMS_TO_TICKS(500));
                    }
                }else{
                    count = 0;
                }
            }else if(DS_mode == GAME_MODE){
                if(distCM<DS_MAX_DISTANCE_DETECT){
                    count++;

                    if(count == 24){
                        skip();
                    }
                }else{
                    if(count > 12){
                        give_card();  // can be removed
                        giveOneCard();
                    }

                    count = 0;
                }
            }
        }
    }

    if(getEvent()==SKIP) clearEvent();
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
    //uart_println("Start TaskStepperMotor.");

    int velocity = 300000;
    while (cont < pv.steps && (getEvent()!=BUTTON2_PRESSED || pv.mode == STOP_MODE))
    {
        makeStep(pv.forward);

        vTaskDelay(HZ / velocity);     // delay
        cont++;

        if(pv.forward)
            updateHomePosition(1);
        else
            updateHomePosition(-1);

        if(getEvent() == PERSON_DETECTED && pv.mode == RECOGNITION_MODE){
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

    if(getEvent()!=BUTTON2_PRESSED)
            end_arrive();


    if(pv.mode == RECOGNITION_MODE){
        vTaskDelete(xTaskXHandle);

        vTaskDelete(NULL);
    }
}

/* ----- Service Routines ----- */

int getCardsLeft(){
    return cards_left;
}

bool cardRemoved(){
    if(cards_left > 0){
        cards_left--;
        return true;
    }else{
        return false;
    }
}

int degreesToSteps(int degrees){
    return (STEPS_360/(360 / degrees));
}

int getHomePosition(){
    return homePosition;
}

void updateHomePosition(int steps){
    homePosition += steps;

    homePosition = homePosition ;
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
