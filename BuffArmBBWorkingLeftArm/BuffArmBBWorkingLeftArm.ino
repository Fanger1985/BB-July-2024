#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WebServer.h>

// Initialize the PCA9685
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);

// WiFi credentials
const char* ssid = "SpectrumSetup-DD";
const char* password = "jeansrocket543";

// Web server running on port 80
WebServer server(80);

#define SERVOMIN  150 // Minimum pulse length out of 4096
#define SERVOMAX  600 // Maximum pulse length out of 4096

bool autoMode = false;

// Current angles of the servos
uint16_t servoAnglesLeft[5] = {90, 90, 90, 90, 90}; // Initial angles for left shoulder, wrist, and claw servos
uint16_t servoAnglesRight[5] = {90, 90, 90, 90, 90}; // Initial angles for right shoulder, wrist, and claw servos

void setup() {
    Serial.begin(9600);
    Serial.println("Starting...");

    // Initialize PCA9685
    pca.begin();
    pca.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Define web server routes
    server.on("/", handleRoot);
    server.on("/moveServo", handleMoveServo);
    server.on("/getAngles", handleGetAngles);
    server.on("/calibrate", handleCalibrate);
    server.on("/demo", handleDemo);

    // Start the server
    server.begin();
    Serial.println("Server started");
}

// Function to set servo angle
void setServoAngle(uint8_t channel, uint16_t angle) {
    uint16_t pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
    pca.setPWM(channel, 0, pulse);

    // Update the current angle for the servo
    if (channel == 0) {
        servoAnglesLeft[0] = angle;
    } else if (channel == 1) {
        servoAnglesLeft[1] = angle;
    } else if (channel == 2) {
        servoAnglesLeft[2] = angle;
    } else if (channel == 3) {
        servoAnglesLeft[3] = angle;
    } else if (channel == 15) {
        servoAnglesLeft[4] = angle;
    } else if (channel == 4) {
        servoAnglesRight[0] = angle;
    } else if (channel == 5) {
        servoAnglesRight[1] = angle;
    } else if (channel == 6) {
        servoAnglesRight[2] = angle;
    } else if (channel == 7) {
        servoAnglesRight[3] = angle;
    } else if (channel == 14) {
        servoAnglesRight[4] = angle;
    }
}

// Function for smooth movement
void moveServoSmooth(uint8_t channel, uint16_t startAngle, uint16_t endAngle, uint16_t steps, uint16_t delayTime) {
    float stepSize = (endAngle - startAngle) / (float)steps;
    for (uint16_t i = 0; i <= steps; i++) {
        setServoAngle(channel, startAngle + stepSize * i);
        delay(delayTime);
    }
}

// Handle root URL "/"
void handleRoot() {
    String html = "<html>\
    <head>\
    <title>Servo Control</title>\
    <meta name='viewport' content='width=device-width, initial-scale=1'>\
    <style>\
    body { font-family: Arial, sans-serif; text-align: center; }\
    .button { padding: 10px 20px; margin: 5px; font-size: 20px; }\
    .slider { width: 90%; }\
    .panel { display: inline-block; width: 45%; vertical-align: top; }\
    .left-panel { float: left; }\
    .right-panel { float: right; }\
    </style>\
    </head>\
    <body>\
    <h1>Control Servos</h1>\
    <button class='button' onclick=\"sendRequest('/calibrate')\">Calibrate</button>\
    <button class='button' onclick=\"sendRequest('/demo')\">Demo</button>\
    <div class='panel left-panel'>\
    <h2>Left Arm</h2>\
    <h3>Shoulder Servo A</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo0' onchange=\"updateServo(0, this.value)\">\
    <span id='angle0'>90</span>\
    <h3>Shoulder Servo B</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo1' onchange=\"updateServo(1, this.value)\">\
    <span id='angle1'>90</span>\
    <h3>Wrist Servo A</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo2' onchange=\"updateServo(2, this.value)\">\
    <span id='angle2'>90</span>\
    <h3>Wrist Servo B</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo3' onchange=\"updateServo(3, this.value)\">\
    <span id='angle3'>90</span>\
    <h3>Claw</h3>\
    <input type='range' min='55' max='176' value='90' class='slider' id='servo15' onchange=\"updateServo(15, this.value)\">\
    <span id='angle15'>90</span>\
    </div>\
    <div class='panel right-panel'>\
    <h2>Right Arm</h2>\
    <h3>Shoulder Servo A</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo4' onchange=\"updateServo(4, this.value)\">\
    <span id='angle4'>90</span>\
    <h3>Shoulder Servo B</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo5' onchange=\"updateServo(5, this.value)\">\
    <span id='angle5'>90</span>\
    <h3>Wrist Servo A</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo6' onchange=\"updateServo(6, this.value)\">\
    <span id='angle6'>90</span>\
    <h3>Wrist Servo B</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo7' onchange=\"updateServo(7, this.value)\">\
    <span id='angle7'>90</span>\
    <h3>Claw</h3>\
    <input type='range' min='55' max='176' value='90' class='slider' id='servo14' onchange=\"updateServo(14, this.value)\">\
    <span id='angle14'>90</span>\
    </div>\
    <script>\
    function sendRequest(endpoint) {\
        fetch(endpoint).then(response => console.log(response));\
    }\
    function updateServo(channel, angle) {\
        fetch(`/moveServo?channel=${channel}&angle=${angle}`).then(response => {\
            if (response.ok) {\
                document.getElementById(`angle${channel}`).innerText = angle;\
            }\
        });\
    }\
    function updateSliders(anglesLeft, anglesRight) {\
        document.getElementById('servo0').value = anglesLeft[0];\
        document.getElementById('angle0').innerText = anglesLeft[0];\
        document.getElementById('servo1').value = anglesLeft[1];\
        document.getElementById('angle1').innerText = anglesLeft[1];\
        document.getElementById('servo2').value = anglesLeft[2];\
        document.getElementById('angle2').innerText = anglesLeft[2];\
        document.getElementById('servo3').value = anglesLeft[3];\
        document.getElementById('angle3').innerText = anglesLeft[3];\
        document.getElementById('servo15').value = anglesLeft[4];\
        document.getElementById('angle15').innerText = anglesLeft[4];\
        document.getElementById('servo4').value = anglesRight[0];\
        document.getElementById('angle4').innerText = anglesRight[0];\
        document.getElementById('servo5').value = anglesRight[1];\
        document.getElementById('angle5').innerText = anglesRight[1];\
        document.getElementById('servo6').value = anglesRight[2];\
        document.getElementById('angle6').innerText = anglesRight[2];\
        document.getElementById('servo7').value = anglesRight[3];\
        document.getElementById('angle7').innerText = anglesRight[3];\
        document.getElementById('servo14').value = anglesRight[4];\
        document.getElementById('angle14').innerText = anglesRight[4];\
    }\
    fetch('/getAngles').then(response => response.json()).then(data => updateSliders(data.left, data.right));\
    </script>\
    </body>\
    </html>";

    server.send(200, "text/html", html);
}

