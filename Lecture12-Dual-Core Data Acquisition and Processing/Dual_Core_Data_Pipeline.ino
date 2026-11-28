#include <Arduino.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

struct SensorReading {
  uint32_t sequence;
  float value;
  TickType_t producedAt;
};

struct SharedStatistics {
  uint32_t sampleCount;
  float sum;
  float runningAverage;
  float minimum;
  float maximum;
  uint32_t droppedSamples;
};

constexpr UBaseType_t QUEUE_LENGTH = 8;

constexpr uint32_t PRODUCER_PERIOD_MS = 500;
constexpr uint32_t QUEUE_WAIT_MS      = 100;
constexpr uint32_t MUTEX_WAIT_MS      = 50;
constexpr uint32_t MONITOR_PERIOD_MS  = 3000;

constexpr uint32_t PRODUCER_STACK = 4096;
constexpr uint32_t CONSUMER_STACK = 4096;

constexpr bool     ENABLE_SLOW_PHASE  = true;
constexpr uint32_t SLOW_PHASE_START   = 30;
constexpr uint32_t SLOW_PHASE_END     = 49;
constexpr uint32_t SLOW_PHASE_PERIOD  = 100;
constexpr uint32_t SLOW_PROCESSING_MS = 1000;

QueueHandle_t     readingQueue    = nullptr;
SemaphoreHandle_t statisticsMutex = nullptr;

SharedStatistics statistics = {};

void fatalError(const char *message) {
  Serial.printf("FATAL: %s\n", message);
  Serial.flush();
  while (true) {
    delay(1000);
  }
}

float makeSimulatedReading(uint32_t sequence) {
  return 22.0f + static_cast<float>(sequence % 8U) * 0.4f;
}

bool isSlowPhase(uint32_t sequence) {
  if (!ENABLE_SLOW_PHASE) {
    return false;
  }
  uint32_t pos = sequence % SLOW_PHASE_PERIOD;
  return pos >= SLOW_PHASE_START && pos <= SLOW_PHASE_END;
}

void sensorProducerTask(void *parameter) {
  (void)parameter;

  uint32_t sequence = 0;
  uint32_t pendingDrops = 0;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    SensorReading reading;
    reading.sequence   = sequence++;
    reading.value      = makeSimulatedReading(reading.sequence);
    reading.producedAt = xTaskGetTickCount();

    if (xQueueSend(readingQueue, &reading,
                   pdMS_TO_TICKS(QUEUE_WAIT_MS)) == pdPASS) {

      Serial.printf("[Producer | Core %d] seq=%lu value=%.2f queued=%u\n",
                    xPortGetCoreID(),
                    static_cast<unsigned long>(reading.sequence),
                    reading.value,
                    static_cast<unsigned>(uxQueueMessagesWaiting(readingQueue)));

    } else {
      pendingDrops++;

      Serial.printf("[Producer | Core %d] queue FULL, dropped seq=%lu\n",
                    xPortGetCoreID(),
                    static_cast<unsigned long>(reading.sequence));
    }

    if (pendingDrops > 0 &&
        xSemaphoreTake(statisticsMutex, pdMS_TO_TICKS(MUTEX_WAIT_MS)) == pdTRUE) {
      statistics.droppedSamples += pendingDrops;
      xSemaphoreGive(statisticsMutex);
      pendingDrops = 0;
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(PRODUCER_PERIOD_MS));
  }
}

