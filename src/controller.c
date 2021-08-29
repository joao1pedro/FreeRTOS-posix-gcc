#include "controller.h"

QueueHandle_t xSensorQueue1;
QueueHandle_t xSensorQueue2;
QueueHandle_t xEthernetQueue;
QueueHandle_t xRS232Queue;

int statusLED = 1;
int systemHealth = 1;

int flagPDA = 0;
int flagWeb = 0;
int flagLED = 0;

void vPlantControlTask(void *pvParameters)
{
    TickType_t xLastWakeTime;

    float receivedData1, receivedData2;

    /* Queue init */
    xSensorQueue1 = xQueueCreate(1, sizeof(float));
    xSensorQueue2 = xQueueCreate(1, sizeof(float));
    xEthernetQueue = xQueueCreate(1, sizeof(float));
    xRS232Queue = xQueueCreate(1, sizeof(float));

    if (xSensorQueue1 == NULL || xSensorQueue2)
    {
        /* Queue was not created and must not be used. */
        printf("Queue create error\n");
    }

    // A
    xLastWakeTime = xTaskGetTickCount();

    // B
    for (;;)
    {
        // C
        vTaskDelayUntil(&xLastWakeTime, CYCLE_RATE_MS);
        // printf("Entrou na plant control\n");

        // D
        if (xQueuePeek(xSensorQueue1, &receivedData1, 0) == pdTRUE)
        {
            // E
            if (xQueuePeek(xSensorQueue2, &receivedData2, 0) == pdTRUE)
            {
                // Transmit Results
                xQueueOverwrite(xEthernetQueue, &receivedData1);
                xQueueOverwrite(xRS232Queue, &receivedData1);
                xQueueOverwrite(xEthernetQueue, &receivedData2);
                xQueueOverwrite(xRS232Queue, &receivedData2);
            }
        }
    }
}
void vSensorControlTask( void *pvParameters ){
    float data1 = 0;
    float data2 = 0;
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();

    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
        data1 = 0.0001*(rand()%15000);

        vTaskDelay(pdMS_TO_TICKS(1));
        data2 = 0.0005*(rand()%25000);

        xQueueOverwrite(xSensorQueue1, &data1);
        xQueueOverwrite(xSensorQueue2, &data2);
    }
    
}

void vKeyScanTask(void *pvParmeters)
{
    char key;
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, DELAY_PERIOD_KP);

        // Scan the keyboard.
        if (kbhit())
        {
            fflush(stdout);

            key = getchar();

            printf("\nkey pressed: %c\n", key);
            if(key == '0'){
                if(flagLED==1)
                    flagLED = 0;
                else
                    flagLED=1;
            }
            // if(key == '1'){
            //     if(flagPDA == 1)
            //         flagPDA=0;
            //     else
            //         flagPDA=1;
            // }
        }
    }
}

void vLEDTask(void *pvParmeters)
{
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        //Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, DELAY_PERIOD);

        // Flash the appropriate LED.
        if (SystemIsHealthy() == 1)
        {
            FlashLED(GREEN, statusLED);
        }
        else
        {
            FlashLED(RED, statusLED);
        }
    }
}

int SystemIsHealthy( void )
{
    /*
    verifica se houve erro de perca de deadline ou sobrecarga de cpu
        estiver tudo ok, retorna 1. Se não, retorna 0;
    */
    return systemHealth;
}

int FlashLED(int led, int status)
{
    const TickType_t xPeriod = pdMS_TO_TICKS(1000);
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        if (status == 1 && led == 0 && flagLED==1)
        {
            printf("\tLED: GREEN \n \t\t---> HIGH\r\n");
            vTaskDelayUntil(&xLastWakeTime, xPeriod);
            status = 0;
        }
        else if (status == 0 && led == 0 && flagLED==1)
        {
            printf("\tLED: GREEN \n \t\t---> LOW\r\n");
            vTaskDelayUntil(&xLastWakeTime, xPeriod);
            status = 1;
        }
        else if (status == 1 && led == 1 && flagLED==1)
        {
            printf("\tLED: RED \n \t\t---> HIGH\r\n");
            vTaskDelayUntil(&xLastWakeTime, xPeriod);
            status = 0;
        }
        else if (status == 0 && led == 1 && flagLED==1)
        {
            printf("\tLED: RED \n \t\t---> LOW\r\n");
            vTaskDelayUntil(&xLastWakeTime, xPeriod);
            status = 1;
        }
    }
}

/* RS232 Task */
void vRS232Task(void *pvParameters)
{
    int data;

    for (;;)
    {
        // Block until data arrives.  xRS232Queue is filled by the
        // RS232 interrupt service routine.  
        if (xQueuePeek(xRS232Queue, &data, 0) == pdTRUE)
        {
            ProcessSerialCharacters(data);

        }
    }
}

void ProcessSerialCharacters(float Data)
{
    if (flagPDA == 1)
    {
        printf("\nPDA Received Data --> \n\tlatest sensor data: %f\n", Data);
        flagPDA = 0;
    }
}

/* Web Server Task */
void vWebServerTask(void *pvParameters)
{
    int data, data2;

    for (;;)
    {
        // Block until data arrives.  xEthernetQueue is filled by the
        // Ethernet interrupt service routine.
        if (xQueuePeek(xEthernetQueue, &data,0) == pdTRUE)
        {
            if (xQueuePeek(xEthernetQueue, &data2,0) == pdTRUE){
                ProcessHTTPData(data, data2);
            }
        }
    }
}

void ProcessHTTPData(int Data, int Data2)
{
    if (flagWeb == 1)
    {
        printf("\nData Received from Web Server --> \n\tlatest sensor1 data: %d, sensor2 data: %d\n", Data, Data2);
        flagWeb = 0;
    }
}