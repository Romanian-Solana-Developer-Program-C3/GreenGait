// #include "WiFiManager.h"
// #include "EEPROM.h"
// #include "WiFiClientSecure.h"
// #include "PubSubClient.h"
// #include "crypto_hmac.h"
// #include "certificates.h"

// int counter = 0;
// const int buttonPin = 14;

// WiFiClientSecure secureClient;
// PubSubClient mqttClient(secureClient);

// const char* mqtt_server = "stepmint.duckdns.org";
// const int mqtt_port = 8883;
// const char* mqtt_username = "step_sensor_device";
// const char* mqtt_password = "Password123";
// const char* mqtt_topic = "devices/step_counter/steps";

// const char* hmac_secret = "my_secret_key";

// unsigned long lastStepTime = 0;
// const unsigned long cooldown = 2000;

// unsigned long lastDebounceTime = 0;
// const unsigned long debounceDelay = 50;
// int lastButtonReading = HIGH;
// int lastStableState = HIGH;

// bool wifiConnected = false;

// void sendStep(int steps) {
//   unsigned long currentTimestamp = time(nullptr); // Fresh timestamp every time
//   Serial.println("🕒 Sending step with timestamp: " + String(currentTimestamp));

//   String nonce = String(random(100000, 999999));
//   String payload = "{\"steps\":" + String(steps) + ",\"timestamp\":" + String(currentTimestamp) + ",\"nonce\":\"" + nonce + "\"}";
//   String hmac = generateHMAC(payload, hmac_secret);
//   String message = payload.substring(0, payload.length() - 1) + ",\"signature\":\"" + hmac + "\"}";

//   Serial.println("[MQTT] Publishing message:");
//   Serial.println(message);

//   bool sent = mqttClient.publish(mqtt_topic, message.c_str());
//   if (sent) {
//     Serial.println("✅ MQTT publish success");
//   } else {
//     Serial.println("❌ MQTT publish failed");
//   }
// }

// void connectWiFi() {
//   if (WiFi.isConnected()) {
//     Serial.println("[WiFi] Already connected.");
//     return;
//   }

//   WiFiManager wm;
//   if (!wm.autoConnect("StepDeviceWiFi", "12345678")) {
//     Serial.println("[WiFi] Failed to connect. Restarting...");
//     delay(3000);
//     ESP.restart();
//   }

//   Serial.println("[WiFi] Connected.");
//   wifiConnected = true;
// }

// void reconnectMQTT() {
//   while (!mqttClient.connected()) {
//     Serial.print("🔄 Attempting MQTT connection...");
//     if (mqttClient.connect("StepDeviceClient", mqtt_username, mqtt_password)) {
//       Serial.println("✅ MQTT Connected");
//     } else {
//       Serial.print("❌ MQTT Failed. Code: ");
//       Serial.print(mqttClient.state());
//       Serial.println(" → retrying in 5s");
//       delay(5000);
//     }
//   }
// }

// void setup() {
//   Serial.begin(115200);
//   delay(1000);
//   Serial.println("🔌 ESP32 Booting...");

//   pinMode(buttonPin, INPUT_PULLUP);

//   EEPROM.begin(512);
//   EEPROM.get(0, counter);
//   if (counter < 0 || counter > 1000000) {
//     counter = 0;
//   }

//   connectWiFi();

//   configTime(0, 0, "pool.ntp.org");
//   Serial.print("⌛ Waiting for NTP time sync");
//   while (time(nullptr) < 100000) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("\n✅ Time synced.");

//   secureClient.setCACert(ca_cert);
//   secureClient.setCertificate(client_cert);
//   secureClient.setPrivateKey(client_key);

//   mqttClient.setServer(mqtt_server, mqtt_port);
//   mqttClient.setKeepAlive(30);  // Ensures long connection
// }

// void loop() {
//   // Serial.println("🔁 Loop running...");

//   if (WiFi.status() != WL_CONNECTED) {
//     if (wifiConnected) {
//       Serial.println("[WiFi] Disconnected, reconnecting...");
//       wifiConnected = false;
//     }
//     connectWiFi();
//   }

