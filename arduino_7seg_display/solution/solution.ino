#include "funshield.h"

int calculatePower(int base, int power) {
  int number = 1;
  for (int i = 0; i < power; ++i) {
    number *= base;
  }
  return number;
}

// Display constants
constexpr byte digitGlyphs[] = {
  0b11000000, // 0
  0b11111001, // 1
  0b10100100, // 2
  0b10110000, // 3
  0b10011001, // 4
  0b10010010, // 5
  0b10000010, // 6
  0b11111000, // 7
  0b10000000, // 8
  0b10010000  // 9
  };

constexpr int positionsCount = 4;
constexpr byte initialPosition = 0;
constexpr byte initialNumber = 0;

constexpr byte emptyGlyphMask =  0b11111111;
constexpr byte allPositionsMask = (1 << positionsCount) - 1;

// button constants
constexpr int buttonPins[] = { button1_pin, button2_pin, button3_pin };
constexpr int buttonsCount = sizeof(buttonPins) / sizeof(buttonPins[0]);
constexpr int incrementButton = 0;
constexpr int decrementButton = 1;
constexpr int positionChangeButton = 2;

constexpr unsigned long buttonDebounceInterval = 10;

// diode constants
constexpr int diodePins[] = { led1_pin, led2_pin, led3_pin, led4_pin };  // indexing diodes
constexpr int diodesCount = sizeof(diodePins) / sizeof(diodePins[0]) ;

// counter constants
constexpr int maxDigit = 10;
int maxCounterValue = calculatePower(maxDigit, positionsCount);

class Display {
private:

public:
    void showGlyph(byte glyphMask, byte positionMask) {
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, glyphMask);
    shiftOut(data_pin, clock_pin, MSBFIRST, positionMask);
    digitalWrite(latch_pin, HIGH);
  }

  void initialize () {
    pinMode(latch_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);
    pinMode(data_pin, OUTPUT);

    showGlyph(emptyGlyphMask, allPositionsMask);
  }

  void showDigit(int digit, int position) {
    byte glyphMask = digitGlyphs[digit];
    byte positionMask = 1 << (positionsCount - 1 - position);
    showGlyph(glyphMask, positionMask);
  }
};

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
  Timer debounceTimer;

public:
  void initialize(int buttonNumber) {
    buttonNumber_ = buttonNumber;
    pinMode(buttonPins[buttonNumber_], INPUT);
    lastButtonState = read();
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
        pressed = true;
      }
    }

    return pressed;
  }
};

Button buttons[buttonsCount];
Display display;

class Counter {

private:
  int value_;
  int limit_;
  int currentPosition_;

public:
  void initialize(int value, int limit, int currentPosition) {
    value_ = value;
    limit_ = limit;
    currentPosition_ = currentPosition; 
  }

  void changePosition() {
    currentPosition_ = (currentPosition_ + 1) % positionsCount;
  }

  void incrementPositionDigit() {
    int incrementNumber = calculatePower(maxDigit, currentPosition_);
    value_ = (value_ + incrementNumber) % limit_;
  }

  void decrementPositionDigit() {
    int decrementNumber = calculatePower(maxDigit, currentPosition_);
    value_ = (value_ - decrementNumber + limit_) % limit_; 
  }

  bool updateValue(unsigned long currentMillis) {
      bool valueChanged = false;

      if (buttons[positionChangeButton].press(currentMillis)) {
        changePosition();
        valueChanged = true;
      }
      
      if (buttons[decrementButton].press(currentMillis)) {
        decrementPositionDigit();
        valueChanged = true;
      }

      if (buttons[incrementButton].press(currentMillis)) {
        incrementPositionDigit();
        valueChanged = true;
      }

      return valueChanged;
  }

  int currentPositionDigit() {
      int displayValue = value_;
      int divisor = calculatePower(maxDigit, currentPosition_);
      return (displayValue / divisor) % maxDigit;
  }

  int getValue() {
    return value_;
  }

  int getPosition() {
    return currentPosition_;
  }

  void requestDisplayUpdate() {
    display.showDigit(currentPositionDigit(), getPosition());
  }
};

Counter valueCounter;

void setup() {

  // initializing Button class objects
  for (int i = 0; i < buttonsCount; ++i) {
    buttons[i].initialize(i);
  }

  // initializing display object
  display.initialize();
  valueCounter.requestDisplayUpdate();

  // initialzing counter objects
  valueCounter.initialize(initialNumber, maxCounterValue, initialPosition);
}

void loop() {
  unsigned long currentMillis = millis();

  if (valueCounter.updateValue(currentMillis)) {
    valueCounter.requestDisplayUpdate();
  }
}

