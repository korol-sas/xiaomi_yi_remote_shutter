#include "ESP8266WiFi.h"
  
WiFiClient client;

// LED status indicator settings
const int ledPin = LED_BUILTIN;
const long ledBlinkRecordingInterval = 1500; 
const long ledBlinkVideoModeInterval = 500; 

// LED status indicator params
int ledState = LOW;
unsigned long previousLedMillis = 0;

// Camera params
bool haveCameraConnection = false;
bool isVideoMode = false;
bool isRecording = false;
String token = "";
unsigned long lastActionTime = 0;

void setWiFiSettings() {
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(100);
}

String searchCameraSSID() {
  setWiFiSettings();
  
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    if (WiFi.SSID(i).startsWith("YDXJ_"))
      return WiFi.SSID(i);

    delay(10);
  }

  return "";
}

bool connectToCamera(String SSID) {
  char ssid[30];
  char *password = "1234567890";
  short retriesRemaining = 30;

  SSID.toCharArray(ssid, SSID.length() + 1);

  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    if (retriesRemaining == 0)
      return false;
    delay(500);
    retriesRemaining--;
  }

  return true;
}

String readResponse(String searchKey) {
  // Give the server time to respond, then read the entire reply
  delay(2000);
  String response;
  while (client.available()) {
    char character = client.read();
    response.concat(character);
  }
  
  String searchString = "\""+searchKey+"\":";
  
  int offset = response.indexOf(searchString);
  if (offset == -1) 
    return "";

  // TODO Add wrapper to all requests
  // if rval is -4  
  // request new token and repeat request

  String searchResult;
  for (int i = offset + searchString.length(); i < response.length(); ++i) {
    char c = response.charAt(i);
    if ((c == ',') || (c == '}') ) 
      break;
    else if (c == '"') 
      continue;
    else 
      searchResult.concat(response.charAt(i));
  }

  searchResult.trim();
  
  return searchResult;
}

String requestToken() {
  client.print("{\"msg_id\":257,\"token\":0}\n\r");

  String response = readResponse("param");

  Serial.print(response);
  Serial.println();

  return response;
}

void fetchDeviceMode(String token) {
  Serial.println("Request camera mode");
  
  clearIncoming();
  
  client.print("{\"msg_id\":3,\"token\":");
  client.print(token);
  client.print("}\n\r");

  String response = readResponse("system_mode");

  Serial.println("Current mode: " + response);
  
  if (response != "record" && response != "capture") {
    delay(1000);
    fetchDeviceMode(token);
  }

  isVideoMode = response == "record";
}

void clearIncoming() {
  while (client.available()) {
    char character = client.read();
    Serial.print(character);
    if ( character == '}' )
      Serial.println();
  }
}

void takePhoto(String token) {
  clearIncoming();

  client.print("{\"msg_id\":769,\"token\":");
  client.print(token);
  client.print("}\n\r");

  clearIncoming();
}

void startRecording(String token) {
  clearIncoming();

  client.print("{\"msg_id\":513,\"token\":");
  client.print(token);
  client.print("}\n\r");

  clearIncoming();

  isRecording = true;
}

void stopRecording(String token) {
  clearIncoming();

  client.print("{\"msg_id\":514,\"token\":");
  client.print(token);
  client.print("}\n\r");

  clearIncoming();

  isRecording = false;
}

void setVideoMode(String token) {
  isVideoMode = true;
}

void setPhotoMode(String token) {
  if (isVideoMode && isRecording ) {
     stopRecording(token);
  }

  isVideoMode = false;

  setLedState(false);
}

void keepalive(String token) {
  clearIncoming();

  client.print("{\"msg_id\":13,\"token\":");
  client.print(token);
  client.print("}\n\r");

  clearIncoming();
}

void setLedState(bool state) {
  ledState = state ? LOW : HIGH;
  
  digitalWrite(ledPin, ledState);
}

void connectToCamera() {
  String cameraSSID = searchCameraSSID();
  if (cameraSSID.length() == 0) 
    return;

  Serial.println("Found network " + cameraSSID);
    
  if (!connectToCamera(cameraSSID))
    return;

  Serial.println("Connecting to network...");

  if (!client.connect("192.168.42.1", 7878))
    return;

  Serial.println("Connected to camera control server");

  token = requestToken();
  if (token.length() == 0) {
     Serial.println("Failed receive request token");
     return;
  }

  Serial.println("Received token: " + token);

  // Give the server time to storage token
  delay(1500);

  fetchDeviceMode(token);

  haveCameraConnection = true;

  setLedState(false);
}


void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);

  setLedState(true);

  // TODO set pin modes and use for two aux pins (from RC and PixHawk)
  
  Serial.println("\nStarted");
}

void loopLed() {
  if (isRecording || isVideoMode) {
    unsigned long currentLedMillis = millis();
  
    if (currentLedMillis - previousLedMillis >= (isRecording ? ledBlinkRecordingInterval : ledBlinkVideoModeInterval)) {
      previousLedMillis = currentLedMillis;

      setLedState(ledState == LOW ? false : true);
    }
  }
}

void loopConnection() {
  if ( !haveCameraConnection ) {
    connectToCamera();
  } else {
    unsigned long now = millis();
    if (now - lastActionTime > (1 * 60000)) {
      keepalive(token);
      lastActionTime = millis();
    }
  }
}

void loopCommands() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    Serial.print("I received command: ");
    Serial.println(cmd);

    if (cmd == "params") {
      fetchDeviceMode(token);
    }
    if (cmd == "set_video") {
      setVideoMode(token);
    }
    if (cmd == "set_photo") {
      setPhotoMode(token);
    }
    if (cmd == "trigger_rc") {
      if (isVideoMode) {
        if (isRecording) {
          stopRecording(token);
        } else {
          startRecording(token);
        }
      } else {
        takePhoto(token);
      }
    }
    if (cmd == "trigger_pix") {
      if (isVideoMode) {
        if (!isRecording) {
          startRecording(token); 
        }
      } else {
        takePhoto(token);
      }
    }
  }
}

void loop() {
  loopLed();
  loopConnection();
  loopCommands();
}
