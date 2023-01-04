#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include <action.h>
#include <variables.h>

void scan_players(){
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext,
                                            (int8_t *)"Scanning people",
                                            AUTO_STRING_LENGTH,
                                            64,
                                            30,
                                            OPAQUE_TEXT);
    peopleDetection();
    event=END_ARRIVED;
}

void select_card();

void give_card(){
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext,
                                                (int8_t *)"Scanning people",
                                                AUTO_STRING_LENGTH,
                                                64,
                                                30,
                                                OPAQUE_TEXT);

    //fra aggiungi funzione di give cards, quando ha finito restituisce valore start che trover� handler
}




void game_play(){
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext,
                                              (int8_t *)"In game",
                                              AUTO_STRING_LENGTH,
                                              64,
                                              30,
                                              OPAQUE_TEXT);
}

void give1_card(){

    //aggiungi funzione di gice 1 card
}


void end_game(){
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext,
                                            (int8_t *)"Waiting to start :",
                                            AUTO_STRING_LENGTH,
                                            64,
                                            30,
                                            OPAQUE_TEXT);
}



void select_card(){
        Graphics_clearDisplay(&g_sContext);
        Graphics_drawStringCentered(&g_sContext,
                                                (int8_t *)"Numero di carte: ",
                                                AUTO_STRING_LENGTH,
                                                64,
                                                30,
                                                OPAQUE_TEXT);
}




