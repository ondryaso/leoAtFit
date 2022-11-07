/*
 * sensor.h
 * Author: Ondrej Ondryas (xondry02@stud.fit.vutbr.cz)
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#define ENABLE_DECIMAL 0

#include <MK60D10.h>

#define SENSOR_TRIG_PIN_NUM 29
#define SENSOR_TRIG_PIN_MASK ((uint32_t) 0x20000000)
#define SENSOR_ECHO_PIN_NUM 28
#define SENSOR_ECHO_PIN_MASK ((uint32_t) 0x10000000)

extern volatile uint64_t micros;

/* Sends a trigger pulse to the sensor. */
void Sensor_SendTrigger(void);

/* Handles an interrupt from the GPIO registering an edge on the sensor's echo signal. */
void Sensor_HandleInterrupt(void);

/* Returns the last measured distance value in centimetres (or millimetres if ENABLE_DECIMAL==1). */
int Sensor_GetValue(void);

#endif /* SENSOR_H_ */
