#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define STARVATION_FIX 1

volatile uint32_t lowCount = 0;
volatile bool hogBusy = false;

void hogTask(void* pv) {
  (void)pv;
  for (;;) {
    hogBusy = true;
    uint32_t start = millis();
#if STARVATION_FIX
    uint32_t lastPause = start;
#endif

    while (millis() - start < 3000) {
#if STARVATION_FIX
      if (millis() - lastPause >= 10) {
        vTaskDelay(10);
        lastPause = millis();
      }
#endif
    }

    hogBusy = false;
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

void lowTask(void* pv) {
  (void)pv;
  for (;;) {
    lowCount = lowCount + 1;
    vTaskDelay(1);
  }
}

void monitorTask(void* pv) {
  (void)pv;
  uint32_t lastLow = 0;
  uint32_t t0 = millis();

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(500));
    uint32_t now = lowCount;
    Serial.printf("t=%4lu ms | Task_Hog: %-5s | Task_Low progress: +%lu\n",
                  (unsigned long)(millis() - t0),
                  hogBusy ? "BUSY" : "SLEEP",
                  (unsigned long)(now - lastLow));
    lastLow = now;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.printf("\n=== Demo 2: Starvation | STARVATION_FIX %d ===\n",
                STARVATION_FIX);
  Serial.printf("FreeRTOS tick rate: %lu Hz\n",
                (unsigned long)configTICK_RATE_HZ);

  BaseType_t monitorCreated = xTaskCreatePinnedToCore(
      monitorTask, "Task_Monitor", 4096, NULL, 3, NULL, 1);
  BaseType_t lowCreated = xTaskCreatePinnedToCore(
      lowTask, "Task_Low", 4096, NULL, 1, NULL, 1);
  BaseType_t hogCreated = xTaskCreatePinnedToCore(
      hogTask, "Task_Hog", 4096, NULL, 2, NULL, 1);

  if (monitorCreated != pdPASS || lowCreated != pdPASS ||
      hogCreated != pdPASS) {
    Serial.println("Task creation failed. Reset the board before testing.");
    for (;;) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

void loop() {
  vTaskDelete(NULL);
}
