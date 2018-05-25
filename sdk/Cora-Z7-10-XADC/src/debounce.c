/*
 * debounce.c
 *
 *  Created on: Apr 23, 2018
 *      Author: arthur
 */

#include "debounce.h"

void Debounce_Init(Debounce *InstancePtr, u32 Period) {
	InstancePtr->Data = 0;
	InstancePtr->Flag = 0;
	InstancePtr->State = 0;
	InstancePtr->Count = 0;
	InstancePtr->Period = Period;
}

void Debounce_Update(Debounce *InstancePtr, u8 NewData) {
	InstancePtr->Flag = 0;
	if (InstancePtr->State == 0) {
		if (InstancePtr->Data != NewData) {
			InstancePtr->State = 1;
			InstancePtr->Count = 0;
		} // else do nothing
	} else {
		if (InstancePtr->Data != NewData) {
			InstancePtr->Count ++;
			if (InstancePtr->Count >= InstancePtr->Period) {
				if (NewData != 0)
					InstancePtr->Flag = 1;
				InstancePtr->State = 0;
				InstancePtr->Data = NewData;
			}
		} else {
			InstancePtr->State = 0;
		}
	}
}
