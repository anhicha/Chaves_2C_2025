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
 * | 10/09/2025 | Document creation		                         |
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
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_MUESTREO_PERIOD 1000 //DEFINE EL TIEMPO QUE SE ESPERA ENTRE MEDICIONES

/*==================[internal data definition]===============================*/
TaskHandle_t MedirDistancia_task_handle = NULL;
TaskHandle_t ControlarLed_task_handle = NULL;
TaskHandle_t Teclas_task_handle = NULL;
TaskHandle_t Display_task_handle = NULL;

volatile bool activar_medicion = false; // booleano para activar la medicion
volatile bool hold = false;             // booleano para mantener el ultimo valor
volatile uint16_t distancia_actual = 0; // ditancia medida en cm

/*==================[internal functions declaration]=========================*/

static void Teclas(void *pvParameter)
{
    uint8_t teclas;
    while (true)
    {   //lee el estado de las teclas
        teclas = SwitchesRead();
        switch (teclas)
        {
        case SWITCH_1:
            activar_medicion = !activar_medicion; // cambio el estado de activar med
            break;
        case SWITCH_2:
            hold = !hold; // cambio el estado de hold
            break;
        }
        vTaskDelay(CONFIG_MUESTREO_PERIOD / portTICK_PERIOD_MS);
    }
}

static void MedirDistancia(void *pvParameter)
{

    while (true)
    {
        if (activar_medicion)
        { // si activar med esta en true
            distancia_actual = HcSr04ReadDistanceInCentimeters();
          //  printf("Distancia: %d cm\r\n", distancia_actual);
        }
        //Refresco de medicion cada 1 s
        vTaskDelay(CONFIG_MUESTREO_PERIOD / portTICK_PERIOD_MS);
    }
}

static void ControlarLed(void *pvParameter)
{
    while (true)
    {
        if (!activar_medicion)
        { // si no se activa la medicion
            LedsOffAll();
        }
        else
        {
            if (distancia_actual < 10)
            { // si la distancia es menor a 10 cm
                LedsOffAll();
            }
            else if (distancia_actual >= 10 && distancia_actual < 20)
            { // si la distancia esta entre 10 y 20 cm
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }
            else if (distancia_actual >= 20 && distancia_actual < 30)
            { // si la distancia esta entre 20 y 30 cm
                LedOn(LED_2);
                LedOn(LED_1);
                LedOff(LED_3);
            }
            else if (distancia_actual > 30)
            { // si la distancia es mayor a 30 cm
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
            }
        }
        vTaskDelay(CONFIG_MUESTREO_PERIOD / portTICK_PERIOD_MS);
    }
}
static void Display(void *pvParameter)
{
    while (true)
    {
        if (!activar_medicion)
        { // si no se activa la medicion apago el display
            LcdItsE0803Off();
        }
        else
        {
            if (!hold)
            { // si no se activa hold muestro la distancia actual
                LcdItsE0803Write(distancia_actual);
            } // si se activa hold mantengo el ultimo valor en el
        }

        vTaskDelay(CONFIG_MUESTREO_PERIOD / portTICK_PERIOD_MS);
    }
}
/*==================[external functions definition]==========================*/
void app_main(void)
{ // Inicializaciones de periféricos
    LedsInit();
    SwitchesInit();
    LcdItsE0803Init();
    HcSr04Init(GPIO_3, GPIO_2);
    
    // crear tareas
    xTaskCreate(&MedirDistancia, "Medir Distancia", 512, NULL, 5, &MedirDistancia_task_handle);
    xTaskCreate(&ControlarLed, "Controlar Led", 512, NULL, 5, &ControlarLed_task_handle);
    xTaskCreate(&Teclas, "Teclas", 512, NULL, 5, &Teclas_task_handle);
    xTaskCreate(&Display, "Display", 512, NULL, 5, &Display_task_handle);
}
/*==================[end of file]============================================*/