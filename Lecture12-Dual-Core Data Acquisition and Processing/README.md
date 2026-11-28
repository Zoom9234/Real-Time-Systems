# Dual Core Data Pipeline

## 1.) Architecture

The application has three parts. A producer task on core 0 creates sensor readings and sends them through a FreeRTOS queue to a consumer task on core 1. The consumer calculates statistics and stores them in a shared struct. The monitor in loop() reads this struct regularly and prints it. The shared struct is protected by one mutex that every part uses.

## 2.) Producer Task

Every 500 ms the producer creates a reading with a sequence number and a simulated value and sends it to the queue. If the queue is still full after 100 ms, it drops the reading and increases the drop counter in the shared statistics.

## 3.) Consumer Task

The consumer waits on the queue without polling. For every reading it updates count, running average, min and max. It copies the statistics while holding the mutex and prints them after releasing it.

## 4.) Monitor

The monitor in loop() copies the whole statistics struct every 3 s under the mutex and prints it afterwards, together with the drop counter and the queue fill level. By default loop() runs on core 1.

## 5.) Why different cores

With two cores, sampling and processing really run in parallel. Even when the consumer is busy, the producer on core 0 keeps its exact 500 ms rhythm. You can see this well during the slow phase.

## 6.) Why a queue

The queue decouples the two tasks and copies each reading, so they never share a buffer. It also works as a buffer for up to 8 readings when the consumer falls behind.

## 7.) Why a mutex

The statistics are several values that belong together. The consumer writes them, the monitor reads them and the producer updates the drop counter, sometimes at the same time from different cores. Without the mutex the monitor could read a new count with an old average. All three use the same statisticsMutex. Inside the lock I only copy values and do simple maths. Printing and delays happen after releasing it.

## 8.) What I changed

I added two extensions.

1. **Dropped sample counter.** The producer counts every reading that doesn't fit into the queue and stores the number in droppedSamples. The monitor prints it as dropped.
2. **Controlled consumer delay.** For seq 30 to 49 (and again every 100 readings) the consumer needs an extra 1000 ms per reading. The producer still sends every 500 ms, so the queue fills up and readings get dropped. The delay is outside the mutex and can be turned off with ENABLE_SLOW_PHASE.

Without the delay the queue would never be full and the counter would always stay at 0, so the two extensions belong together.

## 9.) How to compile, upload and test

1. Install the ESP32 board package in Arduino IDE 2.x and select ESP32S3 Dev Module.
2. Open Dual_Core_Data_Pipeline.ino and upload it.
3. Open the Serial Monitor at 115200 baud and let it run for about 40 s to see the slow phase.

## 10.) Serial Monitor screenshots

Can be found in the images folder. 
The screenshots show the producer on Core 0 with sequence numbers and values, the consumer on Core 1 with the updated statistics and a monitor line every 3 s. When the slow phase starts at seq 30, the queue fills up until it is full (8/8). After that every second reading is dropped and the dropped value in the monitor goes up by 3 every 3 seconds.
