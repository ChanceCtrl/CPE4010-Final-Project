#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <lcd1602.h>

Servo dollServo;
Servo armServo;

// Buttons
const int buttonPin = 2; // Game reset
const int playerPin = 3; // "walk forward"
const int manualPin = 4; // manual green/red toggle

// Game state signs
const int dollServoPin = 5;
const int armServoPin = 6;
const int redLedPin = 7;
const int greenLedPin = 8;

// Distance
const int trigPin = 9;
const int echoPin = 10;

// Buzzer
const int buzzerPin = 11;

// Motor Relay
const int relayPin = 13;

// Landmines
const int landmine1Pin = A0;
const int landmine2Pin = A1;

unsigned long lastLightChange = 0;
unsigned long lastSecondTick = 0;
bool isGreenLight = true;
const int greenDuration = 2000; // 2 sec
const int redDuration = 3000;   // 3 sec
int currentDuration = greenDuration;

int timeLeft = 60;
float lastRedDistance = 0.0;
int landmine1Base = 0;
int landmine2Base = 0;

int gameState = 0; // 0 = idle, 1 = playing, 2 = win, 3 = eliminated

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2.0; // cm
}

void updateLightsAndSounds() {
  if (isGreenLight) {
    digitalWrite(greenLedPin, HIGH);
    digitalWrite(redLedPin, LOW);
    dollServo.write(0);   // face away
    tone(buzzerPin, 523); // low tone (C5)
  } else {
    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, HIGH);
    dollServo.write(180);  // face player
    tone(buzzerPin, 1047); // high tone (C6)
  }
}

void updateLCD() {
  lcd1602SetCursor(0, 0);
  lcd1602WriteString("Time: ");
  lcd1602WriteString(timeLeft);
  lcd1602WriteString("   ");
  lcd1602SetCursor(0, 1);
  lcd1602WriteString(isGreenLight ? "GREEN LIGHT" : "RED LIGHT   ");
}

void startGame() {
  gameState = 1;
  timeLeft = 60;
  isGreenLight = true;
  currentDuration = greenDuration;
  lastLightChange = millis();
  lastSecondTick = millis();
  landmine1Base = analogRead(landmine1Pin);
  landmine2Base = analogRead(landmine2Pin);
  dollServo.write(0);
  armServo.write(0);
  updateLightsAndSounds();
  updateLCD();
  lcd1602SetCursor(0, 1);
  lcd1602WriteString("Game Started!  ");
}

void winGame() {
  gameState = 2;
  noTone(buzzerPin);
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  lcd1602Clear();
  lcd1602WriteString("YOU WIN!");
  lcd1602SetCursor(0, 1);
  lcd1602WriteString("Press button");
}

void eliminatePlayer() {
  gameState = 3;
  noTone(buzzerPin);
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  lcd1602Clear();
  lcd1602WriteString("ELIMINATED!");
  armServo.write(120); // sweep arm to knock player off
  delay(800);
  armServo.write(0); // reset arm
  lcd1602SetCursor(0, 1);
  lcd1602WriteString("Press button");
}

void checkMotionDuringRed() {
  float currentDist = getDistance();
  if (abs(currentDist - lastRedDistance) > 2.0) { // ignore <2 cm noise
    eliminatePlayer();
  }
}

void checkLandmines() {
  int landmine1 = analogRead(landmine1Pin);
  int landmine2 = analogRead(landmine2Pin);
  // "step on and off" = any big change from calibrated base
  if (abs(landmine1 - landmine1Base) > 150 ||
      abs(landmine2 - landmine2Base) > 150) {
    eliminatePlayer();
  }
}

void resetGame() {
  gameState = 0;
  noTone(buzzerPin);
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  dollServo.write(0);
  armServo.write(0);
  lcd1602Clear();
  lcd1602WriteString("Press button to");
  lcd1602SetCursor(0, 1);
  lcd1602WriteString("start game");
}

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  dollServo.attach(dollServoPin);
  armServo.attach(armServoPin);

  lcd1602Init(0x27);
  lcd1602Control(true, false, false);

  resetGame();
}

void loop() {
  // Button handling (start or reset)
  if (digitalRead(buttonPin) == LOW) {
    delay(200); // simple debounce
    if (gameState == 0) {
      startGame();
    } else if (gameState == 2 || gameState == 3) {
      resetGame();
    }
    while (digitalRead(buttonPin) == LOW)
      ; // wait for release
  }

  if (gameState == 1) { // PLAYING
    // Light period timer
    if (millis() - lastLightChange >= currentDuration) {
      isGreenLight = !isGreenLight;
      currentDuration = isGreenLight ? greenDuration : redDuration;
      lastLightChange = millis();
      updateLightsAndSounds();

      if (!isGreenLight) {
        lastRedDistance = getDistance(); // record distance at start of Red
      }
    }

    // 1-second countdown
    if (millis() - lastSecondTick >= 1000) {
      timeLeft--;
      lastSecondTick = millis();
      updateLCD();
      if (timeLeft <= 0) {
        eliminatePlayer();
      }
    }

    // Win condition
    float dist = getDistance();
    if (dist <= 5.0 && dist > 0) {
      winGame();
    }

    // Red Light motion check (multiple readings via fast loop)
    if (!isGreenLight) {
      checkMotionDuringRed();
    }

    // Landmine check
    checkLandmines();
  }
}
