#include "controller.h"

QueueHandle_t xSensorQueue1;
QueueHandle_t xSensorQueue2;
QueueHandle_t xEthernetQueue;
QueueHandle_t xRS232Queue;

SemaphoreHandle_t xBinarySemaphore = NULL;

TimerHandle_t tmrPlantTask;
TimerHandle_t tmrSensorTask;
TimerHandle_t tmrKbTask;
TimerHandle_t tmrLedTask;
TimerHandle_t tmrPDATask;
TimerHandle_t tmrWebTask;
TimerHandle_t tmrCpuTask;

TickType_t xIdleTimeCount = 0;
TickType_t xLastTickTimeCount = 0;

int statusLED = 1;
int systemHealth = 1;

int flagPDA = 0;
int flagWeb = 0;
int flagLED = 0;
int flagPerformance = 0;

float cpuLoad = 0;


void vPlantControlTask(void *pvParameters)
{
    TickType_t xLastWakeTime;

    float receivedData1, receivedData2;

    /* Queue init */
    xSensorQueue1 = xQueueCreate(1, sizeof(float));
    xSensorQueue2 = xQueueCreate(1, sizeof(float));
    xEthernetQueue = xQueueCreate(1, sizeof(float));
    xRS232Queue = xQueueCreate(1, sizeof(float));

    xBinarySemaphore = xSemaphoreCreateBinary();

    if (xSensorQueue1 == NULL || xSensorQueue2)
    {
        /* Queue was not created and must not be used. */
        printf("Queue create error\n");
    }

    // A
    xLastWakeTime = xTaskGetTickCount();

    xTimerStart(tmrPlantTask, 0);

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
                xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
                if(flagPerformance == 1){
                    xSemaphoreGive(xBinarySemaphore);
                    PerformControl(receivedData1, receivedData2);
                }
                else {
                    xSemaphoreGive(xBinarySemaphore);
                }
                xTimerReset(tmrPlantTask, 0);
            }
        }
    }
}

void vSensorControlTask( void *pvParameters ){
    float data1 = 0.0;
    float data2 = 0.0;

    xTimerStart(tmrSensorTask, 0);

    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
        data1 = 0.005*(rand()%10000);


        vTaskDelay(pdMS_TO_TICKS(1));
        data2 = 0.001*(rand()%15000);

        xQueueOverwrite(xSensorQueue1, &data1);
        xQueueOverwrite(xSensorQueue2, &data2);
        xTimerReset(tmrSensorTask, 0);
    }
    
}

void PerformControl(float data1, float data2){
    float result = 0;
    for (int i = 0; i < CTRL_ALG_LOAD; i++)
    {
        result = data1*data2*(data1+data2);
    }
    
}

void vKeyScanTask(void *pvParmeters)
{
    char key;
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();

    xBinarySemaphore = xSemaphoreCreateBinary();

    // xTimerStart(tmrKbTask, 0);

    for (;;)
    {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, DELAY_PERIOD_KP);

        // Scan the keyboard.
        if (kbhit())
        {
            fflush(stdout);

            key = getchar();

            xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
            printf("\n[key pressed]: %c\n", key);
            xSemaphoreGive(xBinarySemaphore);

            switch (key)
            {
            case '1':
                flagPDA = 1;
                break;
            case '2':
                flagWeb = 1;
                break;
            // case '3':
            //     systemHealth = 0;
            //     break;
            // case '4':
            //     systemHealth = 1;
            //     break;
            case '3':
                // systemHealth = 0;
                xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
                if(flagPerformance != 0)
                    flagPerformance = 0;
                    printf("\nFlag performance set to 0\n");
                    xSemaphoreGive(xBinarySemaphore);
                break;
            case '4':
                xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
                if(flagPerformance != 1)
                    flagPerformance = 1;
                    printf("\nFlag performance set to 1\n");
                    xSemaphoreGive(xBinarySemaphore);
                // systemHealth = 1;
                break;
            default:
                xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
                printf("\rComando inválido!\n");
                xSemaphoreGive(xBinarySemaphore);
                break;
            }
        }
        // xTimerReset(tmrKbTask, 0);
    }
}

