/*
 * display.h
 * Author: Ondrej Ondryas (xondry02@stud.fit.vutbr.cz)
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "MK60D10.h"

#define DISP_D1 0x200 // PTA9
#define DISP_D2 0x40  // PTA6
#define DISP_D3 0x800 // PTA11
#define DISP_D4 0x80  // PTA7

#define DISP_N0A 0x0100 // "0"
#define DISP_N0D 0xb300
#define DISP_N1A 0x0100 // "1"
#define DISP_N1D 0x0100
#define DISP_N2A 0x0400 // "2"
#define DISP_N2D 0xb100
#define DISP_N3A 0x0500 // "3"
#define DISP_N3D 0x9100
#define DISP_N4A 0x0500 // "4"
#define DISP_N4D 0x0300
#define DISP_N5A 0x0500 // "5"
#define DISP_N5D 0x9200
#define DISP_N6A 0x0500 // "6"
#define DISP_N6D 0xb200
#define DISP_N7A 0x0100 // "7"
#define DISP_N7D 0x1100
#define DISP_N8A 0x0500 // "8"
#define DISP_N8D 0xb300
#define DISP_N9A 0x0500 // "9"
#define DISP_N9D 0x9300

#define DISP_NEA 0x0400 // "-"
#define DISP_NED 0x0000

#define DISP_DPD 0x4000 // decimal point

#define DISP_PIN_MASK_A 0x0fc0
#define DISP_PIN_MASK_D 0xf300
#define DISP_DIGITS_ALL_A 0xac0

/*
 * Sets the number that should be displayed.
 * Set data to -1 to display "----".
 * dpPos is the index of the digit after which a decimal point will be turned on.
 * Set dpPos to -1 to disable the decimal point.
 */
void Display_SetData(int data, int dpPos);

/*
 * Handles display output. Called periodically from a timer ISR.
 */
void Display_HandleInterrupt(void);

#endif /* DISPLAY_H_ */
