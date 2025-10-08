/*! @mainpage Medidor de distancia por ultrasonido con interrupciones y puerto serie
 *
 * @section genDesc General Description
 *
 * Este programa mide distancia con un sensor HC-SR04 y muestra el resultado en un LCD
 * y con LEDs, utilizando interrupciones para teclas ,temporizador y comunicación serie UART.
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
 * | 01/10/2025 | Document creation		                         |
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
#include "timer_mcu.h"
#include "gpio_mcu.h"
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
#define MEDICION_PERIOD_US 1000000 // TIEMPO 1 s DEL TIMER

/*==================[internal data definition]===============================*/
TaskHandle_t MedirDistancia_task_handle = NULL;
TaskHandle_t ControlarLed_task_handle = NULL;
TaskHandle_t Display_task_handle = NULL;
TaskHandle_t Uart_task_handle = NULL;

volatile bool activar_medicion = false; // booleano para activar la medicion
volatile bool hold = false;             // booleano para mantener el ultimo valor
volatile uint16_t distancia_actual = 0; // ditancia medida en cm

/*==================[internal functions declaration]=========================*/

/**
 * @brief interrupcion por pulsación de TEC1: activa/desactiva medición
 */
void Tecla1(void *param)
{
    activar_medicion = !activar_medicion; // cambio el estado de activar med
}

/**
 * @brief interrupcion por pulsación de TEC2: activa/desactiva HOLD
 */
void Tecla2(void *param)
{
    hold = !hold; // cambio el estado de hold
}

/**
 * @brief Interrupción del timer cada 1 s para despertar tareas
 */
void FuncTimer(void *param)
{
    vTaskNotifyGiveFromISR(MedirDistancia_task_handle, pdFALSE); // notifica a la tarea de medicion
    vTaskNotifyGiveFromISR(ControlarLed_task_handle, pdFALSE);   // notifica a la tarea de control led
    vTaskNotifyGiveFromISR(Display_task_handle, pdFALSE);        // notifica a la tarea de display
}

void FuncUart(void *param)
{  //replicar la funcionalidad de los botones fisicos a traves de comunicacion serie
    uint8_t dato;
    if (UartReadByte(UART_PC, &dato)) // leo dato recibido
    {
        if (dato == 'O') // si es 'o' activo/desactivo medicion
        {
            activar_medicion = !activar_medicion;
        }
        else if (dato == 'H') // si es 'h' activo/desactivo hold
        {
            hold = !hold;
        }
    }
}

/**
 * @brief Tarea que mide la distancia con el sensor ultrasónico y la envía por UART
 */
static void MedirDistancia(void *pvParameter)
{
    char buffer[20];

    while (true)
    {

        // Espera a que el timer la despierte cada 1 s
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (activar_medicion)
        { // si activar med esta en true
            distancia_actual = HcSr04ReadDistanceInCentimeters();
            // envio por uart
        UartSendString(UART_PC, (char*)UartItoa(distancia_actual, 10));
        UartSendString(UART_PC, " cm\r\n");
        } 
    }
}
/**
 * @brief Tarea que controla los LEDs según la distancia medida
 */
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
        // Espera a que el timer la despierte cada 1 s
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}
/**
 * @brief Tarea que actualiza el LCD con la distancia
 */
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
        // Espera a que el timer la despierte cada 1 s
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}
/*==================[external functions definition]==========================*/
void app_main(void)
{ // Inicializaciones
    LedsInit();
    LcdItsE0803Init();
    HcSr04Init(GPIO_3, GPIO_2);
    SwitchesInit();

    // config teclas

    SwitchActivInt(SWITCH_1, Tecla1, NULL);
    SwitchActivInt(SWITCH_2, Tecla2, NULL);
    //config UART
    serial_config_t my_uart = {
        .port = UART_PC, //identifica el puerto UART que esta conectado a la pc
        .baud_rate = 9600, //define la velocidad de comunicacion
        .func_p = FuncUart, //interrupcion isr para el uart,
        .param_p = NULL};
    UartInit(&my_uart);

    // config timer
    timer_config_t timer_medicion = {
        .timer = TIMER_A,
        .period = MEDICION_PERIOD_US, // 1s
        .func_p = FuncTimer,
        .param_p = NULL};
    TimerInit(&timer_medicion);

    // crear tareas
    xTaskCreate(&MedirDistancia, "Medir Distancia", 512, NULL, 5, &MedirDistancia_task_handle);
    xTaskCreate(&ControlarLed, "Controlar Led", 512, NULL, 5, &ControlarLed_task_handle);
    // xTaskCreate(&Teclas, "Teclas", 512, NULL, 5, &Teclas_task_handle);
    xTaskCreate(&Display, "Display", 512, NULL, 5, &Display_task_handle);

    // iniciar timer
    TimerStart(timer_medicion.timer);
}
/*==================[end of file]============================================*/