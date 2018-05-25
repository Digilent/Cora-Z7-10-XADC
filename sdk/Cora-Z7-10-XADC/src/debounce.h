/*
 * debounce.h
 *
 *  Created on: Apr 23, 2018
 *      Author: arthur
 */

#ifndef SRC_DEBOUNCE_H_
#define SRC_DEBOUNCE_H_

#include "xil_types.h"

typedef struct Debounce {
	u8 Data;
	u8 Flag;
	u8 State;
	u32 Count;
	u32 Period;
} Debounce;

void Debounce_Init(Debounce *InstancePtr, u32 Period);

void Debounce_Update(Debounce *InstancePtr, u8 NewData);

#endif /* SRC_DEBOUNCE_H_ */
