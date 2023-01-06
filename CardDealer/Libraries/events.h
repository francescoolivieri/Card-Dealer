#ifndef LIBRARIES_EVENTS_H_
#define LIBRARIES_EVENTS_H_

typedef enum{
    NONE,
    JOYSTICK_PRESSED,
    JOYSTICK_UP,
    JOYSTICK_DOWN,
    END_ARRIVED,
    START_ARRIVED,
    BUTTON1_PRESSED,
    BUTTON2_PRESSED
}event_t;

static event_t event = NONE;

void joystick_press();
void joystick_up();
void joystick_down();
void button1_press();
void button2_press();
void end_arrive();
void start_arrive();

event_t getEvent();
void clearEvent();

#endif
