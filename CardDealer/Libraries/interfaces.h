#ifndef LIBRARIES_INTERFACES_H_
#define LIBRARIES_INTERFACES_H_

#include "card_dealer.h"
#include <FreeRTOS.h>
#include <uart.h>
#include <queue.h>
#include <task.h>

#define MAX_PLAYERS 8

static int peoplePos[MAX_PLAYERS];
static int contPeople;

static TaskHandle_t xTaskXHandle = NULL;

// Queue
#define MAX_QUEUE_SIZE 10
xQueueHandle q1;

static const uint32_t uart_baudrate = 115200;

void initLibInterface(); // calls card_dealer init
void peopleDetection();

static int numStartingCards = 0;
void gameSelection();

int getPeopleNumber();
int getPeoplePosition(int i);
void incPeopleNumber();
void setNewPersonPosition(int steps);


typedef enum{
    GAME_MODE,
    RECOGNITION_MODE,
    STOP_MODE
}task_mode;

// Distance sensor
#define DS_RECOGNITION_MODE 1
#define DS_GAME_MODE 2
#define DS_MAX_DISTANCE_DETECT 20 // cm
void vTaskDistanceSensor(void *pvParameters);

// Step Motor
#define STEPS_360 2048
#define SM_RECOGNITION_MODE 1

typedef struct stepParameter{
    bool forward;
    int steps;
    task_mode mode;
}stepParameter;

void vTaskStepperMotor(void *pvParameters);
int degreesToSteps(int degrees);

// Emergency return
static int homePosition = 0;
void resetPosition();
int getHomePosition();
void updateHomePosition(int steps);
void clearHomePosition();

#define NUM_TOTAL_CARDS 40
static unsigned int cards_left = NUM_TOTAL_CARDS;  // goes down when give card
int getCardsLeft();
bool cardRemoved();

// Distribution
void distributeCards();

// Game
void startGame();

#endif /* LIBRARIES_INTERFACES_H_ */
