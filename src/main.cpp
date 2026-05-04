#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <lcd1602.h>
#include <math.h>

enum GameState { IDLE, PLAYING, WIN, ELIMINATED };
GameState gameState = IDLE;

// Servo objects
Servo dollServo;
Servo armServo;

// Pins
const int buttonPin = 2;
const int playerPin = 3;

const int dollServoPin = 6;
const int armServoPin = 5;
const int redLedPin = 7;
const int greenLedPin = 8;

const int trigPin = 9;
const int echoPin = 10;

const int buzzerPin = 11;
const int relayPin = 12;

const int landmine1Pin = A0;
const int landmine2Pin = A1;

// Input stuffs
unsigned long lastLightChange = 0;
unsigned long lastSecondTick = 0;
unsigned long lastButtonPress = 0;
bool lastButtonState = HIGH;
bool buttonReleased = true;

// Game stuffs
bool isGreenLight = true;
const int greenDuration = 2000;
const int redDuration = 3000;
int currentDuration = greenDuration;

int timeLeft = 60;
float lastRedDistance = 0.0;

int landmine1Base = 0;
int landmine2Base = 0;

unsigned long armMoveTime = 0;
bool armMoving = false;

float getDistanceRaw() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // timeout added
  if (duration == 0)
    return 999; // no reading

  return duration * 0.034 / 2.0;
}

float getDistanceSmooth() {
  float sum = 0;
  int valid = 0;

  for (int i = 0; i < 5; i++) {
    float d = getDistanceRaw();
    if (d < 400) { // ignore bad readings
      sum += d;
      valid++;
    }
    delay(5);
  }

  if (valid == 0)
    return 999;
  return sum / valid;
}

void updateLightsAndSounds() {
  if (isGreenLight) {
    digitalWrite(greenLedPin, HIGH);
    digitalWrite(redLedPin, LOW);
    dollServo.write(0);
    tone(buzzerPin, 523);
  } else {
    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, HIGH);
    dollServo.write(180);
    tone(buzzerPin, 1047);
  }
}

void updateLCD() {
  if (gameState != PLAYING)
    return;

  char buffer[16];
  lcd1602SetCursor(0, 0);
  sprintf(buffer, "Time: %d", timeLeft);
  lcd1602WriteString(buffer);

  lcd1602SetCursor(0, 1);
  lcd1602WriteString(isGreenLight ? "GREEN LIGHT     " : "RED LIGHT       ");
}

void startGame() {
  Serial.println("Game Started");

  gameState = PLAYING;

  timeLeft = 60;
  isGreenLight = true;
  currentDuration = greenDuration;

  lastLightChange = millis();
  lastSecondTick = millis();

  landmine1Base = analogRead(landmine1Pin);
  landmine2Base = analogRead(landmine2Pin);

  dollServo.write(0);
  armServo.write(17);

  updateLightsAndSounds();

  lcd1602SetCursor(0, 0);
  lcd1602WriteString("                ");
  lcd1602SetCursor(0, 1);
  lcd1602WriteString("                ");
  updateLCD();
}

void winGame() {
  Serial.println("YOU WIN");

  gameState = WIN;

  noTone(buzzerPin);
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);

  lcd1602SetCursor(0, 0);
  lcd1602WriteString("YOU WIN!        ");
  lcd1602SetCursor(0, 1);
  lcd1602WriteString("Press button    ");

  delay(50);
}

void eliminatePlayer() {
  Serial.println("ELIMINATED");

  gameState = ELIMINATED;

  noTone(buzzerPin);
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);

  lcd1602SetCursor(0, 0);
  lcd1602WriteString("ELIMINATED!     ");
  lcd1602SetCursor(0, 1);
  lcd1602WriteString("Press button    ");

  armServo.write(90);
  armMoveTime = millis();
  armMoving = true;

  delay(50);
}

void resetGame() {
  Serial.println("Reset");

  gameState = IDLE;

  noTone(buzzerPin);
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);

  dollServo.write(0);
  armServo.write(17);

  lcd1602SetCursor(0, 0);
  lcd1602WriteString("Press button    ");
  lcd1602SetCursor(0, 1);
  lcd1602WriteString("to start game   ");
}

void checkMotionDuringRed(float dist) {
  if (fabs(dist - lastRedDistance) > 6.0) {
    Serial.println("Movement detected!");
    eliminatePlayer();
  }
}

void checkLandmines() {
  int l1 = analogRead(landmine1Pin);
  int l2 = analogRead(landmine2Pin);

  if (abs(l1 - landmine1Base) > 400 || abs(l2 - landmine2Base) > 400) {
    Serial.println("Landmine triggered!");
    eliminatePlayer();
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(playerPin, INPUT_PULLUP);

  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  dollServo.attach(dollServoPin);
  armServo.attach(armServoPin);

  lcd1602Init(0x27);
  lcd1602Control(true, false, false);

  resetGame();
}

void loop() {
  bool currentButtonState = digitalRead(buttonPin);

  if (currentButtonState == HIGH) {
    buttonReleased = true;
  }

  if (buttonReleased && lastButtonState == HIGH && currentButtonState == LOW &&
      millis() - lastButtonPress > 200) {

    buttonReleased = false;
    lastButtonPress = millis();

    if (gameState == IDLE) {
      startGame();
    } else if (gameState == WIN || gameState == ELIMINATED) {
      resetGame();
    }
  }

  // Player Mover™
  if (gameState == PLAYING)
    digitalWrite(relayPin, !digitalRead(playerPin));
  else
    digitalWrite(relayPin, LOW);

  // Arm reset
  if (armMoving && millis() - armMoveTime > 800) {
    armServo.write(10);
    armMoving = false;
  }

  // If we aren't play'in, restart the loop
  if (gameState != PLAYING)
    return;

  float dist = getDistanceSmooth();
  Serial.println("Distance: " + String(dist));

  // Joe light switchington
  if (millis() - lastLightChange >= currentDuration) {
    isGreenLight = !isGreenLight;
    currentDuration = isGreenLight ? greenDuration : redDuration;
    lastLightChange = millis();

    updateLightsAndSounds();

    if (!isGreenLight) {
      lastRedDistance = dist;
      Serial.println("Red baseline: " + String(lastRedDistance));
    }
  }

  // 2 lazy 4 metro
  if (millis() - lastSecondTick >= 1000) {
    timeLeft--;
    lastSecondTick = millis();

    updateLCD();

    if (timeLeft <= 0) {
      eliminatePlayer();
      return;
    }
  }

  // Check logic
  if (!isGreenLight) {
    checkMotionDuringRed(dist);
    if (gameState != PLAYING)
      return;
  }

  checkLandmines();
  if (gameState != PLAYING)
    return;

  if (dist <= 5.0) {
    winGame();
    return;
  }
}
