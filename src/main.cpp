#include <Arduino.h>
#include <lcd1602.h>

bool backlight = true;

void setup() {
  // put your setup code here, to run once:
  lcd1602Init(0x27);
  lcd1602Powerup();
}

void loop() {
  delay(1000);
  lcd1602SetCursor(0, 0);
  backlight = !backlight;

  switch (analogRead(A0)) {
  case 0 ... 300:
    lcd1602WriteString("ALERT: Dry!");
    lcd1602Control(backlight, 0, 1);
    break;
  default:
    lcd1602Clear();
    lcd1602Control(0, 0, 1);
    break;
  }
}
