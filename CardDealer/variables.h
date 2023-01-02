bool end;
bool start;
bool pressed;

int count;
bool semaforo;

//state machine declaration
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

event_t event;

Graphics_Context g_sContext;
