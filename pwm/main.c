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

/* Pin-to-peripheral mapping header
 * e.g., TODO
 */
#include "driverlib/pin_map.h"

/* PWM driver header
 * e.g., TODO
 */
#include "driverlib/pwm.h"

/*******************************************************************************
 *                                Main Method                                  *
 ******************************************************************************/

void main(void) {
	/* Before you begin:
	 * We want a PWM output to appear on PF3 (the green LED on the LaunchPad).
	 * If you go to page 1233 in the MCU's data sheet, you'll see that the PWM
	 * output on PF3 is controlled by PWM Module 1, PWM Generator 3, and PWM
	 * output 7 (see the diagram on page 1232). These numbers will affect the
	 * arguments to the macro calls below.
	 */

	/* Set PWM clock base frequency.
	 * PWM base frequency is the processor clock frequency divided by X, where
	 * the input to SysCtlPWMClockSet is SYSCTL_PWMDIV_X.
	 * If you're using the external oscillator (default), the processor clock
	 * will be 80MHz. In this case, the divisor is 8, so the frequency base is
	 * 10MHz. Note that this isn't the actual frequency of the PWM signal. That
	 * will be set later to be an integer multiple of this value.
	 */
	SysCtlPWMClockSet(SYSCTL_PWMDIV_8);

	// Enable the PWM peripheral and wait for it to be ready.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM1)){}

	// Enable the GPIO peripheral and wait for it to be ready.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){}

	// Configure the internal multiplexer to connect the PWM peripheral to PF3
	GPIOPinConfigure(GPIO_PF3_M1PWM7);

	// Set up the PWM module on pin PF3
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);

	/* Configure PWM mode to count up/down without synchronization
	 * This is just a detail, you probably don't have to worry about it.
	 * See pages 1235 and 1237 in the data sheet if you want to learn more.
	 */
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN |
                    PWM_GEN_MODE_NO_SYNC);

    /* Set the PWM period based on the time base we configured with
     * SysCtlPWMClockSet above.
     *
     * If we want a period of 250Hz:
     * N = (1 / f) * BaseFreq, where N is the last parameter in PWMGenPeriodSet,
     * f is the desired frequency, and BaseFreq is the PWM base frequency set
     * above (10 MHz in this case). (1 / 250Hz) * 10 MHz = 40000.
     *
     * Note that the maximum period you can set is 2^16. If you need more than
     * that, you'll need to modify the clock divisor.
     */
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, 40000);

    /* Set the PWM duty cycle to 25%. The duty cycle is a function of the period.
     * You can get the period set above using PWMGenPeriodGet. In this case
     */
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,
                         PWMGenPeriodGet(PWM1_BASE, PWM_GEN_3) / 4);

    // Enable the PWM output signal
    PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);

    // Enable the PWM peripheral
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);

    // Delay indefinitely so the program never stops.
    while(1){}
}
