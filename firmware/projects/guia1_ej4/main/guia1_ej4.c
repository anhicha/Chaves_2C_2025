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
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/ 

#define BCD_BITS   4   /*!< Cantidad de bits BCD (D1..D4) */
/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;


/*==================[internal functions declaration]=========================*/

int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number) {
    // Rellenar todo con ceros primero
    for (uint8_t i = 0; i < digits; i++) {
        bcd_number[i] = 0;
    }

    // Extraer dígitos desde el final
    for (int8_t i = digits - 1; i >= 0; i--) {
        bcd_number[i] = data % 10;
        data /= 10;
    }

    return 0; // éxito
}

void writeBcdToGpio(uint8_t bcd_digit, gpioConf_t * map) {
    for (int i = 0; i < 4; i++) {
        uint8_t bit = (bcd_digit >> i) & 0x01;   // extraigo bit i del dígito
        if (bit)
            GPIOOn(map[i].pin);   // pongo el GPIO en ‘1’
        else
            GPIOOff(map[i].pin);  // pongo el GPIO en ‘0’
    }
}

void displayNumber(uint32_t number, uint8_t digits, gpioConf_t * bcd_map, gpioConf_t * digit_map) {
    uint8_t bcd[digits];
    convertToBcdArray(number, digits, bcd);

    for (int i = 0; i < digits; i++) {
       
        // Escribo el dígito BCD en los GPIOs
        writeBcdToGpio(bcd[i], bcd_map);

        // Enciendo el dígito correspondiente
        GPIOOn(digit_map[i].pin);
        GPIOOff(digit_map[i].pin);
     
    }
}


/*==================[external functions definition]==========================*/
void app_main(void){
	
	// uint8_t bcd[6];
	// uint32_t numero=181122;
	// uint8_t digitos=6;

    // convertToBcdArray(numero, digitos, bcd);
	// printf("BCD:\n");
	// 	for (int i = 0; i < digitos; i++) {
	// 		printf("%d\n" ,bcd[i]);
	// 	}
	// 	printf("\n");
   gpioConf_t bcd_map[4] = {
    {GPIO_20, GPIO_OUTPUT},  // b0
    {GPIO_21, GPIO_OUTPUT},  // b1
    {GPIO_22, GPIO_OUTPUT},  // b2
    {GPIO_23, GPIO_OUTPUT}   // b3
    };

    // Configuro los GPIOs como salida
    for (int i = 0; i < 4; i++) {
        GPIOInit(bcd_map[i].pin, bcd_map[i].dir);
    }

    gpioConf_t digit_map[3] = {
        {GPIO_19, GPIO_OUTPUT},  // dígito 1
        {GPIO_18, GPIO_OUTPUT},  // dígito 2
        {GPIO_9,  GPIO_OUTPUT}   // dígito 3
    };
    for (int i = 0; i < 3; i++) {
        GPIOInit(digit_map[i].pin, digit_map[i].dir);
     }
    // writeBcdToGpio(5, bcd_map);
	// Mostrar número en el display
    displayNumber(785, 3, bcd_map, digit_map);
}

/*==================[end of file]============================================*/