/*
 * ESP8266 Network Unit
 * This script sets up the ESP8266 as an MQTT broker and a WiFi access point.
 * It also handles the initialization of an OLED display for status output.
 *
 * Functions:
 * - Initializes WiFi in AP mode with fixed IP settings.
 * - Starts an MQTT broker to manage messages between this unit and other devices like ESP32.
 * - Displays system status on an OLED screen.
 *
 * IP Configuration:
 * - MQTT Broker IP: 192.168.1.1 (fixed IP for the ESP8266 AP mode)
 *
 * Usage:
 * - Other devices (ESP32, Python script) connect to this broker to send/receive commands.
 * - This unit mainly acts as a network manager and message router without handling direct device control.
 */


#include <ESP8266WiFi.h>
#include <uMQTTBroker.h>
#include <Wire.h>
#include <SSD1306Wire.h>

// Network credentials
#define WIFI_SSID "BB1"
#define WIFI_PASSWORD "totallysecure"

// OLED settings
#define OLED_ADDRESS 0x3C
#define OLED_SDA D6
#define OLED_SCL D5
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Define the IP address and subnet mask for the Access Point
IPAddress apIP(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Create an instance of the broker
uMQTTBroker myBroker;

// Create an instance of the OLED display
SSD1306Wire display(OLED_ADDRESS, OLED_SDA, OLED_SCL);

void setup() {
    Serial.begin(115200);
    delay(1000); // Wait for serial monitor to open
    Serial.println("\n");

    // Start WiFi in Access Point mode
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, subnet);
    bool apStarted = WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

    if (apStarted) {
      Serial.println("Access Point started successfully!");
    } else {
      Serial.println("Failed to start Access Point. Check configuration.");
    }

    Serial.print("Access Point IP address: ");
    Serial.println(WiFi.softAPIP());

    // Initialize the OLED display
    display.init();
    display.flipScreenVertically(); // Adjust if your display is flipped
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "BB1 PROTOTYPE");
    display.drawString(0, 20, "ALL IN A DAZE");
    display.display();

    // Start the broker
    myBroker.init();

    Serial.println("Broker setup complete");
}

void loop() {
    // Handle client requests or any other loop activities
    // Can be kept empty if nothing else is needed
}
