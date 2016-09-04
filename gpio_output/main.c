/*******************************************************************************
 *                             Include Statements                              *
 ******************************************************************************/

/* C standard integer header
 * e.g., uint32_t
 */
#include <stdint.h>

/* C standard boolean header
 * e.g., bool (used inside gpio.h, sysctl.h)
 */
#include <stdbool.h>

/* Device memory map header
 * e.g., GPIO_PORTF_BASE
 */
#include "inc/hw_memmap.h"

/* GPIO API header
 * eg., GPIO_PIN_1
 */
#include "driverlib/gpio.h"

/* System control driver header
 * e.g., SysCtlPeripheralEnable, SysCtlPeripheralReady
 */
#include "driverlib/sysctl.h"

/*******************************************************************************
 *                                Main Method                                  *
 ******************************************************************************/

void main(void) {
    // Enable GPIO port F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    /* Wait for the peripheral to be ready. Code may work without this, but
     * it's better practice to put this after any peripheral enable command
     */
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){}

    /* Enable the GPIO pin for PF3. Set the direction as output, and enable the
     * GPIO pin for digital function
     */
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);

    /* Index for the sketchy LED timer below.
     * If you don't understand 'volatile', look here:
     * http://www.barrgroup.com/Embedded-Systems/How-To/C-Volatile-Keyword
     */
    volatile uint32_t ui32Loop;

    while(1){
        /* Turn on the LED. Arguments explained:
         * - Arg. 1: Write to port F
         * - Arg. 2: Write to pin 1 of port F
         * - Arg. 3: Write a digital 1 to pin F1
         *
         * Note that Arg. 3 isn't a 1 like it would be with an Arduino. Calling
         * GPIOPinWrite on pin X will write the value it finds in the Xth binary
         * position of Arg. 3.
         *
         * So the GPIO_PIN_1 macro actually expands to a 2. In order to write a
         * HIGH to pin F1, GPIOPinWrite needs a binary input that has a 1 in the
         * first binary position.
         *
         * Example:
         * PORT F: 5 4 3 2 1 0
         * Arg. 3: 0 0 0 0 1 0 == 2 in decimal == GPIO_PIN_1
         *
         * If we wanted to do the same with F3:
         *
         * PORT F: 5 4 3 2 1 0
         * Arg. 3: 0 0 1 0 0 0 == 8 in decimal == GPIO_PIN_3
         *
         * This allows you to play with bitmasks like you can see here:
         * http://processors.wiki.ti.com/index.php/Tiva_TM4C123G_LaunchPad_Blink
         * _the_RGB
         *
         * TL;DR: If you want to write a HIGH to a pin, make Arg. 3 the same as
         * Arg. 2. If you want to write LOW, make Arg. 3 a 0.
         */
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);

        /* Delay for a bit. This is a really sketchy way to implement a delay.
         * It's difficult to predict exactly how long the pin will be high or
         * low for... Don't do this for ANY time-critical applications. Use a
         * timer instead!
         *
         */
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++){}

        // Turn off the LED
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);

        // Delay for a bit
        for(ui32Loop = 0; ui32Loop < 800000; ui32Loop++){}
    }
}