void vLEDTask(void *pvParameters)
{
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();
    int led, curr;

    xBinarySemaphore = xSemaphoreCreateBinary();

    // xTimerStart(tmrLedTask, 0);

    for (;;){
        
        vTaskDelayUntil(&xLastWakeTime, DELAY_PERIOD);
        if (SystemIsHealthy() == 1){
            led = GREEN;
        } 
        else{
            led = RED;
        }
        if(led != curr){
            curr = led;
            if(curr == GREEN){
                // xTimerReset(tmrLedTask, 0);
                xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
                printf("\n[led] -> color ==> GREEN\n");
                xSemaphoreGive(xBinarySemaphore);
            }
            else {
                // xTimerReset(tmrLedTask, 0);
                xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
                printf("\n[led] -> color ==> RED\n");
                xSemaphoreGive(xBinarySemaphore);
            }
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

/* RS232 Task */
void vRS232Task(void *pvParameters)
{
    int data;

    xBinarySemaphore = xSemaphoreCreateBinary();

    for (;;)
    {
        // Block until data arrives.  xRS232Queue is filled by the
        // RS232 interrupt service routine.  
        if (xQueuePeek(xRS232Queue, &data, 0) == pdTRUE)
        {
            xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
            ProcessSerialCharacters(data);
            xSemaphoreGive(xBinarySemaphore);
        }
    }
}

void ProcessSerialCharacters(float Data)
{
    if (flagPDA == 1)
    {
        printf("\n[PDA Received Data] --> \n\tlatest sensor data: %.3f\n", Data);
        flagPDA = 0;
    }
}

/* Web Server Task */
void vWebServerTask(void *pvParameters)
{
    float data;

    xBinarySemaphore = xSemaphoreCreateBinary();

    for (;;)
    {
        // Block until data arrives.  xEthernetQueue is filled by the
        // Ethernet interrupt service routine.
        if (xQueuePeek(xEthernetQueue, &data,0) == pdTRUE)
        {
            xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
            ProcessHTTPData(data);
            xSemaphoreGive(xBinarySemaphore);
        }
    }
}

void ProcessHTTPData(float Data)
{
    if (flagWeb == 1)
    {
        printf("\n[Web Received Data] --> \n\tlatest sensor data: %f\n", Data);
        flagWeb = 0;
    }
}

void vCPUMonitorTask( void * pvParameters ){
    TickType_t xCurrentTickTimeCount = xTaskGetTickCount(); //pega tempo corrente
    TickType_t xTickTimesElapsed = xCurrentTickTimeCount - xLastTickTimeCount; //tempo corrente menos ultimo tempo de ocorrencia da cpu

    for(;;){
        vTaskDelayUntil(&xCurrentTickTimeCount, pdMS_TO_TICKS(5000));
        cpuLoad = ((float) xTickTimesElapsed - (float) (xIdleTimeCount) / (float) (xTickTimesElapsed));
        printf("[CPUMonitorTask] - CPU load --> %f \n", cpuLoad);

        xLastTickTimeCount = xCurrentTickTimeCount;
        xIdleTimeCount = 0;
    }
}

void vApplicationIdleHook( void ){
    unsigned long int curr = xTaskGetTickCount();
    while(xTaskGetTickCount() == curr);
    xIdleTimeCount = xIdleTimeCount + 1;
}

void vTimerCallback(TimerHandle_t xTimer){
    printf("\nDeadline miss!\n");
}

void initCreateTimers( void ){
    tmrPlantTask = xTimerCreate("tmr plant task", pdMS_TO_TICKS(CYCLE_RATE_MS+5), pdTRUE, (void*) 0, vTimerCallback);
    tmrSensorTask = xTimerCreate("tmr sensores", pdMS_TO_TICKS(5), pdTRUE, (void*) 1, vTimerCallback);
    tmrKbTask = xTimerCreate("tmr keyboard task", pdMS_TO_TICKS(DELAY_PERIOD_KP), pdTRUE, (void*) 2, vTimerCallback);
    tmrLedTask = xTimerCreate("tmr led task", pdMS_TO_TICKS(DELAY_PERIOD+5), pdTRUE, (void*) 3, vTimerCallback);
}