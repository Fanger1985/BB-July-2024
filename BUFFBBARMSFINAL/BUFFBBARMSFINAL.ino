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
    <button class='button' onclick=\"sendRequest('/demo2')\">Demo 2</button>\
    <button class='button' onclick=\"sendRequest('/printAngles')\">Print Angles</button>\
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

// Handle URL "/printAngles"
void handlePrintAngles() {
    printCurrentAngles();
    server.send(200, "text/plain", "Printed current angles to serial monitor");
}

// Function to print current angles to the serial monitor
void printCurrentAngles() {
    Serial.println("Current servo angles:");
    for (uint8_t i = 0; i < 5; i++) {
        Serial.print("Left Servo ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(servoAnglesLeft[i]);
    }
    for (uint8_t i = 0; i < 5; i++) {
        Serial.print("Right Servo ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(servoAnglesRight[i]);
    }
}

// Handle URL "/demo"
void handleDemo() {
    const int steps = 30;
    const int delayTime = 20;
    
    // First pose for both arms (based on the calibrate position)
    uint16_t leftAngles[5] = {34, 105, 96, 125, 79};
    uint16_t rightAngles[5] = {151, 178, 67, 125, 136};
    for (int i = 0; i < 5; i++) {
        moveServoSmooth(i, servoAnglesLeft[i], leftAngles[i], steps, delayTime);
        moveServoSmooth(i + 4, servoAnglesRight[i], rightAngles[i], steps, delayTime);
    }

    delay(1000); // Small delay before next movement

    // Open the left claw to avoid knocking the egg
    moveServoSmooth(15, servoAnglesLeft[4], 79, steps, delayTime);

    delay(500); // Small delay before next movement

    // Move left arm to position over the egg
    moveServoSmooth(0, servoAnglesLeft[0], 81, steps, delayTime);
    moveServoSmooth(1, servoAnglesLeft[1], 43, steps, delayTime);
    moveServoSmooth(2, servoAnglesLeft[2], 169, steps, delayTime);
    moveServoSmooth(3, servoAnglesLeft[3], 97, steps, delayTime);

    delay(500); // Small delay before closing the claw

    // Close the claw on the egg
    moveServoSmooth(15, 79, 176, steps, delayTime);

    delay(500); // Small delay before lifting the egg

    // Move to the final position
    moveServoSmooth(0, 81, 56, steps, delayTime);
    moveServoSmooth(1, 43, 57, steps, delayTime);
    moveServoSmooth(2, 169, 12, steps, delayTime);
    moveServoSmooth(3, 97, 112, steps, delayTime);

    moveServoSmooth(4, 151, 154, steps, delayTime);
    moveServoSmooth(5, 178, 114, steps, delayTime);
    moveServoSmooth(6, 67, 82, steps, delayTime);
    moveServoSmooth(7, 125, 143, steps, delayTime);
    moveServoSmooth(14, 136, 84, steps, delayTime);

    server.send(200, "text/plain", "Demo completed");
}

// Handle URL "/demo2"
void handleDemo2() {
    const int steps = 30;
    const int delayTime = 20;
    
    // Initial pose based on the screenshot
    uint16_t leftAngles[5] = {34, 105, 96, 125, 79};
    uint16_t rightAngles[5] = {151, 178, 67, 125, 136};
    for (int i = 0; i < 5; i++) {
        moveServoSmooth(i, servoAnglesLeft[i], leftAngles[i], steps, delayTime);
        moveServoSmooth(i + 4, servoAnglesRight[i], rightAngles[i], steps, delayTime);
    }

    delay(1000); // Small delay before next movement

    // Open the left claw to avoid knocking the egg
    moveServoSmooth(15, servoAnglesLeft[4], 79, steps, delayTime);

    delay(500); // Small delay before next movement

    // Move left arm to position over the egg
    moveServoSmooth(0, servoAnglesLeft[0], 81, steps, delayTime);
    moveServoSmooth(1, servoAnglesLeft[1], 43, steps, delayTime);
    moveServoSmooth(2, servoAnglesLeft[2], 169, steps, delayTime);
    moveServoSmooth(3, servoAnglesLeft[3], 97, steps, delayTime);

    delay(500); // Small delay before closing the claw

    // Close the claw on the egg
    moveServoSmooth(15, 79, 176, steps, delayTime);

    delay(500); // Small delay before lifting the egg

    // Move to the final position
    moveServoSmooth(0, 81, 56, steps, delayTime);
    moveServoSmooth(1, 43, 57, steps, delayTime);
    moveServoSmooth(2, 169, 12, steps, delayTime);
    moveServoSmooth(3, 97, 112, steps, delayTime);

    moveServoSmooth(4, 151, 154, steps, delayTime);
    moveServoSmooth(5, 178, 114, steps, delayTime);
    moveServoSmooth(6, 67, 82, steps, delayTime);
    moveServoSmooth(7, 125, 143, steps, delayTime);
    moveServoSmooth(14, 136, 84, steps, delayTime);

    server.send(200, "text/plain", "Demo 2 completed");
}

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
    server.on("/demo2", handleDemo2);
    server.on("/printAngles", handlePrintAngles);

    // Start the server
    server.begin();
    Serial.println("Server started");
}

void loop() {
    // Handle client requests
    server.handleClient();
}
