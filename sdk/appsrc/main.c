/*
 * main.c
 *
 *  Created on: Apr 18, 2018
 *      Author: arthur
 */

#include "PWM.h"
#include "xsysmon.h"
#include "xparameters.h"
#include "sleep.h"
#include "stdio.h"
#include "xgpio.h"
#include "xil_types.h"
#include "debounce.h"

#define RGBLED_BASEADDR XPAR_PWM_0_PWM_AXI_BASEADDR
#define LED_ON_DUTY 0x3FFF
#define LED_OFF_DUTY 0x3000
#define XADC_DEVICE_ID XPAR_XADC_WIZ_0_DEVICE_ID
#define BTN_DEVICE_ID XPAR_AXI_GPIO_0_DEVICE_ID
// Channels 0, 1, 5, 6, 8, 9, 12, 13, 15, VPVN are available
// Channels 0, 8, 12 are differential 1.0V max
// Channels 1, 5, 6, 9, 13, 15 are single-ended 3.3V max
// All channels should be used in differential input mode
#define XADC_SEQ_CHANNELS 0xB3630800
#define XADC_CHANNELS 0xB3630008
#define NUMBER_OF_CHANNELS 10
const u8 Channel_List[NUMBER_OF_CHANNELS] = {
	3, // Start with VP/VN
	28, 16, 24, // Diff. Channels in ascending order
	17, 25, 22, 31, 21, 29 // Single-Ended Channels in ascending order
}; // 00008
const char *Channel_Names[32] = {
	"", "", "", "VP-VN",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"A8-A9", "A0", "", "",
	"", "A4", "A2", "",
	"A10-A11", "A1", "", "",
	"A6-A7", "A5", "", "A3"
};

#define Test_Bit(VEC,BIT) ((VEC&(1<<BIT))!=0)

void RGBLED_SetColor(u32 base_address, u16 r, u16 g, u16 b) {
	PWM_Set_Duty(RGBLED_BASEADDR, b, 0);
	PWM_Set_Duty(RGBLED_BASEADDR, g, 1);
	PWM_Set_Duty(RGBLED_BASEADDR, r, 2);
}
void RGBLED_Init(u32 base_address) {
	PWM_Set_Period(base_address, 0xffff);
	RGBLED_SetColor(base_address, 0, 0, 0);
	PWM_Enable(base_address);
}

void Xadc_Init(XSysMon *InstancePtr, u32 DeviceId) {
	XSysMon_Config *ConfigPtr;
	ConfigPtr = XSysMon_LookupConfig(DeviceId);
	XSysMon_CfgInitialize(InstancePtr, ConfigPtr, ConfigPtr->BaseAddress);

	// Disable the Channel Sequencer before configuring the Sequence registers.
	XSysMon_SetSequencerMode(InstancePtr, XSM_SEQ_MODE_SAFE);
	// Disable all alarms
	XSysMon_SetAlarmEnables(InstancePtr, 0x0);
	// Set averaging for all channels to 16 samples
	XSysMon_SetAvg(InstancePtr, XSM_AVG_16_SAMPLES);
	// Set differential input mode for all channels
	XSysMon_SetSeqInputMode(InstancePtr, XADC_SEQ_CHANNELS);
	// Set 6ADCCLK acquisition time in all channels
	XSysMon_SetSeqAcqTime(InstancePtr, XADC_SEQ_CHANNELS);
	// Disable averaging in all channels
	XSysMon_SetSeqAvgEnables(InstancePtr, XADC_SEQ_CHANNELS);
	// Enable all channels
	XSysMon_SetSeqChEnables(InstancePtr, XADC_SEQ_CHANNELS);
	// Set the ADCCLK frequency equal to 1/32 of System clock
	XSysMon_SetAdcClkDivisor(InstancePtr, 32);
	// Enable Calibration
	XSysMon_SetCalibEnables(InstancePtr, XSM_CFR1_CAL_PS_GAIN_OFFSET_MASK | XSM_CFR1_CAL_ADC_GAIN_OFFSET_MASK);
	// Enable the Channel Sequencer in continuous sequencer cycling mode
	XSysMon_SetSequencerMode(InstancePtr, XSM_SEQ_MODE_CONTINPASS);
	// Clear the old status
//	XSysMon_GetStatus(InstancePtr);
}

#define READDATA_DBG 0
u32 Xadc_ReadData (XSysMon *InstancePtr, u16 RawData[32])
{
	u8 Channel;

	if (READDATA_DBG != 0)
		xil_printf("Waiting for EOS...\r\n");

	// Clear the Status
	XSysMon_GetStatus(InstancePtr);
	// Wait until the End of Sequence occurs
	while ((XSysMon_GetStatus(InstancePtr) & XSM_SR_EOS_MASK) != XSM_SR_EOS_MASK);

	if (READDATA_DBG != 0)
		xil_printf("Capturing XADC Data...\r\n");

	for (Channel=0; Channel<32; Channel++) {
		if (((1 << Channel) & XADC_CHANNELS) != 0) {
			if (READDATA_DBG != 0)
				xil_printf("Capturing Data for Channel %d\r\n", Channel);
			RawData[Channel] = XSysMon_GetAdcData(InstancePtr, Channel);
		}
	}
	return XADC_CHANNELS; // return a high bit for each channel successfully read
}

