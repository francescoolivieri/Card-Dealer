#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <stdio.h>

typedef enum{
    WAITING,
    SCAN,
    CARTE,
    DISTRIBUZIONE,
    GAME
}State_t;

typedef enum{
    NONE,
    JOYSTICK_PRESSED,
    END_ARRIVED,
    START_ARRIVED,
    BUTTON1_PRESSED,
    BUTTON2_PRESSED,
}event_t;

extern event_t event;

typedef struct{
    State_t state;
    void (*state_function)(void);
} StateMachine_t;

extern State_t current_state;

Graphics_Context g_sContext;

bool end;
bool start;
extern bool pressed;

extern int count;
extern bool semaforo;
