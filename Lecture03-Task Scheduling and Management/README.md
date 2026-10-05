# 1.) Assignment purpose

In this assignment, I investigated how the FreeRTOS scheduler selects tasks.

I used two tasks running on the same ESP32-S3 core and observed:

Running, Ready and Blocked states
Task priorities
Preemption
Equal-priority behaviour

The goal was not only to get the program working, but also to observe the scheduler and explain what happens.

----------------------------
# 2.) Hardware and software

Hardware: ESP32-S3-DevKitC-1 v1.1

Software: Arduino IDE 2.x; Serial Monitor; 115200 baud

----------------------------
# 3.) Your three test scenarios

To investigate the scheduler, I kept the task code identical in all three runs and only changed the priority of Task B, as required by the assignment. Task A always used xTaskNotifyGive() while printing its message character by character, and Task B always used ulTaskNotifyTake() to wait for that notification.

Scenario A: Task A = priority 1, Task B = priority 2, meaning Task B has higher priority than Task A

Scenario B: Task A = priority 1, Task B = priority 1, meaning both tasks have the same priority

Scenario C: Task A = priority 1, Task B = priority 3, meaning Task B has an even higher priority than in Scenario A

Both tasks were pinned to the same CPU core (core 1) in all three scenarios, so any difference in behaviour could only come from the priority setting and not from running on different cores.

----------------------------
# 4.) Prediction and observation table

## A: Task B priority = 2

- Prediction: 
Task B has a higher priority than Task A. As soon as Task A calls xTaskNotifyGive(), Task B moves from Blocked to Ready. Because it is the highest priority Ready task, the scheduler should immediately switch to it and preempt Task A in the middle of its message. Task B prints its message and then calls ulTaskNotifyTake() again, which puts it back into the Blocked state. Task A is then the highest priority Ready task again and continues its message.

- Actual Observation:
Task B's message appeared in the middle of Task A's text, before Task A had finished printing.
After that, Task A continued exactly where it had stopped.
 
- Match? Why?:
Yes, the prediction matched my observation. Task B preempted Task A immediately because it has the higher priority and became Ready at the moment of the notification. Task A did not finish its message first, which shows that the scheduler always runs the highest priority Ready task. Task A could continue at the correct character because its context was saved during the switch and restored when Task B blocked again.

## B: Task B priority = 1	

- Prediction:
Both tasks now have the same priority. At the start, Task B is in the Blocked state because it is waiting in ulTaskNotifyTake(), so only Task A is Running. When Task A calls xTaskNotifyGive(), Task B moves from Blocked to Ready. Since Task B no longer has a higher priority, it should not preempt Task A immediately. Task A stays in the Running state, and Task B waits in the Ready state until the scheduler switches to it at the next tick (round-robin time slicing between tasks of equal priority). Because Task B only has to wait for the next tick, which is very short compared to the 150 ms Task A needs per character, I expect the output to look almost the same as in Scenario A. Task A prints the first part of its message, Task B prints its message, and Task A then continues. After printing, Task B calls ulTaskNotifyTake() again and goes back into the Blocked state, so Task A is the only Ready task and continues its message.

- Actual Observation:
I observed the same behaviour as in Scenario A. Task B's message appeared in the middle of Task A's text, and Task A then continued exactly where it had stopped.

- Match? Why?:
Yes, the prediction matched my observation, but the reason is different from Scenario A. In Scenario A, Task B preempted Task A immediately because it had a higher priority. In Scenario B, Task B did not preempt Task A. It became Ready and had to wait until the next tick, where the scheduler gave the CPU to it through time slicing, since both tasks have the same priority. The difference is not visible to me.

## C: Task B priority = 3	 

- Prediction:
There will be no difference to Scenario A. Everything will work the same way because Task B again got assigned a higher priority than Task A. In this case it doesn't matter if that higher priority is called 2 or 3. For the scheduler, a higher priority means it will prefer that task once it is in the Ready state, and it will preempt Task A, which is currently Running.

- Actual Observation:
I observed the same behaviour as in Scenario A. Task B's message appeared in the middle of Task A's text, and Task A then continued exactly where it had stopped

- Match? Why?:
Yes, the prediction matched my observation. Task B preempted Task A immediately, just like in Scenario A, because it has a higher priority than Task A and became Ready at the moment of the notification. The scheduler only decides which Ready task has the highest priority, so the size of the difference doesn't matter for the result. Task A could continue at the correct character because its context was saved during the switch and restored when Task B returned to the Blocked state.

----------------------------
# 5.) Your explanation

In FreeRTOS, a task can be in one of several states. The Running state means the task is currently executing on the CPU, since both my tasks were pinned to the same core, only one of them could be Running at any moment. The Ready state means a task is able to run but is not currently given the CPU, because another task with equal or higher priority is Running instead. The Blocked state means a task is waiting for an event, such as a notification or a timeout, and cannot be selected by the scheduler until that event occurs.

Task B was initially Blocked because it called ulTaskNotifyTake() with an infinite timeout, and no notification had been sent yet. When Task A called xTaskNotifyGive(), Task B's notification was given, so it moved from Blocked to Ready. In Scenarios A and C, Task B had a higher priority than Task A, so the scheduler immediately switched to it, preempting Task A in the middle of its message, since FreeRTOS always runs the highest priority Ready task.

This is exactly what I saw in the Serial Monitor. In one run, Task A had only printed "Task A is pri" when it was interrupted; "--Task B is running--" appeared right in the middle of the word "printing", and only afterwards did the rest of the line, "nting slowly...", get printed. This confirms that the interruption happens at a point decided by the scheduler, not at the end of a line or a word, and that the exact position can differ slightly between runs depending on timing.

In Scenario B, both tasks had equal priority, so Task B did not preempt Task A immediately, it only received the CPU through round robin time slicing at a later tick, which was too short to be visible in the Serial Monitor output. This showed me that "becoming Ready" and "being run immediately" are not the same thing, and that priority, not just readiness, decides preemption.

In every scenario, Task A was able to continue exactly where it had stopped, because the scheduler saved its context when it was switched out, and restored that context when it was scheduled again.