//   if (!mqttClient.connected()) {
//     reconnectMQTT(); // More reliable reconnect logic
//   }

//   mqttClient.loop();
//   handleButtonPress();

//   delay(100); // Slight delay to reduce serial spam
// }

// void handleButtonPress() {
//   int reading = digitalRead(buttonPin);

//   // Debounce logic
//   if (reading != lastButtonReading) {
//     lastDebounceTime = millis();
//   }

//   if ((millis() - lastDebounceTime) > debounceDelay) {
//     // Detect press transition: HIGH → LOW
//     if (lastStableState == HIGH && reading == LOW) {
//       unsigned long now = millis();

//       // ✅ Check cooldown only when a *new press* is confirmed
//       if (now - lastStepTime > cooldown) {
//         lastStepTime = now;

//         counter++;
//         Serial.print("[BUTTON] Step detected! Counter: ");
//         Serial.println(counter);

//         EEPROM.put(0, counter);
//         EEPROM.commit();

//         sendStep(counter);
//       } else {
//         Serial.println("⏱️ Step ignored due to cooldown.");
//       }
//     }

//     // Save the current stable reading
//     lastStableState = reading;
//   }

//   lastButtonReading = reading;
// }


#include "WiFiManager.h"
#include "EEPROM.h"
#include "WiFiClientSecure.h"
#include "PubSubClient.h"
#include "crypto_hmac.h"
#include "certificates.h"
#include <sodium.h>

int counter = 0;
const int buttonPin = 14;

WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

const char* mqtt_server = "stepmint.duckdns.org";  
const int mqtt_port = 8883;
const char* mqtt_username = "step_sensor_device";
const char* mqtt_password = "Password123";
const char* mqtt_topic = "devices/step_counter/steps";

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

int lastButtonReading = HIGH;
int lastStableState = HIGH;
bool wifiConnected = false;

unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
unsigned char private_key[crypto_sign_SECRETKEYBYTES];

void connectWiFi() {
  if (WiFi.isConnected()) {
    Serial.println("[WiFi] Already connected.");
    return;
  }

  WiFiManager wm;
  if (!wm.autoConnect("StepDeviceWiFi", "12345678")) {
    Serial.println("[WiFi] Failed to connect. Restarting...");
    delay(3000);
    ESP.restart();
  }
  Serial.println("[WiFi] Connected.");
  wifiConnected = true;
}

void connectMQTT() {
  if (mqttClient.connected()) return;

  Serial.println("[MQTT] Connecting...");
  if (mqttClient.connect("StepDeviceClient", mqtt_username, mqtt_password)) {
    Serial.println("[MQTT] Connected.");
  } else {
    Serial.print("[MQTT] Failed, rc=");
    Serial.println(mqttClient.state());
  }
}

void loadOrGenerateKeypair() {
  EEPROM.begin(512);
  if (EEPROM.read(100) != 0xAA) {
    Serial.println("[KEYPAIR] Generating new Solana keypair...");
    crypto_sign_keypair(public_key, private_key);
    for (int i = 0; i < crypto_sign_SECRETKEYBYTES; i++)
      EEPROM.write(101 + i, private_key[i]);
    for (int i = 0; i < crypto_sign_PUBLICKEYBYTES; i++)
      EEPROM.write(101 + crypto_sign_SECRETKEYBYTES + i, public_key[i]);
    EEPROM.write(100, 0xAA);
    EEPROM.commit();
  } else {
    Serial.println("[KEYPAIR] Loading Solana keypair from EEPROM...");
    for (int i = 0; i < crypto_sign_SECRETKEYBYTES; i++)
      private_key[i] = EEPROM.read(101 + i);
    for (int i = 0; i < crypto_sign_PUBLICKEYBYTES; i++)
      public_key[i] = EEPROM.read(101 + crypto_sign_SECRETKEYBYTES + i);
  }
}

