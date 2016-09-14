#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/qei.h"

/*
 * main.c
 */
int main(void) {

	//set the value of the maximum number of pulses per revolution
	uint32_t ui32_MAX_PULSES = 3999;
	uint32_t ui32_VEL_PERIOD = 32;

	//set the clock to run at 40MHz
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	//Enable GPIO Clock
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	//It takes 3 clock cycles to enable a peripheral
	//this loop waits for the peripheral to be ready before proceeding.
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)){}

	//Enable QEI0 clock
	SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);
	//this loop waits for the peripheral to be ready before proceeding.
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_QEI0)){}

	//Configure GPIO pins to act as QEI pins
	//GPIO PD3 -> IDX0, PD6 -> PhA0, PD7 -> PhB0
	GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_7|GPIO_PIN_6|GPIO_PIN_3);
	GPIOPinConfigure(GPIO_PD3_IDX0);
	GPIOPinConfigure(GPIO_PD6_PHA0);
	GPIOPinConfigure(GPIO_PD7_PHB0);


	//Configure the quadrature encoder wih X4 encoding: captures edges on both channels
	//Configure the index signal to maintain absolute position readings.
	QEIConfigure(QEI0_BASE, (QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_RESET_IDX |
	QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), ui32_MAX_PULSES);

	QEIFilterEnable(QEI0_BASE);
	//Enables the QEI module
	QEIEnable(QEI0_BASE);
	//Configure the velocity capture module
	//QEI_VELDIV_1: no predivision
	//ui32_VEL_PERIOD: number of clock ticks over which to count pulses
	QEIVelocityConfigure(QEI0_BASE, QEI_VELDIV_1,  ui32_VEL_PERIOD);
	//Enable the Velocity capture module
	QEIVelocityEnable(QEI0_BASE);

	while (1){
		//Add delay
		//Read Encoder position when needed
		QEIPositionGet(QEI0_BASE);
		//Read number of pulses per time period
		QEIVelocityGet(QEI0_BASE);
	}

	return 0;
}
