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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
TaskHandle_t MedirDistancia_task_handle = NULL;
TaskHandle_t ControlarLed_task_handle = NULL;
TaskHandle_t Teclas_task_handle = NULL;
volatile uint16_t distancia_actual=0;

/*==================[internal functions declaration]=========================*/
static void MedirDistancia(void *pvParameter){
	

    while(true){
        distancia_actual = HcSr04ReadDistanceInCentimeters();
		printf("Distancia: %d cm\r\n", distancia_actual);
		vTaskDelay(pdMS_TO_TICKS(500));
	}
    }


static void ControlarLed(void *pvParameter){
    while(true){
        if(distancia_actual < 10 ){
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}else{
		}
		vTaskDelay(pdMS_TO_TICKS(100));
    
    }
}

static void Teclas(void *pvParameter){
    while(true){
      
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);   
    xTaskCreate(&MedirDistancia, "LED_1", 512, NULL, 5, &MedirDistancia_task_handle);
    xTaskCreate(&ControlarLed, "LED_2", 512, NULL, 5, &ControlarLed_task_handle);
    xTaskCreate(&Teclas, "LED_3", 512, NULL, 5, &Teclas_task_handle);
}
/*==================[end of file]============================================*/