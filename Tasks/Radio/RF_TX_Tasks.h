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
#include "HardLink.h"
#include <time.h>
#include <stdio.h>

Task_Struct txDataTask;

static uint8_t txDataTaskStack[700];
#pragma DATA_ALIGN(txDataTaskStack, 8)

/*
 * Set to 1 if you want to attempt to retransmit a packet that couldn't be
 * transmitted after the CCA
 */

uint8_t message[30] = {0x20, 0x53, 0x50, 0x41, 0x43, 0x45, 0x20, 0x53,
        0x59, 0x53, 0x54, 0x45, 0x4d, 0x53, 0x20, 0x44, 0x45, 0x53,
        0x49, 0x47, 0x4e, 0x20, 0x53, 0x54, 0x55, 0x44, 0x49, 0x4f,
        0x20, 0x20};

int done = 1;

Void txDataTaskFunc(UArg arg0, UArg arg1)
{
//    clock_t start = clock();
//    // unit 10 microseconds, which means 1 second = Task_sleep(1000000)
//    Task_sleep(200000);
//    clock_t end = clock();
//    double time_taken = (double)(end - start)/(CLOCKS_PER_SEC/1000); // calculate the elapsed time
//    printf("The program took %f seconds to execute", time_taken);

    HardLink_init();
    HardLink_setRfPower(5);
    HardLink_setFrequency(915000000);
    long iteration = 0;

    while(1) {
        printf("sending iteration : %d\n", iteration);
        ++iteration;
//        // wait for sensor data from 1 thread
        Semaphore_pend(txDataSemaphoreHandle, 10000);
        // ???
        Power_setDependency(PowerCC26XX_PERIPH_TRNG);
        // initialize transmitting packet
        struct HardLink_packet txPacket;

        // read sensor data into the payload of sending packet
        txPacket.payload[0] = 0x1e;

        txPacket.payload[1] = upperPart(ax);
        txPacket.payload[2] = lowerPart(ax);
        txPacket.payload[3] = upperPart(ay);
        txPacket.payload[4] = lowerPart(ay);
        txPacket.payload[5] = upperPart(az);
        txPacket.payload[6] = lowerPart(az);

        txPacket.payload[7] = upperPart(gx);
        txPacket.payload[8] = lowerPart(gx);
        txPacket.payload[9] = upperPart(gy);
        txPacket.payload[10] = lowerPart(gy);
        txPacket.payload[11] = upperPart(gz);
        txPacket.payload[12] = lowerPart(gz);

        txPacket.payload[13] = upperPart(mx);
        txPacket.payload[14] = lowerPart(mx);
        txPacket.payload[15] = upperPart(my);
        txPacket.payload[16] = lowerPart(my);
        txPacket.payload[17] = upperPart(mz);
        txPacket.payload[18] = lowerPart(mz);

        txPacket.payload[19] = upperPart(tx_temp);
        txPacket.payload[20] = lowerPart(tx_temp);

        txPacket.payload[21] = upperPart(tx_humidity);
        txPacket.payload[22] = lowerPart(tx_humidity);

        txPacket.payload[23] = upperPart(tx_light_top);
        txPacket.payload[24] = lowerPart(tx_light_top);
        txPacket.payload[25] = upperPart(tx_light_bottom);
        txPacket.payload[26] = lowerPart(tx_light_bottom);
        //remove this
        txPacket.payload[27] = upperPart(capacitor_charge);
        txPacket.payload[28] = lowerPart(capacitor_charge);

        uint16_t packetlen = RFEASYLINKTXPAYLOAD_LENGTH;// + bytes_read;
        txPacket.size = packetlen;

        // watchdog to set timeout of program
        Watchdog_clear(watchdogHandle);
        Watchdog_close(watchdogHandle);

        PIN_setOutputValue(pinHandle, Board_PIN_LED0,1);
        PIN_setOutputValue(pinHandle, Board_PIN_LED1,1);

        // for testing, copy data of message into txPacket
        memcpy(txPacket.payload, message, RFEASYLINKTXPAYLOAD_LENGTH);
        HardLink_send(&txPacket);

        PIN_setOutputValue(pinHandle, Board_PIN_LED0,0);
        PIN_setOutputValue(pinHandle, Board_PIN_LED1,0);

//        // unit 10 microseconds, which means 1 second = Task_sleep(1000000)
        Task_sleep(100000);
//        Semaphore_post(startSemaphoreHandle);
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
