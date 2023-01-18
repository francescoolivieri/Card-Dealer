/*
 * screens.c
 *
 *  Created on: Jan 18, 2023
 *      Author: fed
 */


#include "screens.h"

void screen_start_game(Graphics_Context context){

    Graphics_clearDisplay(&context);

    Graphics_drawStringCentered(&context,(int8_t *)"Press 2   =>", AUTO_STRING_LENGTH, 64, 10, OPAQUE_TEXT);
    Graphics_drawStringCentered(&context,(int8_t *)"to Stop ", AUTO_STRING_LENGTH, 64, 20, OPAQUE_TEXT);


    Graphics_drawStringCentered(&context,(int8_t *)"BUTTON1   =>", AUTO_STRING_LENGTH, 64, 60, OPAQUE_TEXT);
    Graphics_drawStringCentered(&context,(int8_t *)"to Start", AUTO_STRING_LENGTH, 64, 70, OPAQUE_TEXT);
}

void screen_scanning(Graphics_Context context){

    Graphics_clearDisplay(&context);
    Graphics_drawStringCentered(&context, (int8_t*) "Scanning people",AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);

}

void screen_selecting_cards(Graphics_Context context, int n_cards, int n_people){

    char string[20];
    sprintf(string, "%d people", n_people);
    Graphics_drawStringCentered(&context,(int8_t *) string, AUTO_STRING_LENGTH, 64, 20, OPAQUE_TEXT);
    Graphics_drawStringCentered(&context,(int8_t *) " were detected", AUTO_STRING_LENGTH, 64, 33, OPAQUE_TEXT);

    sprintf(string, " %d ", n_cards);
    Graphics_drawStringCentered(&context,(int8_t *) "number of cards:", AUTO_STRING_LENGTH, 64, 70, OPAQUE_TEXT);
    Graphics_drawStringCentered(&context, (int8_t*) string, AUTO_STRING_LENGTH, 64, 83, OPAQUE_TEXT);

}

void screen_center_string(Graphics_Context context, char *string){
    Graphics_drawStringCentered(&context, (int8_t*) string, 15, 64, 70, OPAQUE_TEXT);
}

void screen_card_distribution(Graphics_Context context, int n_people, int n_cards){

    char string[20];
    sprintf(string, "Player: %d", n_people);

    Graphics_drawStringCentered(&context,(int8_t *) string, AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);

    sprintf(string, "Cards left: X/30");
    Graphics_drawStringCentered(&context,(int8_t *) string, AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
}


