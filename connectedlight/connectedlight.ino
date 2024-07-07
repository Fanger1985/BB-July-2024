#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // For JSON processing
#include <driver/ledc.h>  // For LEDC functions

// Define LED RGB pin mappings
#define LED1_R 32
#define LED1_G 33
#define LED1_B 25

#define LED2_R 26
#define LED2_G 27
#define LED2_B 14

#define LED3_R 12
#define LED3_G 13
#define LED3_B 15

#define LED4_R 2
#define LED4_G 4
#define LED4_B 19

#define PWM_FREQ 5000  // PWM frequency
#define PWM_RESOLUTION 8  // PWM resolution (0-255)

// LEDC channels for each LED color
#define CHANNEL_1_R 0
#define CHANNEL_1_G 1
#define CHANNEL_1_B 2
#define CHANNEL_2_R 3
#define CHANNEL_2_G 4
#define CHANNEL_2_B 5
#define CHANNEL_3_R 6
#define CHANNEL_3_G 7
#define CHANNEL_3_B 8
#define CHANNEL_4_R 9
#define CHANNEL_4_G 10
#define CHANNEL_4_B 11

// WiFi credentials
const char* ssid = "BB1";  // Change to your WiFi SSID
const char* password = "totallysecure";  // Change to your WiFi password

// Mobile unit's IP address
const char* mobileUnitIP = "192.168.1.100";  // Change to your mobile unit's IP address

void setup() {
  Serial.begin(115200);  // Initialize Serial for debugging
  connectToWiFi();  // Connect to WiFi

  // Set up LED PWM channels
  setupPWMChannels();

  // Set LED pins as outputs
  setLEDsAsOutput();
}

void connectToWiFi() {
  WiFi.begin(ssid, password);  // Start connecting to WiFi
  Serial.println("Connecting to WiFi...");

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {  // Retry 20 times
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());  // Display IP address
  } else {
    Serial.println("Failed to connect to WiFi.");
  }
}

void setupPWMChannels() {
  // Initialize PWM channels for each LED color
  ledcSetup(CHANNEL_1_R, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED1_R, CHANNEL_1_R);
  ledcSetup(CHANNEL_1_G, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED1_G, CHANNEL_1_G);
  ledcSetup(CHANNEL_1_B, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED1_B, CHANNEL_1_B);

  ledcSetup(CHANNEL_2_R, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED2_R, CHANNEL_2_R);
  ledcSetup(CHANNEL_2_G, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED2_G, CHANNEL_2_G);
  ledcSetup(CHANNEL_2_B, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED2_B, CHANNEL_2_B);

  ledcSetup(CHANNEL_3_R, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED3_R, CHANNEL_3_R);
  ledcSetup(CHANNEL_3_G, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED3_G, CHANNEL_3_G);
  ledcSetup(CHANNEL_3_B, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED3_B, CHANNEL_3_B);

  ledcSetup(CHANNEL_4_R, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED4_R, CHANNEL_4_R);
  ledcSetup(CHANNEL_4_G, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED4_G, CHANNEL_4_G);
  ledcSetup(CHANNEL_4_B, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED4_B, CHANNEL_4_B);
}

void setLEDsAsOutput() {
  Serial.println("LED pins set as outputs.");
}

void loop() {
  requestSensorData();  // Check sensor data for color changes
  requestMood();  // Check mood for specific conditions like "happy"
  delay(1000);  // Poll every second
}

void requestSensorData() {
  HTTPClient http;
  http.begin("http://" + String(mobileUnitIP) + "/sensors");  // Endpoint to get sensor data
  int httpCode = http.GET();  // Send the HTTP GET request

  if (httpCode == 200) {  // If the request is successful
    String payload = http.getString();  // Get the response data
    Serial.println("Received sensor data:");
    Serial.println(payload);  // Display the raw data
    DynamicJsonDocument doc(256);  // For deserialization
    deserializeJson(doc, payload);

    int irLeft = doc["ir_left"];  // IR sensor left
    int irRight = doc["ir_right"];  // IR sensor right
    int distance = doc["distance"];  // Ultrasonic distance

    if (irLeft == 0) {
      Serial.println("Left rear IR sensor triggered. Setting to Orange.");
      setSolidColor(255, 165, 0);  // Orange for left rear IR sensor
    } else if (irRight == 0) {
      Serial.println("Right rear IR sensor triggered. Setting to Red.");
      setSolidColor(255, 0, 0);  // Red for right rear IR sensor
    } else if (distance < 30) {
      Serial.println("Ultrasonic sensor detected close proximity. Setting to Red.");
      setSolidColor(255, 0, 0);  // Red for close proximity
    } else {
      Serial.println("All clear. Setting to Green.");
      setSolidColor(0, 255, 0);  // Green for all clear
    }
  } else {
    Serial.println("Failed to get sensor data. HTTP code: ");
    Serial.println(httpCode);  // Log the error code
  }

  http.end();  // End the HTTP session
}

void requestMood() {
  HTTPClient http;
  http.begin("http://" + String(mobileUnitIP) + "/expressEmotion");  // Endpoint for mood
  int httpCode = http.GET();  // Send HTTP GET request

  if (httpCode == 200) {  // If the request is successful
    String payload = http.getString();  // Get the response data
    Serial.println("Received mood data:");
    Serial.println(payload);  // Display the raw data
    DynamicJsonDocument doc(256);  // For deserialization
    deserializeJson(doc, payload);

    String emotion = doc["emotion"];  // Extract the mood or emotion

    if (emotion == "happy") {
      Serial.println("Setting color to Yellow for happy.");
      setSolidColor(255, 255, 0);  // Yellow for happy
    }

  } else {
    Serial.println("Failed to get mood data. HTTP code: ");
    Serial.println(httpCode);  // Log the error code
  }

  http.end();  // End the HTTP session
}

void setSolidColor(int r, int g, int b) {
  Serial.print("Setting all LEDs to solid RGB (");
  Serial.print(r);
  Serial.print(", ");
  Serial.print(g);
  Serial.print(", ");
  Serial.print(b);
  Serial.println(").");

  ledcWrite(CHANNEL_1_R, r);  // Set color for LED 1
  ledcWrite(CHANNEL_1_G, g);
  ledcWrite(CHANNEL_1_B, b);

  ledcWrite(CHANNEL_2_R, r);  // Set color for LED 2
  ledcWrite(CHANNEL_2_G, g);
  ledcWrite(CHANNEL_2_B, b);

  ledcWrite(CHANNEL_3_R, r);  // Set color for LED 3
  ledcWrite(CHANNEL_3_G, g);
  ledcWrite(CHANNEL_3_B, b);

  ledcWrite(CHANNEL_4_R, r);  // Set color for LED 4
  ledcWrite(CHANNEL_4_G, g);
  ledcWrite(CHANNEL_4_B, b);
}
