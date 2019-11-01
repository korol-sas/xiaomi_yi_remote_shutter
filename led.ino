// LED status indicator settings
const int ledPin = LED_BUILTIN;
const long ledBlinkRecordingInterval = 1500; 
const long ledBlinkVideoModeInterval = 500; 

// LED status indicator params
int ledState = LOW;
unsigned long previousLedMillis = 0;

void setLedState(bool state) {
  ledState = state ? LOW : HIGH;
  
  digitalWrite(ledPin, ledState);
}

void setupLed() {
  pinMode(ledPin, OUTPUT);
  setLedState(true);
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
