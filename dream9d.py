import asyncio
import aiohttp
import random
import logging
from collections import deque
import RPi.GPIO as GPIO
import time
import board
import busio
from adafruit_vl53l4cd import VL53L4CD
import numpy as np
import json
import heapq

# Logging setup
logging.basicConfig(level=logging.INFO)

# GPIO setup
GPIO.setmode(GPIO.BCM)
LEFT_EAR_PIN = 12
RIGHT_EAR_PIN = 13

GPIO.setup(LEFT_EAR_PIN, GPIO.OUT)
GPIO.setup(RIGHT_EAR_PIN, GPIO.OUT)

left_ear = GPIO.PWM(LEFT_EAR_PIN, 50)  # 50Hz frequency
right_ear = GPIO.PWM(RIGHT_EAR_PIN, 50)  # 50Hz frequency

left_ear.start(0)
right_ear.start(0)

# ToF sensor setup
i2c = busio.I2C(board.SCL, board.SDA)
tof_sensor = VL53L4CD(i2c)
tof_sensor.distance_mode = 2  # Long range mode
tof_sensor.start_ranging()

# Define the bot's environment and state
environment_map = {}
current_position = (0, 0)
path_stack = deque()

# Q-learning parameters
alpha = 0.1  # Learning rate
gamma = 0.9  # Discount factor
epsilon = 0.1  # Exploration rate

q_table = {}

# BB's body dimensions
FOOTPRINT_LENGTH = 14  # inches
FOOTPRINT_WIDTH = 14   # inches
SAFE_CUSHION = 2       # Extra cushion around BB

# ESP32 IP address
ESP32_IP_ADDRESS = '192.168.4.1'

# Functions to control the ESP32
async def send_command(session, command):
    url = f'http://{ESP32_IP_ADDRESS}{command}'
    async with session.get(url) as response:
        if response.status == 200:
            logging.info(f"Command {command} executed successfully.")
        else:
            logging.error(f"Failed to execute command {command} with status {response.status}.")

async def move_forward(session):
    await send_command(session, '/forward')

async def move_backward(session):
    await send_command(session, '/backward')

async def spin_left(session):
    await send_command(session, '/left')

async def spin_right(session):
    await send_command(session, '/right')

async def stop_motors(session):
    await send_command(session, '/stop')

# Q-learning functions
def get_state():
    front_distance = tof_sensor.distance
    rear_distance = 100  # Replace with actual rear sensor reading if available
    return (front_distance, rear_distance)

def choose_action(state):
    if random.uniform(0, 1) < epsilon:
        return random.choice(['forward', 'backward', 'left', 'right'])
    else:
        return max(q_table.get(state, {}), key=q_table.get(state, {}).get, default='forward')

def update_q_table(state, action, reward, next_state):
    old_value = q_table.get(state, {}).get(action, 0)
    future_value = max(q_table.get(next_state, {}).values(), default=0)
    new_value = old_value + alpha * (reward + gamma * future_value - old_value)
    if state not in q_table:
        q_table[state] = {}
    q_table[state][action] = new_value

async def explore_environment():
    async with aiohttp.ClientSession() as session:
        while True:
            state = get_state()
            action = choose_action(state)

            if action == 'forward':
                await move_forward(session)
            elif action == 'backward':
                await move_backward(session)
            elif action == 'left':
                await spin_left(session)
            elif action == 'right':
                await spin_right(session)

            await asyncio.sleep(1)
            next_state = get_state()
            reward = compute_reward(next_state)
            update_q_table(state, action, reward, next_state)
            await stop_motors(session)
            move_ears()

def compute_reward(state):
    front_distance, rear_distance = state
    if front_distance < 20:
        return -100  # High penalty for being too close to an obstacle
    elif front_distance > 100:
        return 100  # High reward for having open space
    else:
        return 0  # Neutral reward

# A* Pathfinding
def a_star_search(start, goal):
    open_set = []
    heapq.heappush(open_set, (0, start))
    came_from = {}
    g_score = {start: 0}
    f_score = {start: heuristic(start, goal)}

    while open_set:
        _, current = heapq.heappop(open_set)

        if current == goal:
            return reconstruct_path(came_from, current)

        for neighbor in get_neighbors(current):
            tentative_g_score = g_score[current] + 1  # Assuming uniform cost
            if tentative_g_score < g_score.get(neighbor, float('inf')):
                came_from[neighbor] = current
                g_score[neighbor] = tentative_g_score
                f_score[neighbor] = g_score[neighbor] + heuristic(neighbor, goal)
                if neighbor not in open_set:
                    heapq.heappush(open_set, (f_score[neighbor], neighbor))

    return []

def heuristic(a, b):
    return abs(a[0] - b[0]) + abs(a[1] - b[1])

def reconstruct_path(came_from, current):
    total_path = [current]
    while current in came_from:
        current = came_from[current]
        total_path.append(current)
    return total_path[::-1]

def get_neighbors(position):
    x, y = position
    return [(x+1, y), (x-1, y), (x, y+1), (x, y-1)]

async def navigate_to_goal(goal):
    async with aiohttp.ClientSession() as session:
        path = a_star_search(current_position, goal)
        for step in path:
            await move_to(session, step)

async def move_to(session, step):
    global current_position
    x, y = current_position
    target_x, target_y = step

    if target_x > x:
        await move_forward(session)
    elif target_x < x:
        await move_backward(session)
    elif target_y > y:
        await spin_right(session)
    elif target_y < y:
        await spin_left(session)

    current_position = step
    await asyncio.sleep(1)  # Adjust sleep time as needed

# Move the ears to add personality
def move_ears():
    left_angle = random.randint(0, 180)
    right_angle = random.randint(0, 180)
    left_ear.ChangeDutyCycle(angle_to_duty_cycle(left_angle))
    right_ear.ChangeDutyCycle(angle_to_duty_cycle(right_angle))

def angle_to_duty_cycle(angle):
    return 2.5 + (angle / 18.0) * 2.5

# Main loop
async def main():
    # Start with ears up
    left_ear.ChangeDutyCycle(angle_to_duty_cycle(90))
    right_ear.ChangeDutyCycle(angle_to_duty_cycle(90))
    await asyncio.sleep(1)  # Wait for ears to move up

    await explore_environment()

if __name__ == '__main__':
    try:
        asyncio.run(main())
    finally:
        left_ear.stop()
        right_ear.stop()
        GPIO.cleanup()
