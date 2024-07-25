#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Initialize the PCA9685
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);

#define SERVOMIN  150 // Minimum pulse length out of 4096
#define SERVOMAX  600 // Maximum pulse length out of 4096

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");

  // Initialize PCA9685
  pca.begin();
  pca.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
}

// Function to set servo angle
void setServoAngle(uint8_t channel, uint16_t angle) {
  uint16_t pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  pca.setPWM(channel, 0, pulse);
}

void loop() {
  // Shoulder Servo 1 (Forward and Backward)
  setServoAngle(15, 110);  // Forward
  delay(1000);
  setServoAngle(15, 10);   // Backward
  delay(1000);

  // Shoulder Servo 2 (Left and Right)
  setServoAngle(14, 160);   // Move Away from Body  bigger number away from body
  delay(2000);
  setServoAngle(14, 100);  // Move Towards Body (Adjusted outward to 90 degrees)
  delay(1000);
  setServoAngle(14, 160);   // Move Away from Body  bigger number away from body
  delay(1000);
  setServoAngle(14, 100);  // Move Towards Body (Adjusted outward to 90 degrees)
  delay(1000);
  // Wrist Servo 1 (Left and Right)
  setServoAngle(13, 0);   // Left
  delay(1000);
  setServoAngle(13, 170); // Right
  delay(1000);

  // Wrist Servo 2 (Jiggle) fingers
  setServoAngle(12, 0);
  delay(500);
  setServoAngle(12, 180);
  delay(500);
  setServoAngle(12, 90);
  delay(500);
    setServoAngle(15, 10);   // Backward
  delay(1000);
  setServoAngle(12, 0);
  delay(500);
  setServoAngle(12, 180);
  delay(500);
  setServoAngle(12, 90);
  delay(500);

//dont break it
  setServoAngle(15, 90);  // Forward
  delay(5000);   
  setServoAngle(15, 130);  // Forward
  delay(5000);  
  setServoAngle(14, 90);   // Move Away from Body (Adjusted outward to 20 degrees) bigger number away from body
  delay(4000);
    setServoAngle(13, 90); // Right
  delay(3000);
    setServoAngle(12, 0);
  delay(3000);
    setServoAngle(12, 180);
  delay(500);
}
