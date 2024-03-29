#include "actions.h"

void init(){

    // Initialize UART
    uart_init(uart_baudrate);
    contPeople = 0;
    cards_left = NUM_TOTAL_CARDS;

    _hwInit();
}

/* ---------- Main Functionalities ---------- */

void peopleDetection(){
    BaseType_t result = pdPASS;

    // set-up display
    screen_center_string(g_sContext, "Scanning...");

    // SETUP TASK STEP MOTOR
    stepParameter SM_param;
    SM_param.move_cclockwise = true;
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
        SM_param.move_cclockwise = false;
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
            pv.move_cclockwise = true;
            pv.mode = GAME_MODE;

            vTaskStepMotor((void *)&pv);

            task_mode DS_mode = GAME_MODE;
            vTaskDistanceSensor((void *)&DS_mode);

        }

        resetPosition();
    }

}

/* ---------- Step Motor & Distance Sensor Utilities ---------- */

void vTaskDistanceSensor(void *pvParameters)
{
    uart_println("Start TaskDistanceSensor.");
    int count = 0;
    int count_out = 0;      // variable used to handle random spikes given by the DS
    task_mode DS_mode = (*((task_mode *)pvParameters));

    while (1) {
        if((DS_mode==GAME_MODE && getEvent()==SKIP) || (DS_mode==GAME_MODE && getEvent()==BUTTON2_PRESSED)) // cases where the distance sensor need to be stopped
           break;

        DS_sendTrigger();
        //vTaskDelay(HZ / 8);     // delay
        vTaskDelay(pdMS_TO_TICKS(DS_SAMPLE_RATE)); // wait for the defined sample_rate

        if (DS_hasNewMeasure() == true)
        {
            int distCM = DS_getMeasure();
            printf("Distance: %i cm \n", distCM);
            fflush(stdout);


            if(DS_mode == RECOGNITION_MODE){ // recognition mode
                if(distCM<DS_MAX_DISTANCE_DETECT && distCM > 5){
                    count++;

                    if(count == dsMS_TO_SAMPLE(500)){
                        person_detected();

                    }
                }else{
                    count = 0;
                }
            }else if(DS_mode == GAME_MODE){ // game mode
                if((distCM<DS_MAX_DISTANCE_DETECT && distCM>=5) || count_out < 1){ // if I sample only one value out of bound I keep going since it may be an error of the sensor
                    if(distCM < DS_MAX_DISTANCE_DETECT)
                        count++;
                    else
                        count_out++;


                    if(count == dsMS_TO_SAMPLE(1500)){   // hand in front of sensor for 1.5 sec -> skip
                        skip();
                    }
                }else{
                    count_out = 0;

                    if(count > dsMS_TO_SAMPLE(500)){ // hand in front of sensor for AT LEAST 0.5 sec -> give_card
                        giveOneCard();

                        if(getCardsLeft() == 0)
                            cardsRefill();

                        screen_card_distribution(g_sContext, get_number_player(), getCardsLeft());
                    }

                    count = 0;
                }
            }
        }
    }

    if(getEvent()==SKIP) clearEvent();
}



void vTaskStepMotor(void *pvParameters)
{
    stepParameter pv = (*((stepParameter *)pvParameters));
    int cont = 0;

    uart_println("Start TaskStepperMotor.");

    while (cont < pv.steps && (getEvent()!=BUTTON2_PRESSED || pv.mode == STOP_MODE))
    {
        SM_makeStep(pv.move_cclockwise);

        vTaskDelay(pdMS_TO_TICKS(10));     // delay
        cont++;

        if(pv.move_cclockwise)
            updateHomePosition(1);
        else
            updateHomePosition(-1);

        if(getEvent() == PERSON_DETECTED && pv.mode == RECOGNITION_MODE){
            clearEvent();
            if(getPeopleNumber() < 8){
                setNewPersonPosition(cont-1);

                vTaskSuspend(xTaskXHandle);
                turnOnBuzzer();
                vTaskDelay(pdMS_TO_TICKS(1000));  // buzz for that time
                turnOffBuzzer();
                vTaskResume(xTaskXHandle);
            }
        }
    }

    if(getEvent()!=BUTTON2_PRESSED)
            end_arrive();   // notifies that it is arrived at destination


    if(pv.mode == RECOGNITION_MODE){
        vTaskDelete(xTaskXHandle);  // delete distance sensor task

        vTaskDelete(NULL);  // delete current task
    }
}

/* ---------- Homing Procedure Utilities ---------- */

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


void resetPosition(){

    stepParameter pv;
     pv.steps = getHomeDistance();
     pv.move_cclockwise = false;
     pv.mode = STOP_MODE;


     vTaskStepMotor( (void*) &pv );

    clearHomePosition();
}

/* ---------- Cards Utilities ---------- */
int getCardsLeft(){
    return cards_left;
}

void cardRemoved(){
    if(cards_left > 0)
        cards_left--;
}

void cardsRefill(){
    screen_cards_refill(g_sContext);

    clearEvent();
    while (getEvent() != BUTTON1_PRESSED && getEvent() != BUTTON2_PRESSED) ;

    Graphics_clearDisplay(&g_sContext);

    if(getEvent() == BUTTON1_PRESSED)
        cards_left = NUM_TOTAL_CARDS;
}

void giveOneCard(){

    if(getCardsLeft()>0){
        float scaleFactor = 1000000;
        float spinForwardTime = 0.013;
        float spinBackwardsTime = 0.01;

        /*wheel spins forward to give the card*/
        Timer32_setCount(TIMER32_1_BASE, 24 * scaleFactor * spinForwardTime);
        DCM_moveForward();
        while (Timer32_getValue(TIMER32_1_BASE) > 0); // Wait 0.015sec

        /*wheel spins backwards to align moved cards*/
        DCM_moveBackward();
        Timer32_setCount(TIMER32_1_BASE, 24 * scaleFactor*spinBackwardsTime);
        while (Timer32_getValue(TIMER32_1_BASE) > 0); // Wait 0.008sec

        /*motor is turned off*/
        DCM_turnOff();

        cardRemoved();
        vTaskDelay(pdMS_TO_TICKS(500));
    }else{
        button2_press();
    }

}

/* ---------- Service Functions ---------- */

int degreesToSteps(int degrees){
    return (STEPS_360/(360 / degrees));
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
