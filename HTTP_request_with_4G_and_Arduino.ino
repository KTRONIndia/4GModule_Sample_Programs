#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3);

const char* HTTP_PAYLOAD = "Enter your message";  // HTTP payload
const char* Post_URL = "https://postman-echo.com/post";  // Post URL . you can change it with your URL
const char* Get_URL = "https://www.google.com";  // Get URL. you can change it with your URL

bool exec = false;
bool baud = false;


void setup() {
  mySerial.begin(115200);  // Setting the baud rate of A7670 Module
  Serial.begin(19200);
  delay(1000);

  
}

void loop() {
  if (millis() >= 3000) {
    if (!baud) {
      ATCommandOK("AT+IPR=19200\r\n", true);
      mySerial.end();
      mySerial.begin(19200);
      baud = true;
      Serial.println("Press 'a' to Start HTTP");
      Serial.println("Press 'g' to make GET request");
      Serial.println("Press 'p' to make POST request");
      Serial.println("Press 'r' to Stop HTTP");
    }
  }
  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {    // Check for user input from Serial Monitor
      case 'a':
        // = false;
        startHTTP();
        break;
      case 'g':
        //ubscribe = false;
        HTTP_get_data();
        break;
      case 'p':
        HTTP_post_data();
        break;
      case 'r':
     
        stopHTTP();
        break;
    }
  }
 
}


// Function to send POST request
void HTTP_post_data() {
  ATCommandOK("AT+HTTPPARA=\"URL\",\"" + String(Post_URL) + "\"\r\n", true);
  delay(100);
  String payloadCommand = "AT+HTTPDATA=" + String(strlen(HTTP_PAYLOAD)) + ",1000\r\n";
  if (sendBlockingATCommand(payloadCommand, true)) {
    if (ATCommandOK(HTTP_PAYLOAD, true)) {
      if (ATCommandOK("AT+HTTPACTION=1\r\n", true)) {
        // Check for incoming HTTP responses until +HTTPACTION response is received
        checkIncomingHTTPResponse();
      }
    }
  }
}


// Function to send get request
void HTTP_get_data() {
  ATCommandOK("AT+HTTPPARA=\"URL\",\"" + String(Get_URL) + "\"\r\n", true);
  delay(1000);
  if (ATCommandOK("AT+HTTPACTION=0\r\n", true)){
   
    checkIncomingHTTPResponse();
  }
  
}

// Function to initialize HTTP connection
void startHTTP() {
  ATCommandOK("AT\r\n", true);
  ATCommandOK("ATE0\r\n", true);
  ATCommandOK("AT+HTTPINIT\r\n", true);
  delay(1000);
}


// Function to terminate HTTP connection
void stopHTTP() {
  ATCommandOK("AT+HTTPTERM\r\n", true);
  Serial.println("HTTP stopped successfully");
}


// Function to send AT commands and wait for the response "DOWNLOAD"
bool sendBlockingATCommand(String cmd, bool extendTime) {
  Serial.print("M ");
  Serial.print(cmd);
  mySerial.print(cmd);

  unsigned long t = millis();
  String str;
  unsigned long int t2 = 4000L;
  if (extendTime)
    t2 = 6000L;
  while (millis() - t < t2) {
    if (mySerial.available() > 0) {
      str = ReadString();
      if (str.indexOf("DOWNLOAD") != -1)
        return true;
      else if (matchString(str, "ERROR\r") == true)
        return false;
    }
  }
  Serial.println("time out 2");
  return false;
}

// Function to send AT commands and wait for the response "OK"
bool ATCommandOK(String cmd, bool extendTime) {
  Serial.print("M ");
  Serial.print(cmd);
  mySerial.print(cmd);

  unsigned long t = millis();
  String str;
  unsigned long int t2 = 6000L;
  if (extendTime)
    t2 = 8000L;
  while (millis() - t < t2) {
    if (mySerial.available() > 0) {
      str = ReadString();
      if (matchString(str, "OK\r") == true)
        return true;
      else if (matchString(str, "ERROR\r") == true)
        return false;
    }
  }
  Serial.println("time out 1");
  return false;
}

// Function to read response from Serial
String ReadString() {
  String str = "";
  if (mySerial.available() > 0) {
    str = mySerial.readStringUntil('\n');
    Serial.print("C ");
    Serial.println(str);
  }
  return str;
}

// Function to check if two strings match
bool matchString(String m1, String m2) {
  return (m1 == m2);
}

// Function to continuously read and process incoming HTTP responses until +HTTPACTION response is received
void checkIncomingHTTPResponse() {
  String response;
  while (!response.startsWith("+HTTPACTION")) {
    response = ReadString();
   
  }

}