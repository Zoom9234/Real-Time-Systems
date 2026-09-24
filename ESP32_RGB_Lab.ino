#define RGB_BUILTIN 38 
 
void setup() {} 
 
void loop() { 

  // ===== H (....) =====
  // H - dot 1 (red)
  neopixelWrite(RGB_BUILTIN, 50, 0, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // H - dot 2 (green)
  neopixelWrite(RGB_BUILTIN, 0, 50, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // H - dot 3 (blue)
  neopixelWrite(RGB_BUILTIN, 0, 0, 50);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // H - dot 4 (white)
  neopixelWrite(RGB_BUILTIN, 50, 50, 50);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(400); // letter gap

  // ===== E (.) =====
  // E - dot 1 (red)
  neopixelWrite(RGB_BUILTIN, 50, 0, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(400); // letter gap

  // ===== L (.-..) =====
  // L - dot 1 (green)
  neopixelWrite(RGB_BUILTIN, 0, 50, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // L - dash 1 (blue)
  neopixelWrite(RGB_BUILTIN, 0, 0, 30);
  delay(600);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // L - dot 2 (white)
  neopixelWrite(RGB_BUILTIN, 50, 50, 50);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // L - dot 3 (red)
  neopixelWrite(RGB_BUILTIN, 50, 0, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(400); // letter gap

  // ===== L (.-..) =====
  // L - dot 1 (green)
  neopixelWrite(RGB_BUILTIN, 0, 50, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // L - dash 1 (blue)
  neopixelWrite(RGB_BUILTIN, 0, 0, 30);
  delay(600);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // L - dot 2 (white)
  neopixelWrite(RGB_BUILTIN, 50, 50, 50);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // L - dot 3 (red)
  neopixelWrite(RGB_BUILTIN, 50, 0, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(400); // letter gap

  // ===== O (---) =====
  // O - dash 1 (green)
  neopixelWrite(RGB_BUILTIN, 0, 30, 0);
  delay(600);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // O - dash 2 (blue)
  neopixelWrite(RGB_BUILTIN, 0, 0, 30);
  delay(600);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // O - dash 3 (white)
  neopixelWrite(RGB_BUILTIN, 30, 30, 30);
  delay(600);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(800); // word gap

  // ===== T (-) =====
  // T - dash 1 (red)
  neopixelWrite(RGB_BUILTIN, 30, 0, 0);
  delay(600);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(400); // letter gap

  // ===== H (....) =====
  // H - dot 1 (green)
  neopixelWrite(RGB_BUILTIN, 0, 50, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // H - dot 2 (blue)
  neopixelWrite(RGB_BUILTIN, 0, 0, 50);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // H - dot 3 (white)
  neopixelWrite(RGB_BUILTIN, 50, 50, 50);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // H - dot 4 (red)
  neopixelWrite(RGB_BUILTIN, 50, 0, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(400); // letter gap

  // ===== E (.) =====
  // E - dot 1 (green)
  neopixelWrite(RGB_BUILTIN, 0, 50, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(400); // letter gap

  // ===== R (.-.) =====
  // R - dot 1 (blue)
  neopixelWrite(RGB_BUILTIN, 0, 0, 50);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // R - dash 1 (white)
  neopixelWrite(RGB_BUILTIN, 30, 30, 30);
  delay(600);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // R - dot 2 (red)
  neopixelWrite(RGB_BUILTIN, 50, 0, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(400); // letter gap

  // ===== E (.) =====
  // E - dot 1 (green)
  neopixelWrite(RGB_BUILTIN, 0, 50, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(400); // letter gap

  // ===== ! (-.-.--) =====
  // ! - dash 1 (blue)
  neopixelWrite(RGB_BUILTIN, 0, 0, 30);
  delay(600);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // ! - dot 1 (white)
  neopixelWrite(RGB_BUILTIN, 50, 50, 50);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // ! - dash 2 (red)
  neopixelWrite(RGB_BUILTIN, 30, 0, 0);
  delay(600);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // ! - dot 2 (green)
  neopixelWrite(RGB_BUILTIN, 0, 50, 0);
  delay(200);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // ! - dash 3 (blue)
  neopixelWrite(RGB_BUILTIN, 0, 0, 30);
  delay(600);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(200);
  // ! - dash 4 (white)
  neopixelWrite(RGB_BUILTIN, 30, 30, 30);
  delay(600);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
  delay(2000); 
} 


