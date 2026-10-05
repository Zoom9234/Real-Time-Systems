# 1. Hardware and Software
Board: ESP32-S3-DevKitC-1 v1.1
RGB LED: GPIO 38
IDE: Arduino IDE 2.x
Serial Monitor: 115200 baud

------------------------------------------------------------------------------
# 2. Task Design

The Serial Task runs an endless loop that checks the Serial port for incoming characters. When I press enter, it reads the full line and checks what was typed. If the input is 250, 500 or 1000, it updates the shared blinkInterval variable. If the input is "suspend" or "resume", it calls the matching FreeRTOS function on the LED task handle. Everything else triggers a short error message. At the end of each cycle the task calls vTaskDelay for 10 ms and goes Blocked, freeing the CPU for the LED task. 

The RGB LED Task also runs an endless loop. At the start of each cycle it reads the current blinkInterval value, turns the LED on in green using rgbLedWrite, waits that many milliseconds, turns it off and waits again.
Both tasks are pinned to core 1 and both have priority 1. The FreeRTOS scheduler treats them as equal. Because they share the same priority, the scheduler switches between them both when one yields the CPU by calling vTaskDelay and through round-robin time slicing, so neither task can monopolise the core. 

------------------------------------------------------------------------------
# 3. Task Handles

Two handle variables are declared globally at the top of the sketch. Both are set to NULL before any task is created.
In setup, xTaskCreatePinnedToCore is called for each task. The second to last argument is the address of the handle variable. FreeRTOS writes a reference to the newly created task into that address.
Once the tasks are running, the Serial Task uses ledTaskHandle to control the LED task directly.

It helped me to think of these three concepts separately. The task function is just the code that gets executed. In this sketch ledTask is the function. The task instance is the actual running task that FreeRTOS creates from that function, with its own stack and its own state in memory. The task handle is the reference that points to that specific instance. Without ledTaskHandle there is no way for the Serial Task to tell FreeRTOS which task to suspend or resume.

------------------------------------------------------------------------------
# 4. Suspend / Resume Test
Explain what happened when you used: suspend, resume

When I typed "suspend" and pressed enter, the LED stopped blinking immediately. The Serial Task called vTaskSuspend with ledTaskHandle and then turned the LED off with rgbLedWrite. I could still type into the Serial monitor without any problem. I tested this by entering 250 while the LED was suspended. The application accepted the command and printed a confirmation. The Serial Task was completely unaffected. 

When I typed "resume", the LED started blinking again. It used the interval that was active at the time of the resume. I had changed the interval to 250 while the LED was suspended, and when I resumed it the LED came back at 250 ms. This confirmed that blinkInterval was shared correctly between the two tasks the whole time.

------------------------------------------------------------------------------
# 5. Prediction

## LED task running normally

- Prediction: Both tasks work at the same time. LED blinks at 500 ms and Serial accepts commands.
  
- Actual Observation: LED blinked at 500 ms and Serial responded to all commands correctly

- Match? Yes because both tasks call vTaskDelay in their loops, which means neither one holds the CPU. They take turns naturally and both stay responsive at the same time.
  

## LED task suspended	 

- Prediction: LED stops blinking. Serial Task keeps working on its own
  
- Actual Observation: LED stopped immediately and Serial monitor kept accepting input

- Match? Yes because vTaskSuspend removes the LED task from the scheduler completely but has no effect on the Serial Task at all. The Serial Task has its own loop and its own vTaskDelay cycle and does not depend on the LED task being active.

## LED task resumed

- Prediction: LED starts blinking again using the interval that was set last.
  
- Actual Observation: LED came back at the correct interval right away

- Match? Yes because vTaskResume puts the LED task back into Ready state and the scheduler picks it up shortly after. When the task runs again it reads the current blinkInterval value at the top of its loop, so whatever interval was set before or during the suspend was used immediately.

------------------------------------------------------------------------------
# 6. State Analysis

## Situation 1, Normal operation

The Serial Task is mostly in Blocked state. After each loop it calls vTaskDelay for 10 ms and waits there. Every 10 ms its delay expires and it moves to Running to check the Serial port, if characters have arrived it processes them, otherwise it goes back to the Blocked right away.
The RGB LED Task is also mostly in Blocked state. After each rgbLedWrite call it calls vTaskDelay for the full blink interval. When that delay expires it moves to Ready and then to Running as soon as the scheduler gives it CPU time.
Both tasks are Blocked most of the time. This is why they can share a single core without one preventing the other from doing its work.

## Situation 2, LED task is suspended

The Serial Task continues switching between Blocked and Running exactly as before. 
The RGB LED Task is in Suspended state. This is different from Blocked. A Blocked task is still tracked by the scheduler and wakes up automatically when its delay expires. A Suspended task is removed from the scheduler entirely. It will not run again until something explicitly calls vTaskResume on its handle.

## Situation 3, LED task is resumed

The Serial Task continues as before.
The RGB LED Task moves from Suspended to Ready when vTaskResume is called. Ready means the task is now eligible for scheduling again but it is not Running yet. The scheduler decides when it actually gets CPU time. Once it does it moves to Running and starts the next blink cycle using the current value of blinkInterval.

------------------------------------------------------------------------------
# 7. Reflection

I expected suspend and resume to work like simple on and off switches. What I learned is that there is more to it than that.
The task handle is the reason one task can control another. When I created the LED task and passed &ledTaskHandle, FreeRTOS stored a reference to that specific task instance in that variable. That reference is what makes vTaskSuspend and vTaskResume target the right task. The task function is just code. The handle points to the running instance. 

When vTaskSuspend(ledTaskHandle) was called, the LED task moved into Suspended state. The scheduler stopped scheduling it entirely and it was no longer eligible to run.
When vTaskResume(ledTaskHandle) was called, the LED task moved to Ready state. The scheduler put it back in the pool and eventually gave it CPU time, at which point it entered Running state. 
The Serial Task kept working throughout all of this because it has no dependency on the LED task. Both tasks use vTaskDelay to yield the CPU, which is the only coordination they need. The LED task being Suspended changed nothing for the Serial Task because they are independed. 
