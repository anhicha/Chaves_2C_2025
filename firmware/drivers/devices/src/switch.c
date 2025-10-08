/**
 * @file switch.c
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 * @brief 
 * @version 0.1
 * @date 2023-10-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/*==================[inclusions]=============================================*/
#include "switch.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
#define GPIO_SWITCH1 GPIO_4
#define GPIO_SWITCH2 GPIO_15
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
int8_t SwitchesInit(void){
	/* GPIO configurations como entrada */
	GPIOInit(GPIO_SWITCH1, GPIO_INPUT);	// SWITCH_1
	GPIOInit(GPIO_SWITCH2, GPIO_INPUT);	// SWITCH_2
	GPIOInputFilter(GPIO_SWITCH1);		
	GPIOInputFilter(GPIO_SWITCH2);
	return true;
}

int8_t SwitchesRead(void){
	//estado de los interruptores 0000 0000 ,ningun boton presionado.
	int8_t mask = 0;
	//los botones estan en low cuando se presionan
	if (!GPIORead(GPIO_SWITCH1))
	//Or lógico para ir seteando los bits del mask, enciende el bit 0 de la mask
		  mask |= SWITCH_1;
	if (!GPIORead(GPIO_SWITCH2))  
	//enciende el bit 1 de la mask (0010)
		  mask |= SWITCH_2;
	return mask;
}
//puntero a la funcion de interrupcion, argumentos opcionales
//instala una alarma en el pin fisico gpio asociado a las teclas
void SwitchActivInt(switch_t sw, void *ptr_int_func, void *args){
	switch(sw){ //identifica el switch
		case SWITCH_1:
			GPIOActivInt(GPIO_SWITCH1, ptr_int_func, false, args);
		break;
		case SWITCH_2:
			GPIOActivInt(GPIO_SWITCH2, ptr_int_func, false, args);
		break;
	}
}
/*==================[end of file]============================================*/
