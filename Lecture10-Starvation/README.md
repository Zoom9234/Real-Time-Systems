# Starvation

## Part A

Task_Display (priority 2) and Task_Log (priority 1) may starve. Task_Control has the highest priority and never blocks, so the scheduler always picks it first. On the same core, the lower-priority tasks never get CPU time.

A high-priority task that never blocks keeps the CPU all the time. Lower priority tasks stay ready but never run, so the display is not updated and no logs are sent. Important work can be delayed for a very long time. 

Two ways to give CPU time to lower priority tasks without changing priorities:
Let Task_Control block regularly, for example with vTaskDelay() or vTaskDelayUntil().
Let Task_Control wait for an event (a queue, semaphore or task notification) instead of polling all the time. It only runs when there is work to do.

## Part B

### Step 1

With STARVATION_FIX 0, Task_Low makes no progress while Task_Hog is BUSY (+0). While Task_Hog is in SLEEP, Task_Low makes progress again (+500 per report). This shows starvation, a higher priority task that never blocks stops a lower priority task completely. As soon as the high priority task sleeps, the lower priority task runs.

Lines from my Serial Monitor
```
=== Demo 2: Starvation | STARVATION_FIX 0 ===
t=2499 ms | Task_Hog: BUSY  | Task_Low progress: +0
t=2999 ms | Task_Hog: BUSY  | Task_Low progress: +0
t=3499 ms | Task_Hog: SLEEP | Task_Low progress: +500
t=3999 ms | Task_Hog: SLEEP | Task_Low progress: +500
t=5999 ms | Task_Hog: SLEEP | Task_Low progress: +500
t=6499 ms | Task_Hog: BUSY  | Task_Low progress: +0
t=6999 ms | Task_Hog: BUSY  | Task_Low progress: +0
```
### Step 2

With STARVATION_FIX 1, Task_Low makes progress during the BUSY period (about +50 per report instead of +0). The SLEEP period is the same as before (+500). Task_Hog now calls vTaskDelay(1), which puts it into the Blocked state for one tick. While it is blocked, it is not ready, so the scheduler runs the highest priority task that is ready, which is Task_Low. Without blocking, Task_Low never gets this chance.

Lines from my Serial Monitor

```
=== Demo 2: Starvation | STARVATION_FIX 1 ===
t= 499 ms | Task_Hog: BUSY  | Task_Low progress: +49
t= 999 ms | Task_Hog: BUSY  | Task_Low progress: +50
t=2999 ms | Task_Hog: BUSY  | Task_Low progress: +50
t=3499 ms | Task_Hog: SLEEP | Task_Low progress: +500
t=5999 ms | Task_Hog: SLEEP | Task_Low progress: +500
t=6499 ms | Task_Hog: BUSY  | Task_Low progress: +50
t=6999 ms | Task_Hog: BUSY  | Task_Low progress: +50
```
### Step 3

Prediction before the change: I expected Task_Low to make more progress during the BUSY period, because Task_Hog would be blocked for longer in every loop.

Observation: after changing vTaskDelay(1) to vTaskDelay(10), Task_Low made about +260 per report during BUSY, compared to about +50 before. The SLEEP period stayed at +500. The delay is given in ticks. With the tick rate of 1000 Hz shown in output, 10 ticks are about 10 ms, so Task_Hog is blocked longer and Task_Low gets more CPU time. The longer the high priority task blocks, the more the low priority task can do.

Lines from my Serial Monitor
```
=== Demo 2: Starvation | STARVATION_FIX 1 ===
FreeRTOS tick rate: 1000 Hz
t= 499 ms | Task_Hog: BUSY  | Task_Low progress: +260
t= 999 ms | Task_Hog: BUSY  | Task_Low progress: +262
t=1499 ms | Task_Hog: BUSY  | Task_Low progress: +266
t=2999 ms | Task_Hog: BUSY  | Task_Low progress: +266
t=3499 ms | Task_Hog: SLEEP | Task_Low progress: +500
t=6499 ms | Task_Hog: BUSY  | Task_Low progress: +263
t=6999 ms | Task_Hog: BUSY  | Task_Low progress: +260
```
## What I learned

I learned that a high priority task which never blocks can starve all lower priority tasks on the same core. Priorities alone do not share the CPU. A task must block, even for a short time, so the scheduler can run other tasks. A longer delay gives lower priority tasks more CPU time.
