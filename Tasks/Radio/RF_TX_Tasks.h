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
#include "../I2C/LSM9DS1.h"
#include "../Semaphore_Initialization.h"
#include "../Shared_Resources.h"
#include <ti/devices/cc13x0/driverlib/sys_ctrl.h>

Task_Struct txDataTask;

static uint8_t txDataTaskStack[700];
#pragma DATA_ALIGN(txDataTaskStack, 8)

bool bAttemptRetransmission = false;

uint8_t message[30] = {0x20, 0x53, 0x50, 0x41, 0x43, 0x45, 0x20, 0x53,
		0x59, 0x53, 0x54, 0x45, 0x4d, 0x53, 0x20, 0x44, 0x45, 0x53,
		0x49, 0x47, 0x4e, 0x20, 0x53, 0x54, 0x55, 0x44, 0x49, 0x4f,
		0x20, 0x20};

Void txDataTaskFunc(UArg arg0, UArg arg1)
{
    HardLink_init();
    HardLink_setRfPower(14);
    HardLink_setFrequency(915000000);

    // delay to wait for initializing
    Task_sleep(200000);
	while(1) {

		Semaphore_pend(txDataSemaphoreHandle, BIOS_WAIT_FOREVER);

		Power_setDependency(PowerCC26XX_PERIPH_TRNG);

		struct HardLink_packet txPacket;
		//Load the payload
		if(bAttemptRetransmission == false){
			txPacket.payload[0] = 0x1e;

			txPacket.payload[1] = upperPart(ax);
			txPacket.payload[2] = lowerPart(ax);
			txPacket.payload[3] = upperPart(ay);
			txPacket.payload[4] = lowerPart(ay);
			txPacket.payload[5] = upperPart(az);
			txPacket.payload[6] = lowerPart(az);
			txPacket.size = 6;
		}
		//Green light on
		PIN_setOutputValue(pinHandle, Board_PIN_LED1,1);

		//Transmit
		HardLink_status result;
		do{
		    result = HardLink_send(&txPacket);
		}while(result != HardLink_status_Success);

		Task_sleep(10000);
		//Green light off
		PIN_setOutputValue(pinHandle, Board_PIN_LED1,0);


		Watchdog_clear(watchdogHandle);
		Watchdog_close(watchdogHandle);

		Semaphore_post(startSemaphoreHandle);

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
