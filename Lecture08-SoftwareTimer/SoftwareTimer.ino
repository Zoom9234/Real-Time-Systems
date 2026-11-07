#include <Arduino.h>
#include <esp32-hal-rgb-led.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>

static constexpr uint8_t RGB_LED_PIN = 38;
static constexpr uint8_t LED_BRIGHTNESS = 32;
static constexpr uint32_t SESSION_MS = 10000;
static constexpr uint32_t UPDATE_MS = 1000;
static constexpr uint32_t EVENT_UPDATE = 1UL << 0;
static constexpr uint32_t EVENT_COMPLETE = 1UL << 1;
static constexpr uint32_t ALL_EVENTS = EVENT_UPDATE | EVENT_COMPLETE;
static const TickType_t SESSION_TICKS = pdMS_TO_TICKS(SESSION_MS);
static const TickType_t SECOND_TICKS = pdMS_TO_TICKS(UPDATE_MS);
static const TickType_t INPUT_POLL_TICKS = pdMS_TO_TICKS(10);
static const TickType_t TIMER_COMMAND_WAIT = pdMS_TO_TICKS(100);
static constexpr unsigned SERIAL_BYTES_PER_PASS = 64;

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

enum class SessionState { Ready, Running, Complete, Error };

static TimerHandle_t one_shot_timer = nullptr;
static TimerHandle_t auto_reload_timer = nullptr;
static TaskHandle_t application_task = nullptr;
static SessionState state = SessionState::Ready;
static TickType_t session_started_tick = 0;
static uint32_t last_displayed_seconds = UINT32_MAX;
static unsigned serial_bytes_to_ignore = 0;

void updateTimerCallback(TimerHandle_t) {
  xTaskNotify(application_task, EVENT_UPDATE, eSetBits);
}

void completeTimerCallback(TimerHandle_t) {
  xTaskNotify(application_task, EVENT_COMPLETE, eSetBits);
}

void setSessionLed(bool on) {
  rgbLedWrite(RGB_LED_PIN, 0, on ? LED_BRIGHTNESS : 0, 0);
}

void showMessage(const char *line1, const char *line2 = nullptr) {
  Serial.print("[SCREEN] ");
  Serial.print(line1);
  if (line2 != nullptr) {
    Serial.print(" | ");
    Serial.print(line2);
  }
  Serial.println();
}

void showCountdown(uint32_t remaining_seconds) {
  if (remaining_seconds == 0 || remaining_seconds == last_displayed_seconds) {
    return;
  }
  last_displayed_seconds = remaining_seconds;
  char line[24];
  snprintf(line, sizeof(line), "Remaining: %lu s",
           static_cast<unsigned long>(remaining_seconds));
  showMessage("Session running", line);
}

uint32_t remainingSeconds() {
  const TickType_t elapsed_ticks = xTaskGetTickCount() - session_started_tick;
  if (elapsed_ticks >= SESSION_TICKS) {
    return 0;
  }
  return (SESSION_TICKS - elapsed_ticks + SECOND_TICKS - 1) / SECOND_TICKS;
}

void enterError(const char *reason) {
  state = SessionState::Error;
  setSessionLed(false);

  if (one_shot_timer != nullptr &&
      xTimerStop(one_shot_timer, TIMER_COMMAND_WAIT) != pdPASS) {
    Serial.println("Error: could not queue one-shot timer stop");
  }
  if (auto_reload_timer != nullptr &&
      xTimerStop(auto_reload_timer, TIMER_COMMAND_WAIT) != pdPASS) {
    Serial.println("Error: could not queue periodic timer stop");
  }

  Serial.print("Error: ");
  Serial.println(reason);
  Serial.println("Fix the cause and reset the board.");
  showMessage("Error", "See Serial; reset");
}

void startSession() {
  if (state != SessionState::Ready && state != SessionState::Complete) {
    return;
  }

  uint32_t discarded_events = 0;
  xTaskNotifyWait(0, ALL_EVENTS, &discarded_events, 0);
  last_displayed_seconds = UINT32_MAX;
  session_started_tick = xTaskGetTickCount();
  state = SessionState::Running;

  if (xTimerStart(one_shot_timer, TIMER_COMMAND_WAIT) != pdPASS) {
    enterError("could not start one-shot timer");
    return;
  }
  if (xTimerStart(auto_reload_timer, TIMER_COMMAND_WAIT) != pdPASS) {
    enterError("could not start periodic timer");
    return;
  }

  setSessionLed(true);
  showCountdown(SESSION_MS / 1000);
}

void completeSession() {
  serial_bytes_to_ignore = static_cast<unsigned>(Serial.available());
  state = SessionState::Complete;
  setSessionLed(false);

  if (xTimerStop(auto_reload_timer, TIMER_COMMAND_WAIT) != pdPASS) {
    enterError("could not stop periodic timer");
    return;
  }

  showMessage("Session complete", "Send s to start");
}

void processSerialInput() {
  for (unsigned i = 0; i < SERIAL_BYTES_PER_PASS; ++i) {
    if (Serial.available() <= 0) {
      break;
    }
    const int input = Serial.read();
    if (input < 0) {
      break;
    }
    if (serial_bytes_to_ignore > 0) {
      --serial_bytes_to_ignore;
      continue;
    }
    if (input == 's' &&
        (state == SessionState::Ready || state == SessionState::Complete)) {
      startSession();
    }
  }
}

void applicationTask(void *) {
  for (;;) {
    uint32_t events = 0;
    xTaskNotifyWait(0, ALL_EVENTS, &events, INPUT_POLL_TICKS);

    if (state == SessionState::Running) {
      if ((events & EVENT_COMPLETE) != 0) {
        completeSession();
      } else if ((events & EVENT_UPDATE) != 0) {
        showCountdown(remainingSeconds());
      }
    }
    processSerialInput();
  }
}

void setup() {
  Serial.begin(115200);
  setSessionLed(false);
  vTaskDelay(pdMS_TO_TICKS(1000));
  showMessage("Ready", "Send s to start");

  one_shot_timer = xTimerCreate(
      "Session end", SESSION_TICKS, pdFALSE, nullptr,
      completeTimerCallback);
  auto_reload_timer = xTimerCreate(
      "Countdown update", SECOND_TICKS, pdTRUE, nullptr,
      updateTimerCallback);

  if (one_shot_timer == nullptr || auto_reload_timer == nullptr) {
    enterError("timer creation failed");
    return;
  }

  if (xTaskCreatePinnedToCore(applicationTask, "Study session", 4096, nullptr,
                            1, &application_task, app_cpu) != pdPASS) {
    enterError("application task creation failed");
    return;
  }
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
