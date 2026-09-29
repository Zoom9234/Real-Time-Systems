#define LED_PIN 38

volatile uint32_t blinkInterval = 500;

TaskHandle_t serialTaskHandle = NULL;
TaskHandle_t ledTaskHandle    = NULL;

void serialTask(void *pvParameters) {
    String buf = "";
    for (;;) {
        while (Serial.available()) {
            char c = (char)Serial.read();
            if (c == '\n' || c == '\r') {
                buf.trim();
                if (buf == "250" || buf == "500" || buf == "1000") {
                    blinkInterval = buf.toInt();
                    Serial.println("Interval: " + buf + " ms");
                } else if (buf == "suspend") {
                    vTaskSuspend(ledTaskHandle);
                    rgbLedWrite(LED_PIN, 0, 0, 0);
                    Serial.println("LED task suspended.");
                } else if (buf == "resume") {
                    vTaskResume(ledTaskHandle);
                    Serial.println("LED task resumed.");
                } else if (buf.length() > 0) {
                    Serial.println("Invalid. Use: 250 | 500 | 1000 | suspend | resume");
                }
                buf = "";
            } else {
                buf += c;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void ledTask(void *pvParameters) {
    for (;;) {
        uint32_t interval = blinkInterval;
        rgbLedWrite(LED_PIN, 0, 150, 0);
        vTaskDelay(pdMS_TO_TICKS(interval));
        rgbLedWrite(LED_PIN, 0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(interval));
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Commands: 250 | 500 | 1000 | suspend | resume");

    xTaskCreatePinnedToCore(ledTask,    "LED Task",    2048, NULL, 1, &ledTaskHandle,    1);
    xTaskCreatePinnedToCore(serialTask, "Serial Task", 4096, NULL, 1, &serialTaskHandle, 1);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
