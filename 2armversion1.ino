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
uint16_t servoAnglesLeft[4] = {90, 90, 90, 90}; // Initial angles for servos 15, 14, 13, 12
uint16_t servoAnglesRight[4] = {90, 90, 90, 90}; // Initial angles for servos 11, 10, 9, 8

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
    server.on("/start", handleStart);
    server.on("/stop", handleStop);
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
    if (channel >= 8 && channel <= 11) {
        servoAnglesRight[channel - 8] = angle;
    } else if (channel >= 12 && channel <= 15) {
        servoAnglesLeft[channel - 12] = angle;
    }
}

// Function to get current servo angle from pulse width
uint16_t getServoAngle(uint8_t channel) {
    uint16_t pulse = pca.getPWM(channel, 0); // Get current pulse width
    return map(pulse, SERVOMIN, SERVOMAX, 0, 180);
}

// Function for linear interpolation for smooth movements
void moveServoSmooth(uint8_t channel, uint16_t targetAngle, uint16_t steps, uint16_t stepDelay) {
    uint16_t currentAngle = getServoAngle(channel); // Get current servo angle
    int16_t angleDiff = targetAngle - currentAngle;
    int16_t angleIncrement = angleDiff / steps;

    for (uint16_t i = 0; i < steps; i++) {
        currentAngle += angleIncrement;
        setServoAngle(channel, currentAngle);
        delay(stepDelay);
    }
    // Ensure the final position is accurate
    setServoAngle(channel, targetAngle); 
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
    <button class='button' onclick=\"sendRequest('/start')\">Start</button>\
    <button class='button' onclick=\"sendRequest('/stop')\">Stop</button>\
    <div class='panel left-panel'>\
    <h2>Left Arm</h2>\
    <h3>Shoulder Servo 1 (Up/Down)</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo15' onchange=\"updateServo(15, this.value)\">\
    <span id='angle15'>90</span>\
    <h3>Shoulder Servo 2 (Inward/Outward)</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo14' onchange=\"updateServo(14, this.value)\">\
    <span id='angle14'>90</span>\
    <h3>Wrist Servo 1 (Counterclockwise/Clockwise)</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo13' onchange=\"updateServo(13, this.value)\">\
    <span id='angle13'>90</span>\
    <h3>Wrist Servo 2 (Down/Open Palmed)</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo12' onchange=\"updateServo(12, this.value)\">\
    <span id='angle12'>90</span>\
    </div>\
    <div class='panel right-panel'>\
    <h2>Right Arm</h2>\
    <h3>Shoulder Servo 1 (Up/Down)</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo11' onchange=\"updateServo(11, this.value)\">\
    <span id='angle11'>90</span>\
    <h3>Shoulder Servo 2 (Inward/Outward)</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo10' onchange=\"updateServo(10, this.value)\">\
    <span id='angle10'>90</span>\
    <h3>Wrist Servo 1 (Counterclockwise/Clockwise)</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo9' onchange=\"updateServo(9, this.value)\">\
    <span id='angle9'>90</span>\
    <h3>Wrist Servo 2 (Down/Open Palmed)</h3>\
    <input type='range' min='0' max='180' value='90' class='slider' id='servo8' onchange=\"updateServo(8, this.value)\">\
    <span id='angle8'>90</span>\
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
        for (let i = 12; i <= 15; i++) {\
            document.getElementById(`servo${i}`).value = anglesLeft[i - 12];\
            document.getElementById(`angle${i}`).innerText = anglesLeft[i - 12];\
        }\
        for (let i = 8; i <= 11; i++) {\
            document.getElementById(`servo${i}`).value = anglesRight[i - 8];\
            document.getElementById(`angle${i}`).innerText = anglesRight[i - 8];\
        }\
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

// Handle URL "/start"
void handleStart() {
    autoMode = true;
    server.send(200, "text/plain", "Started automatic mode");
}

// Handle URL "/stop"
void handleStop() {
    autoMode = false;
    server.send(200, "text/plain", "Stopped automatic mode");
}

// Handle URL "/getAngles"
void handleGetAngles() {
    String json = "{\"left\":[";
    for (int i = 0; i < 4; i++) {
        json += String(servoAnglesLeft[i]);
        if (i < 3) json += ",";
    }
    json += "], \"right\":[";
    for (int i = 0; i < 4; i++) {
        json += String(servoAnglesRight[i]);
        if (i < 3) json += ",";
    }
    json += "]}";
    server.send(200, "application/json", json);
}

void loop() {
    // Handle client requests
    server.handleClient();

    if (autoMode) {
        // Original behavior routine for left arm and right arm simultaneously

        // Shoulder Servo 1 (Forward and Backward)
        moveServoSmooth(15, 110, 40, 25);   // Left Forward (smoother, same timing)
        moveServoSmooth(11, 110, 40, 25);   // Right Forward (smoother, same timing)
        delay(500);                           // Add a short delay in the middle for fluidity
        moveServoSmooth(15, 40, 40, 25);    // Left Backward (smoother, same timing)
        moveServoSmooth(11, 40, 40, 25);    // Right Backward (smoother, same timing)
        delay(500);

        // Shoulder Servo 2 (Left and Right)
        moveServoSmooth(14, 160, 60, 16);   // Left Move Away  (smoother, same timing)
        moveServoSmooth(10, 160, 60, 16);   // Right Move Away  (smoother, same timing)
        moveServoSmooth(14, 100, 60, 16);   // Left Move Towards 
        moveServoSmooth(10, 100, 60, 16);   // Right Move Towards 
        moveServoSmooth(14, 165, 60, 16);   // Left Move Away  (smoother, same timing)
        moveServoSmooth(10, 165, 60, 16);   // Right Move Away  (smoother, same timing)
        moveServoSmooth(14, 100, 60, 16);   // Left Move Towards 
        moveServoSmooth(10, 100, 60, 16);   // Right Move Towards 

        // Wrist Servo 1 (Left and Right)
        moveServoSmooth(13, 0, 20, 50);    // Left (smoother, same timing)
        moveServoSmooth(9, 0, 20, 50);    // Right (smoother, same timing)
        moveServoSmooth(13, 170, 20, 50);  // Left (smoother, same timing)
        moveServoSmooth(9, 170, 20, 50);  // Right (smoother, same timing)

        // Wrist Servo 2 (Jiggle) fingers
        moveServoSmooth(12, 0, 10, 50);   // Left Jiggle 1
        moveServoSmooth(8, 0, 10, 50);   // Right Jiggle 1
        moveServoSmooth(12, 180, 10, 50);  // Left Jiggle 2
        moveServoSmooth(8, 180, 10, 50);  // Right Jiggle 2
        moveServoSmooth(12, 90, 10, 50);   // Left Center
        moveServoSmooth(8, 90, 10, 50);   // Right Center
        moveServoSmooth(15, 10, 20, 50);   // Left Move shoulder back during jiggle
        moveServoSmooth(11, 10, 20, 50);   // Right Move shoulder back during jiggle
        moveServoSmooth(12, 0, 10, 50);   // Left Jiggle 1
        moveServoSmooth(8, 0, 10, 50);   // Right Jiggle 1
        moveServoSmooth(12, 180, 10, 50);  // Left Jiggle 2
        moveServoSmooth(8, 180, 10, 50);  // Right Jiggle 2
        moveServoSmooth(12, 90, 10, 50);   // Left Center
        moveServoSmooth(8, 90, 10, 50);   // Right Center

        // Don't Break It Section (Smoothed)
        moveServoSmooth(15, 90, 40, 25);      // Left Forward  
        moveServoSmooth(11, 90, 40, 25);      // Right Forward
        delay(5000);                            
        moveServoSmooth(15, 140, 40, 25);     // Left Forward (further) 
        moveServoSmooth(11, 140, 40, 25);     // Right Forward (further) 
        delay(5000);                           
        moveServoSmooth(14, 140, 80, 12);      // Left Move away from body (wider range, slower)
        moveServoSmooth(10, 140, 80, 12);      // Right Move away from body (wider range, slower)
        delay(4000);
        moveServoSmooth(13, 90, 60, 16);      // Left Wrist center
        moveServoSmooth(9, 90, 60, 16);       // Right Wrist center
        delay(3000);
        moveServoSmooth(12, 0, 10, 50);       // Left Fingers open
        moveServoSmooth(8, 0, 10, 50);        // Right Fingers open
        delay(3000); 
        moveServoSmooth(12, 180, 10, 50);     // Left Fingers close (fast)
        moveServoSmooth(8, 180, 10, 50);      // Right Fingers close (fast)
        delay(500);  
    }
}
