#include "events.h"


void joystick_press(){
    event = JOYSTICK_PRESSED;
}

void joystick_up(){
    event = JOYSTICK_UP;
}
void joystick_down(){
    event = JOYSTICK_DOWN;
}

void button1_press(){
    event=BUTTON1_PRESSED;
}

void button2_press(){
    event=BUTTON2_PRESSED;
}

void end_arrive(){
    event=END_ARRIVED;
}

void start_arrive(){
    event=START_ARRIVED;
}

event_t getEvent(){
    return event;
}

void clearEvent(){
    event = NONE;
}



