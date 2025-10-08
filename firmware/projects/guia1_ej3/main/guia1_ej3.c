/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Anahí Chaves (natalia.chaves@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 100
#define ON 1
#define OFF 0
#define TOGGLE 2

/*==================[internal data definition]===============================*/
struct leds
{
    uint8_t mode;     // ON, OFF, TOGGLE
    uint8_t n_led;    // indica el número de led a controlar
    uint8_t n_ciclos; // indica la cantidad de ciclos de encendido/apagado
    uint16_t periodo; // indica el tiempo de cada ciclo en ms
} my_leds;
/*==================[internal functions declaration]=========================*/

void controlarLed(struct leds *my_leds)
{   //verifica que el puntero no sea nulo
    if (my_leds == NULL)
        return;

    //  MODO ON
    //se accede a los campos del struct con el puntero ->
    if (my_leds->mode == ON)
    {
        if (my_leds->n_led == 1)
        {
            LedOn(LED_1);
        }
        else if (my_leds->n_led == 2)
        {
            LedOn(LED_2);
        }
        else if (my_leds->n_led == 3)
        {
            LedOn(LED_3);
        }
    }
    //  MODO OFF
    else if (my_leds->mode == OFF)
    {
        if (my_leds->n_led == 1)
        {
            LedOff(LED_1);
        }
        else if (my_leds->n_led == 2)
        {
            LedOff(LED_2);
        }
        else if (my_leds->n_led == 3)
        {
            LedOff(LED_3);
        }
    }
    // MODO TOGGLE
    else if (my_leds->mode == TOGGLE)
    {
        for (int i = 0; i < my_leds->n_ciclos; i++)
        {

            if (my_leds->n_led == 1)
            {
                LedToggle(LED_1);
            }
            else if (my_leds->n_led == 2)
            {
                LedToggle(LED_2);
            }
            else if (my_leds->n_led == 3)
            {
                LedToggle(LED_3);
            }
            for (int j = 0; j < my_leds->periodo / CONFIG_BLINK_PERIOD; j++)
            {
                vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
            }
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{   
    //inicializacion de leds como salida
    LedsInit();

    //creación de una variable del tipo struct leds y carga de los campos
    struct leds my_leds;
    my_leds.mode = TOGGLE; 
    my_leds.n_led = 2; 
    my_leds.n_ciclos = 10;
    my_leds.periodo = 500; //pausa de 500 ms entre cada toggle
    controlarLed(&my_leds);//llama a la funcion pasandole la direccion de memoria de la variable


    //no se usa
    SwitchesInit();
    while (1)
    {
        
    }
    vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
}

/*==================[end of file]============================================*/