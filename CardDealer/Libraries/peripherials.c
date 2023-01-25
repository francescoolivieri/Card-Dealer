#include "peripherials.h"

/* ---------- Step Motor ---------- */

static int step_count = 0;
void SM_makeStep(bool move_forward){
    if(move_forward){
        step_count++;
        step_count = step_count%4;
     }else{
        if(step_count == 0)
            step_count = 3;
        else
            step_count--;
    }

    switch(step_count){
        case 0:
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN7);
            GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0);
            GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN2);
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);

            break;
        case 1:
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN7);
            GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN0);
            GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN2);
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);

            break;
        case 2:
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN7);
            GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0);
            GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN2);
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);

            break;
        case 3:
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN7);
            GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0);
            GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN2);
            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6);

            break;
    }

}

/* ---------- Distance Sensor ---------- */

void DS_sendTrigger(){ // triggers Pin 5.6 for 1 us
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN6); // 5.6 trigger pin of distance sensor
    Timer32_setCount(TIMER32_0_BASE, 24 * 10); // multiply by 24 -> 1 us * 10 -> 1 us
    while (Timer32_getValue(TIMER32_0_BASE) > 0) // Wait 10us
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN6);
}

/* ---------- DC Motor ---------- */

void DCM_moveForward(){
    GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
}

void DCM_moveBackward(){
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);
}

void DCM_turnOff(){
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1);
}




