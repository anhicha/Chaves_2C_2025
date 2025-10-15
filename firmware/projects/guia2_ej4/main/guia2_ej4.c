/*! @mainpage guia2_ej4
 *
 * @section genDesc General Description
 *
 *Este programa genera una señal ECG digital por DAC y la mide con el ADC.
 *El valor leido se envía por UART simulando el comportamiento de un osciloscopio.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	DAC_OUT 	| 	GPIO_X		|
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
#define FREC_M 500						   // 1 muestra cada 2 ms
#define TIMER_PERIOD_US (1000000 / FREC_M) // periodo en microseg
#define BUFFER_SIZE 231					   // tamaño del buffer de la UART
/*==================[internal data definition]===============================*/

volatile uint16_t adc_value = 0;	 // valor convertido(0-4095)
TaskHandle_t ADC_task_handle = NULL; // Guarda el handle de la tarea necesario para despertarla desde la isr
TaskHandle_t DAC_task_handle = NULL; // tarea de generacion DAC

// ECG digital
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
/*==================[internal functions declaration]=========================*/

/**
 * @brief Interrupción del timer cada 2 ms (500 Hz)
 *  
 * Esta ISR despierta las tareas de conversión DAC y adquisición ADC.
 */
void TimerISR(void *param)
{
	vTaskNotifyGiveFromISR(ADC_task_handle, pdFALSE); // Despierta tarea de muestreo
	vTaskNotifyGiveFromISR(DAC_task_handle, pdFALSE); // Despierta tarea de generacion DAC
}

/**
 * @brief Tarea de generación de señal ECG por DAC
 */
void DAC_task(void *pvParameter)
{
	uint16_t i = 0;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// escribir valor en DAC(8 bits→ 0–255)
		AnalogOutputWrite(ecg[i]);
		i++;
		if (i >= BUFFER_SIZE)
		{
			i = 0; // reinicia señal ecg
		}
	}
}

/**
 * @brief Tarea que adquiere y envía la señal analógica por UART.
 */
void ADC_task(void *pvParameter)
{
	char buffer[32];

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// leer valor analógico de CH1
		AnalogInputReadSingle(CH1, &adc_value);

		// enviar valor por UART en formato compatible
		sprintf(buffer, ">brightness:%u\r\n", adc_value);
		UartSendString(UART_PC, buffer);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	/* === Inicialización del ADC === */
	analog_input_config_t adc_config = {
		.input = CH1,		// Canal 1 de entrada
		.mode = ADC_SINGLE, // Modo de lectura simple, una conversión cada vez que lo leemos
		.func_p = NULL,		// No se usa callback
		.param_p = NULL,	// No se usa callback
	};
	AnalogInputInit(&adc_config); // inicializa el ADC

	/* === Inicialización del DAC === */
	AnalogOutputInit();

	/* === Inicialización de UART === */
	serial_config_t uart_config = {
		.port = UART_PC,
		.baud_rate = 115200,   // velocidad alta para no perder muestras
		.func_p = UART_NO_INT, // no se usa interrupcion
		.param_p = NULL,
	};
	UartInit(&uart_config); // inicializa la UART

	/* === Inicialización del Timer === */
	timer_config_t timer_cfg = {
		.timer = TIMER_A,
		.period = TIMER_PERIOD_US, // cada dos ms
		.func_p = TimerISR,		   // despierta la tarea adc task
		.param_p = NULL};
	TimerInit(&timer_cfg); // inicializa el timer

	/* === Crear tarea de muestreo y envío === */
	xTaskCreate(&ADC_task, "ADC_Task", 2048, NULL, 5, &ADC_task_handle);
	xTaskCreate(&DAC_task, "DAC_Task", 2048, NULL, 5, &DAC_task_handle);

	TimerStart(timer_cfg.timer);
}

/*==================[end of file]============================================*/