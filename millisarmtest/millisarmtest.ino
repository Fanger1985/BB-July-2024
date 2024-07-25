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
unsigned long previousMillis = 0;
const long interval = 20; // Interval for smooth movement updates

// Current angles of the servos
uint16_t servoAnglesLeft[4] = {90, 90, 90, 90}; // Initial angles for servos 15, 14, 13, 12
uint16_t servoAnglesRight[4] = {90, 90, 90, 90}; // Initial angles for servos 11, 10, 9, 8

// Structure for movement commands
struct MovementCommand {
    uint8_t channel;
    uint16_t startAngle;
    uint16_t targetAngle;
    uint16_t steps;
    uint16_t currentStep;
};

MovementCommand movements[8];
int currentMovement = 0;
int numMovements = 0;

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
    server.on("/stretch", handleStretch);
    server.on("/basePose", handleBasePose);
    server.on("/rabbitPose", handleRabbitPose);
    server.on("/comeAtMeBro", handleComeAtMeBro);
    server.on("/lazorPose", handleLazorPose);
    server.on("/iKnowKungFu", handleIKnowKungFu);
    server.on("/setServoAngle", handleSetServoAngle);
    
    // Start the server
    server.begin();
    Serial.println("Server started");

    // Perform stretching before moving to Base Pose
    stretch();
    delay(2000);
    moveToBasePose();
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

// Cubic easing function for smooth movements
float easeInOutCubic(float t, float b, float c, float d) {
    t /= d / 2;
    if (t < 1) return c / 2 * t * t * t + b;
    t -= 2;
    return c / 2 * (t * t * t + 2) + b;
}

