# 1. Hardware and software 
Board: ESP32-S3-DevKitC-1 v1.1
RGB-LED: GPIO 38 (neopixelWrite)
Arduino IDE 2.x + esp32-Boardpaket, Espressif
Serial Monitor: 115200 Baud
Priorities used: 2 (highest), 1 (lowest)

# 2. Baseline implementation
After verifying and uploading the sketch I observed it for a while and found out that it works fine as can be seen in the screenshot below.
<img width="1503" height="869" alt="Screenshot 2026-09-27 at 10 40 13" src="https://github.com/user-attachments/assets/0571d7a5-8f35-4cf2-ac4f-20322d1d74ae" />
The taskA() function triggers the "Task A alive" printing to the serial monitor with a 1000ms gap between each print. 
The taskB() function triggers the LED to blink red with brightness 50 and a 500ms delay. After that, the LED is being turned off for 500ms after which the cycle repeats.


# 3. Prediction and observation table 
--------------------------------------------------------
## A. Both tasks at priority 1
  
-> Prediction before test: 
Both tasks run smoothly because both have the same priority and are able to block regularly.
  
-> Actual observation:
Task A printed 'Task A alive' to the Serial Monitor approximately every 1000 ms, while the RGB LED blinked red with a 500 ms on/off cycle. Both continued running                      simultaneously and periodically for the full observation period, with no noticeable delay or interruption in either task.
  
-> Did it match? Why?:
Yes. Since both tasks share the same priority, the scheduler doesn't favour either one when choosing which task to run next. For most of the time, each task sits in the Blocked state, waiting on its vTaskDelay() call. Only when a tasks delay expires does it move to the Ready state, and since the two delay periods (1000 ms and 500 ms) rarely line up exactly, the tasks are almost never Ready at the same moment, so there is no real competition between them. Whichever task is Ready at that moment simply gets picked by the scheduler and briefly enters the Running state to execute its short piece of code (print the message, or toggle the LED), before calling vTaskDelay() again and returning to Blocked. Because of this, the timing is driven only by the delay values, not by the priority setting.
  
--------------------------------------------------------  
## B. Task A priority 2; Task B priority 1

-> Prediction before test: 
Since task two has the highes priority it will be Running for most of the time, meaning the LED will always blink red whereas the "Task A alive" will only be printed when Task B is in Blocked state. 
  
-> Actual observation: 
The red LED is blinking non stop, the "Task A alive" is printed non stop as well but there is a slight mismatch between execution, at least thats what it seems like when looking at it. The red blinking and the printing in the serial monitor happen out of sync, kind of like in #A above.

-> Did it match? Why?:
Not quite. Part of the mismatch was my own assumption going in. I had assumed priority 1 was the highest, when in FreeRTOS a higher number actually means higher priority, so Task A (priority 2) was in fact the higher-priority task all along, not Task B.
More importantly, the actual behaviour itself didn't match the "one dominates the other" prediction either way. Both tasks kept running periodically, the LED kept blinking and "Task A alive" kept printing, because both tasks still call vTaskDelay(), which regularly moves each one into the Blocked state regardless of its priority. Priority only decides which task the scheduler picks when more than one task is Ready at the same instant, it doesn't stop a task from blocking on its own delay. Since Task A's and Task B's delay periods (1000 ms vs. 500 ms) don't line up evenly, the exact moments they become Ready and get scheduled gradually shift, which is likely what caused the LED and Serial output to appear slightly out of sync, rather than any task being starved.
  
--------------------------------------------------------
## C. Task A priority 1; Task B priority 2

-> Prediction before test: 
Judging from the findings above I assume that both tasks will get executed even though task B has the higher priority. The only thing that changes might be that the execution will now happen reversed to the above, which to the human eye will barely be visible. 
  
-> Actual observation: 
The LED blinks red, the serial monitor prints the text. Both happen slightly out of sync again, aligning with each other as the time period goes on but then losing their syncronisation again.
  
-> Did it match? Why?:
Yes, largely. As predicted, both tasks kept running regardless of which one had the higher priority, Task B being priority 2 this time didn't stop Task A from executing, just like Task A being priority 2 in the previous scenario didn't stop Task B. Both tasks still call vTaskDelay(), so each one regularly enters the Blocked state on its own, independent of priority. The scheduler only uses priority to decide which task to run when two tasks are Ready at the exact same moment, and since actual code execution (printing to Serial, writing to the LED) takes a small, not perfectly constant amount of time each cycle, the moments they become Ready keep shifting slightly, which explains why the LED and Serial output drift in and out of sync rather than staying perfectly aligned or one completely dominating the other.

--------------------------------------------------------

# 4. Priority experiments 
After changing the priorities of taskA and taskB during the exercise above, it showed that both tasks kept executing regardless of their priority.

