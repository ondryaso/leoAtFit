/*
 * MK60D10_Project.c
 * Author: Ondrej Ondryas (xondry02@stud.fit.vutbr.cz)
 */

#include "MK60D10.h"

#include "display.h"
#include "sensor.h"
#include "board/clock_config.h"

#include <stdio.h>

/* Initialise the MCU */
void MCUInit(void) {
	/*
	 MCG->C4 |= ( MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01));
	 SIM->CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
	 */

	// Let's use generated clock config tools because why not. The two lines above can be used instead.
	BOARD_InitBootClocks();

	// Deactivate watchdog
	WDOG->STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}

/* Set the mode of the required pins and GPIO */
void PortsInit(void) {
	/* Turn on clocks for ports A and D */
	SIM->SCGC5 = SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTD_MASK
			| SIM_SCGC5_PORTA_MASK;

	/* Set PortA (6, 7, 8, 9, 10, 11) and PortD (8, 9, 12, 13, 14, 15) pins for GPIO functionality */
	uint32_t portConfig = PORT_PCR_DSE(1) | PORT_PCR_MUX(1);

	PORTA->GPCLR = (DISP_PIN_MASK_A << 16) | portConfig; // 0FC0 selects pins (GPWE), 0140 is written to PCR (high drive strength, mux 001 - GPIO)
	PORTD->GPCLR = (DISP_PIN_MASK_D << 16) | portConfig;

	/* Set PortA pin 29 for GPIO (sensor pulse) */
	PORTA->PCR[SENSOR_TRIG_PIN_NUM] = PORT_PCR_MUX(1); // mux 001 - GPIO

	/* Set PortE pin 28 for GPIO (interrupt on both edges) */
	PORTE->PCR[SENSOR_ECHO_PIN_NUM] = PORT_PCR_ISF_MASK
			| PORT_PCR_IRQC(0xB) | PORT_PCR_MUX(1) | PORT_PCR_PE(1); // pulldown

	/* Configure GPIO pins (all outputs) */
	GPIOA->PDDR = DISP_PIN_MASK_A | SENSOR_TRIG_PIN_MASK;
	GPIOD->PDDR = DISP_PIN_MASK_D;
	// GPIOE is configured for input on reset

	/* Set all outputs to zero */
	GPIOA->PCOR = DISP_PIN_MASK_A | SENSOR_TRIG_PIN_MASK;
	GPIOD->PCOR = DISP_PIN_MASK_D;

	/* Enable interrupts for PORTE */
	NVIC_EnableIRQ(PORTE_IRQn);
}

/* Initialise the low-power timer (used to refresh the display) */
void LPTMR0Init(void) {
	SIM->SCGC5 |= SIM_SCGC5_LPTIMER_MASK; // Enable clock to LPTMR
	LPTMR0->CSR &= ~LPTMR_CSR_TEN_MASK;   // Turn OFF LPTMR to perform setup
	LPTMR0->PSR = (LPTMR_PSR_PBYP_MASK | LPTMR_PSR_PCS(1)); // Use LPO 1kHz clock, bypass the prescaler
	LPTMR0->CMR = 3; // Set compare value (3 -> 1 kHz divided by 4 -> 250 Hz refresh rate -> 62.5 Hz per digit)
	LPTMR0->CSR = (LPTMR_CSR_TCF_MASK     // Clear any pending interrupt (now)
	| LPTMR_CSR_TIE_MASK);                // LPT interrupt enabled

	NVIC_EnableIRQ(LPTMR0_IRQn);          // enable interrupts from LPTMR0
	LPTMR0->CSR |= LPTMR_CSR_TEN_MASK;    // Turn ON LPTMR0 and start counting
}

/* Initialise the flexible timer (used when counting pulse lengths) */
void FTM0Init(void) {
	SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK; // Enable clock to FTM0
	FTM0->SC = 0; // Disable FTM0
	FTM0->CONF = 0xc0; // Enable FTM0 in BDM mode
	FTM0->MODE = 0xb; // FTM enable, initialize channels output
	FTM0->CNTIN = 0; // Counter initial value
	FTM0->MOD = UINT16_MAX - 1;
	FTM0->SC = 0xC | FTM_SC_TOIE(1); // Divide by 16, Clock source = system, interrupt on overflow

	NVIC_EnableIRQ(FTM0_IRQn);
}

void LPTMR0_IRQHandler(void) {
	Display_HandleInterrupt();
	LPTMR0->CSR |= LPTMR_CSR_TCF_MASK;
}

void PORTE_IRQHandler(void) {
	if (PORTE->ISFR & SENSOR_ECHO_PIN_MASK) {
		Sensor_HandleInterrupt();
	}

	// Clear the interrupt flags
	PORTE->ISFR = PORTE->ISFR;
}

volatile uint64_t micros = 0;
void FTM0_IRQHandler(void) {
	if (FTM0->SC & FTM_SC_TOF_MASK) {
		FTM0->SC &= 0xFFFFFFFF ^ FTM_SC_TOF_MASK; // Clear TOF flag
		micros += UINT16_MAX; // Add to overflow counter
	}
}

int main(void) {
	// Init components
	MCUInit();
	PortsInit();
	LPTMR0Init();
	FTM0Init();

	int lastValue = -1;
	Display_SetData(-1, -1);

	// Periodically get values from the sensor
	while (1) {
		int value = Sensor_GetValue();

		if (lastValue == value) {
			Sensor_SendTrigger();
		} else {
			lastValue = value;

			if (value == 0 || value >= 10000) {
				Display_SetData(-1, -1);
			} else {
#if ENABLE_DECIMAL == 1
				Display_SetData(value, 2);
#else
				Display_SetData(value, -1);
#endif
			}
		}
	}

	return 0;
}