String encodeBase58(const unsigned char* data, size_t len) {
  static const char* alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
  String result = "";
  uint8_t temp[64] = {0};
  memcpy(temp, data, len);

  int zeroes = 0, length = 0;
  while (zeroes < len && temp[zeroes] == 0) zeroes++;

  uint8_t b58[128] = {0};
  for (size_t i = zeroes; i < len; i++) {
    int carry = temp[i];
    int j = 0;
    for (int k = sizeof(b58) - 1; (carry != 0 || j < length) && (k >= 0); k--, j++) {
      carry += 256 * b58[k];
      b58[k] = carry % 58;
      carry /= 58;
    }
    length = j;
  }

  int it = sizeof(b58) - length;
  while (it < 128 && b58[it] == 0) it++;

  for (int i = 0; i < zeroes; i++) result += '1';
  for (; it < 128; ++it) result += alphabet[b58[it]];
  return result;
}

void sendStep(int steps) {
  long timestamp = time(nullptr);
  String nonce = String(random(100000, 999999));
  String pubkey_b58 = encodeBase58(public_key, crypto_sign_PUBLICKEYBYTES);

  String payload = "{\"pubkey\":\"" + pubkey_b58 + "\",\"steps\":" + String(steps) +
                   ",\"timestamp\":" + String(timestamp) + ",\"nonce\":\"" + nonce + "\"}";

  String hmac = generateHMAC(payload, "my_secret_key");
  String message = payload.substring(0, payload.length() - 1) + ",\"signature\":\"" + hmac + "\"}";

  Serial.println("[MQTT] Publishing message:");
  Serial.println(message);

  mqttClient.publish(mqtt_topic, message.c_str());
}

void handleButtonPress() {
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonReading) lastDebounceTime = millis();

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (lastStableState == HIGH && reading == LOW) {
      counter++;
      Serial.print("[BUTTON] Step detected! Counter: ");
      Serial.println(counter);

      EEPROM.put(0, counter);
      EEPROM.commit();

      sendStep(1);  // ✅ Trimitem DOAR incrementul (+1), nu valoarea totală
    }
    lastStableState = reading;
  }

  lastButtonReading = reading;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("[SYSTEM] Booting...");

  pinMode(buttonPin, INPUT_PULLUP);

  EEPROM.begin(512);
  EEPROM.get(0, counter);
  if (counter < 0 || counter > 1000000) counter = 0;

  if (sodium_init() < 0) {
    Serial.println("[ERROR] Libsodium init failed!");
    while (true);
  }

  loadOrGenerateKeypair();
  connectWiFi();
  configTime(0, 0, "pool.ntp.org");

  secureClient.setCACert(ca_cert);
  secureClient.setCertificate(client_cert);
  secureClient.setPrivateKey(client_key);

  mqttClient.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      Serial.println("[WiFi] Disconnected, reconnecting...");
      wifiConnected = false;
    }
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    connectMQTT();
  }

  mqttClient.loop();
  handleButtonPress();
}

















/////// buuuuuuuuuuuuun

// #include "WiFiManager.h"
// #include "EEPROM.h"
// #include "WiFiClientSecure.h"
// #include "PubSubClient.h"
// #include "crypto_hmac.h"
// #include "certificates.h"

// int counter = 0;
// const int buttonPin = 14;

// WiFiClientSecure secureClient;
// PubSubClient mqttClient(secureClient);

// // const char* mqtt_server = "34.31.17.160";
// const char* mqtt_server = "stepmint.duckdns.org";  
// const int mqtt_port = 8883;
// const char* mqtt_username = "step_sensor_device";
// const char* mqtt_password = "Password123";
// const char* mqtt_topic = "devices/step_counter/steps";

// unsigned long lastDebounceTime = 0;
// const unsigned long debounceDelay = 50; 

// int lastButtonReading = HIGH;
// int lastStableState = HIGH;

// bool wifiConnected = false;

// void sendStep(int steps) {
//   long timestamp = time(nullptr);
//   String nonce = String(random(100000, 999999));
//   String payload = "{\"steps\":" + String(steps) + ",\"timestamp\":" + String(timestamp) + ",\"nonce\":\"" + nonce + "\"}";

//   String hmac = generateHMAC(payload, "my_secret_key");
//   String message = payload.substring(0, payload.length() - 1) + ",\"signature\":\"" + hmac + "\"}";

