// Original teaching implementation, October 2026. SPDX-License-Identifier: 0BSD
// ESP32-S3, Arduino IDE 2.x, Serial Monitor 115200. No external wiring.
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t DEMO_CORE = 0;
#else
static const BaseType_t DEMO_CORE = 1;
#endif
void fatal(const char *message) {
  Serial.println(message);
  while (true) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}
void startTask(TaskFunction_t fn, const char *name, void *arg) {
  if (xTaskCreatePinnedToCore(fn, name, 4096, arg, 1, nullptr, DEMO_CORE) != pdPASS)
    fatal("ERROR: task creation failed. Reset after checking available memory.");
}

// Inspired by Shawn Hymel / DigiKey Introduction to RTOS Part 7 challenge.
// Newly written scaffold: stable arguments and automatic frequency checking.
static const int CAPACITY=5, PRODUCERS=5, WRITES=3, CONSUMERS=2;
// Test 1: 400 ms. Test 2 (current): 1000 ms. Change only the consumer delay.
static const uint32_t PRODUCER_DELAY_MS=100, CONSUMER_DELAY_MS=1000;
static const uint32_t TEST_TIMEOUT_MS=30000;
static int buffer[CAPACITY], head=0, tail=0, consumed=0;
static int counts[PRODUCERS]={0}, invalidValues=0;
static int producerIds[PRODUCERS]={0,1,2,3,4};
static int consumerIds[CONSUMERS]={0,1};
SemaphoreHandle_t emptySlots, filledSlots, bufferMutex, allConsumed;

bool createObjects() {
  // Initially, all five places are free and no values are ready.
  emptySlots=xSemaphoreCreateCounting(CAPACITY, CAPACITY);
  filledSlots=xSemaphoreCreateCounting(CAPACITY, 0);
  bufferMutex=xSemaphoreCreateMutex();
  allConsumed=xSemaphoreCreateBinary(); // Test-completion signal is provided.
  return emptySlots && filledSlots && bufferMutex && allConsumed;
}
void putValue(int value) {
  xSemaphoreTake(emptySlots, portMAX_DELAY);
  xSemaphoreTake(bufferMutex, portMAX_DELAY);
  buffer[head]=value;
  head=(head+1)%CAPACITY;
  xSemaphoreGive(bufferMutex);
  xSemaphoreGive(filledSlots);
}
void getValue(int consumerId) {
  xSemaphoreTake(filledSlots, portMAX_DELAY);
  xSemaphoreTake(bufferMutex, portMAX_DELAY);
  int value=buffer[tail];
  tail=(tail+1)%CAPACITY;
  if (value>=0 && value<PRODUCERS) ++counts[value]; else ++invalidValues;
  ++consumed;
  Serial.printf("Consumer %d read %d (total=%d)\n",consumerId,value,consumed);
  if (consumed==PRODUCERS*WRITES) xSemaphoreGive(allConsumed);
  xSemaphoreGive(bufferMutex);
  xSemaphoreGive(emptySlots);
}

void producer(void *arg) {
  const int id=*static_cast<int *>(arg);
  for (int i=0; i<WRITES; ++i) {
    putValue(id);
    vTaskDelay(pdMS_TO_TICKS(PRODUCER_DELAY_MS));
  }
  vTaskDelete(nullptr);
}
void consumer(void *arg) {
  const int id=*static_cast<int *>(arg);
  while (true) {
    getValue(id);
    vTaskDelay(pdMS_TO_TICKS(CONSUMER_DELAY_MS));
  }
}
void monitor(void *) {
  if (xSemaphoreTake(allConsumed, pdMS_TO_TICKS(TEST_TIMEOUT_MS)) != pdTRUE) {
    // Do not take bufferMutex here: a wrong order may hold it forever.
    Serial.println("TIMEOUT: expected 15 consumed values. Check waits and Give calls.");
    vTaskDelete(nullptr);
  }
  if (xSemaphoreTake(bufferMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
    Serial.println("ERROR: completion signal received but mutex unavailable");
    vTaskDelete(nullptr);
  }
  bool pass=(consumed==PRODUCERS*WRITES && invalidValues==0);
  Serial.printf("TOTAL=%d expected=%d invalid=%d\n", consumed, PRODUCERS*WRITES, invalidValues);
  for (int i=0; i<PRODUCERS; ++i) {
    Serial.printf("Value %d: count=%d expected=%d\n",i,counts[i],WRITES);
    if (counts[i]!=WRITES) pass=false;
  }
  Serial.println(pass ? "PASS: total and all frequencies correct" : "FAIL: inspect values and counts");
  xSemaphoreGive(bufferMutex);
  vTaskDelete(nullptr);
}
void setup() {
  Serial.begin(115200); delay(1500);
  Serial.println("Semaphore buffer challenge");
  if (!createObjects()) fatal("ERROR: semaphore or mutex creation failed. No worker tasks started.");
  for (int i=0;i<CONSUMERS;++i) startTask(consumer,"consumer",&consumerIds[i]);
  for (int i=0;i<PRODUCERS;++i) startTask(producer,"producer",&producerIds[i]);
  startTask(monitor,"monitor",nullptr);
}
void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
