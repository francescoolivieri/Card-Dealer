#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include <condition.h>
#include <action.h>
#include <varaibles.h>



static uint16_t resultsBuffer[2];

//adc initialization
void _adcInit(){
        GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION);
        GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);

        ADC14_enableModule();
        ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_64, ADC_DIVIDER_8, 0);

        ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);
        ADC14_configureConversionMemory(ADC_MEM0,
                ADC_VREFPOS_AVCC_VREFNEG_VSS,
                ADC_INPUT_A15, ADC_NONDIFFERENTIAL_INPUTS);

        ADC14_configureConversionMemory(ADC_MEM1,
                ADC_VREFPOS_AVCC_VREFNEG_VSS,
                ADC_INPUT_A9, ADC_NONDIFFERENTIAL_INPUTS);

        ADC14_enableInterrupt(ADC_INT1);

        Interrupt_enableInterrupt(INT_ADC14);
        Interrupt_enableMaster();

        ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

        ADC14_enableConversion();
        ADC14_toggleConversionTrigger();
}

//lcd initialization
void _graphicsInit()
{
    Crystalfontz128x128_Init();

    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128, &g_sCrystalfontz128x128_funcs);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
        Graphics_clearDisplay(&g_sContext);

        Graphics_drawStringCentered(&g_sContext,
                                        (int8_t *)"Waiting to start :",
                                        AUTO_STRING_LENGTH,
                                        64,
                                        30,
                                        OPAQUE_TEXT);
}

//hardware initialization
void _hwInit()
{
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    PCM_setCoreVoltageLevel(PCM_VCORE1);

    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);

    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    _graphicsInit();
    _adcInit();
}








//funzioni di cambio stato
void fn_WAITING(){
    if(event==JOYSTICK_PRESSED){
        scan_players();
        current_state=SCAN;
    }
}

void fn_SCAN(){
    if(event==END_ARRIVED){
        select_card();
        current_state=CARTE;
    }
}

void fn_CARTE(){
    if(event==JOYSTICK_PRESSED){
        give_card();
        current_state=DISTRIBUZIONE;
    }
}

void fn_DISTRIBUZIONE(){
    if(event==START_ARRIVED){
        game_play();
        current_state=GAME;
    }
}

void fn_GAME1(){
    if(event==BUTTON1_PRESSED){
        give1_card();
        current_state=GAME;
    }
}


void fn_GAME2(){
    if(event==BUTTON2_PRESSED){
        end_game();
        current_state=WAITING;
    }
}


StateMachine_t fsm[]={
                      {WAITING, fn_WAITING},
                      {SCAN, fn_SCAN},
                      {CARTE, fn_CARTE},
                      {DISTRIBUZIONE, fn_DISTRIBUZIONE},
                      {GAME, fn_GAME1},
                      {GAME, fn_GAME2}
};



//main
int main(void)
{
    _hwInit();


    while(1)
    {
        PCM_gotoLPM0();
        (*fsm[current_state].state_function)();
    }
}







//handler sul joystick
void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if(status & ADC_INT1 && current_state==CARTE)
    {
        resultsBuffer[1] = ADC14_getResult(ADC_MEM1);
        char string[10];

        if(resultsBuffer[1]>14000 && semaforo==true){
            count=count+1;
            semaforo=false;
        }

        if(resultsBuffer[1]<10000 && resultsBuffer[1]>4000 && semaforo==false){
            semaforo=true;
        }


        if(resultsBuffer[1]<2000 && semaforo==true){
            count=count-1;
            semaforo=false;
            }

        sprintf(string, "%d", count);
        Graphics_drawStringCentered(&g_sContext,
                                    (int8_t *)string,
                                    8,
                                    64,
                                    70,
                                    OPAQUE_TEXT);
    }

    if(status & ADC_INT1 && current_state==WAITING){
            resultsBuffer[1] = ADC14_getResult(ADC_MEM1);
            if (!(P4IN & GPIO_PIN1)){
                pressed=true;
            }

        }

}



//handler sui bottoni
