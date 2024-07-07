from flask import Flask, request, jsonify
import os
import subprocess

app = Flask(__name__)

@app.route('/control', methods=['POST'])
def control():
    data = request.json
    command = data.get('command')

    if command:
        print(f'Received command: {command}')
        # Execute the command by calling the move_robot function
        move_robot(command)

    return jsonify({'status': 'success', 'command': command})

@app.route('/wiggle_ears', methods=['POST'])
def wiggle_ears():
    print('Wiggling ears...')
    subprocess.run(['python3', 'wiggle_ears.py'])
    return jsonify({'status': 'success', 'action': 'wiggle_ears'})

def move_robot(command):
    robot_ip = "<ROBOT_IP_ADDRESS>"  # Replace with your robot's IP address
    if command == '/forward':
        os.system(f"curl -X GET http://{robot_ip}/forward")
    elif command == '/backward':
        os.system(f"curl -X GET http://{robot_ip}/backward")
    elif command == '/left':
        os.system(f"curl -X GET http://{robot_ip}/left")
    elif command == '/right':
        os.system(f"curl -X GET http://{robot_ip}/right")
    elif command == '/stop':
        os.system(f"curl -X GET http://{robot_ip}/stop")
    elif command == '/spin_left':
        os.system(f"curl -X GET http://{robot_ip}/spin_left")
    elif command == '/spin_right':
        os.system(f"curl -X GET http://{robot_ip}/spin_right")
    elif command == '/dance':
        os.system(f"curl -X GET http://{robot_ip}/dance")
    elif command == '/return_home':
        os.system(f"curl -X GET http://{robot_ip}/return_home")

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
