#include <Arduino.h>

#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change 0x27 if your LCD address is different

Servo dollServo;
Servo armServo;

const int buttonPin = 2;
const int trigPin = 9;
const int echoPin = 10;
const int dollServoPin = 5;
const int armServoPin = 6;
const int redLedPin = 7;
const int greenLedPin = 8;
const int buzzerPin = 11;
const int ldr1Pin = A0;
const int ldr2Pin = A1;

unsigned long lastLightChange = 0;
unsigned long lastSecondTick = 0;
bool isGreenLight = true;
const int greenDuration = 2000;   // 2 sec
const int redDuration = 3000;     // 3 sec
int currentDuration = greenDuration;

int timeLeft = 60;
float lastRedDistance = 0.0;
int ldr1Base = 0;
int ldr2Base = 0;

int gameState = 0;  // 0 = idle, 1 = playing, 2 = win, 3 = eliminated

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2.0;  // cm
}

void updateLightsAndSounds() {
  if (isGreenLight) {
    digitalWrite(greenLedPin, HIGH);
    digitalWrite(redLedPin, LOW);
    dollServo.write(0);        // face away
    tone(buzzerPin, 523);      // low tone (C5)
  } else {
    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, HIGH);
    dollServo.write(180);      // face player
    tone(buzzerPin, 1047);     // high tone (C6)
  }
}

void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(timeLeft);
  lcd.print("   ");
  lcd.setCursor(0, 1);
  lcd.print(isGreenLight ? "GREEN LIGHT" : "RED LIGHT   ");
}