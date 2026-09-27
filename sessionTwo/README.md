1. Hardware and software 

2. Baseline implementation
   After verifying and uploading the sketch I observed it for a while and found out that it works fine as can be seen in the screenshot below.
   <img width="1503" height="869" alt="Screenshot 2026-09-27 at 10 40 13" src="https://github.com/user-attachments/assets/0571d7a5-8f35-4cf2-ac4f-20322d1d74ae" />
   The taskA() function triggers the "Task A alive" printing to the serial monitor with a 1000ms gap between each print. 
   The taskB() function triggers the LED to blink red with brightness 50 and a 500ms delay. After that, the LED is being turned off for 500ms after which the cycle repeats.


3. Prediction and observation table 

--------------------------------------------------------
# A. Both tasks at priority 1
  
-> Prediction before test: 
Both tasks run smoothly because both have the same priority and are able to block regularly.
  
-> Actual observation:
Task A printed 'Task A alive' to the Serial Monitor approximately every 1000 ms, while the RGB LED blinked red with a 500 ms on/off cycle. Both continued running                      simultaneously and periodically for the full observation period, with no noticeable delay or interruption in either task.
  
-> Did it match? Why?:
Yes. Since both tasks share the same priority, the scheduler doesn't favour either one when choosing which task to run next. For most of the time, each task sits in the Blocked state, waiting on its vTaskDelay() call. Only when a tasks delay expires does it move to the Ready state, and since the two delay periods (1000 ms and 500 ms) rarely line up exactly, the tasks are almost never Ready at the same moment, so there is no real competition between them. Whichever task is Ready at that moment simply gets picked by the scheduler and briefly enters the Running state to execute its short piece of code (print the message, or toggle the LED), before calling vTaskDelay() again and returning to Blocked. Because of this, the timing is driven only by the delay values, not by the priority setting.
  
--------------------------------------------------------  
# B. Task A priority 2; Task B priority 1

-> Prediction before test: 
Since task two has the highes priority it will be Running for most of the time, meaning the LED will always blink red whereas the "Task A alive" will only be printed when Task B is in Blocked state. 
  
-> Actual observation: 
The red LED is blinking non stop, the "Task A alive" is printed non stop as well but there is a slight mismatch between execution, at least thats what it seems like when looking at it. The red blinking and the printing in the serial monitor happen out of sync, kind of like in #A above.

-> Did it match? Why?:
Not quite. Part of the mismatch was my own assumption going in. I had assumed priority 1 was the highest, when in FreeRTOS a higher number actually means higher priority, so Task A (priority 2) was in fact the higher-priority task all along, not Task B.
More importantly, the actual behaviour itself didn't match the "one dominates the other" prediction either way. Both tasks kept running periodically, the LED kept blinking and "Task A alive" kept printing, because both tasks still call vTaskDelay(), which regularly moves each one into the Blocked state regardless of its priority. Priority only decides which task the scheduler picks when more than one task is Ready at the same instant, it doesn't stop a task from blocking on its own delay. Since Task A's and Task B's delay periods (1000 ms vs. 500 ms) don't line up evenly, the exact moments they become Ready and get scheduled gradually shift, which is likely what caused the LED and Serial output to appear slightly out of sync, rather than any task being starved.
  
--------------------------------------------------------
# C. Task A priority 1; Task B priority 2

-> Prediction before test: 
Judging from the findings above I assume that both tasks will get executed even though task B has the higher priority. The only thing that changes might be that the execution will now happen reversed to the above, which to the human eye will barely be visible. 
  
-> Actual observation: 
The LED blinks red, the serial monitor prints the text. Both happen slightly out of sync again, aligning with each other as the time period goes on but then losing their syncronisation again.
  
-> Did it match? Why?:
Yes, largely. As predicted, both tasks kept running regardless of which one had the higher priority, Task B being priority 2 this time didn't stop Task A from executing, just like Task A being priority 2 in the previous scenario didn't stop Task B. Both tasks still call vTaskDelay(), so each one regularly enters the Blocked state on its own, independent of priority. The scheduler only uses priority to decide which task to run when two tasks are Ready at the exact same moment, and since actual code execution (printing to Serial, writing to the LED) takes a small, not perfectly constant amount of time each cycle, the moments they become Ready keep shifting slightly, which explains why the LED and Serial output drift in and out of sync rather than staying perfectly aligned or one completely dominating the other.

--------------------------------------------------------

4. Priority experiments 

5. Starvation experiment 

6. Ready, Running and Blocked explanation 

7. Final restored configuration 
