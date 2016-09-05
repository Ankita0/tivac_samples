/* HOW TO USE THIS CODE
 * Connect a potentiometer to PB5, and watch the LED change in brightness
 * as you turn the dial.
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

/* Pin-to-peripheral mapping header
 * e.g., TODO
 */
#include "driverlib/pin_map.h"

/* PWM driver header
 * e.g., SysCtlPWMClockSet
 */
#include "driverlib/pwm.h"

/* ADC driver header
 * e.g., ADCSequenceConfigure
 */
#include "driverlib/adc.h"

/*******************************************************************************
 *                                Main Method                                  *
 ******************************************************************************/

void main(void) {

	// Set system clock to 80MHz using a PLL (200MHz / 2.5 = 80MHz)
	SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
	                       SYSCTL_XTAL_16MHZ);

	// ====== ADC Code ====================================================== //

	/* Before you begin:
	 * We are connecting the potentiometer to PB5, which corresponds to AIN11.
	 * See page 801 in the datasheet for pin assignments. Note that the two
	 * ADC modules, ADC0 and ADC1, share the same 12 input channels.
	 */

    /* This array is used for storing the data read from the ADC FIFO. It
     * must be as large as the FIFO for the sequencer in use.  This example
     * uses sequence 3 which has a FIFO depth of 1.  If another sequence
     * was used with a deeper FIFO, then the array size must be changed.
     */
    uint32_t pui32ADC0Value[1];

	// Enable the ADC peripheral and wait for it to be ready.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)){}

	// Enable GPIO port B and wait for it to be ready.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)){}

	// Configure PB5 as an ADC input
	GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);

    /* Configure the ADC sample sequence.
     * Enable sample sequence 3 with a processor signal trigger.  Sequence 3
     * will do a single sample when the processor sends a signal to start the
     * conversion.
     */
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    /* Configure the ADC sample sequence steps
     * Configure step 0 on sequence 3.  Sample channel 11 (ADC_CTL_CH11) in
     * single-ended mode (default) and configure the interrupt flag
     * (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
     * that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
     * 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
     * sequence 0 has 8 programmable steps.  Since we are only doing a single
     * conversion using sequence 3 we will only configure step 0.  For more
     * information on the ADC sequences and steps, reference the datasheet.
     */
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH11 | ADC_CTL_IE |
                             ADC_CTL_END);

    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);

    /* Clear the interrupt status flag.  This is done to make sure the
     * interrupt flag is cleared before we sample.
     */
    ADCIntClear(ADC0_BASE, 3);

    // ====== PWM Code ====================================================== //

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

	// Enable GPIO port A and wait for it to be ready.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)){}

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

    // Enable the PWM output signal
    PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);

    // Enable the PWM peripheral
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);

    // Sample the ADC indefinitely
    while(1) {
		// Trigger the ADC conversion.
		ADCProcessorTrigger(ADC0_BASE, 3);

		// Wait for conversion to be completed.
		while(!ADCIntStatus(ADC0_BASE, 3, false)){}

		// Clear the ADC interrupt flag.
		ADCIntClear(ADC0_BASE, 3);

		/* Read ADC Value. ADC has 12-bit precision so the output ranges from
		 * 0 to 4095
		 */
		ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);

		/* Set the PWM duty cycle to based on the ADC reading. The ADC is
		 * 12-bit, so its max reading is 4095.
		 *
		 * Warning! The PWM module has hard time hitting 100%, so when you turn
		 * the potentiometer all the way to the max, the LED may shut off or
		 * flicker. This is a known issue with the PWM module, not an ADC issue:
		 * https://e2e.ti.com/support/microcontrollers/tiva_arm/f/908/t/448664
		 *
		 * The LED may blink a little when you turn the potentiometer to 0.
		 * I think this is just noise that's more visible when the LED
		 * brightness is low. The custom Tiva board should have a better
		 * analog power supply, so hopefully the issue will be resolved with
		 * that. We'll see...
		 */
	    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,
	                         PWMGenPeriodGet(PWM1_BASE, PWM_GEN_3)
							 * pui32ADC0Value[0] / 4095);
    }
}
