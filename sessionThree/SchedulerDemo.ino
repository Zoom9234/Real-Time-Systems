#include <Arduino.h>

#define PRIO_TASK_A 1
#define PRIO_TASK_B 2      

#define CORE_ID      1     
#define CHAR_TIME_MS 150  

TaskHandle_t taskBHandle = NULL;

void busyWaitMs(uint32_t ms) {
  int64_t start = esp_timer_get_time();
  while ((esp_timer_get_time() - start) < (int64_t)ms * 1000) {
  }
}

void taskA(void *parameter) {
  const char *msg = "Task A is printing slowly... ";
  const int notifyAt = 12;              

  for (;;) {
    Serial.println();
    for (int i = 0; msg[i] != '\0'; i++) {
      Serial.print(msg[i]);
      busyWaitMs(CHAR_TIME_MS);

      if (i == notifyAt) {
        xTaskNotifyGive(taskBHandle);
      }
    }
    Serial.println();
    busyWaitMs(1000);                  
  }
}

void taskB(void *parameter) {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   
    Serial.print("\n --Task B is running!-- \n");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);  

  xTaskCreatePinnedToCore(taskB, "Task B", 4096, NULL, PRIO_TASK_B, &taskBHandle, CORE_ID);
  xTaskCreatePinnedToCore(taskA, "Task A", 4096, NULL, PRIO_TASK_A, NULL, CORE_ID);
}

void loop() {
  vTaskDelete(NULL);  
}
