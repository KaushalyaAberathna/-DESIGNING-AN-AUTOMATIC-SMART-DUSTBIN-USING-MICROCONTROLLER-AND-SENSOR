#include <Arduino.h>
#include <ESP32Servo.h>  // Ensure this library is installed in PlatformIO

#define TRIG_PIN 32
#define ECHO_PIN 33
#define IR_SENSOR_PIN 34
#define SERVO_PIN 18  // Pin connected to the servo motor

const int MAX_HEIGHT = 15;  // Maximum height of the dustbin in cm
long duration;
int distance;
int fillingPercentage;

Servo dustbinServo;  // Create a Servo object to control the dustbin lid

void setup() {
    Serial.begin(115200);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(IR_SENSOR_PIN, INPUT);
    dustbinServo.attach(SERVO_PIN);
}

int measureDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);
    return duration * 0.034 / 2;
}

void loop() {
    distance = measureDistance();

    if (distance >= 0 && distance <= MAX_HEIGHT) {
        fillingPercentage = 100 - ((distance * 100) / MAX_HEIGHT);
    } else {
        fillingPercentage = 0;
    }

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm, Filling Percentage: ");
    Serial.println(fillingPercentage);

    if (digitalRead(IR_SENSOR_PIN) == HIGH) {
        dustbinServo.write(90);
        Serial.println("Object detected, opening lid...");
    } else {
        dustbinServo.write(0);
        Serial.println("Lid closed.");
    }

    delay(500);
}
