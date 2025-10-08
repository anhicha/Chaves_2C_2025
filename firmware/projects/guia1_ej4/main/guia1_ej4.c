/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 /*! @file display_driver.c
 *  @brief Driver para el uso del display numérico de 3 dígitos en la placa EDU-ESP.
 *
 *  Este archivo implementa las funciones necesarias para mostrar números en un display
 *  de 7 segmentos a través de conversores BCD a 7 segmentos.  
 *  Utiliza 4 líneas BCD (GPIO_20..23) y 3 líneas de selección/latch para cada dígito (GPIO_19, GPIO_18 y GPIO_9).  
 *
 *  El driver convierte un número de hasta 3 dígitos en formato decimal, lo traduce a BCD
 *  y lo envía al hardware correspondiente. Cada dígito se activa mediante un pulso en
 *  la línea de selección, que provoca el almacenamiento ("latch") de los datos en el
 *  CD4543.
 * 
 *
 * @section hardConn Hardware Connection
 * |   Display      |   EDU-CIAA	|
 * |:--------------:|:-------------:|
 * | 	Vcc 	    |	5V      	|
 * | 	BCD1		| 	GPIO_20		|
 * | 	BCD2	 	| 	GPIO_21		|
 * | 	BCD3	 	| 	GPIO_22		|
 * | 	BCD4	 	| 	GPIO_23		|
 * | 	SEL1	 	| 	GPIO_19		|
 * | 	SEL2	 	| 	GPIO_18		|
 * | 	SEL3	 	| 	GPIO_9		|
 * | 	Gnd 	    | 	GND     	|
 * 
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 1/09/2025 | Document creation		                         |
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

/**
 * @brief Convierte un número entero en un arreglo de dígitos decimales (BCD).
 *
 * @param[in] data Número entero de entrada (hasta 32 bits).
 * @param[in] digits Cantidad de dígitos a extraer.
 * @param[out] bcd_number Puntero a un arreglo donde se guardan los dígitos. *                        bcd_number[0] contiene el dígito más significativo.
 *
 * @return 0 si tuvo éxito, -1 en caso de error.
 */
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number) {
    // Rellenar todo con ceros primero
    for (uint8_t i = 0; i < digits; i++) {
        bcd_number[i] = 0;
    }

    // Extraer dígitos desde el final
    for (int8_t i = digits - 1; i >= 0; i--) {
        bcd_number[i] = data % 10;   //guarda el digito menos significativo 
        data /= 10; //elimina ese digito del numero original
    }

    return 0; // éxito
}

/**
 * @brief Escribe un dígito BCD en los GPIOs correspondientes.
 *
 * @param[in] bcd_digit Dígito BCD (0-9) a escribir.
 * @param[in] map Puntero a un arreglo de estructuras que mapean los bits BCD a los GPIOs.
 *     map[0] corresponde al bit menos significativo (b0).
 *     map[3] corresponde al bit más significativo (b3).
 */
void writeBcdToGpio(uint8_t bcd_digit, gpioConf_t * map) {
    for (int i = 0; i < 4; i++) {
        //extraigo bit i del dígito 
        uint8_t bit = (bcd_digit >> i) & 0x01;   // >>mueve el bit i a la posición 0, &0x01 lo aísla
        if (bit)
            GPIOOn(map[i].pin);   // pongo el GPIO en ‘1’
        else
            GPIOOff(map[i].pin);  // pongo el GPIO en ‘0’
    }
}
/**
 * @brief Muestra un número completo en el display de 3 dígitos.
 *
 * Convierte el número a BCD, lo escribe en las líneas correspondientes y activa
 * cada dígito mediante su pin de selección.
 * 
 * @param[in] number Número a mostrar (0-999).
 * @param[in] digits Cantidad de dígitos del display (3 en este caso).
 * @param[in] bcd_map Puntero a un arreglo de estructuras que mapean los bits BCD a los GPIOs.
 * @param[in] digit_map Puntero a un arreglo de estructuras que mapean los dígitos del display a los GPIOs.
 */

void displayNumber(uint32_t number, uint8_t digits, gpioConf_t * bcd_map, gpioConf_t * digit_map) {
    uint8_t bcd[digits];
    //convierto el número en un arreglo BCD
    convertToBcdArray(number, digits, bcd);

    for (int i = 0; i < digits; i++) {
       
        // Escribo el dígito BCD en los GPIOs
        writeBcdToGpio(bcd[i], bcd_map);

        // Enciendo el dígito correspondiente del display
        //cada digito comparte los 4 pines BCD pero solo se enicende uno a la vez
        //Multiplexado
        GPIOOn(digit_map[i].pin); //enciendo el digito actual
        GPIOOff(digit_map[i].pin);//apago el digito actual
        //se hace rapidamente para que el ojo humano no lo note
    }
}


/*==================[external functions definition]==========================*/
/**
 * @brief Función principal de la aplicación.
 *
 */


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