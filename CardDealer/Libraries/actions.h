#ifndef LIBRARIES_ACTIONS_H_
#define LIBRARIES_ACTIONS_H_

#include <screens.h>
#include <FreeRTOS.h>
#include <peripherials.h>
#include <inizializations.h>
#include <uart.h>
#include <queue.h>
#include <task.h>

#define MAX_PLAYERS 8

static int peoplePosition[MAX_PLAYERS];
static int contPeople;

static TaskHandle_t xTaskXHandle = NULL;

static const uint32_t uart_baudrate = 115200;

/*
 * Initialize uart and calls _hwInit which initialize
 * all the others pins, timers, ecc
 */
void init();

/* ---------- Main Functionalities ---------- */

/*
 * Function that using FreeRTOS manages the step motor and
 *  distance sensor in order to perform people detection
 *  (it will fill the vector peoplePos
 */
void peopleDetection();

static int numStartingCards = 0;
/*
 * Game selection is the part of the game where you
 *  must select how many cards each person should start with
 */
void gameSelection();

/*
 * Starts the distribution of cards
 */
void distributeCards();

/*
 * Starts the game
 */
void startGame();

/* ---------- Step Motor & Distance Sensor Utilities ---------- */

/*
 * task_mode is a parameter used to modify the behavior of the task
 */
typedef enum{
    GAME_MODE,
    RECOGNITION_MODE,
    STOP_MODE
}task_mode;

/*
 * Used as parameter for vTaskStepMotor to give instructions
 * about direction, number of steps and mode
 */
typedef struct stepParameter{
    bool move_cclockwise;
    int steps;
    task_mode mode;
}stepParameter;

/* -- Step Motor -- */

#define STEPS_360 2048
#define SM_RECOGNITION_MODE 1

/*
 * Function that is responsible for Step Motor movements
 */
void vTaskStepMotor(void *pvParameters);


/* -- Distance Sensor -- */

#define DS_MAX_DISTANCE_DETECT 20 // cm
#define DS_SAMPLE_RATE 125 // ms

#define dsMS_TO_SAMPLE( xTimeInMs ) ( xTimeInMs/DS_SAMPLE_RATE )   // return the number of sample taken in xTimeInMs

/*
 *  Function that is responsible for Distance Sensor measurements
 *     and measurements driven actions
 */
void vTaskDistanceSensor(void *pvParameters);


/* ---------- Homing Procedure Utilities ---------- */

static int homePosition = 0;

void clearHomePosition();  // sets homePosition to 0
int getHomeDistance();
void updateHomePosition(int steps); // updates homePosition relatively to steps

/*
 * Makes the structure head back to the starting position
 */
void resetPosition();

/* ---------- Cards Utilities ---------- */
#define NUM_TOTAL_CARDS 40
static unsigned int cards_left = NUM_TOTAL_CARDS;  // keep track of cards remaining

int getCardsLeft();
void cardRemoved();

/*
 * Function that manage the DC Motor in order
 *  to give a single card
 */
void giveOneCard();

/*
 * Notifies that the tank has no more cards available so
 *  it is needed a refill, the player can now press B1 to continue
 *  or end the game by pressing B2
 */
void cardsRefill();


/* ---------- Service Functions ---------- */

void incPeopleNumber();
void setNewPersonPosition(int steps);

int getPeoplePosition(int i);  // Returns the position in steps of the i-th person detected
int getPeopleNumber();

/*
 * Converts the degrees passed by parameter to steps,
 *   useful for Step Motor
 */
int degreesToSteps(int degrees);

#endif /* LIBRARIES_ACTIONS_H_ */
