/*
 * display.c
 * Author: Ondrej Ondryas (xondry02@stud.fit.vutbr.cz)
 */

#include "display.h"

const unsigned int digitsAct[] = { DISP_D1, DISP_D2, DISP_D3, DISP_D4 };
const unsigned int digitsA[] = { DISP_N0A, DISP_N1A, DISP_N2A, DISP_N3A,
DISP_N4A, DISP_N5A, DISP_N6A, DISP_N7A, DISP_N8A, DISP_N9A };
const unsigned int digitsD[] = { DISP_N0D, DISP_N1D, DISP_N2D, DISP_N3D,
DISP_N4D, DISP_N5D, DISP_N6D, DISP_N7D, DISP_N8D, DISP_N9D };

volatile static int currentDigits[4];
volatile static int currentDpPos;
volatile static int currentlyShownDigitIndex = 0;

void Display_SetData(int data, int dpPos) {
	NVIC_DisableIRQ(LPTMR0_IRQn);
	// Clear pending interrupts?

	if (data == -1) {
		currentDigits[0] = -2;
		currentDigits[1] = -2;
		currentDigits[2] = -2;
		currentDigits[3] = -2;
	} else {
		currentDigits[0] = data / 1000;
		currentDigits[1] = (data % 1000) / 100;
		currentDigits[2] = (data % 100) / 10;
		currentDigits[3] = data % 10;
		currentDpPos = dpPos;

		if (data < 10)
			currentDigits[0] = currentDigits[1] = currentDigits[2] = -1;
		else if (data < 100)
			currentDigits[0] = currentDigits[1] = -1;
		else if (data < 1000)
			currentDigits[0] = -1;
	}

	NVIC_EnableIRQ(LPTMR0_IRQn);
}

void Display_HandleInterrupt(void) {
	int digit = currentDigits[currentlyShownDigitIndex];

	if (digit == -1) {
		PTA->PDOR = (PTA->PDOR & ~DISP_PIN_MASK_A) | DISP_DIGITS_ALL_A;
		PTD->PCOR = DISP_PIN_MASK_D;
	} else if (digit == -2) {
		PTA->PDOR = (PTA->PDOR & ~DISP_PIN_MASK_A) | DISP_NEA | (DISP_DIGITS_ALL_A & ~digitsAct[currentlyShownDigitIndex]);
		PTD->PCOR = DISP_PIN_MASK_D;
	} else {
		int previousI = (currentlyShownDigitIndex + 3) & 3; // same as % 4

		// Disable previous digit (set -> set to log. 1 -> no voltage difference)
		PTA->PSOR = digitsAct[previousI];

		// Set segments for current digit
		PTD->PDOR = (PTD->PDOR & ~DISP_PIN_MASK_D) | digitsD[digit];
		PTA->PDOR = (PTA->PDOR & ~DISP_PIN_MASK_A) | digitsA[digit]
				| DISP_DIGITS_ALL_A;

		if (currentlyShownDigitIndex == currentDpPos) {
			PTD->PSOR = DISP_DPD;
		}

		// Enable current digits (clear -> set to log. 0 -> provide voltage difference)
		PTA->PCOR = digitsAct[currentlyShownDigitIndex];
	}

	currentlyShownDigitIndex = (currentlyShownDigitIndex + 1) & 3; // same as % 4
}
