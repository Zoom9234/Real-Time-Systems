#include <Arduino.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#ifndef TASK_DELAY_MS
#define TASK_DELAY_MS 20
#endif
static_assert(TASK_DELAY_MS == 20 || TASK_DELAY_MS == 500,
              "Select 20 ms (A) or 500 ms (B).");

constexpr UBaseType_t QUEUE_LENGTH = 4;
constexpr uint32_t TIMER_FREQUENCY_HZ = 1000000;
constexpr uint64_t ALARM_TICKS = 100000;

QueueHandle_t eventQueue = nullptr;
hw_timer_t *eventTimer = nullptr;

void ARDUINO_ISR_ATTR onTimer() {
    static uint32_t sequence = 0;
    ++sequence;

    BaseType_t taskWoken = pdFALSE;
    (void)xQueueSendFromISR(eventQueue, &sequence, &taskWoken);
    if (taskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

void receiveTask(void *) {
    uint32_t received = 0;
    for (;;) {
        if (xQueueReceive(eventQueue, &received, portMAX_DELAY) == pdTRUE) {
            Serial0.println(received);
            vTaskDelay(pdMS_TO_TICKS(TASK_DELAY_MS));
        }
    }
}

void setup() {
    Serial0.begin(115200);

    eventQueue = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));
    if (eventQueue == nullptr) {
        Serial0.println("ERROR: Queue creation failed.");
        return;
    }

    eventTimer = timerBegin(TIMER_FREQUENCY_HZ);
    if (eventTimer == nullptr) {
        Serial0.println("ERROR: Timer creation failed.");
        vQueueDelete(eventQueue);
        eventQueue = nullptr;
        return;
    }
    timerStop(eventTimer);
    timerWrite(eventTimer, 0);

    if (xTaskCreate(receiveTask, "TimerEvents", 4096, nullptr, 2,
                    nullptr) != pdPASS) {
        Serial0.println("ERROR: Task creation failed.");
        timerEnd(eventTimer);
        eventTimer = nullptr;
        vQueueDelete(eventQueue);
        eventQueue = nullptr;
        return;
    }

    timerAttachInterrupt(eventTimer, onTimer);
    timerAlarm(eventTimer, ALARM_TICKS, true, 0);
    timerStart(eventTimer);
}

void loop() {
    delay(1000);
}

