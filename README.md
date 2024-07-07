BB1 Robot Project

BB1 is an advanced, autonomous robot designed for navigation and interaction within its environment. This README provides a detailed overview of BB1's hardware configuration and functions based on its current setup.

Hardware Configuration
Microcontrollers and Main Components
ESP32: Handles motor control, sensor reading, and basic navigation logic. It runs a web server to receive commands for manual and automatic control.
Raspberry Pi 4: Acts as the main brain, running complex algorithms, including Q-learning for pathfinding and decision-making. It communicates with the ESP32 for motor control and receives sensor data.
Sensors and Actuators
Ultrasonic Sensors:

Front Sensor: Trig pin on GPIO 2, Echo pin on GPIO 17.
Rear Sensor: Trig pin on GPIO 16, Echo pin on GPIO 13.
Used for obstacle detection and distance measurement.
PIR Sensor:

Pin: GPIO 12.
Used to detect human presence.
ToF Sensor (VL53L4CD):

Connected via I2C (SCL and SDA pins).
Used for precise distance measurement.
MPU6050 Gyroscope/Accelerometer:

Connected via I2C.
Used for motion detection and orientation.
Motors and Motor Control
Left Motor:
IN1 on GPIO 19.
IN2 on GPIO 15.
Right Motor:
IN1 on GPIO 5.
IN2 on GPIO 18.
Motor control: Utilizes H-bridge configuration for forward, backward, left, and right movements.
Additional Components
Ears:
Left Ear: Controlled via PWM on GPIO 12.
Right Ear: Controlled via PWM on GPIO 13.
Adds personality by moving randomly.
Functions
Movement and Navigation
Manual Control: Commands received via HTTP requests to move forward, backward, spin left, spin right, or stop.
Automatic Control: Autonomous navigation using Q-learning and A* pathfinding algorithms.
Exploration Mode: BB1 autonomously explores its environment, avoiding obstacles and updating its environment map.
Obstacle Avoidance
Ultrasonic Sensors: Measure distances to obstacles and trigger avoidance maneuvers.
PIR Sensor: Detects human presence and adjusts behavior accordingly.
ToF Sensor: Provides precise distance measurements for navigation and obstacle avoidance.
Pathfinding and Q-learning
Q-learning: Utilizes reinforcement learning to make decisions based on rewards and penalties.
A Pathfinding*: Calculates the shortest path to a given goal using a heuristic approach.
Interaction and Personality
Ears Movement: Moves ears randomly to give BB1 a personality.
Emotion Expression: BB1 can express basic emotions through movements and sounds.
Sensor Data and Logging
Distance Measurement: Continuously measures distances using ultrasonic and ToF sensors.
Gyroscope/Accelerometer Data: Logs motion and orientation data.
HTTP Server: Provides a web interface to control BB1 and view sensor data.
Future Plans
BB1's development roadmap includes several exciting enhancements:

Advanced Perception and Interaction
Computer Vision: Integration of OpenCV for object detection and face recognition, using models like MobileNetSSD and dlib for facial landmarks.
Emotion Detection: Implementing facial expression analysis to detect and respond to human emotions.
Enhanced Navigation and Mapping
SLAM (Simultaneous Localization and Mapping): Using rtabmap for real-time 3D mapping and localization.
Advanced Path Planning: Enhancing the pathfinding algorithms with A* and potential field methods for more efficient navigation.
Improved Sensor Integration
Additional Sensors: Integration of APDS9960 for color, proximity, and gesture detection.
Multi-Sensor Fusion: Combining data from multiple sensors (ToF, ultrasonic, gyroscope) for more accurate environment perception.
Machine Learning and AI
Deep Learning Models: Incorporating TensorFlow Lite for on-device machine learning, enabling more complex decision-making and pattern recognition.
Q-learning with Experience Replay: Implementing experience replay to improve the efficiency of the reinforcement learning process.
Decision Tree Classifier: Training and deploying a decision tree classifier for better action selection based on sensor inputs and learned behaviors.
Voice Interaction
Speech Recognition and Synthesis: Utilizing libraries like SpeechRecognition and pyttsx3 for voice command recognition and response.
Emotion-Based Speech: Adjusting the robot's responses based on detected emotions and environmental context.
Real-Time Monitoring and Control
Websockets and Real-Time Data: Implementing websockets for real-time sensor data streaming and control.
Web Interface Enhancements: Adding more interactive controls and visualizations to the web interface.
Personality and Interaction
Dynamic Ear Movements: Enhancing the ear movements to reflect the robot's mood and actions dynamically.
Interactive Display: Using a display to show messages and expressions based on the robot's mood and interactions.
Energy Management
Battery Monitoring: Implementing battery level monitoring and alerts for low battery conditions.
These enhancements aim to make BB1 more intelligent, interactive, and capable of operating autonomously in dynamic environments.

Setup and Usage
Prerequisites
Hardware: ESP32, Raspberry Pi 4, Ultrasonic Sensors, PIR Sensor, ToF Sensor, MPU6050, Motors, PWM-controlled servos for ears.
Software: Arduino IDE for ESP32, Python 3.x for Raspberry Pi, necessary libraries (asyncio, aiohttp, adafruit-circuitpython-vl53l4cd, tensorflow, etc.).
Installation
ESP32 Setup:
Flash the provided Arduino sketch to the ESP32.
Raspberry Pi Setup:
Install required Python libraries: pip install asyncio aiohttp adafruit-circuitpython-vl53l4cd tensorflow.
Run the provided Python script on the Raspberry Pi.
Running the Robot
Power on the ESP32 and Raspberry Pi.
Access the web interface via the ESP32's IP address (default: http://192.168.4.1).
Use the web interface to control BB1 manually or switch to automatic mode.
Observe BB1 as it navigates, interacts, and explores its environment autonomously.
