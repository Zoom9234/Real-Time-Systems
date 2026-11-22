#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define DEMO_MODE 1

SemaphoreHandle_t mutexA;
SemaphoreHandle_t mutexB;

struct LockPlan {
  const char* taskName;
  SemaphoreHandle_t first;
  const char* firstName;
  SemaphoreHandle_t second;
  const char* secondName;
  volatile uint32_t rounds;
};

LockPlan plan1;
LockPlan plan2;

void initPlan(LockPlan& p, const char* name,
              SemaphoreHandle_t first, const char* firstName,
              SemaphoreHandle_t second, const char* secondName);

void initPlan(LockPlan& p, const char* name,
              SemaphoreHandle_t first, const char* firstName,
              SemaphoreHandle_t second, const char* secondName) {
  p.taskName = name;
  p.first = first;
  p.firstName = firstName;
  p.second = second;
  p.secondName = secondName;
  p.rounds = 0;
}

void workerTask(void* pv) {
  LockPlan* p = static_cast<LockPlan*>(pv);
  for (;;) {
    Serial.printf("[%s] taking %s ...\n", p->taskName, p->firstName);
    xSemaphoreTake(p->first, portMAX_DELAY);
    Serial.printf("[%s] has %s, now needs %s\n",
                  p->taskName, p->firstName, p->secondName);

    vTaskDelay(pdMS_TO_TICKS(100));

#if DEMO_MODE == 1
    TickType_t waitTime = pdMS_TO_TICKS(500);
#else
    TickType_t waitTime = portMAX_DELAY;
#endif

    if (xSemaphoreTake(p->second, waitTime) == pdTRUE) {
      p->rounds = p->rounds + 1;
      Serial.printf("[%s] has BOTH locks -> work done (round %lu)\n",
                    p->taskName, static_cast<unsigned long>(p->rounds));
      vTaskDelay(pdMS_TO_TICKS(50));
      xSemaphoreGive(p->second);
      xSemaphoreGive(p->first);
      vTaskDelay(pdMS_TO_TICKS(100));
    } else {
      Serial.printf("[%s] TIMEOUT waiting for %s -> release %s, back off\n",
                    p->taskName, p->secondName, p->firstName);
      xSemaphoreGive(p->first);
      vTaskDelay(pdMS_TO_TICKS(random(100, 400)));
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.printf("\n=== Demo 1: Deadlock | DEMO_MODE %d ===\n", DEMO_MODE);

  mutexA = xSemaphoreCreateMutex();
  mutexB = xSemaphoreCreateMutex();
  if (mutexA == nullptr || mutexB == nullptr) {
    Serial.println("Could not create the mutexes.");
    while (true) {
      delay(1000);
    }
  }

  initPlan(plan1, "Task_1", mutexA, "A", mutexB, "B");
  initPlan(plan2, "Task_2", mutexA, "A", mutexB, "B");

  BaseType_t result1 = xTaskCreatePinnedToCore(
      workerTask, "Task_1", 4096, &plan1, 2, nullptr, 1);
  BaseType_t result2 = xTaskCreatePinnedToCore(
      workerTask, "Task_2", 4096, &plan2, 2, nullptr, 1);
  if (result1 != pdPASS || result2 != pdPASS) {
    Serial.println("Task creation failed. Reset the board before testing.");
    while (true) {
      delay(1000);
    }
  }
}

void loop() {
  delay(1000);
  Serial.printf("[HEARTBEAT] t=%lu s | rounds: Task_1=%lu Task_2=%lu\n",
                static_cast<unsigned long>(millis() / 1000),
                static_cast<unsigned long>(plan1.rounds),
                static_cast<unsigned long>(plan2.rounds));
}
