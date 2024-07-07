import asyncio
import websockets
import json
import RPi.GPIO as GPIO
import time

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

# Function to move ears based on volume
def move_ears(volume):
    angle = min(max((volume - 128) * 1.5, 0), 180)  # Scale and clamp volume to angle
    duty = (angle / 18) + 2
    left_ear.ChangeDutyCycle(duty)
    right_ear.ChangeDutyCycle(duty)

# WebSocket server handler
async def handler(websocket, path):
    async for message in websocket:
        data = json.loads(message)
        volume = data.get('volume', 128)
        move_ears(volume)

# Start WebSocket server
start_server = websockets.serve(handler, "localhost", 8765)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()

# Clean up GPIO on exit
left_ear.stop()
right_ear.stop()
GPIO.cleanup()
