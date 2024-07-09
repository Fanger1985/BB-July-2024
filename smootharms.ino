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

// Function for linear interpolation for smooth movements
void moveServoSmooth(uint8_t channel, uint16_t targetAngle, uint16_t steps, uint16_t stepDelay) {
    uint16_t currentAngle = pca.getPWM(channel, 0); // Get current servo angle
    int16_t angleDiff = targetAngle - currentAngle;
    int16_t angleIncrement = angleDiff / steps;

    for (uint16_t i = 0; i < steps; i++) {
        currentAngle += angleIncrement;
        setServoAngle(channel, currentAngle);
        delay(stepDelay);
    }
    // Ensure the final position is accurate
    setServoAngle(channel, targetAngle); 
}

void loop() {
    // Shoulder Servo 1 (Forward and Backward)
    moveServoSmooth(15, 110, 40, 25);   // Forward (smoother, same timing)
    delay(500);                           // Add a short delay in the middle for fluidity
    moveServoSmooth(15, 40, 40, 25);    // Backward (smoother, same timing)
    delay(500);

    // Shoulder Servo 2 (Left and Right)
    moveServoSmooth(14, 160, 60, 16);   // Move Away  (smoother, same timing)
    moveServoSmooth(14, 100, 60, 16);   // Move Towards 
    moveServoSmooth(14, 165, 60, 16);   // Move Away  (smoother, same timing)
    moveServoSmooth(14, 100, 60, 16);   // Move Towards 

    // Wrist Servo 1 (Left and Right)
    moveServoSmooth(13, 0, 20, 50);    // Left (smoother, same timing)
    moveServoSmooth(13, 170, 20, 50);  // Right (smoother, same timing)

    // Wrist Servo 2 (Jiggle) fingers
    moveServoSmooth(12, 0, 10, 50);   // Jiggle 1
    moveServoSmooth(12, 180, 10, 50);  // Jiggle 2
    moveServoSmooth(12, 90, 10, 50);   // Center
    moveServoSmooth(15, 10, 20, 50);   // Move shoulder back during jiggle
    moveServoSmooth(12, 0, 10, 50);   // Jiggle 1
    moveServoSmooth(12, 180, 10, 50);  // Jiggle 2
    moveServoSmooth(12, 90, 10, 50);   // Center

    // Don't Break It Section (Smoothed)
    moveServoSmooth(15, 90, 40, 25);      // Forward  
    delay(5000);                            
    moveServoSmooth(15, 140, 40, 25);     // Forward (further) 
    delay(5000);                           
    moveServoSmooth(14, 140, 80, 12);      // Move away from body (wider range, slower)
    delay(4000);
    moveServoSmooth(13, 90, 60, 16);      // Wrist center
    delay(3000);
    moveServoSmooth(12, 0, 10, 50);       // Fingers open
    delay(3000); 
    moveServoSmooth(12, 180, 10, 50);     // Fingers close (fast)
    delay(500);                          
}
