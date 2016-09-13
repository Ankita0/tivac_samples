#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"

#define PWM_FREQUENCY 55
/*
 * main.c
 */
int main(void) {
	//the value for the PWM counter
	volatile uint32_t ui32Load;
	volatile uint32_t ui32PWMClock;

	//middle position -> PW = 1.5ms, PWMf = 55Hz -> P = 18.2ms, /1000 -> resolution = 1.82us, 83*1.82 = 1.51ms
	volatile uint8_t ui8Adjust;
	ui8Adjust = 83;

	//using the function call in ROM
	//setting the clock at 40 Mhz
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

	//PWM is clocked from the system clock through a divider with a range from 2 to 64
	//PWM Clock runs at 625 KHz
	ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	//We need to enable the PWM1 and GPIOD modules (for the PWM output on PD0) and
	//the GPIOF module (for the LaunchPad Reset buttons on PF0 and PF4).

	//Enable PWM module 1
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	//Enable GPIO ports with base D
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	//Enable GPIO ports with base F
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	//Configure PD0 as PWM output
	ROM_GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);
	//connect it to PWM generator 0 (This selects a line from a multiplexer)
	ROM_GPIOPinConfigure(GPIO_PD0_M1PWM0);


	//GPIO port PF0 is locked by default since it can be configured to be an NMI input (Non-Maskable Interrupt).
	//We need to unlock the GPIO commit control register.


	//HWREF is a function-like macro defined in hw_types.h which returns the contents of a register address
	//GPIO_O_LOCK is a macro for the GPIOLOCK register offset.
	//Set status of GPIO_O_CR to unlocked
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	//Enable writing to PF0
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	//Lock the GPIO_O_CR
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

	//Set the direction and mode of PF0 and PF4
	//Arguments: GPIO port base, Pins to set, direction mode: in, out or hw
	ROM_GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	//specify drive strength as 2mA and weak pull-up in push-pull mode
	ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


	//divide the system clock by 64 to find the PWMClock (we used the 64 divider)
	ui32PWMClock = SysCtlClockGet() / 64;
	//divide the PWM clock frequency by the signal frequency, and subtract 1 to get the PWM counter load
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
	//configure PWM module 1 generator 0 with countdown mode
	PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
	//Set the PWM counter load
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, ui32Load);

	//Set the pulse width corresponding to the middle postion
	ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
	//Enable as output
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
	//Enable to run
	ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0);

	while(1)
	{
		//if PF4 is pressed
		if(ROM_GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
		{
			ui8Adjust--; //decrement position
			if (ui8Adjust < 56) //corresponds to min position at 1 ms.
			{
				ui8Adjust = 56;
			}
			ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
		}


		if(ROM_GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
		{
			ui8Adjust++;
			if (ui8Adjust > 168)
			{
				uiAdjust = 168;
			}
			ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
		}
	}
}

ROM_SysCtlDelay(100000);
return 0;
}

