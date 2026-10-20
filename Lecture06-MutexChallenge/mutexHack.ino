#include <semphr.h>

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// ESP32-S3-DevKitC-1: onboard RGB LED (WS2812) on GPIO 38
static const int rgb_pin = 38;

// Mutex used to coordinate setup() and blinkLED()
static SemaphoreHandle_t mutex;

// Tasks

// Blink LED based on rate passed by parameter
void blinkLED(void *parameters) {

  // Take the mutex before reading the parameter
  xSemaphoreTake(mutex, portMAX_DELAY);

  // Copy the parameter into a local variable
  int num = *(int *)parameters;

  // Give the mutex back after copying (so setup() may continue)
  xSemaphoreGive(mutex);

  // Print the parameter
  Serial.print("Received: ");
  Serial.println(num);

  // Blink the onboard RGB LED (no pinMode needed for the WS2812)
  while (1) {
    rgbLedWrite(rgb_pin, 0, 0, 64);   // on (dim blue)
    vTaskDelay(num / portTICK_PERIOD_MS);
    rgbLedWrite(rgb_pin, 0, 0, 0);    // off
    vTaskDelay(num / portTICK_PERIOD_MS);
  }
}

// Main (runs as its own task with priority 1 on core 1)

void setup() {

  int delay_arg;

  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Mutex Challenge---");
  Serial.println("Enter a number for delay (milliseconds)");

  // Wait for input from Serial
  while (Serial.available() <= 0);

  // Read integer value
  delay_arg = Serial.parseInt();
  Serial.print("Sending: ");
  Serial.println(delay_arg);

  // Create the mutex (initially available)
  mutex = xSemaphoreCreateMutex();

  // Start task 1 (same priority 1 as setup/loopTask, as in the reference).
  // The task takes the mutex, copies the value and gives it back.
  xTaskCreatePinnedToCore(blinkLED,
                          "Blink LED",
                          2048,
                          (void *)&delay_arg,
                          1,
                          NULL,
                          app_cpu);

  // Yield one tick so the task can take the mutex before setup() does
  vTaskDelay(portTICK_PERIOD_MS);

  // Wait until the task has copied the value (mutex is free again)
  xSemaphoreTake(mutex, portMAX_DELAY);

  // Show that we accomplished our task of passing the stack-based argument
  Serial.println("Done!");
}

void loop() {

  // Do nothing but allow yielding to lower-priority tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}



