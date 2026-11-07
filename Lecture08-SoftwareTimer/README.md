# Explanation 
The OLED display module available to me did not work. The screen texts are therefore printed to the Serial Monitor with the prefix [SCREEN]. The onboard RGB LED is controlled normally.

# Wiring
- OLED (as specified in the assignment): SSD1306, I2C address 0x3C
- SDA: GPIO 8
- SCL: GPIO 9
- VCC: 3V3
- GND: GND
- Onboard RGB LED: GPIO 38 

# Required Libraries
No external libraries. Only the Arduino-ESP32 core (esp32 by Espressif Systems, version 3.x) is used, including its built-in FreeRTOS headers and the RGB LED function rgbLedWrite.

# Board Settings
- Board: ESP32S3 Dev Module
- CPU Frequency: 240 MHz
- PSRAM: Disabled
- USB CDC On Boot: Disabled when using the USB-UART port, Enabled when using the native USB port
- Serial Monitor: 115200 baud

# Test Results (adjusted to not working display by using the serial monitor as a replacement)

Test 1: Restart the board and wait over 10 seconds
- Expected: Ready screen remains visible, LED stays off
- Observed: When restarting all serial monitor text disappears, the reloads and shows the Ready sign again, the LED stays off.
- Pass or Fail: pass

Test 2: Send s
- Expected: Session starts, countdown appears and LED turns on
- Observed: When typing s, the session starts immediately, the serial monitor counts down from 10 to 1 second, for this time period the LED is on, after 1 second it turns off.
- Pass or Fail: pass

Test 3: Send s repeatedly during the session
- Expected: Original session continues without extending
- Observed: When typing s while a session is running, the original session stays running and is not interrupted by the new s.
- Pass or Fail: pass

Test 4: Allow the session to finish
- Expected: Completion screen appears and LED turns off after approximately 10 seconds
- Observed: After the count down finishes and the last second has passed, the "Session complete" text shows up and the LED turns off.
- Pass or Fail: pass

Test 5: Wait after completion
- Expected: Completion screen remains visible
- Observed: After completion the "Session complete" is still visible. The LED remains turned off.
- Pass or Fail: pass

Test 6: Send s again
- Expected: A new session starts with the full duration
- Observed: After completion starting a new session with s is possible and works just like the one before.
- Pass or Fail: pass

# Answers

## 1.) Why does this system need both a one-shot and an auto-reload timer?

The auto-reload timer fires every second and triggers the countdown update. The one-shot timer fires exactly once after 10 seconds and marks the end of the session. One timer alone cannot do both jobs, a one-shot timer does not repeat, and an auto-reload timer would never end the session on its own.

## 2.) Which task executes the timer callbacks?

The FreeRTOS timer service task (also called the timer daemon task). It is created by FreeRTOS.

## 3.) Why do OLED updates belong in the application task?

The callbacks run in the shared timer service task and must return quickly. Display transfers take time and would delay all other timers. The callbacks therefore only send a task notification, and the application task does the display, Serial and LED work.

## 4.) When are the timers dormant?

Both timers are dormant after they are created, until the first valid s command starts them. After a session ends they are dormant again, the one-shot timer has expired, and the application task stops the auto-reload timer. In the error state both timers are stopped as well.

## 5.) How does your program prevent extra s commands from extending the session?

A start is only accepted in the Ready or Complete state. While a session is running, the application task still reads the characters from Serial but discards them. The start time is not changed, the timers are not restarted, and xTimerReset is never called.

# Screenshot
<img width="1503" height="836" alt="image" src="https://github.com/user-attachments/assets/0acb3d7a-67cf-4996-820b-8c49f0b5749e" />
(Screenshot from Serial Monitor as a replacement for the OLED Screen since it didn't work even after using multiple different cables, checking wiring a thousand times and changing ports.)

