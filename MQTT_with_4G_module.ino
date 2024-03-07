#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3);

// Define MQTT topic and payload
const char* MQTT_PUB_TOPIC = "Publish topic name";                  // MQTT PUB topic
const char* MQTT_SUB_TOPIC = "Subscribe Topic name";               //MQTT SUB topic
const char* MQTT_PAYLOAD = "your message";                        // MQTT payload
const char* MQTT_SERVER = "tcp://************";                  // MQTT server address
const char* MQTT_USERNAME = "*****";                            // MQTT username
const char* MQTT_PASSWORD = "*****";                           // MQTT password

bool exec = false;
bool baud = false;
bool susbcribe = false;
void setup() {
  mySerial.begin(115200);  // Setting the baud rate of A7670 Module
  Serial.begin(19200);
  delay(1000);
}

void loop() {
    if (millis() >= 3000) {                      // Check if the time since startup is greater than 300 milliseconds
    if (!baud) {                              // Check if baud rate has not been set
      ATCommandOK("AT+IPR=19200\r\n", true);  // Set the baud rate to 19200
      mySerial.end();                         // End the serial communication
      mySerial.begin(19200);                  // Begin serial communication at 19200 baud rate
      baud = true;   
      Serial.println("Press 'a' to Start MQTT");
      Serial.println("Press 's' to Subscribe on given MQTT Topic");
      Serial.println("Press 'p' to Publish data on given MQTT Topic"); 
      Serial.println("Press 'r' to Stop MQTT");    // Print message to Serial Monitor                         // Set baud flag to true
    }
  }
  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case 'a':
        susbcribe = false;
        startMQTT();  // Call function to start or setup the MQTT connection
        break;
      case 'p':
        susbcribe = false;
        PublishMQTTData(MQTT_PUB_TOPIC, MQTT_PAYLOAD);  // Call function to send MQTT data
        break;
      case 's':
        subscribeMQTT(MQTT_SUB_TOPIC);
        break;
      case 'r':
        susbcribe = false;
        stopMQTT();  // Call function to stop the MQTT connection
        break;
    }
  }
  if (susbcribe) {
    checkIncomingMQTTMessages();
  }
}

void PublishMQTTData(const char* topic, const char* payload) {
  if (sendBlockingATCommand("AT+CMQTTTOPIC=0," + String(strlen(topic)) + "\r\n", true)) {
    if (ATCommandOK(topic, true)) {
      if (sendBlockingATCommand("AT+CMQTTPAYLOAD=0," + String(strlen(payload)) + "\r\n", true)) {
        if (ATCommandOK(payload, true)) {
          if (ATCommandOK("AT+CMQTTPUB=0,0,60\r\n", true)) {
            Serial.println("Publish done");
          }
        }
      }
    }
  }
}

void subscribeMQTT(const char* topic) {
  if (sendBlockingATCommand("AT+CMQTTSUB=0," + String(strlen(topic)) + ",1,1\r\n", true)) {
    if (ATCommandOK(topic, true)) {
      susbcribe = true;
      Serial.println("Subscribed on given topic");
    }
  }
}
bool sendBlockingATCommand(String cmd, bool extendTime) {
  Serial.print("M ");
  Serial.print(cmd);
  mySerial.print(cmd);

  unsigned long t = millis();
  String str;
  unsigned long int t2 = 4000L;  // Set default timeout value to 4 seconds
  if (extendTime)
    t2 = 6000L;  // Set extended timeout value to 6 seconds
  while (millis() - t < t2) {
    if (mySerial.available() > 0) {
      str = ReadString();
      if (str.indexOf(">") != -1)
        return true;  // Return true if '>' character is received
      else if (matchString(str, "ERROR\r") == true)
        return false;  // Return false if 'ERROR' is received
    }
  }
  Serial.println("time out 2");  // Print timeout message if the loop exits due to timeout
  return false;                  // Return false if timeout occurs
}

bool ATCommandOK(String cmd, bool extendTime) {
  Serial.print("M ");
  Serial.print(cmd);
  mySerial.print(cmd);

  unsigned long t = millis();
  String str;
  unsigned long int t2 = 6000L;  // Set default timeout value to 6 seconds
  if (extendTime)
    t2 = 8000L;  // Set extended timeout value to 8 seconds
  while (millis() - t < t2) {
    if (mySerial.available() > 0) {
      str = ReadString();
      if (matchString(str, "OK\r") == true)
        return true;  // Return true if 'OK' is received
      else if (matchString(str, "ERROR\r") == true)
        return false;  // Return false if 'ERROR' is received
    }
  }
  Serial.println("time out 1");  // Print timeout message if the loop exits due to timeout
  return false;                  // Return false if timeout occurs
}

String ReadString() {
  String str = "";
  if (mySerial.available() > 0) {
    str = mySerial.readStringUntil('\n');
    Serial.print("C ");
    Serial.println(str);
  }
  return str;
}

bool matchString(String m1, String m2) {
  return (m1 == m2);
}

void startMQTT() {

  ATCommandOK("AT\r\n", true);
  ATCommandOK("ATE0\r\n", true);
  ATCommandOK("AT+CMQTTSTART\r\n", true);
  delay(1000);
  ATCommandOK("AT+CMQTTACCQ=0,\"admin1\"\r\n", true);
  delay(1000);
  ATCommandOK("AT+CMQTTCONNECT=0,\"" + String(MQTT_SERVER) + "\",1200,1,\"" + String(MQTT_USERNAME) + "\",\"" + String(MQTT_PASSWORD) + "\"\r\n", true);
  delay(1000);

  //Serial.println("Now press 's' to start sending MQTT data or press 'r' to stop MQTT connection");
}

void stopMQTT() {
  if (!ATCommandOK("AT+CMQTTDISC=0,120\r\n", true)) {  // disconnect MQTT server
    return;
  }
  // Send AT+CMQTTREL=0 command
  if (!ATCommandOK("AT+CMQTTREL=0\r\n", true)) {  // release  accured lient
    //Serial.println("Failed to send AT+CMQTTREL=0 command");
    return;
  }

  // Sendr AT+CMQTTSTOP command
  if (!ATCommandOK("AT+CMQTTSTOP\r\n", true)) {  // Stop MQTT
    //Serial.println("Failed to send AT+CMQTTSTOP command");
    return;
  }

  Serial.println("MQTT stopped successfully");
}

void checkIncomingMQTTMessages() {
  // Check for incoming MQTT messages
  String message = ReadString();  // Read incoming message from serial

  if (message.length() > 0) {
    // Handle the received MQTT message
    Serial.print("Received MQTT message: ");
    Serial.println(message);
  }
}