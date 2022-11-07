/*
 * sensor.c
 * Author: Ondrej Ondryas (xondry02@stud.fit.vutbr.cz)
 */

#include "sensor.h"

void Sensor_SendTrigger(void) {
	NVIC_DisableIRQ(LPTMR0_IRQn);

	GPIOA->PSOR = SENSOR_TRIG_PIN_MASK;
	// 10 uS ~ 210 ticks
	for (int i = 0; i < 420; i++)
		;

	GPIOA->PCOR = SENSOR_TRIG_PIN_MASK;

	NVIC_EnableIRQ(LPTMR0_IRQn);
	for (int i = 0; i < 400000; i++)
		asm("nop");

	// 450 = ((x * 34000) / 1310720) / 2
	// x ~ 34 697 -> This should be the maximum pulse length from the sensor (in CPU cycles)
	// and it take some time for the sensor send the measurement pulses
	// Using 40 000 at minimum here should be fine then
	// However, it would be too fast anyway, so let's use much higher value
}

volatile uint64_t clocks;
volatile uint16_t delta;

void Sensor_HandleInterrupt(void) {
	int value = GPIOE->PDIR & SENSOR_ECHO_PIN_MASK;

	if (value) {
		clocks = (uint16_t) FTM0->CNT + micros;
	} else {
		uint64_t now = FTM0->CNT + micros;
		if (now < clocks) {
			delta = (uint16_t) (UINT64_MAX + now - clocks);
		} else {
			delta = (uint16_t) (now - clocks);
		}
	}
}

int Sensor_GetValue(void) {
	unsigned long value = (unsigned long) delta;

	// 30 ms * (20.97152 MHz / 16) ~ 39321 clocks for invalid values
	if (value > 39321) {
		return -1;
	}

#if ENABLE_DECIMAL == 1
	value *= 340000;
#else
	value *= 34000;
#endif
	value /= 1310720;
	value = value >> 1; // divide by two (we are measuring round trip time)

	return (int) value;
}
