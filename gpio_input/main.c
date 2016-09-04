/* HOW TO USE THIS CODE
 * Push SW1 and the green LED should light up until you release it.
 * Repeat until you get bored.
 */

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
     * GPIO pin for digital function (Green User LED)
     */
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);

    /* Enable the GPIO pin for PF4. Set the direction as input, and enable the
     * GPIO pin for digital function (User Switch 1)
     */
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);

    /* Set the pin's drive strength to 2mA (not a relevant parameter considering
     * we want to use the pin as an input), and enable the internal pull-up on
     * the pushbutton pin.
     */
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA,
    		GPIO_PIN_TYPE_STD_WPU);

    volatile uint32_t pinVal;

    while(1) {
    	// Read input on F4
    	pinVal = GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4);

    	if(pinVal == GPIO_PIN_4) {
    		// Turn on the LED
        	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
    	} else {
    		// Turn off the LED
    		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    	}
    }
}
