#ifndef USER_USB_RAM
#error "This example needs to be compiled with a USER USB setting"
#endif

#include "src/userUsbHidKeyboard/USBHIDKeyboard.h"

#define LED_PIN 33  // P1.1

void setup() {
  USBInit();
  pinMode(LED_PIN, OUTPUT);

  // Startup signal — 3 blinks
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }

  delay(2000); // Wait for HID to be recognized by the PC

  // Win + R to open Run dialog
  Keyboard_press(KEY_LEFT_GUI);
  Keyboard_press('r');
  delay(100);
  Keyboard_releaseAll();
  delay(500);

  // Type the URL
  Keyboard_print("https://www.netrunsecurity.com/");
  delay(200);

  // Enter
  Keyboard_press(KEY_RETURN);
  delay(100);
  Keyboard_releaseAll();

  // LED stays on = payload sent
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  // One-shot payload, nothing to do here
}