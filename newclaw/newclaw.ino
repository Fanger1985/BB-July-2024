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

// Safe pulse lengths for servos
#define SERVOMIN  150  // Minimum pulse length out of 4096
#define SERVOMAX  600  // Maximum pulse length out of 4096

// Current angles of the servos
int16_t clawServoAngle = 60; // Initial angle for claw servo on pin 13
int16_t leftShoulderForwardBackAngle = 0; // Initial angle for left shoulder forward/back servo on pin 8 (corrected label)
int16_t leftShoulderSideSideAngle = 90; // Initial angle for left shoulder side/side servo on pin 9 (corrected label)
int16_t wristSideSideAngle = 90; // Initial angle for wrist side/side servo on pin 10
int16_t wristUpDownAngle = 90; // Initial angle for wrist up/down servo on pin 11

void setup() {
    Serial.begin(9600);
    Serial.println("Starting...");

    // Initialize PCA9685
    pca.begin();
    pca.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

    // Set the initial positions
    setServoAngle(13, clawServoAngle);
    setServoAngle(8, leftShoulderForwardBackAngle);
    setServoAngle(9, leftShoulderSideSideAngle);
    setServoAngle(10, wristSideSideAngle);
    setServoAngle(11, wristUpDownAngle);

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
    server.on("/zeroServos", handleZeroServos);  // New route for zeroing servos

    // Start the server
    server.begin();
    Serial.println("Server started");
}

// Function to set servo angle
void setServoAngle(uint8_t channel, int16_t angle) {
    uint16_t pulse;
    if (channel == 8) {  // Forward-back servo
        pulse = map(angle, -50, 70, SERVOMIN, SERVOMAX);
    } else if (channel == 9) {  // Side-to-side servo
        pulse = map(angle, -50, 50, SERVOMIN, SERVOMAX);
    } else if (channel == 10) {  // Wrist side-to-side servo
        pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
    } else if (channel == 11) {  // Wrist up/down servo
        pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
    } else {  // Normal mapping for claw servo
        pulse = map(angle, 0, 60, SERVOMIN, SERVOMAX);
    }
    pca.setPWM(channel, 0, pulse);
    Serial.print("Setting servo on channel ");
    Serial.print(channel);
    Serial.print(" to angle ");
    Serial.print(angle);
    Serial.print(" (pulse ");
    Serial.print(pulse);
    Serial.println(")");
    if (channel == 13) {
        clawServoAngle = angle;
    } else if (channel == 8) {
        leftShoulderForwardBackAngle = angle;
    } else if (channel == 9) {
        leftShoulderSideSideAngle = angle;
    } else if (channel == 10) {
        wristSideSideAngle = angle;
    } else if (channel == 11) {
        wristUpDownAngle = angle;
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
    </style>\
    </head>\
    <body>\
    <h1>Control Servos</h1>\
    <h2>Claw Servo</h2>\
    <input type='range' min='0' max='60' value='60' class='slider' id='servo13' onchange=\"updateServo(13, this.value)\">\
    <span id='angle13'>60</span>\
    <h2>Left Shoulder Forward/Back</h2>\
    <input type='range' min='-50' max='70' value='0' class='slider' id='servo8' onchange=\"updateServo(8, this.value)\">\
    <span id='angle8'>0</span>\
    <h2>Left Shoulder Side/Side</h2>\
    <input type='range' min='-50' max='50' value='90' class='slider' id='servo9' onchange=\"updateServo(9, this.value)\">\
    <span id='angle9'>90</span>\
    <h2>Wrist Side/Side</h2>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo10' onchange=\"updateServo(10, this.value)\">\
    <span id='angle10'>90</span>\
    <h2>Wrist Up/Down</h2>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo11' onchange=\"updateServo(11, this.value)\">\
    <span id='angle11'>90</span>\
    <button class='button' onclick=\"zeroServos()\">Calibrate to 90</button>\
    <script>\
    function updateServo(channel, angle) {\
        fetch(`/moveServo?channel=${channel}&angle=${angle}`).then(response => {\
            if (response.ok) {\
                document.getElementById(`angle${channel}`).innerText = angle;\
            }\
        });\
    }\
    function zeroServos() {\
        fetch('/zeroServos').then(response => {\
            if (response.ok) {\
                document.getElementById('servo13').value = 0;\
                document.getElementById('angle13').innerText = 0;\
                document.getElementById('servo8').value = 10;\
                document.getElementById('angle8').innerText = 10;\
                document.getElementById('servo9').value = 90;\
                document.getElementById('angle9').innerText = 90;\
                document.getElementById('servo10').value = 90;\
                document.getElementById('angle10').innerText = 90;\
                document.getElementById('servo11').value = 90;\
                document.getElementById('angle11').innerText = 90;\
            }\
        });\
    }\
    function updateSliders(angles) {\
        document.getElementById('servo13').value = angles.claw;\
        document.getElementById('angle13').innerText = angles.claw;\
        document.getElementById('servo8').value = angles.shoulderForwardBack;\
        document.getElementById('angle8').innerText = angles.shoulderForwardBack;\
        document.getElementById('servo9').value = angles.shoulderSideSide;\
        document.getElementById('angle9').innerText = angles.shoulderSideSide;\
        document.getElementById('servo10').value = angles.wristSideSide;\
        document.getElementById('angle10').innerText = angles.wristSideSide;\
        document.getElementById('servo11').value = angles.wristUpDown;\
        document.getElementById('angle11').innerText = angles.wristUpDown;\
    }\
    fetch('/getAngles').then(response => response.json()).then(data => updateSliders(data));\
    </script>\
    </body>\
    </html>";

    server.send(200, "text/html", html);
}

// Handle URL "/moveServo"
void handleMoveServo() {
    if (server.hasArg("channel") && server.hasArg("angle")) {
        uint8_t channel = server.arg("channel").toInt();
        int16_t angle = server.arg("angle").toInt();
        setServoAngle(channel, angle);
        server.send(200, "text/plain", "Moved servo");
    } else {
        server.send(400, "text/plain", "Invalid request");
    }
}

// Handle URL "/zeroServos"
void handleZeroServos() {
    setServoAngle(13, 0);
    setServoAngle(8, 10); // Forward-back to 10 degrees instead of 90
    setServoAngle(9, 90); // Side-to-side to 90 degrees
    setServoAngle(10, 90); // Wrist side-to-side to 90 degrees
    setServoAngle(11, 90); // Wrist up/down to 90 degrees
    server.send(200, "text/plain", "Servos zeroed to calibration positions");
}

// Handle URL "/getAngles"
void handleGetAngles() {
    String json = "{\"claw\":" + String(clawServoAngle) +
                  ", \"shoulderForwardBack\":" + String(leftShoulderForwardBackAngle) +
                  ", \"shoulderSideSide\":" + String(leftShoulderSideSideAngle) +
                  ", \"wristSideSide\":" + String(wristSideSideAngle) +
                  ", \"wristUpDown\":" + String(wristUpDownAngle) + "}";
    server.send(200, "application/json", json);
}

void loop() {
    // Handle client requests
    server.handleClient();
}