//   Serial.println("[MQTT] Publishing message:");
//   Serial.println(message);

//   mqttClient.publish(mqtt_topic, message.c_str());
// }

// void connectWiFi() {
//   if (WiFi.isConnected()) {
//     Serial.println("[WiFi] Already connected.");
//     return;
//   }

//   WiFiManager wm;
//   if (!wm.autoConnect("StepDeviceWiFi", "12345678")) {
//     Serial.println("[WiFi] Failed to connect. Restarting...");
//     delay(3000);
//     ESP.restart();
//   }
//   Serial.println("[WiFi] Connected.");
//   wifiConnected = true;
// }

// void connectMQTT() {
//   if (mqttClient.connected()) {
//     return;
//   }
//   Serial.println("[MQTT] Connecting...");
//   if (mqttClient.connect("StepDeviceClient")) {
//     Serial.println("[MQTT] Connected.");
//   } else {
//     Serial.print("[MQTT] Failed, rc=");
//     Serial.println(mqttClient.state());
//   }
// }

// void setup() {
//   Serial.begin(115200);
//   delay(1000); // Allow Serial monitor to catch up
//   Serial.println("[SYSTEM] Booting...");

//   pinMode(buttonPin, INPUT_PULLUP);

//   EEPROM.begin(512);
//   EEPROM.get(0, counter);
//   if (counter < 0 || counter > 1000000) {
//     counter = 0;
//   }

//   connectWiFi();
//   configTime(0, 0, "pool.ntp.org");

//   secureClient.setCACert(ca_cert);
//   secureClient.setCertificate(client_cert);
//   secureClient.setPrivateKey(client_key);

//   mqttClient.setServer(mqtt_server, mqtt_port);
// }

// void loop() {
//   if (WiFi.status() != WL_CONNECTED) {
//     if (wifiConnected) {
//       Serial.println("[WiFi] Disconnected, reconnecting...");
//       wifiConnected = false;
//     }
//     connectWiFi();
//   }

//   if (!mqttClient.connected()) {
//     connectMQTT();
//   }

//   mqttClient.loop();
//   handleButtonPress();
// }

// void handleButtonPress() {
//   int reading = digitalRead(buttonPin);

//   if (reading != lastButtonReading) {
//     lastDebounceTime = millis();
//   }

//   if ((millis() - lastDebounceTime) > debounceDelay) {
//     if (lastStableState == HIGH && reading == LOW) {
//       counter++;
//       Serial.print("[BUTTON] Step detected! Counter: ");
//       Serial.println(counter);

//       EEPROM.put(0, counter);
//       EEPROM.commit();

//       sendStep(counter);
//     }
//     lastStableState = reading;
//   }

//   lastButtonReading = reading;
// }



















// #include "WiFiManager.h"
// #include "EEPROM.h"
// #include "WiFiClientSecure.h"
// #include "PubSubClient.h"
// #include "crypto_hmac.h"
// #include "certificates.h"

// int counter = 0;
// int buttonPin = 14;

// WiFiClientSecure secureClient;
// PubSubClient mqttClient(secureClient);

// const char* mqtt_server = "o3e3b612.ala.eu-central-1.emqxsl.com";
// const int mqtt_port = 8883;
// const char* mqtt_username = "esp32device";
// const char* mqtt_password = "StepCounterSecure123";
// const char* mqtt_topic = "steps/device";

// unsigned long lastDebounceTime = 0;
// unsigned long debounceDelay = 50; // 50 ms debounce time

// int lastButtonReading = HIGH;
// int lastStableState = HIGH; // new variable

// void sendStep(int steps) {
//   long timestamp = time(nullptr);
//   String nonce = String(random(100000, 999999));
//   String payload = String("{\"steps\":") + steps + ",\"timestamp\":" + timestamp + ",\"nonce\":\"" + nonce + "\"}";
//   String hmac = generateHMAC(payload, "my_secret_key");
//   String message = String("{\"steps\":") + steps + ",\"timestamp\":" + timestamp + ",\"nonce\":\"" + nonce + "\",\"signature\":\"" + hmac + "\"}";

