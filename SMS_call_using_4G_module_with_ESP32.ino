// Flags and variables
bool baud = false;                    // Flag to ensure baud rate setup only once
bool incomingCall = false;            // Flag for incoming call status
bool outgoingCallDetected = false;    // Flag to prevent duplicate outgoing call detection
String newSMSIndex;                   // Index of the new SMS
const char* callingNumber = "+91**********";   // Number to call
const char* messageToSend = "Hello from Arduino!";  // Message to send

void setup() {
  Serial2.begin(115200, SERIAL_8N1, 17, 16);  // Setting the baud rate of 4G Module
  Serial.begin(19200);
  delay(1000);
}

void loop() {
  // Baud rate setup
  if (millis() >= 300) {
    if (!baud) {
      ATCommandOK("AT+IPR=19200\r\n", true);  // Set baud rate
      Serial2.end();
      Serial2.begin(19200);
      baud = true;
      // Display menu options
      Serial.println("Press 'c' to make Outgoing call");
      Serial.println("Press 'a' to Receive a Incoming call");
      Serial.println("Press 's' to send an SMS");
    }
  }
  
  // Check for user input from serial monitor
  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case 'c':
        incomingCall = false;
        makeCall(callingNumber);    // Initiate outgoing call
        break;
      case 's':
        sendSMS(callingNumber, messageToSend);   // Send SMS
        break;
      case 'a':
        attendCall();   // Answer incoming call
        break;
    }
  }
  
  // Check for responses from 4G module
  if (Serial2.available() > 0) {
    String response = ReadString();
    if (response.indexOf("+CLCC:") != -1) {
      if (!outgoingCallDetected) {
        // Outgoing call initiated
        outgoingCallDetected = true;
      }
    } else if (response.indexOf("+CMTI") != -1) {
      Serial.println("New SMS received!");
      // Extract index of the new SMS
      int startPos = response.indexOf(",") + 1;
      int endPos = response.indexOf("\",", startPos);
      newSMSIndex = response.substring(startPos, endPos);
      Serial.print("Index of new SMS: ");
      Serial.println(newSMSIndex);
      // Read the content of the new SMS using the obtained index
      ATCommandOK("AT+CMGF=1\r\n", true);  // Set SMS text mode
      ATCommandOK("AT+CMGR=" + newSMSIndex + "\r\n", true);
      ATCommandOK("AT+CMGD=1,4\r\n", true);  // Delete the read SMS
    }
  }
}

// Function to make a call
void makeCall(const char* phoneNumber) {
  Serial.println("Making a call...");
  ATCommandOK("ATD" + String(phoneNumber) + ";\r\n", true);  // Command to make a call
  delay(1000);  // Wait for the call to be initiated
}

// Function to attend an incoming call
void attendCall() {
  Serial.println("Attending the call...");
  ATCommandOK("ATA\r\n", true);  // Command to answer the call
}

// Function to send an SMS
void sendSMS(const char* phoneNumber, const char* message) {
  Serial.println("Sending an SMS...");
  ATCommandOK("AT+CMGF=1\r\n", true);  // Set SMS text mode
  sendBlockingATCommand("AT+CMGS=\"" + String(phoneNumber) + "\"\r\n", true);  // Set SMS recipient
  Serial2.println(message);  // Send the message
  Serial2.write(26);  // Ctrl+Z to send the message
}

// Function to send an AT command and wait for 'OK' or 'ERROR' response
bool ATCommandOK(String cmd, bool extendTime) {
  Serial.print("M ");
  Serial.print(cmd);
  Serial2.print(cmd);

  unsigned long t = millis();
  String str;
  unsigned long int t2 = 4000L;  // Default timeout: 4 seconds
  if (extendTime)
    t2 = 6000L;  // Extended timeout: 6 seconds
  while (millis() - t < t2) {
    if (Serial2.available() > 0) {
      str = ReadString();
      if (str.indexOf("OK") != -1)
        return true;  // 'OK' received
      else if (str.indexOf("ERROR") != -1)
        return false;  // 'ERROR' received
    }
  }
  Serial.println("Timeout 1");  // Timeout message
  return false;                  // Timeout occurred
}

// Function to send an AT command and wait for '>' response
bool sendBlockingATCommand(String cmd, bool extendTime) {
  Serial.print("M ");
  Serial.print(cmd);
  Serial2.print(cmd);

  unsigned long t = millis();
  String str;
  unsigned long int t2 = 4000L;  // Default timeout: 4 seconds
  if (extendTime)
    t2 = 6000L;  // Extended timeout: 6 seconds
  while (millis() - t < t2) {
    if (Serial2.available() > 0) {
      str = ReadString();
      if (str.indexOf(">") != -1)
        return true;  // '>' received
      else if (str.indexOf("ERROR") != -1)
        return false;  // 'ERROR' received
    }
  }
  Serial.println("Timeout 2");  // Timeout message
  return false;                  // Timeout occurred
}

// Function to read a string from serial
String ReadString() {
  String str = "";
  if (Serial2.available() > 0) {
    str = Serial2.readStringUntil('\n');
    Serial.print("C ");
    Serial.println(str);
  }
  return str;
}