float Xadc_RawToVoltage(u16 Data, u8 Channel) {
	float FloatData;
	float Scale;

	switch (Channel) {
	case 3: // VP/VN (Cora Dedicated Analog Input)
	case 16: // AUX0 (Cora A8/A9 Diff. Analog Input)
	case 24: // AUX8 (Cora A10/A11 Diff. Analog Input)
	case 28: Scale = 1.0; break; // AUX12 (Cora A6/A7 Diff. Analog Input)
	case 17: // AUX1 (Cora A0 Single-Ended Analog Input)
	case 21: // AUX5 (Cora A4 Single-Ended Analog Input)
	case 22: // AUX6 (Cora A2 Single-Ended Analog Input)
	case 25: // AUX9 (Cora A1 Single-Ended Analog Input)
	case 29: // AUX13 (Cora A5 Single-Ended Analog Input)
	case 31: Scale = 3.3; break; // AUX15 (Cora A3 Single-Ended Analog Input)
	default: Scale = 0.0;
	}
	if (Test_Bit(Data, 15)) {
		FloatData = -Scale;
		Data = ~Data + 1;
	} else
		FloatData = Scale;
	FloatData *= (float)Data / (float)0xFFFF;
	return FloatData;
}

void Btn_Init(XGpio *InstancePtr, u32 DeviceId) {
	XGpio_Config *ConfigPtr;
	printf("Btn_Init 1");
	ConfigPtr = XGpio_LookupConfig(DeviceId);
	printf("Btn_Init 2");
	XGpio_CfgInitialize(InstancePtr, ConfigPtr, ConfigPtr->BaseAddress);
	printf("Btn_Init 3");
	XGpio_SetDataDirection(InstancePtr, 1, 0b11);
	printf("Btn_Init 4");
}

void Xadc_Demo(XSysMon *InstancePtr, u32 RGBLED_BaseAddr, u32 ChannelSelect) {
	u16 Xadc_RawData[32];
	u32 ChannelValidVector;
	float Xadc_VoltageData;
	ChannelValidVector = Xadc_ReadData(InstancePtr, Xadc_RawData);
	if (Test_Bit(ChannelValidVector, ChannelSelect)) {
		Xadc_VoltageData = Xadc_RawToVoltage(Xadc_RawData[ChannelSelect], ChannelSelect);
		printf("Analog Input %s: %.3fV\r\n", Channel_Names[ChannelSelect], Xadc_VoltageData);
		if (Xadc_VoltageData > 0.5)
			RGBLED_SetColor(RGBLED_BaseAddr, 0x0000, 0x8000, 0x0000);
		else if (Xadc_VoltageData < -0.5)
			RGBLED_SetColor(RGBLED_BaseAddr, 0x8000, 0x0000, 0x0000);
		else
			RGBLED_SetColor(RGBLED_BaseAddr, 0x0000, 0x0000, 0x0000);
	} else {
		printf("Channel %d (%s) Not Available\r\n", (int)ChannelSelect, Channel_Names[ChannelSelect]);
		RGBLED_SetColor(RGBLED_BaseAddr, 0, 0, 0);
	}
}

int main () {
	XSysMon Xadc;
	u8 ChannelIndex = 0;
	XGpio Btn;
	u32 Btn_Data;
	const u32 RGBLED_BaseAddr = RGBLED_BASEADDR;
	Debounce Btn0_Db, Btn1_Db;
	u32 time_count = 0;

	Xadc_Init(&Xadc, XADC_DEVICE_ID);
	RGBLED_Init(RGBLED_BaseAddr);
	Btn_Init(&Btn, BTN_DEVICE_ID);
	Debounce_Init(&Btn0_Db, 10000);
	Debounce_Init(&Btn1_Db, 10000);

	printf("Cora XADC Demo Initialized!\r\n");

	while(1) {
		Btn_Data = XGpio_DiscreteRead(&Btn, 1) & 0b11;
		Debounce_Update(&Btn0_Db, Btn_Data & 0b01);
		Debounce_Update(&Btn1_Db, Btn_Data & 0b10);

		if (Btn1_Db.Flag == 1 && Btn0_Db.Flag == 0) {
			if (ChannelIndex + 1 < NUMBER_OF_CHANNELS)
				ChannelIndex ++;
			else
				ChannelIndex = 0;
		} else if (Btn1_Db.Flag == 0 && Btn0_Db.Flag == 1) {
			if (ChannelIndex > 0)
				ChannelIndex --;
			else
				ChannelIndex = NUMBER_OF_CHANNELS-1;
		}

		time_count ++;
		if (time_count == 100000) { // print channel reading approx. 10x per second
			time_count = 0;
			Xadc_Demo(&Xadc, RGBLED_BaseAddr, Channel_List[ChannelIndex]);
		}
		usleep(1);
	}
}
