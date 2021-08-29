#include "controller.h"


int main(void)
{
    xTaskCreate(&vKeyScanTask, "keyboard scan", 1024, NULL, 1, NULL);
    xTaskCreate(&vPlantControlTask, "plant control", 1024, NULL, 2, NULL);
    xTaskCreate(&vSensorControlTask, "sensor control", 1024, NULL, 3, NULL);
    xTaskCreate(&vLEDTask, "led control", 1024, NULL, 0, NULL);
    // xTaskCreate(&vRS232Task, "PDA task", 1024, NULL, 1, NULL);

    vTaskStartScheduler();

    return 0;
}
