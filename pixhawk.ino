// Signals params
const int togglePixhawkPin = D5;
const int toggleRcPin = D6;
const int modeRcPin = D7;

const int neutalPinOutput = 988;

volatile int togglePixhawkValue = neutalPinOutput;
volatile int toggleRcValue = neutalPinOutput;
volatile int modeRcValue = neutalPinOutput;

volatile unsigned long togglePixhawkStartPeriod = 0;
volatile unsigned long toggleRcStartPeriod = 0;
volatile unsigned long modeRcStartPeriod = 0;

volatile boolean togglePixhawkNewSignal = false;
volatile boolean toggleRcNewSignal = false;
volatile boolean modeRcNewSignal = false;

void ICACHE_RAM_ATTR calcTogglePixhawkValue()
{
  if (digitalRead(togglePixhawkPin) == HIGH)
  {
    togglePixhawkStartPeriod = micros();
  }
  else
  {
    if (togglePixhawkStartPeriod && (togglePixhawkNewSignal == false))
    {
      togglePixhawkValue = (int)(micros() - togglePixhawkStartPeriod);
      togglePixhawkStartPeriod = 0;
      togglePixhawkNewSignal = true;
    }
  }
}

void ICACHE_RAM_ATTR calcToggleRcValue()
{
  if (digitalRead(toggleRcPin) == HIGH)
  {
    toggleRcStartPeriod = micros();
  }
  else
  {
    if (toggleRcStartPeriod && (toggleRcNewSignal == false))
    {
      toggleRcValue = (int)(micros() - toggleRcStartPeriod);
      toggleRcStartPeriod = 0;
      toggleRcNewSignal = true;
    }
  }
}

void ICACHE_RAM_ATTR calcModeRcValue()
{
  if (digitalRead(modeRcPin) == HIGH)
  {
    modeRcStartPeriod = micros();
  }
  else
  {
    if (modeRcStartPeriod && (modeRcNewSignal == false))
    {
      modeRcValue = (int)(micros() - modeRcStartPeriod);
      modeRcStartPeriod = 0;
      modeRcNewSignal = true;
    }
  }
}

void setupPixhawk() {
  pinMode(togglePixhawkPin, INPUT);
  pinMode(toggleRcPin, INPUT);
  pinMode(modeRcPin, INPUT);
  
  attachInterrupt(togglePixhawkPin, calcTogglePixhawkValue, CHANGE);
  attachInterrupt(toggleRcPin, calcToggleRcValue, CHANGE);
  attachInterrupt(modeRcPin, calcModeRcValue, CHANGE);
}

void loopPixhawk()
{
  if (togglePixhawkNewSignal)
  {
     if( togglePixhawkValue > 1600 ) {
        Serial.println(togglePixhawkValue); 
        triggerPixhawk();
     }
     togglePixhawkNewSignal = false;
  }

  if (toggleRcNewSignal)
  {
     if( toggleRcValue > 1600 ) {
        Serial.println(toggleRcValue); 
        triggerRc();
     }
     toggleRcNewSignal = false;
  }
}
