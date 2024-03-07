void setup() {
  Serial.begin(115200); // Initialize Serial Monitor
  Serial2.begin(115200, SERIAL_8N1, 17, 16); // Initialize hardware Serial for ESP32 communication
  Serial.setTimeout(1000); // Set a timeout for Serial input
}

void loop() {
  // Check if any data is available on Serial Monitor
  if (Serial.available() > 0) {
    // Read the input from Serial Monitor
    char input = Serial.read();
    // Check if the input is 'S'
    if (input == 'S') {
      // Print the command to Serial Monitor
      //Serial.println("Sending command: AT");
      // Send the AT command to ESP32
      Serial2.println("AT");
      // Wait for ESP32 to respond
      delay(1000); // Adjust delay according to response time
      // Print the response from ESP32 to Serial Monitor
      waitForResponse();
      
      // Send the AT+CSQ command to ESP32
      //Serial.println("Sending command: AT+CSQ");
      Serial2.println("AT+CSQ");
      // Wait for ESP32 to respond
      delay(1000); // Adjust delay according to response time
      // Print the response from ESP32 to Serial Monitor
      waitForResponse();
    }
  }
}

void waitForResponse() {
  // Wait for response from ESP32
  unsigned long timeout = millis() + 10000; // Timeout after 10 seconds
  while (!Serial2.available() && millis() < timeout) {
    // Wait for data to be available or timeout
  }
  // Print the response from ESP32 to Serial Monitor
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}
