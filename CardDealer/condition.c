#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include <condition.h>
#include <varaibles.h>

int x=0;
void joystick_press(){
    if (pressed==true){
        event=JOYSTICK_PRESSED;
        pressed=false;
    }
}


void button1_press(){
    if (x==1){
           event=BUTTON1_PRESSED;
       }
}

void button2_press(){
    if (x==2){
             event=BUTTON2_PRESSED;
         }
}

void end_arrive(){
    if(end==true){
        event=END_ARRIVED;
    }
}

void start_arrive(){
    if(start==true){
           event=START_ARRIVED;
       }
}





