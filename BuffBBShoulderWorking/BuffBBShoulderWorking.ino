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
uint16_t servoAnglesLeft[1] = {90}; // Initial angle for left shoulder servo
uint16_t servoAnglesRight[1] = {90}; // Initial angle for right shoulder servo

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
    } else if (channel == 4) {
        servoAnglesRight[0] = angle;
    }
}

// Function to get current servo angle from pulse width
uint16_t getServoAngle(uint8_t channel) {
    uint16_t pulse = pca.getPWM(channel, 0); // Get current pulse width
    return map(pulse, SERVOMIN, SERVOMAX, 0, 180);
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
    <h1>Control Shoulder Servos</h1>\
    <div class='panel left-panel'>\
    <h2>Left Shoulder</h2>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo0' onchange=\"updateServo(0, this.value)\">\
    <span id='angle0'>90</span>\
    </div>\
    <div class='panel right-panel'>\
    <h2>Right Shoulder</h2>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo4' onchange=\"updateServo(4, this.value)\">\
    <span id='angle4'>90</span>\
    </div>\
    <script>\
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
        document.getElementById('servo4').value = anglesRight[0];\
        document.getElementById('angle4').innerText = anglesRight[0];\
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
    json += String(servoAnglesLeft[0]);
    json += "], \"right\":[";
    json += String(servoAnglesRight[0]);
    json += "]}";
    server.send(200, "application/json", json);
}

void loop() {
    // Handle client requests
    server.handleClient();
}
