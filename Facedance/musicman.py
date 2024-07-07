import RPi.GPIO as GPIO
import time
import threading

# GPIO setup
GPIO.setmode(GPIO.BCM)
left_ear_pin = 12
right_ear_pin = 13

GPIO.setup(left_ear_pin, GPIO.OUT)
GPIO.setup(right_ear_pin, GPIO.OUT)

# Initialize PWM
left_ear = GPIO.PWM(left_ear_pin, 50)  # 50Hz frequency
right_ear = GPIO.PWM(right_ear_pin, 50)  # 50Hz frequency

left_ear.start(0)
right_ear.start(0)

# Function to move ears
def move_ears(left_angle, right_angle):
    left_duty = (left_angle / 18) + 2
    right_duty = (right_angle / 18) + 2
    left_ear.ChangeDutyCycle(left_duty)
    right_ear.ChangeDutyCycle(right_duty)
    time.sleep(0.5)

# Function to make ears dance
def dance_ears():
    while True:
        move_ears(90, 90)  # Center position
        move_ears(45, 135)  # Move ears
        move_ears(135, 45)  # Move ears
        move_ears(90, 90)  # Back to center
        time.sleep(0.1)

# Start the dancing in a separate thread
threading.Thread(target=dance_ears).start()

# Clean up GPIO on exit
try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    pass
finally:
    left_ear.stop()
    right_ear.stop()
    GPIO.cleanup()
