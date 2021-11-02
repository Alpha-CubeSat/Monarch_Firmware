/*
 * RF_TX_Tasks.h
 *
 *  Created on: Sep 24, 2017
 *      Author: hunteradams
 */

#ifndef TASKS_RADIO_RF_TX_TASKS_H_
#define TASKS_RADIO_RF_TX_TASKS_H_

#include "RF_Globals.h"
#include <ti/sysbios/knl/Task.h>
#include "../../Peripherals/Pin_Initialization.h"
#include "../Semaphore_Initialization.h"
#include "../IMU/LSM9DS1.h"
#include "HardLink.h"
//#include "TRIAD.h"

Task_Struct txDataTask;


static uint8_t txDataTaskStack[700];
#pragma DATA_ALIGN(txDataTaskStack, 8)

// receive order : 0010 0000 -> 0000 0100, 0101 0011 -> 1100 1010 ...
uint8_t message[30] = {0x20, 0x53, 0x50, 0x41, 0x43, 0x45, 0x20, 0x53,
		0x59, 0x53, 0x54, 0x45, 0x4d, 0x53, 0x20, 0x44, 0x45, 0x53,
		0x49, 0x47, 0x4e, 0x20, 0x53, 0x54, 0x55, 0x44, 0x49, 0x4f,
		0x20, 0x20};

Void txDataTaskFunc(UArg arg0, UArg arg1)
{
    HardLink_init();
    HardLink_setRfPower(14);
    HardLink_setFrequency(915000000);
    long iteration = 0;

	uint16_t counter = 0x00;
	// delay to wait for initializing
	Task_sleep(200000);
	while(1) {
		Semaphore_pend(txDataSemaphoreHandle, BIOS_WAIT_FOREVER);
		Semaphore_pend(batonSemaphoreHandle, BIOS_WAIT_FOREVER);

		// goodToGo is unlocked in task magTaskFunc
		if(goodToGo){

//			EasyLink_abort();
		    struct HardLink_packet txPacket;

//			txPacket.payload[0] = BEACON;
//			txPacket.payload[1] = PERSONAL_ADDRESS;

			txPacket.payload[0] = (counter>>8)&0xff;
			txPacket.payload[1] = counter&0xff;
			txPacket.payload[2] = upperPart(ax);
			txPacket.payload[3] = lowerPart(ax);
			txPacket.payload[4] = upperPart(ay);
			txPacket.payload[5] = lowerPart(ay);
			txPacket.payload[6] = upperPart(az);
			txPacket.payload[7] = lowerPart(az);

			// how may Bytes of data is going to be sent
	        uint16_t packetlen = RFEASYLINKTXPAYLOAD_LENGTH;
	        txPacket.size = packetlen;

			if (counter > 0xfffe){
				counter = 0;
			}
			else{
				counter = counter + 0x01;
			}
//			int i=0;
//			for (i = 0; i < sizeof(message); i++)
//			{
//			  txPacket.payload[i] = message[i];
//			}
//			txPacket.payload[4] = counter;


			memcpy(txPacket.payload, message, 8);
			HardLink_status result = HardLink_send(&txPacket);
			Task_sleep(1000 * 100000);

			if (result == HardLink_status_Success)
			{
				/* Toggle LEDs to indicate TX */
		        PIN_setOutputValue(pinHandle, Board_PIN_LED0,1);
		        PIN_setOutputValue(pinHandle, Board_PIN_LED1,1);
		        Task_sleep(20000);
			}

	        PIN_setOutputValue(pinHandle, Board_PIN_LED0,0);
	        PIN_setOutputValue(pinHandle, Board_PIN_LED1,0);

			Semaphore_post(rxRestartSemaphoreHandle);
		}
		Semaphore_post(batonSemaphoreHandle);
	}
}

void createRFTXTasks()
{
    Task_Params task_params;
    Task_Params_init(&task_params);
    task_params.stackSize = 700;
    task_params.priority = 2;
	task_params.stack = &txDataTaskStack;
	Task_construct(&txDataTask, txDataTaskFunc,
				   &task_params, NULL);
}


#endif /* TASKS_RADIO_RF_TX_TASKS_H_ */