The observed periods in all three scenarios were essentially identical to the baseline from Section 2. Task A continued printing "Task A alive", and the LED continued blinking in a 500 ms on/off cycle. Changing the priority had, if anything, just a slight visible effect on how frequently either task executed. But this observation might just be imagination. After all, it is difficult to spot the synchronicity once the Serial Monitor field is overflowing with text. The only way to tell the Serial Monitor is still working is by fixating on the scrollbar on the right, which basically makes spotting out of sync behaviour impossible.

Both tasks spend most of their time in the Blocked state, since they regularly call vTaskDelay(). From what I observed, a task's priority only seems to matter when multiple tasks are Ready at the exact same instant, and the scheduler has to pick between them. Since Task A (1000 ms period) and Task B (500 ms period) run on different delay periods, they're almost never Ready at the same moment, so there is rarely a real conflict for the scheduler to resolve using priority in the first place. As soon as a tasks delay runs out, it briefly moves to Running, does its short bit of work (printing the message, or toggling the LED), and then immediately calls vTaskDelay() again, going straight back to Blocked. So a higher priority doesn't make a task run more often in my tests, it would only change the outcome if both tasks happened to be Ready at the same time, which basically never happened here. The actual execution frequency I observed was determined entirely by each tasks own delay value, not by its priority.

# 5. Starvation experiment 
After making the changes of upping the priority of taskA to 2 and commenting out its vTaskDelay() function, I uploaded the sketch and found that the LED kept blinking red like it did before. What changed dramatically was the printing of the text from taskA which moved up in frequency. It was now printing non stop with seemingly zero delay, which makes sense, because the delay function is no longer part of the code. After the experiment, I changed the code back to its previous state which led to both tasks working as normally again.

This wasn't quite what I expected. Based on task states and priority alone, taskA should have stayed in the Running state permanently. With no delay call, it never voluntarily enters the Blocked state, so the scheduler should never have a reason to switch to a lower priority task on the same core. In theory, taskB should not have worked anymore.

What I think actually happened is that taskA isn't Running the whole time, printing to the Serial Monitor takes it out of Running for tiny moments here and there, even without an explicit vTaskDelay(). Those tiny gaps seem to be just enough for the scheduler to briefly switch over to Task B, let it toggle the LED, and switch back. So taskB wasn't starved completely, but it might not have run as smoothly as before, which still was hard to judge for my eyes. It only got CPU time in the small gaps taskA left behind, purely because of what taskA happens to be doing (printing), not because taskA blocked on purpose.

# 6. Ready, Running and Blocked explanation 
A task is Running when the scheduler has actually assigned it the CPU and its code is executing, for example when Task A is inside Serial.println() or Task B is calling neopixelWrite(). vTaskDelay() moves a task to the Blocked state because it tells the scheduler it doesn't need the CPU again until a set number of ticks has passed, so it's removed from contention instead of wasting cycles. Once the delay expires, the task becomes Ready, eligible to run, but not guaranteed the CPU immediately. The scheduler then picks the highest priority Ready task to enter Running.

In my scenarios A, B and C, priority didn't visibly change execution frequency. TaskA kept printing roughly every 1000 ms and the LED kept its 500 ms cycle regardless of which task had priority 2. What I did notice was the LED and Serial output drifting in and out of sync over time (especially in B and C), rather than one task dominating. This fits the state model, since the two periods (1000 ms vs. 500 ms) don't align evenly, the tasks are almost never Ready at the same instant, so the scheduler rarely has to use priority to choose between them, the drift comes from small timing shifts, not priority favoring one task.

The starvation test showed why priority matters once blocking stops. Theoretically, removing Task A's delay keeps it indefinitely Ready/Running, and since the scheduler always preempts lower priority tasks while a higher priority one is Ready, taskB should suffer total starvation. In my trace, taskB still blinked, which I explain by taskA briefly leaving Running during each Serial.println() call, small gaps apparently enough for the scheduler to pick taskB, not deliberate blocking.

# 7. Final restored configuration 
#define RGB_BUILTIN 38 
#define RGB_BRIGHTNESS 50 
 
void taskA(void *parameter) { 
  for (;;) { 
    Serial.println("Task A alive"); 
    vTaskDelay(pdMS_TO_TICKS(1000)); 
  } 
} 
 
void taskB(void *parameter) { 
  for (;;) { 
    neopixelWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0); 
    vTaskDelay(pdMS_TO_TICKS(500)); 
    neopixelWrite(RGB_BUILTIN, 0, 0, 0); 
    vTaskDelay(pdMS_TO_TICKS(500)); 
  } 
} 
 
void setup() { 
  Serial.begin(115200); 
  delay(500); 
 
  xTaskCreatePinnedToCore(taskA, "Task A", 2048, NULL, 1, NULL, 1); 
  xTaskCreatePinnedToCore(taskB, "Task B", 2048, NULL, 1, NULL, 1); 
} 
 
void loop() { 
  // Work is performed by the FreeRTOS tasks. 
} 

