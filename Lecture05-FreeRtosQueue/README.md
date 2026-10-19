## 1) Purpose

This program shows how a FreeRTOS queue can pass data from one task to another. A Sensor Task simulates a temperature sensor and sends a rising temperature value every second. A Display Task receives those values from the queue and prints them to the Serial Monitor.


## 2) How It Works

The application uses two FreeRTOS tasks and one FreeRTOS queue.

**Sensor Task**
Starts with a temperature of 20, sends the current value to the queue with xQueueSend(), increases the value by 1 and then waits one second before repeating.

**Queue**
Created with xQueueCreate(5, sizeof(int)) so it can store up to 5 integer values. It acts as a thread safe buffer between the two tasks.

**Display Task**
Waits for a value with xQueueReceive(). When a value arrives it prints it to the Serial Monitor in the format Temperature: XX C.

**Communication flow:** Sensor Task -> temperature -> Queue -> temperature -> Display Task

Both tasks are created in setup() with xTaskCreate() and then run independently, scheduled by the FreeRTOS kernel.
In short, the Sensor Task is the producer, the Display Task is the consumer and the queue is the thread safe channel between them. The producer decides when to send and the consumer decides when to receive, and the queue decouples their timing so neither task has to wait for the other to finish its work.


## 3) Serial Monitor

<img width="1502" height="842" alt="image" src="https://github.com/user-attachments/assets/56d25631-2024-4c16-b98c-2a5cd69edd20" />


## 4) Reflection

The queue decouples the Sensor Task from the Display Task so each one can run at its own pace without calling the other directly. It provides thread safe communication, which means the producer and the consumer can exchange data without race conditions or shared global variables. It also buffers up to five values, so short timing differences between producing and consuming data do not cause any loss of information.




