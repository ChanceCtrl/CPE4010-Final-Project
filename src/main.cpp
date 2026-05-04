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
const int ldr1Pin = A0;
const int ldr2Pin = A1;

unsigned long lastLightChange = 0;
unsigned long lastSecondTick = 0;
bool isGreenLight = true;
const int greenDuration = 2000; // 2 sec
const int redDuration = 3000;   // 3 sec
int currentDuration = greenDuration;

int timeLeft = 60;
float lastRedDistance = 0.0;
int ldr1Base = 0;
int ldr2Base = 0;

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

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(playerPin, INPUT_PULLUP);
  pinMode(manualPin, INPUT_PULLUP);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(trigPin, LOW);
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(relayPin, LOW);

  dollServo.attach(dollServoPin);
  armServo.attach(armServoPin);

  lcd1602Init(0x27);
  lcd1602Control(true, false, false);

  digitalWrite(relayPin, HIGH);
  digitalWrite(relayPin, LOW);
}

void loop() {
  lcd1602SetCursor(0, 0);
  lcd1602WriteString(":3");

  digitalWrite(redLedPin, HIGH);
  digitalWrite(greenLedPin, HIGH);
  digitalWrite(buzzerPin, HIGH);

  delay(500);

  // for (int i = 0; i < 180; i++) {
  //   armServo.write(i);
  //   delay(50);
  // }
  //
  // for (int i = 0; i < 180; i++) {
  //   dollServo.write(i);
  //   delay(50);
  // }

  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(buzzerPin, LOW);

  Serial.println(analogRead(A0));
  Serial.println(analogRead(A1));
  Serial.println("mrow");
  Serial.println(getDistance());
  Serial.println(digitalRead(buttonPin));
  Serial.println(digitalRead(playerPin));
  Serial.println(digitalRead(manualPin));
}
