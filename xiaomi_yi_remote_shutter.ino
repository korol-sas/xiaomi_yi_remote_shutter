
void setup() {
  Serial.begin(115200);
  
  setupLed();
  setupCamera();
  setupPixhawk();
  
  Serial.println("Started");
}

void loop() {
  loopLed();
  loopCamera();
  loopPixhawk();
}
