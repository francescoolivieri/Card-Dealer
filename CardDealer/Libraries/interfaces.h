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
int peopleDetection();

static int numCards = 0;
void gameSelection();

int getPeopleNumber();
int* getPeoplePositions();
void incPeopleNumber();
void setNewPersonPosition(int steps);

// Distance sensor
#define DS_RECOGNITION_MODE 1
#define DS_MAX_DISTANCE_DETECT 20 // cm
void vTaskDistanceSensor(void *pvParameters);

// Step Motor
#define STEPS_360 512
void vTaskStepperMotor(void *pvParameters);

#endif /* LIBRARIES_INTERFACES_H_ */
