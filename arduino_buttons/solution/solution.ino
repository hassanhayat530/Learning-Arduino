#include "funshield.h"

// global constants

// button constants
constexpr int buttonPins[] = { button1_pin, button2_pin, button3_pin };
constexpr int buttonsCount = sizeof(buttonPins) / sizeof(buttonPins[0]);
constexpr int incrementButton = 0;
constexpr int decrementButton = 1;

const unsigned long buttonDebounceInterval = 10;
const unsigned long buttonInitialHoldDelay = 1000;
const unsigned long buttonRepeatHoldDelay = 300;

// diode constants
constexpr int diodePins[] = { led1_pin, led2_pin, led3_pin, led4_pin };  // indexing diodes
constexpr int diodesCount = sizeof(diodePins) / sizeof(diodePins[0]) ;  

// Counter constants
constexpr int maxCount = 1 << diodesCount;
constexpr int initialValue = 0;

// classes
class Timer {

private:
  unsigned long interval;
  unsigned long previousMillis;

public:
  void changeInterval(unsigned long millis) {
    interval = millis;
  }

  bool timeUp(unsigned long currentMillis) {
    if (currentMillis - previousMillis >= interval) {
      previousMillis += interval;
      return true;
    }
    return false;
  }

  bool timePassed(unsigned long currentMillis) {
    if (currentMillis - previousMillis >= interval) {
      return true;
    }
    return false;
  }

  void updateTimeStamp(unsigned long newTimeStamp) {
    previousMillis = newTimeStamp;
  }
};

class Button {

private:
  int buttonNumber_;
  bool lastButtonState;
  bool buttonHold;
  Timer debounceTimer;
  Timer HoldTimer;

public:
  void initialize(int buttonNumber) {
    buttonNumber_ = buttonNumber;
    pinMode(buttonPins[buttonNumber_], INPUT);
    lastButtonState = read();
    buttonHold = false;
    debounceTimer.changeInterval(buttonDebounceInterval);
  }

  bool read() {
    bool reading = (digitalRead(buttonPins[buttonNumber_])) == LOW;
    return reading;
  }

  bool press(unsigned long currentMillis) {
    bool reading = read();
    bool pressed = false;

    // Check if the button state has changed and passed debounce interval
    if (reading != lastButtonState && debounceTimer.timeUp(currentMillis)) {
      debounceTimer.updateTimeStamp(currentMillis);
      lastButtonState = reading;

      if (reading) {
        buttonHold = true;
        pressed = true;
        HoldTimer.updateTimeStamp(currentMillis);
      }
      else {
        buttonHold = false;
      }
    }

    if (buttonHold) {
      if (HoldTimer.timePassed(currentMillis)) {
        HoldTimer.changeInterval(buttonRepeatHoldDelay);
        HoldTimer.updateTimeStamp(currentMillis);
        pressed = true;
      }
    }
    else HoldTimer.changeInterval(buttonInitialHoldDelay);

    return pressed;
  }
};

class Diode {

private:
  int diodeNumber_;
  bool currentState_;

public:
  void initialize(int diodeNumber) {
    diodeNumber_ = diodeNumber;
    pinMode(diodePins[diodeNumber_], OUTPUT);
    change(false);
  }

  void change(bool newState) {
      currentState_ = newState;
      
      digitalWrite(diodePins[diodeNumber_], currentState_ ? LOW : HIGH);
  }

  bool getCurrentState() {
    return currentState_;
  }
};

// Button objects defined here to be used in Counter class
Button buttons[buttonsCount];

class Counter {

private:
  int value_;
  int previousValue;

public:
  void initialize(int value) {
    value_ = value;
  }

  void incrementValue() {
    value_ = (value_ + 1) % maxCount;
  }

  void decrementValue() {
    value_ = (value_ - 1 + maxCount) % maxCount;
  }

  bool isValueChanged() {
    if (value_ != previousValue) {
      previousValue = value_;
      return true;
    }
    return false;
  }

  int getValue() {
    return value_;
  }

  void changeValue (unsigned long currentMillis) {
    if (buttons[incrementButton].press(currentMillis)) {
      incrementValue();
    }

    if (buttons[decrementButton].press(currentMillis)) {
      decrementValue();
    }
  }
};

// Defining class objects
Diode diodes[diodesCount];
Counter counter;

void ledDisplay() {
  int value = counter.getValue();


  for (int i = 0; i < diodesCount; ++i) {
    bool diodeState = ((value >> (diodesCount - 1 - i) & 0x01) == 1) ? true : false;

    if (diodeState != diodes[i].getCurrentState()) {
      diodes[i].change(diodeState);
    }
  }
}


void setup() {
  // initializing Diode class objects
  for (int i = 0; i < diodesCount; ++i) {
    diodes[i].initialize(i);
  }

  // initializing Button class objects
  for (int i = 0; i < buttonsCount; ++i) {
    buttons[i].initialize(i);
  }

  // initializing Counter class object
  counter.initialize(initialValue);
  ledDisplay();
}

void loop() {
  counter.changeValue(millis());
  
  if (counter.isValueChanged()) {
    ledDisplay();
  }
}
