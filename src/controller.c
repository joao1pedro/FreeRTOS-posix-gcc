#include "controller.h"

QueueHandle_t xSensorQueue1;
QueueHandle_t xSensorQueue2;
QueueHandle_t xEthernetQueue;
QueueHandle_t xRS232Queue;
QueueHandle_t xCpuLoadQueue;

SemaphoreHandle_t xBinarySemaphore = NULL;

TimerHandle_t tmrPlantTask;
TimerHandle_t tmrSensorTask;
TimerHandle_t tmrKbTask;
TimerHandle_t tmrLedTask;
TimerHandle_t tmrPDATask;
TimerHandle_t tmrWebTask;
TimerHandle_t tmrCpuTask;
TimerHandle_t tmrLCDTask;

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
        // printf("Queue create error\n");
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
        data1 = 0.005*(rand()%100);


        vTaskDelay(pdMS_TO_TICKS(1));
        data2 = 0.005*(rand()%150);

        xQueueOverwrite(xSensorQueue1, &data1);
        xQueueOverwrite(xSensorQueue2, &data2);
        xTimerReset(tmrSensorTask, 0);
    }
    
}

void PerformControl(float data1, float data2){
    float result = 0;
    for (int i = 0; i < CTRL_ALG_LOAD; i++)
    {
        result = pow(data1,data2);
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
            case '3':
                xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
                if(flagPerformance != 0)
                    flagPerformance = 0;
                    printf("\nFlag performance set to 0\n");
                    // systemHealth = 0;
                    xSemaphoreGive(xBinarySemaphore);
                break;
            case '4':
                xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
                if(flagPerformance != 1)
                    flagPerformance = 1;
                    printf("\nFlag performance set to 1\n");
                    // systemHealth = 1;
                    xSemaphoreGive(xBinarySemaphore);
                break;
            case '5':
                xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
                initMenu();
                xSemaphoreGive(xBinarySemaphore);
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

    xTimerStart(tmrLedTask, 0);

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
                xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
                printf("\n[led] -> color ==> GREEN\n");
                xTimerReset(tmrLedTask, 0);
                xSemaphoreGive(xBinarySemaphore);
            }
            else {
                xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
                printf("\n[led] -> color ==> RED\n");
                xTimerReset(tmrLedTask, 0);
                xSemaphoreGive(xBinarySemaphore);
            }
        }
        else {
            xTimerReset(tmrLedTask, 0);
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
    float data;

    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();

    xBinarySemaphore = xSemaphoreCreateBinary();

    xTimerStart(tmrPDATask, 0);

    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, RS232_CHAR_PROC_LOAD);
        // Block until data arrives.  xRS232Queue is filled by the
        // RS232 interrupt service routine.  
        if (xQueuePeek(xRS232Queue, &data, 0) == pdTRUE)
        {
            xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
            ProcessSerialCharacters(data);
            xTimerReset(tmrPDATask, 0);
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
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();

    float data;

    xBinarySemaphore = xSemaphoreCreateBinary();

    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, HTTP_REQUEST_PROC_LOAD);
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
    TickType_t xTickTimesElapsed = xCurrentTickTimeCount - xIdleTimeCount; //tempo corrente menos ultimo tempo de ocorrencia da cpu

    xCpuLoadQueue = xQueueCreate(1, sizeof(float));

    xBinarySemaphore = xSemaphoreCreateBinary();

    xTimerStart(tmrCpuTask, 0);

    for(;;){
        vTaskDelayUntil(&xCurrentTickTimeCount, pdMS_TO_TICKS(10));
        cpuLoad = (((float) xTickTimesElapsed) - ((float) xIdleTimeCount))/ ((float) xTickTimesElapsed );
        // printf("[CPUMonitorTask] - CPU load --> %.2f \n", cpuLoad);
        xQueueOverwrite(xCpuLoadQueue, &cpuLoad);

        xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
        xLastTickTimeCount = xCurrentTickTimeCount;
        xIdleTimeCount = 0;
        if(cpuLoad > 0.85){
            systemHealth = 0;
        } else{
            systemHealth = 1;
        }
        xTimerReset(tmrCpuTask, 0);
        xSemaphoreGive(xBinarySemaphore);
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

void vLCDTask( void * pvParameters ){
    TickType_t xCurrentTickTimeCount = xTaskGetTickCount(); 
    float cpu;
    xBinarySemaphore = xSemaphoreCreateBinary();
    
    xCpuLoadQueue = xQueueCreate(1, sizeof(float));

    xTimerStart(tmrLCDTask, 0);

    for(;;){
        vTaskDelayUntil(&xCurrentTickTimeCount, pdMS_TO_TICKS(5000));
        if (xQueuePeek(xRS232Queue, &cpu, 0) == pdTRUE){
            xSemaphoreTake(xBinarySemaphore,pdMS_TO_TICKS(1));
            printf("[LCD Task] - CPU load --> %.2f \n", cpu);
            xTimerReset(tmrLCDTask, 0);
            xSemaphoreGive(xBinarySemaphore);
        }
    }
}

void initCreateTimers( void ){
    tmrPlantTask = xTimerCreate("tmr plant task", pdMS_TO_TICKS(CYCLE_RATE_MS+5), pdTRUE, (void*) 0, vTimerCallback);
    tmrSensorTask = xTimerCreate("tmr sensores", pdMS_TO_TICKS(5), pdTRUE, (void*) 1, vTimerCallback);
    tmrKbTask = xTimerCreate("tmr keyboard task", pdMS_TO_TICKS(DELAY_PERIOD_KP), pdTRUE, (void*) 2, vTimerCallback);
    tmrLedTask = xTimerCreate("tmr led task", pdMS_TO_TICKS(DELAY_PERIOD+5), pdTRUE, (void*) 3, vTimerCallback);
    tmrPDATask = xTimerCreate("tmr pda task", pdMS_TO_TICKS(RS232_CHAR_PROC_LOAD+5), pdTRUE, (void*) 4, vTimerCallback);
    tmrWebTask = xTimerCreate("tmr web task", pdMS_TO_TICKS(HTTP_REQUEST_PROC_LOAD+5), pdTRUE, (void*) 5, vTimerCallback);
    tmrCpuTask = xTimerCreate("tmr cpu task", pdMS_TO_TICKS(15), pdTRUE, (void*) 6, vTimerCallback);
    tmrLCDTask = xTimerCreate("tmr lcd task", pdMS_TO_TICKS(5000+2), pdTRUE, (void*) 7, vTimerCallback);
}

void initMenu(void)
{
    printf("Hello from Freertos\r\n");
    printf("\n-------------------------------------\n");
    printf("\n Menu de opções do sistema\r\n");
    printf("\n opc = 1 --> Acesso a PDA interface\r\n");
    printf("\n opc = 2 --> Acesso a Web interface\r\n");
    printf("\n opc = 3 --> Flag performance set to 0\r\n");
    printf("\n opc = 4 --> Flag performance set to 1\r\n");
    printf("\n opc = 5 --> Visualizar menu de opções do sistema\r\n");
    printf("\n-------------------------------------\n");
}