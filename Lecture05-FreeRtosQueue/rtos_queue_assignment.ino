#include <Arduino.h>

QueueHandle_t temperatureQueue;

void sensorTask(void *parameter){
    int temperature = 20;
    while (1){
        xQueueSend(temperatureQueue, &temperature, portMAX_DELAY);
        temperature = temperature + 1;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void displayTask(void *parameter){
    int receivedTemperature;
    while (1){
        if (xQueueReceive(temperatureQueue, &receivedTemperature, portMAX_DELAY) == pdPASS){

            Serial.print("Temperature: ");
            Serial.print(receivedTemperature);
            Serial.println(" C");
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void setup(){
    Serial.begin(115200);
    temperatureQueue = xQueueCreate(5, sizeof(int));
    xTaskCreate(sensorTask, "Sensor Task", 2048, NULL, 1, NULL);
    xTaskCreate(displayTask, "Display Task", 2048, NULL, 1, NULL);
}

void loop(){
}

