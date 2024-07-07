import RPi.GPIO as GPIO
import time

# GPIO pin setup
LEFT_EAR_PIN = 12
RIGHT_EAR_PIN = 13

GPIO.setmode(GPIO.BCM)
GPIO.setup(LEFT_EAR_PIN, GPIO.OUT)
GPIO.setup(RIGHT_EAR_PIN, GPIO.OUT)

# PWM setup
left_ear_pwm = GPIO.PWM(LEFT_EAR_PIN, 50)  # 50Hz frequency
right_ear_pwm = GPIO.PWM(RIGHT_EAR_PIN, 50)  # 50Hz frequency

def wiggle_ears():
    left_ear_pwm.start(7.5)  # Neutral position
    right_ear_pwm.start(7.5)  # Neutral position
    time.sleep(0.5)

    left_ear_pwm.ChangeDutyCycle(5)  # Move to 0 degrees
    right_ear_pwm.ChangeDutyCycle(10)  # Move to 180 degrees
    time.sleep(0.5)

    left_ear_pwm.ChangeDutyCycle(10)  # Move to 180 degrees
    right_ear_pwm.ChangeDutyCycle(5)  # Move to 0 degrees
    time.sleep(0.5)

    left_ear_pwm.ChangeDutyCycle(7.5)  # Back to neutral
    right_ear_pwm.ChangeDutyCycle(7.5)  # Back to neutral
    time.sleep(0.5)

    left_ear_pwm.stop()
    right_ear_pwm.stop()
    GPIO.cleanup()

if __name__ == "__main__":
    wiggle_ears()