//   Serial.println("Publishing message...");
//   Serial.println(message);

//   mqttClient.publish(mqtt_topic, message.c_str());
// }

// void connectWiFi() {
//   WiFiManager wm;
//   //wm.resetSettings();
//   if (!wm.autoConnect("StepDeviceWiFi", "12345678")) {
//     Serial.println("Failed to connect to WiFi! Restarting...");
//     ESP.restart();
//   }
//   Serial.println("Connected to WiFi");
// }

// void connectMQTT() {
//   if (!mqttClient.connected()) {
//     Serial.print("Connecting to EMQX...");
//     if (mqttClient.connect("StepDeviceClient", mqtt_username, mqtt_password)) {
//       Serial.println("Connected!");
//     } else {
//       Serial.print("Failed to connect, rc=");
//       Serial.print(mqttClient.state());
//       Serial.println(" Trying again in 5 seconds...");
//       delay(5000);
//     }
//   }
// }

// void setup() {
//   Serial.begin(115200);
//   pinMode(buttonPin, INPUT_PULLUP);

//   EEPROM.begin(1024);
//   EEPROM.get(0, counter);
//   if (counter < 0 || counter > 1000000) counter = 0;

//   connectWiFi();
//   configTime(0, 0, "pool.ntp.org");

//   secureClient.setCACert(ca_cert);
//   mqttClient.setServer(mqtt_server, mqtt_port);
// }

// void loop() {
//   if (WiFi.status() != WL_CONNECTED) {
//     Serial.println("WiFi lost, reconnecting...");
//     connectWiFi();
//   }
//   if (!mqttClient.connected()) {
//     connectMQTT();
//   }
//   mqttClient.loop();

//   int reading = digitalRead(buttonPin);

//   if (reading != lastButtonReading) {
//     lastDebounceTime = millis(); // reset debounce timer
//   }

//   if ((millis() - lastDebounceTime) > debounceDelay) {
//     // if the button state has stabilized
//     if (lastStableState == HIGH && reading == LOW) {
//       // button pressed!
//       counter++;
//       EEPROM.put(0, counter);
//       EEPROM.commit();

//       Serial.print("Step count: ");
//       Serial.println(counter);

//       sendStep(counter);
//     }
//     lastStableState = reading;
//   }

//   lastButtonReading = reading;
// }
























//  #include "WiFiManager.h" // Library to manage Wi-Fi connections easily (including portal mode)
// #include "EEPROM.h" // Library for reading and writing data to EEPROM (non-volatile memory)
// #include "WiFiClientSecure.h" // Library for secure Wi-Fi connections using TLS/SSL
// #include "PubSubClient.h" // Library for MQTT communication
// #include "crypto_hmac.h" // Custom library for HMAC (Hash-based Message Authentication Code)
// #include "hiveMQ_certificates.h" // Certificate file for secure communication with HiveMQ

// // Counter to track the number of steps
// int counter = 0;

// // Store the previous state of the button for state change detection
// int lastButtonState = HIGH;

// // Create WiFi and MQTT client objects
// WiFiClientSecure secureClient; 
// PubSubClient mqttClient(secureClient); // MQTT client that uses secure WiFi client

// // Define the pin used for the button
// const int buttonPin = 14;

// // Define the MQTT server and connection details
// const char* mqtt_server = "5f74c4e5bc124a93930d2d4bde48b8d8.s1.eu.hivemq.cloud";
// const int mqtt_port = 8883; // Secure MQTT port
// const char* mqtt_username = "device2"; // MQTT username
// const char* mqtt_password = "Password123"; // MQTT password
// const char* mqtt_topic = "step/device"; // Topic to publish step count data

// // Function to send step data to the MQTT server
// void sendStep(int steps) {
//   // Get the current timestamp, time(nullptr) returns the current time in Unix timestamp format, which is the number of seconds that have passed since January 1, 1970 (the epoch).
//   long timestamp = time(nullptr);
  
//   // Create a random nonce for the message to avoid replay attacks
//   String nonce = String(random(100000, 999999));

