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
  0b10010000,  // 9
  };

constexpr int positionsCount = 4;
constexpr int maxDigit = 10;
constexpr byte initialPosition = 0;
constexpr byte initialNumber = 0;

constexpr byte emptyGlyphMask =  0b11111111;
constexpr byte allPositionsMask = (1 << positionsCount) - 1;
constexpr byte decimalPointMask = 0b01111111;

// button constants
constexpr int buttonPins[] = { button1_pin, button2_pin, button3_pin };
constexpr int buttonsCount = sizeof(buttonPins) / sizeof(buttonPins[0]);
constexpr int incrementButton = 0;
constexpr int decrementButton = 1;
constexpr int positionChangeButton = 2;

constexpr unsigned long buttonDebounceInterval = 10;

// Stopwatch constants
constexpr int initialTime = 0; 
constexpr int timeDecimalPlaces = 1;
constexpr int timeStartButton = 0;
constexpr int timeLapButton = 1;
constexpr int timeResetButton = 2;
constexpr int millisInOneSecond = 1000;
int timeRoundOffDivisor = millisInOneSecond / calculatePower(maxDigit, timeDecimalPlaces);

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

class Display {

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

  byte getPosition(int position) {
    return 1 << (positionsCount - 1 - position);
  }

  void showEmptyGlyph(int position) {
    byte positionMask = getPosition(position);

    showGlyph(emptyGlyphMask, positionMask);
  }

  void showDigit(int digit, int position, bool showDecimalPoint = false) {
    byte glyphMask = digitGlyphs[digit];
    byte positionMask = getPosition(position);

    if (showDecimalPoint) {
      glyphMask &= decimalPointMask;
    }

    showGlyph(glyphMask, positionMask);
  }
};

class NumericDisplay : public Display {

private:
  int number_;
  int highestOrder_;
  int decimalPlaces_;
  int currentPosition_;
  bool isDecimal_;
  bool isON_;

  void calculateHighestOrder() {
      highestOrder_ = 0;
      int calculatedNumber = 1;
      while (calculatedNumber <= number_) {
          ++highestOrder_;
          calculatedNumber *= maxDigit;
      }
  }

  int getPositionDigit() {
    int divisor = calculatePower(maxDigit, currentPosition_);
    int digit = (number_ / divisor) % maxDigit;
    return digit;
  }


public:
  void initialize() {
    Display::initialize();
    currentPosition_ = positionsCount;
  }

  void turnOff() {
    isON_ = false;
    showGlyph(emptyGlyphMask, emptyGlyphMask); // Turn off all segments
  }

  void setInteger(int number) {
    isON_ = true;
    number_ = number;
    decimalPlaces_ = 0;
    isDecimal_ = false;
    calculateHighestOrder();
  }

  void setDecimal(int number, int decimalPlaces) {
    isON_ = true;
    number_ = number;
    decimalPlaces_ = decimalPlaces;
    isDecimal_ = true;
    calculateHighestOrder();
  }

  void update() {
    if (isON_) {
      currentPosition_ = (currentPosition_ - 1 + positionsCount) % positionsCount;
      int currentPositionDigit = getPositionDigit();
      bool showDot = (currentPosition_ == decimalPlaces_ && isDecimal_) ? true : false;

      if (currentPosition_ < highestOrder_ || currentPosition_ <= decimalPlaces_) {
        showDigit(currentPositionDigit, currentPosition_, showDot);
      }
    }
  }
};

NumericDisplay numericDisplay;

class Stopwatch {

  private:
    bool running;
    bool lapped;
    bool firstStart;
    int decimalPlaces_;
    unsigned long displayedTime;
    unsigned long timeDifference;

  public:
    void initialize(int decimalPlaces) {
      decimalPlaces_ = decimalPlaces;
      displayedTime = 0;
      numericDisplay.setDecimal(displayedTime, decimalPlaces_);
    }

    void start() {
      if (lapped == false) {
        running = !running;
      }
    }

    void lap() {
      if (running) {
        lapped = !lapped;
      }
    }

    void reset() {
      if (!running && !lapped) {
        initialize(timeDecimalPlaces);
      }
    }

    void detectButtonPress(unsigned long currentMillis) {
      if (buttons[timeStartButton].press(currentMillis)) {
        start();
      }
      if (buttons[timeLapButton].press(currentMillis)) {
        lap();
      }
      if (buttons[timeResetButton].press(currentMillis)) {
        reset();
      }
    }

    void update(unsigned long currentMillis) {
      detectButtonPress(currentMillis);

      unsigned long actualTime = currentMillis;

      if (running && lapped != true) {
        displayedTime = (actualTime - timeDifference);
        numericDisplay.setDecimal(displayedTime / timeRoundOffDivisor, timeDecimalPlaces);
      }
      else if (lapped != true){
        timeDifference = actualTime - displayedTime;
      }
    }
};

Stopwatch stopwatch;

void setup() {
  // initializing Button class objects
  for (int i = 0; i < buttonsCount; ++i) {
    buttons[i].initialize(i);
  }

  // initializing display object
  numericDisplay.initialize();

  // initializing stopwatch object
  stopwatch.initialize(timeDecimalPlaces);
}

void loop() {
  unsigned long currentMillis = millis();

  stopwatch.update(currentMillis);
  numericDisplay.update();
}
