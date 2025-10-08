/*! @mainpage guia2_ej4
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
 * | 08/10/2025 | Document creation		                         |
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

#include "analog_io_mcu.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
#define FREC_M 500
#define TIMER_PERIOD_US (1000000/FREC_M) //periodo en microseg
/*==================[internal data definition]===============================*/

volatile uint16_t adc_value =0;   //valor convertido(0-4095)
TaskHandle_t ADC_task_handle= NULL; //Tarea que toma y envia muestras

/*==================[internal functions declaration]=========================*/

/**
 * @brief Interrupción del timer cada 2 ms (500 Hz)
 */
void TimerISR(void *param) {
    vTaskNotifyGiveFromISR(ADC_task_handle, pdFALSE);  // Despierta tarea de muestreo
}

/**
 * @brief Tarea de muestreo ADC y envío UART
 */
void ADC_task(void *pvParameter){
	char buffer[32];
	
	while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	    //leer valor analógico de CH1
		AnalogInputReadSingle(CH1, &adc_value);

        //enviar valor por UART en formato compatible
		sprintf(buffer, ">brightness:%u\r\n", adc_value);
        UartSendString(UART_PC, buffer);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	/* === Inicialización del ADC === */
	analog_input_config_t adc_config ={
		.input = CH1,					// Canal 1 de entrada
		.mode = ADC_SINGLE,		// Modo de lectura simple
		.func_p = NULL,				// No se usa callback
		.param_p = NULL,			// No se usa callback
	}; 
	AnalogInputInit(&adc_config); //inicializa el ADC
    
    /* === Inicialización de UART === */
	serial_config_t uart_config={
		.port = UART_PC,
		.baud_rate = 115200,  //velocidad alta para no perder muestras
		.func_p= UART_NO_INT, //no se usa interrupcion
		.param_p =NULL,
	}; 
	UartInit(&uart_config); //inicializa la UART

	/* === Inicialización del Timer === */
	timer_config_t timer_cfg={
		.timer=TIMER_A,
		.period=TIMER_PERIOD_US,
		.func_p=TimerISR,
		.param_p=NULL
	};
	TimerInit(&timer_cfg); //inicializa el timer
	TimerStart(timer_cfg.timer);

    /* === Crear tarea de muestreo y envío === */
    xTaskCreate(&ADC_task, "ADC_Task",2048, NULL,5, &ADC_task_handle);
		}
		
/*==================[end of file]============================================*/