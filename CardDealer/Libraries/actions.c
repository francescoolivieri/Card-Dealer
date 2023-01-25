#include <actions.h>


void init(){

    // Initialize UART
    uart_init(uart_baudrate);
    contPeople = 0;
    cards_left = NUM_TOTAL_CARDS;

    _hwInit();
}

void peopleDetection(){
    BaseType_t result = pdPASS;

    // set-up display
    screen_center_string(g_sContext, "Scanning...");

    // SETUP TASK STEP MOTOR
    stepParameter SM_param;
    SM_param.forward = true;
    SM_param.steps = degreesToSteps(360);
    SM_param.mode = RECOGNITION_MODE;

    result = xTaskCreate(vTaskStepMotor, "TaskStepperMotor", 1000, (void*) &SM_param, 1, NULL);
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

    while(getEvent() != END_ARRIVED && getEvent() != BUTTON2_PRESSED); // wait for end of scanning

}

void gameSelection(){
    Interrupt_enableInterrupt(INT_ADC14);  // start detecting joystick movements

    Graphics_clearDisplay(&g_sContext);
    numStartingCards = 0;

    while(getEvent() != BUTTON1_PRESSED && getEvent() != BUTTON2_PRESSED){
        if(getEvent() == JOYSTICK_UP && (numStartingCards*contPeople)<cards_left){  // increment
            numStartingCards++;
            clearEvent();
        }else if(getEvent() == JOYSTICK_DOWN){ // decrement
            if(numStartingCards > 0)
                numStartingCards--;
            clearEvent();
        }

        screen_selecting_cards(g_sContext, numStartingCards, getPeopleNumber());
    }

    Interrupt_disableInterrupt(INT_ADC14);  // stop detecting joystick movements
}

void distributeCards(){
    int i;
    Graphics_clearDisplay(&g_sContext);

    for(i=getPeopleNumber()-1 ; i>=0 && getEvent()!=BUTTON2_PRESSED ; i--){


        screen_center_string(g_sContext, "distributing...");

        stepParameter SM_param;
        SM_param.steps = getHomeDistance() - getPeoplePosition(i); // distance from home pos - distance between home pos and the i-th person
        SM_param.forward = false;
        SM_param.mode = GAME_MODE;

        vTaskStepMotor((void *)&SM_param);

        int j;
        for(j=0 ; j<numStartingCards && getEvent() != BUTTON2_PRESSED; j++){  // give cards

            if(getCardsLeft() > 0){
                giveOneCard();
            }else{
                cardsRefill();
                j--;
            }
        }

    }

}

void startGame(){
    int i;
    Graphics_clearDisplay(&g_sContext);

    while(getCardsLeft()>0 && getEvent()!=BUTTON2_PRESSED){
        for(i=0 ; i<getPeopleNumber() && getEvent()!=BUTTON2_PRESSED ; i++){

            screen_card_distribution(g_sContext, i+1, getCardsLeft()); //we need to do a function to make count cards

            stepParameter pv;
            pv.steps = getPeoplePosition(i) - getHomeDistance();
            pv.forward = true;
            pv.mode = GAME_MODE;

            vTaskStepMotor((void *)&pv);

            task_mode DS_mode = GAME_MODE;
            vTaskDistanceSensor((void *)&DS_mode);

        }

        resetPosition();
    }

}


void cardsRefill(){
    screen_cards_refill(g_sContext);

    clearEvent();
    while (getEvent() != BUTTON1_PRESSED && getEvent() != BUTTON2_PRESSED) ;
}

void giveOneCard(){

    if(getCardsLeft()>0){
        float scaleFactor = 1000000;
        float spinForwardTime = 0.075;
        float spinBackwardsTime = 0.05;

        /*wheel spins forward to give the card*/
        Timer32_setCount(TIMER32_1_BASE, 24 * scaleFactor * spinForwardTime); // multiply by 24 -> 1 us *
        DCM_moveForward();
        while (Timer32_getValue(TIMER32_1_BASE) > 0); // Wait 0.015sec

        /*wheel spins backwards to align moved cards*/
        DCM_moveBackward();
        Timer32_setCount(TIMER32_1_BASE, 24 * scaleFactor*spinBackwardsTime);
        while (Timer32_getValue(TIMER32_1_BASE) > 0); // Wait 0.008sec

        /*motor is turned off*/
        DCM_turnOff();

        cardRemoved();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }else{
        button2_press();
    }

}

void resetPosition(){

    stepParameter pv;
     pv.steps = getHomeDistance();
     pv.forward = false;
     pv.mode = STOP_MODE;


     vTaskStepMotor( (void*) &pv );

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
    int count_out = 0;
    task_mode DS_mode = (*((task_mode *)pvParameters));

    while (1) {
        if((DS_mode==GAME_MODE && getEvent()==SKIP) || (DS_mode==GAME_MODE && getEvent()==BUTTON2_PRESSED)) // cases where the distance sensor need to be stopped
           break;

        DS_sendTrigger();
        //vTaskDelay(HZ / 8);     // delay
        vTaskDelay(pdMS_TO_TICKS(500));

        if (DS_hasNewMeasure() == true)
        {
            fflush(stdout);
            int distCM = DS_getMeasure();
            printf("Distance: %i cm \n", distCM);
            fflush(stdout);


            if(DS_mode == RECOGNITION_MODE){ // recognition mode
                if(distCM<DS_MAX_DISTANCE_DETECT && distCM > 5){
                    count++;

                    if(count == 4){
                        person_detected();

                        vTaskDelay(pdMS_TO_TICKS(500));
                    }
                }else{
                    count = 0;
                }
            }else if(DS_mode == GAME_MODE){ // game mode
                if((distCM<DS_MAX_DISTANCE_DETECT && distCM>=5) || count_out < 1){ // had to do this due to sensor inaccuracy
                    if(distCM < DS_MAX_DISTANCE_DETECT)
                        count++;
                    else
                        count_out++;


                    if(count == 24){
                        skip();
                    }
                }else{
                    count_out = 0;

                    if(count > 8){
                        giveOneCard();

                        if(getCardsLeft() == 0)
                            cardsRefill();
                        else
                            screen_card_distribution(g_sContext, get_number_player(), getCardsLeft());
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
void vTaskStepMotor(void *pvParameters)
{
    stepParameter pv = (*((stepParameter *)pvParameters));
    int cont = 0;
    //uart_println("Start TaskStepperMotor.");

    int velocity = 300000;
    while (cont < pv.steps && (getEvent()!=BUTTON2_PRESSED || pv.mode == STOP_MODE))
    {
        SM_makeStep(pv.forward);

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

void cardRemoved(){
    if(cards_left > 0)
        cards_left--;
}

int degreesToSteps(int degrees){
    return (STEPS_360/(360 / degrees));
}

int getHomeDistance(){
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
        return peoplePosition[i];
    else
        return -1;
}

void incPeopleNumber(){
    if(contPeople < MAX_PLAYERS)
        contPeople++;
}
void setNewPersonPosition(int steps){
    if(contPeople < MAX_PLAYERS){
        peoplePosition[contPeople] = steps;
        incPeopleNumber();
    }

}
