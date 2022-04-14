/*
 * GPS_Task.h
 *
 *  Created on: Sep 24, 2017
 *      Author: hunteradams
 */

#ifndef TASKS_GPS_TASK_H_
#define TASKS_GPS_TASK_H_

#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>
#include <ti/sysbios/knl/Task.h>
#include "Semaphore_Initialization.h"
#include "Shared_Resources.h"

#include <string.h>
#include <stdlib.h>

int bytesRead = 0;

Task_Struct gpsTask;
static uint8_t gpsTaskStack[2000];

//copy string (pos n to pos m) from s2 to s1
char* StrnmCpy(char *s1, char *s2, size_t n, size_t m)
{
   uint8_t i;
   char *s;

   for (s=s1, i=n, s2+=n; i<=m; i++)
      *s++ = *s2++;
   *s = '\0';

   return s1;
}

// find c in s starting from pos st
int8_t StrFnd(char *s, char c, size_t st)
{
   int8_t l;

   for (l=st, s+=st ; *s != '\0' ; l++, s++)
      if (*s == c)
         return l;
   return -1;
}

void Serial_RxDataCallback(UART_Handle handle, void *buffer, size_t size)
{

    Semaphore_post(readSemaphoreHandle);
    bytesRead = size;
    bytes_read = size;
}

void GPRMC_decode(char *GPRMCStr, GPRMCInfo *RMCInfo)
{
   int8_t p1, p2;
   char TempStr[16];

   p1 = StrFnd(GPRMCStr, ',', 0);      //find first comma
   if (p1 == 6)
   {
      //check for valid packet:
      if ( (StrFnd(GPRMCStr, 'A', p1+1) == 18) && (GPRMCStr[0]=='$')) //valid?
      {
         RMCInfo->Valid = 'A';

         //Get time
         p1 = StrFnd(GPRMCStr, ',', 0);      //find first comma
         p2 = StrFnd(GPRMCStr, ',', p1+1);   //find next comma
         RMCInfo->DT.Hour = atoi(StrnmCpy(TempStr, GPRMCStr, p1+1, p1+2));   //hour
         RMCInfo->DT.Minute = atoi(StrnmCpy(TempStr, GPRMCStr, p1+3, p1+4)); //min
         RMCInfo->DT.Second = atoi(StrnmCpy(TempStr, GPRMCStr, p1+5, p1+6)); //sec

         //Get latitude & direction
         p1 = StrFnd(GPRMCStr, ',', p2+1);   //find next comma
         p2 = StrFnd(GPRMCStr, ',', p1+1);   //find next comma
         RMCInfo->Latitude = atof(StrnmCpy(TempStr, GPRMCStr, p1+1, p2-1));
         RMCInfo->N_S = GPRMCStr[p2+1];

         //Get longitude & direction
         p1 = StrFnd(GPRMCStr, ',', p2+1);   //find next comma
         p2 = StrFnd(GPRMCStr, ',', p1+1);   //find next comma
         RMCInfo->Longitude = atof(StrnmCpy(TempStr, GPRMCStr, p1+1, p2-1));
         RMCInfo->E_W = GPRMCStr[p2+1];

         //Get speed
         p1 = StrFnd(GPRMCStr, ',', p2+1);   //find next comma
         p2 = StrFnd(GPRMCStr, ',', p1+1);   //find next comma
         RMCInfo->Speed = atof(StrnmCpy(TempStr, GPRMCStr, p1+1, p2-1));

         //Get date
         p1 = StrFnd(GPRMCStr, ',', p2+1);   //find next comma
         p2 = StrFnd(GPRMCStr, ',', p1+1);   //find next comma
         RMCInfo->DT.Day = atoi(StrnmCpy(TempStr, GPRMCStr, p1+1, p1+2));  //dd
         RMCInfo->DT.Month = atoi(StrnmCpy(TempStr, GPRMCStr, p1+3, p1+4));//mm
         RMCInfo->DT.Year = atoi(StrnmCpy(TempStr, GPRMCStr, p1+5, p1+6)); //yy
      }
      else                                //not valid
      {
         RMCInfo->Valid = 'V';
      }
   }
}

Void gpsFunc(UArg arg0, UArg arg1)
{

//	bool gotFix;

	while (1) {
		/* Check whether the GPS has a navigation fix */
//		gotFix = Semaphore_pend(gpsfixSemaphoreHandle, 100);
		Semaphore_pend(gpsLockSemaphoreHandle, BIOS_WAIT_FOREVER);

		uart = UART_open(Board_UART0, &uartParams);

		if (uart == NULL) {
			while (1);
		}
		UART_control(uart, UARTCC26XX_CMD_RETURN_PARTIAL_ENABLE, NULL);

//		uint8_t query[] = {0xa0, 0xa1, 0x00, 0x04, 0x64, 0x0a, 0x01, 0x01, 0x6e, 0x0d, 0x0a};
//		UART_write(uart, query, sizeof(query));
//		uint8_t nmea[] = {0xa0, 0xa1, 0x00, 0x09, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x08, 0x0d, 0x0a};
//		UART_write(uart, nmea, sizeof(nmea));

		bool gotData = 0;

		while (!(gotData & (bytesRead==72))) {
			/* Clear UART FIFO */
			UART_control(uart, UARTCC26XX_CMD_RX_FIFO_FLUSH, NULL);
			/* Read UART from GPS */
			int numBytes = UART_read(uart, &input, sizeof(input));
			//Parse the GPRMC sentence
			GPRMC_decode(input, &gps_data);
			/* Pend on a semaphore indicating data received */
			gotData = Semaphore_pend(readSemaphoreHandle, 200000);
		}

		/* Clear the watchdog timer */
//		Watchdog_clear(watchdogHandle);
		UART_write(uart, &input, bytesRead);
		UART_control(uart, UARTCC26XX_CMD_RX_FIFO_FLUSH, NULL);

		UART_readCancel(uart);
		UART_writeCancel(uart);
		UART_close(uart);

		PIN_setOutputValue(pinHandle, IOID_21, 0);
		PIN_setOutputValue(pinHandle, Board_PIN_LED0,0);
		PIN_setOutputValue(pinHandle, Board_PIN_LED1,0);

		Semaphore_post(txDataSemaphoreHandle);

		/* Sleep (1 hr) */
//		Task_sleep(360000000);

	}
}

void createGPSTask()
{
	Task_Params task_params;
	Task_Params_init(&task_params);
	task_params.stackSize = 2000;
	task_params.priority = 4;
	task_params.stack = &gpsTaskStack;
	Task_construct(&gpsTask, gpsFunc,
				   &task_params, NULL);
}

#endif /* TASKS_GPS_TASK_H_ */
