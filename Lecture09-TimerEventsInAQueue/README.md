# 1.) Assignment purpose
Sending hardware timer events from an ISR to a FreeRTOS task through a queue and looking at what happens when the task cannot keep up with the interrupt rate.

# 2.) Hardware and software
ESP32-S3-DevKitC-1, no extra wiring. Arduino IDE 2.3.10, board ESP32S3 Dev Module, ESP32 Arduino, Serial Monitor at 115200 baud.

# 3.) How it works
The hardware timer runs at 1 MHz with an auto reload alarm every 100000 ticks, so the ISR fires every 100 ms. The queue (4 slots of uint32_t) and the receiving task are created before the alarm is enabled. The ISR owns a static counter that starts at 1 and increases on every call, even if the queue is full. It sends a copy with xQueueSendFromISR(), drops the new item if the queue is full and requests a yield if a task was woken. The task waits with xQueueReceive(), prints one number per line and then calls vTaskDelay(). TASK_DELAY_MS is 20 for Run A and 500 for Run B. I uploaded and restarted the board for each run.

# 4.) Run A 
- Prediction was no gaps, because the task is much faster than the 100 ms timer period and the queue stays almost empty.
- The observation matched, the sequence has no gaps.

# 5.) Prediction for Run B
The ISR sends about 5 numbers while the task handles only one. I predicted that the queue fills up, that the first gap appears shortly after the first few numbers and that later only about every other number is printed.

# 6.) Run B 

<img width="554" height="422" alt="image" src="https://github.com/user-attachments/assets/48d9db72-f88d-4b1c-92cf-c530e7632237" />

This matches the prediction. The only difference is that number 6 still fit into the queue, so the gap started one number later than I guessed.

# 7.) Explanation
The queue holds 4 items. In Run A the task empties it faster than it fills, so nothing is lost. In Run B the ISR delivers a number every 100 ms but the task needs 500 ms per number, so the queue fills up and stays full. A full queue rejects the new item. The counter still increases on every interrupt because it lives in the ISR, so the dropped numbers 7 to 10 show up as a gap.

Older numbers appear before the gap because the queue is FIFO. The numbers 2 to 6 were queued while the task was sleeping and are printed first, in order. The new numbers are the ones that get dropped, so the gap only becomes visible after the task has worked through the older items. Number 6 was created at about 0.6 s but printed at about 2.6 s. After that each wake up of the task frees one slot, the next interrupt fills it with the current number and the four interrupts in between are lost, which gives 11, 16, 21 and so on.