//   // Create the payload in JSON format
//   String payload = String("{\"steps\":") + steps +
//                    ",\"timestamp\":" + timestamp +
//                    ",\"nonce\":\"" + nonce + "\"}";

//   // Generate the HMAC signature using a secret key
//   String hmac = generateHMAC(payload, "my_secret_key");

//   // Create the full message to send, including the signature
//   String message = String("{\"steps\":") + steps +
//                    ",\"timestamp\":" + timestamp +
//                    ",\"nonce\":\"" + nonce + "\",\"signature\":\"" + hmac + "\"}";

//   // Print the message to the Serial Monitor for debugging
//   Serial.println("Publishing message...");
//   Serial.println(message);

//   // Publish the message to the specified MQTT topic
//   mqttClient.publish(mqtt_topic, message.c_str());
// }

// // Function to connect to Wi-Fi
// void connectWiFi() {
//   // Create WiFiManager object to manage WiFi connections
//   WiFiManager wm;
  
//   // Optional: Reset Wi-Fi settings to clear previously stored configurations
//   // wm.resetSettings();
  
//   // Try to connect to WiFi, and if it fails, start the WiFiManager portal
//   if (!wm.autoConnect("StepDeviceWiFiInitialize", "12345678")) {
//     Serial.println("Failed to connect to WiFi! Restarting...");
//     ESP.restart();  // Restart the ESP32 to try connecting again
//   }

//   // Once connected, print a confirmation message
//   Serial.println("Connected to WiFi");
// }

// void setup() {
//   // Start serial communication for debugging
//   Serial.begin(115200);
  
//   // Set up the button pin as an input with a pull-up resistor
//   pinMode(buttonPin, INPUT_PULLUP);
  
//   // Load the previous step count from EEPROM
//   EEPROM.begin(1024);
//   EEPROM.get(0, counter);
//   if (counter < 0 || counter > 1000000) counter = 0; // Ensure valid counter value

//   // Connect to Wi-Fi
//   connectWiFi();

//   // Synchronize the current time from a public NTP server
//   configTime(0, 0, "pool.ntp.org");

//   // Set the CA certificate for secure MQTT connection
//   secureClient.setCACert(hiveMQ_ca_cert); // Load HiveMQ CA certificate

//   // Set the MQTT server details
//   mqttClient.setServer(mqtt_server, mqtt_port);
// }

// void loop() {
//   // Check if Wi-Fi is still connected, and if not, reconnect
//   if (WiFi.status() != WL_CONNECTED) {
//     Serial.println("WiFi lost, reconnecting...");
//     // Open WiFiManager portal for reconnection if Wi-Fi is lost
//     WiFiManager wm;
//     wm.startConfigPortal("StepDeviceWiFiInitialize", "12345678");
//   }

//   // Check if MQTT is connected, and reconnect if necessary
//   if (!mqttClient.connected()) {
//     Serial.print("Connecting to HiveMQ Cloud...");
//     if (mqttClient.connect("StepDeviceClient", mqtt_username, mqtt_password)) {
//       Serial.println("Connected!"); // Successfully connected to MQTT
//     } else {
//       Serial.print("Failed to connect, rc=");
//       Serial.print(mqttClient.state());
//       Serial.println(" Trying again in 5 seconds...");
//       delay(5000); // Retry after 5 seconds
//       return;
//     }
//   }

//   // Keep the MQTT connection alive by calling the loop function
//   mqttClient.loop();

//   // Read the button state (LOW means the button is pressed)
//   int buttonState = digitalRead(buttonPin);

//   // Only increment the counter when the button is pressed (button state changes)
//   if (buttonState == LOW && lastButtonState == HIGH) {
//     counter++; // Increment step counter
//     EEPROM.put(0, counter); // Store the updated step count in EEPROM
//     EEPROM.commit(); // Save changes to EEPROM
  
//     Serial.print("Step count: ");
//     Serial.println(counter); // Print the updated step count to the Serial Monitor

//     // Send the updated step count to the MQTT server
//     sendStep(counter);
//     delay(200); // Add a small delay to debounce the button
//   }

//   // Update the last button state for the next loop iteration
//   lastButtonState = buttonState;
// }
