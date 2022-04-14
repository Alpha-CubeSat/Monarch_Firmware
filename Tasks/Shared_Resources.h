/*
 * Shared_Resources.h
 *
 *  Created on: Sep 24, 2017
 *      Author: hunteradams
 */

#ifndef TASKS_SHARED_RESOURCES_H_
#define TASKS_SHARED_RESOURCES_H_


typedef struct _DateTimeInfo
{
   uint8_t Day;
   uint8_t Month;
   uint8_t Year;
   uint8_t Hour;
   uint8_t Minute;
   uint8_t Second;
} DateTimeInfo;

typedef struct _GPRMCInfo
{
   char Valid;
   DateTimeInfo DT;
   float Latitude;
   char N_S;
   float Longitude;
   char E_W;
   float Speed;
} GPRMCInfo;


// Name
// Payload Size

UART_Handle uart;
UART_Params uartParams;

/* Capacitor state of charge */
uint16_t capacitor_charge;
uint32_t capacitor_charge_uv;

/* Light sensors */
uint16_t tx_light_top = 0;
uint32_t tx_light_top_uv = 0;
uint16_t tx_light_bottom = 0;
uint32_t tx_light_bottom_uv = 0;

/* Temperature/Humidity sensor */
uint16_t tx_temp = 0;
uint16_t tx_humidity = 0;

/* IMU */
int16_t tx_gx = 0;
int16_t tx_gy = 0;
int16_t tx_gz = 0;

int16_t tx_ax = 0;
int16_t tx_ay = 0;
int16_t tx_az = 0;

int16_t tx_mx = 0;
int16_t tx_my = 0;
int16_t tx_mz = 0;

// GPS Data
int bytes_read = 0;
char input[512] = {0};
GPRMCInfo gps_data;
// Checksum


#endif /* TASKS_SHARED_RESOURCES_H_ */