// Handle URL "/moveServo"
void handleMoveServo() {
    if (server.hasArg("channel") && server.hasArg("angle")) {
        uint8_t channel = server.arg("channel").toInt();
        uint16_t angle = server.arg("angle").toInt();
        setServoAngle(channel, angle);
        server.send(200, "text/plain", "Moved servo");
    } else {
        server.send(400, "text/plain", "Invalid request");
    }
}

// Handle URL "/getAngles"
void handleGetAngles() {
    String json = "{\"left\":[";
    json += String(servoAnglesLeft[0]) + ",";
    json += String(servoAnglesLeft[1]) + ",";
    json += String(servoAnglesLeft[2]) + ",";
    json += String(servoAnglesLeft[3]) + ",";
    json += String(servoAnglesLeft[4]);
    json += "], \"right\":[";
    json += String(servoAnglesRight[0]) + ",";
    json += String(servoAnglesRight[1]) + ",";
    json += String(servoAnglesRight[2]) + ",";
    json += String(servoAnglesRight[3]) + ",";
    json += String(servoAnglesRight[4]);
    json += "]}";
    server.send(200, "application/json", json);
}

// Handle URL "/calibrate"
void handleCalibrate() {
    setServoAngle(0, 90);
    setServoAngle(1, 90);
    setServoAngle(2, 90);
    setServoAngle(3, 90);
    setServoAngle(15, 90);
    setServoAngle(4, 90);
    setServoAngle(5, 90);
    setServoAngle(6, 90);
    setServoAngle(7, 90);
    setServoAngle(14, 90);
    server.send(200, "text/plain", "Servos calibrated to 90 degrees");
}

// Handle URL "/demo"
void handleDemo() {
    for (int i = 0; i < 3; i++) { // Repeat the demo 3 times
        moveServoSmooth(0, 40, 125, 30, 20);   // Left shoulder servo A
        moveServoSmooth(0, 125, 40, 30, 20);   // Left shoulder servo A
        moveServoSmooth(1, 60, 125, 30, 20);   // Left shoulder servo B
        moveServoSmooth(1, 125, 60, 30, 20);   // Left shoulder servo B
        moveServoSmooth(2, 18, 160, 30, 20);   // Left wrist servo A
        moveServoSmooth(2, 160, 18, 30, 20);   // Left wrist servo A
        moveServoSmooth(3, 110, 175, 30, 20);  // Left wrist servo B
        moveServoSmooth(3, 175, 110, 30, 20);  // Left wrist servo B
        moveServoSmooth(15, 110, 176, 30, 20); // Left claw
        moveServoSmooth(15, 176, 110, 30, 20); // Left claw

        moveServoSmooth(4, 40, 125, 30, 20);   // Right shoulder servo A
        moveServoSmooth(4, 125, 40, 30, 20);   // Right shoulder servo A
        moveServoSmooth(5, 60, 125, 30, 20);   // Right shoulder servo B
        moveServoSmooth(5, 125, 60, 30, 20);   // Right shoulder servo B
        moveServoSmooth(6, 18, 160, 30, 20);   // Right wrist servo A
        moveServoSmooth(6, 160, 18, 30, 20);   // Right wrist servo A
        moveServoSmooth(7, 110, 175, 30, 20);  // Right wrist servo B
        moveServoSmooth(7, 175, 110, 30, 20);  // Right wrist servo B
        moveServoSmooth(14, 110, 176, 30, 20); // Right claw
        moveServoSmooth(14, 176, 110, 30, 20); // Right claw
    }
    server.send(200, "text/plain", "Demo completed");
}

void loop() {
    // Handle client requests
    server.handleClient();
}