void dataConsumerTask(void *parameter) {
  (void)parameter;

  SensorReading reading;
  bool wasSlow = false;

  for (;;) {
    if (xQueueReceive(readingQueue, &reading, portMAX_DELAY) != pdPASS) {
      continue;
    }

    SharedStatistics snapshot;
    bool updated = false;

    if (xSemaphoreTake(statisticsMutex, pdMS_TO_TICKS(MUTEX_WAIT_MS)) == pdTRUE) {

      if (statistics.sampleCount == 0) {
        statistics.minimum = reading.value;
        statistics.maximum = reading.value;
      } else {
        if (reading.value < statistics.minimum) {
          statistics.minimum = reading.value;
        }
        if (reading.value > statistics.maximum) {
          statistics.maximum = reading.value;
        }
      }

      statistics.sampleCount++;
      statistics.sum += reading.value;
      statistics.runningAverage = statistics.sum / statistics.sampleCount;

      snapshot = statistics;
      updated = true;

      xSemaphoreGive(statisticsMutex);
    }

    if (updated) {
      Serial.printf("[Consumer | Core %d] seq=%lu avg=%.2f min=%.2f max=%.2f count=%lu\n",
                    xPortGetCoreID(),
                    static_cast<unsigned long>(reading.sequence),
                    snapshot.runningAverage,
                    snapshot.minimum,
                    snapshot.maximum,
                    static_cast<unsigned long>(snapshot.sampleCount));
    } else {
      Serial.printf("[Consumer | Core %d] mutex timeout for seq=%lu\n",
                    xPortGetCoreID(),
                    static_cast<unsigned long>(reading.sequence));
    }

    bool slow = isSlowPhase(reading.sequence);
    if (slow != wasSlow) {
      Serial.printf("[Consumer | Core %d] ---- slow processing %s (%lu ms per reading) ----\n",
                    xPortGetCoreID(),
                    slow ? "STARTED" : "ENDED",
                    static_cast<unsigned long>(SLOW_PROCESSING_MS));
      wasSlow = slow;
    }
    if (slow) {
      vTaskDelay(pdMS_TO_TICKS(SLOW_PROCESSING_MS));
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Dual-Core Data Pipeline - Producer/Consumer/Monitor");
  Serial.printf("setup() is running on core %d\n", xPortGetCoreID());

  readingQueue = xQueueCreate(QUEUE_LENGTH, sizeof(SensorReading));
  if (readingQueue == nullptr) {
    fatalError("Could not create reading queue");
  }

  statisticsMutex = xSemaphoreCreateMutex();
  if (statisticsMutex == nullptr) {
    vQueueDelete(readingQueue);
    readingQueue = nullptr;
    fatalError("Could not create statistics mutex");
  }

  BaseType_t consumerCreated = xTaskCreatePinnedToCore(
      dataConsumerTask, "DataConsumer", CONSUMER_STACK,
      nullptr, 2, nullptr, 1);

  if (consumerCreated != pdPASS) {
    fatalError("Could not create consumer task on Core 1");
  }

  BaseType_t producerCreated = xTaskCreatePinnedToCore(
      sensorProducerTask, "SensorProducer", PRODUCER_STACK,
      nullptr, 2, nullptr, 0);

  if (producerCreated != pdPASS) {
    fatalError("Could not create producer task on Core 0");
  }

  if (ENABLE_SLOW_PHASE) {
    Serial.printf("Queue pressure demo: seq %lu..%lu of every %lu are processed slowly\n",
                  static_cast<unsigned long>(SLOW_PHASE_START),
                  static_cast<unsigned long>(SLOW_PHASE_END),
                  static_cast<unsigned long>(SLOW_PHASE_PERIOD));
  }
  Serial.println("Tasks created. Watch the core IDs below.");
}

void loop() {
  static uint32_t lastMonitorTime = 0;

  if (millis() - lastMonitorTime >= MONITOR_PERIOD_MS) {
    lastMonitorTime = millis();

    SharedStatistics snapshot;
    bool copied = false;

    if (xSemaphoreTake(statisticsMutex, pdMS_TO_TICKS(MUTEX_WAIT_MS)) == pdTRUE) {
      snapshot = statistics;
      copied = true;
      xSemaphoreGive(statisticsMutex);
    }

    UBaseType_t queued = uxQueueMessagesWaiting(readingQueue);

    if (copied) {
      Serial.printf("[Monitor  | Core %d] count=%lu avg=%.2f min=%.2f max=%.2f "
                    "dropped=%lu queue=%u/%u\n",
                    xPortGetCoreID(),
                    static_cast<unsigned long>(snapshot.sampleCount),
                    snapshot.runningAverage,
                    snapshot.minimum,
                    snapshot.maximum,
                    static_cast<unsigned long>(snapshot.droppedSamples),
                    static_cast<unsigned>(queued),
                    static_cast<unsigned>(QUEUE_LENGTH));
    } else {
      Serial.printf("[Monitor  | Core %d] mutex timeout\n", xPortGetCoreID());
    }
  }

  delay(10);
}
