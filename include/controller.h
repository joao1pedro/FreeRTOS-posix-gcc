#ifndef CONTROLLER_H
#define CONTROLLER_H

/* Plant Control  */
#define CYCLE_RATE_MS           10
#define MAX_COMMS_DELAY         5

/* Keypad Scan*/
#define DELAY_PERIOD_KP         15
#define DELAY_PERIOD_LCD        50

/* LED  */
#define DELAY_PERIOD            1000
#define GREEN                   0
#define RED                     1

/* Web Server */
#define HTTP_REQUEST_PROC_LOAD  2000

/* PDA */
#define RS232_CHAR_PROC_LOAD    1500

#define CTRL_ALG_LOAD           5000

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"


#include "kb.h"
//#include <string.h>

/* Plant Control Task */
void vPlantControlTask( void *pvParameters );
void vSensorControlTask( void *pvParameters );
void PerformControl(float data1, float data2);

/* Web Server Task */
void vWebServerTask( void *pvParameters );
void ProcessHTTPData( float Data );
/* RS232 Task */
void vRS232Task( void *pvParameters );
void ProcessSerialCharacters( float Data);
/* Keypad Scan Task */
void vKeyScanTask( void *pvParmeters );

/* LED Task */
void vLEDTask( void *pvParameters );
int SystemIsHealthy( void );
int FlashLED( int led, int status );
/* CPU usage */
void vApplicationIdleHook( void );
void vCPUMonitorTask( void * pvParameters );

void initMenu( void );
void initCreateTimers( void );

/* Timers callback */
void vTimerCallback(TimerHandle_t xTimer);

#endif /* CONTROLLER_H */