// Function for linear interpolation for smooth movements
void addMovement(uint8_t channel, uint16_t targetAngle, uint16_t steps) {
    uint16_t currentAngle = getServoAngle(channel); // Get current servo angle

    movements[numMovements] = {channel, currentAngle, targetAngle, steps, 0};
    numMovements++;
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
    <button class='button' onclick=\"sendRequest('/stretch')\">Stretch</button>\
    <button class='button' onclick=\"sendRequest('/basePose')\">Base Pose</button>\
    <button class='button' onclick=\"sendRequest('/rabbitPose')\">Rabbit Pose</button>\
    <button class='button' onclick=\"sendRequest('/comeAtMeBro')\">Come At Me Bro</button>\
    <button class='button' onclick=\"sendRequest('/lazorPose')\">Lazor Pose</button>\
    <button class='button' onclick=\"sendRequest('/iKnowKungFu')\">I Know Kung Fu</button>\
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

// Handle URL "/setServoAngle"
void handleSetServoAngle() {
    if (server.hasArg("channel") && server.hasArg("angle")) {
        uint8_t channel = server.arg("channel").toInt();
        uint16_t angle = server.arg("angle").toInt();
        setServoAngle(channel, angle);
        server.send(200, "text/plain", "Servo angle set");
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
    currentMovement = 0;
    numMovements = 0;
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

// Handle URL "/stretch"
void handleStretch() {
    stretch();
    server.send(200, "text/plain", "Stretching");
}

// Handle URL "/basePose"
void handleBasePose() {
    moveToBasePose();
    server.send(200, "text/plain", "Moved to Base Pose");
}

// Handle URL "/rabbitPose"
void handleRabbitPose() {
    moveToRabbitPose();
    server.send(200, "text/plain", "Moved to Rabbit Pose");
}

// Handle URL "/comeAtMeBro"
void handleComeAtMeBro() {
    performComeAtMeBro();
    server.send(200, "text/plain", "Performing Come At Me Bro");
}

// Handle URL "/lazorPose"
void handleLazorPose() {
    moveToLazorPose();
    server.send(200, "text/plain", "Moved to Lazor Pose");
}

// Handle URL "/iKnowKungFu"
void handleIKnowKungFu() {
    performIKnowKungFu();
    server.send(200, "text/plain", "Performing I Know Kung Fu");
}

// Function to perform stretching
void stretch() {
    // Left Arm
    addMovement(15, 39, 40);  // Shoulder Servo 1
    addMovement(14, 150, 40); // Shoulder Servo 2
    addMovement(13, 0, 40);   // Wrist Servo 1
    addMovement(12, 178, 40); // Wrist Servo 2

    // Right Arm
    addMovement(11, 162, 40); // Shoulder Servo 1
    addMovement(10, 146, 40); // Shoulder Servo 2
    addMovement(9, 180, 40);  // Wrist Servo 1
    addMovement(8, 167, 40);  // Wrist Servo 2
}

// Function to move to Base Pose
void moveToBasePose() {
    // Left Arm
    addMovement(15, 141, 40); // Shoulder Servo 1
    addMovement(14, 76, 40);  // Shoulder Servo 2
    addMovement(13, 128, 40); // Wrist Servo 1
    addMovement(12, 162, 40); // Wrist Servo 2

    // Right Arm
    addMovement(11, 53, 40);  // Shoulder Servo 1
    addMovement(10, 79, 40);  // Shoulder Servo 2
    addMovement(9, 39, 40);   // Wrist Servo 1
    addMovement(8, 168, 40);  // Wrist Servo 2
}

// Function to move to Rabbit Pose
void moveToRabbitPose() {
    // Left Arm
    addMovement(15, 27, 40);  // Shoulder Servo 1
    addMovement(14, 72, 40);  // Shoulder Servo 2
    addMovement(13, 176, 40); // Wrist Servo 1
    addMovement(12, 147, 40); // Wrist Servo 2

    // Right Arm
    addMovement(11, 150, 40); // Shoulder Servo 1
    addMovement(10, 90, 40);  // Shoulder Servo 2
    addMovement(9, 4, 40);    // Wrist Servo 1
    addMovement(8, 113, 40);  // Wrist Servo 2
}

// Function to perform Come At Me Bro
void performComeAtMeBro() {
    for (int i = 0; i < 3; i++) {
        // Left Arm
        addMovement(15, 35, 20);   // Shoulder Servo 1
        addMovement(14, 76, 20);   // Shoulder Servo 2
        addMovement(13, 19, 20);   // Wrist Servo 1
        addMovement(12, 90, 20);   // Wrist Servo 2

        // Right Arm
        addMovement(11, 152, 20);  // Shoulder Servo 1
        addMovement(10, 73, 20);   // Shoulder Servo 2
        addMovement(9, 142, 20);   // Wrist Servo 1
        addMovement(8, 132, 20);   // Wrist Servo 2

        // Wait for completion
        while (currentMovement < numMovements) {
            delay(20);
        }

        // Next set of movements
        addMovement(15, 23, 20);   // Shoulder Servo 1
        addMovement(14, 129, 20);  // Shoulder Servo 2
        addMovement(13, 63, 20);   // Wrist Servo 1
        addMovement(12, 40, 20);   // Wrist Servo 2

        addMovement(11, 161, 20);  // Shoulder Servo 1
        addMovement(10, 107, 20);  // Shoulder Servo 2
        addMovement(9, 92, 20);    // Wrist Servo 1
        addMovement(8, 31, 20);    // Wrist Servo 2
    }
}

// Function to move to Lazor Pose
void moveToLazorPose() {
    // Left Arm
    addMovement(15, 50, 40);  // Shoulder Servo 1
    addMovement(14, 122, 40); // Shoulder Servo 2
    addMovement(13, 180, 40); // Wrist Servo 1
    addMovement(12, 178, 40); // Wrist Servo 2

    // Right Arm
    addMovement(11, 131, 40); // Shoulder Servo 1
    addMovement(10, 120, 40); // Shoulder Servo 2
    addMovement(9, 39, 40);   // Wrist Servo 1
    addMovement(8, 168, 40);  // Wrist Servo 2
}

// Function to perform I Know Kung Fu
void performIKnowKungFu() {
    // Left Arm
    addMovement(15, 30, 40);  // Shoulder Servo 1
    addMovement(14, 49, 40);  // Shoulder Servo 2
    addMovement(13, 68, 40);  // Wrist Servo 1
    addMovement(12, 168, 40); // Wrist Servo 2

    // Right Arm
    addMovement(11, 126, 40); // Shoulder Servo 1
    addMovement(10, 72, 40);  // Shoulder Servo 2
    addMovement(9, 95, 40);   // Wrist Servo 1
    addMovement(8, 52, 40);   // Wrist Servo 2

    for (int i = 0; i < 3; i++) {
        // Right Arm Wrist Servo 2 movements
        addMovement(8, 131, 20);
        addMovement(8, 47, 20);
        addMovement(8, 131, 20);
        addMovement(8, 47, 20);
    }

    // Finish with Left Arm Wrist Servo 2 moving to 65
    addMovement(12, 65, 40);
}

void processMovements() {
    if (numMovements > 0) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            MovementCommand &cmd = movements[currentMovement];

            float t = cmd.currentStep;
            float b = cmd.startAngle;
            float c = cmd.targetAngle - cmd.startAngle;
            float d = cmd.steps;
            uint16_t easedAngle = easeInOutCubic(t, b, c, d);

            setServoAngle(cmd.channel, easedAngle);

            cmd.currentStep++;
            if (cmd.currentStep >= cmd.steps) {
                setServoAngle(cmd.channel, cmd.targetAngle); // Ensure final position is accurate
                currentMovement++;
                if (currentMovement >= numMovements) {
                    currentMovement = 0;
                    numMovements = 0;
                }
            }
        }
    }
}

void loop() {
    // Handle client requests
    server.handleClient();

    if (autoMode) {
        // Process movements
        processMovements();
    }
}
