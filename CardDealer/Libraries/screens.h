/*
 * screens.h
 *
 *  Created on: Jan 18, 2023
 *      Author: fed
 */

#ifndef LIBRARIES_SCREENS_H_
#define LIBRARIES_SCREENS_H_

#include <stdio.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <string.h>
#include "events.h"
#include <FreeRTOS.h>

static int number_player = 0;

void screen_start_game(Graphics_Context context);
void screen_scanning(Graphics_Context context);
void screen_selecting_cards(Graphics_Context context, int n_cards, int n_people);
void screen_center_string(Graphics_Context context, char *string);
void screen_card_distribution(Graphics_Context context, int n_people, int n_cards);
void screen_cards_refill(Graphics_Context context);
int get_number_player();
void screen_continue(Graphics_Context context);
void screen_restart(Graphics_Context context);





#endif /* LIBRARIES_SCREENS_H_ */
