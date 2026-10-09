#include "funshield.h"
#include "input.h"

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

constexpr byte letterGlyphs[] {
  0b10001000,   // A
  0b10000011,   // b
  0b11000110,   // C
  0b10100001,   // d
  0b10000110,   // E
  0b10001110,   // F
  0b10000010,   // G
  0b10001001,   // H
  0b11111001,   // I
  0b11100001,   // J
  0b10000101,   // K
  0b11000111,   // L
  0b11001000,   // M
  0b10101011,   // n
  0b10100011,   // o
  0b10001100,   // P
  0b10011000,   // q
  0b10101111,   // r
  0b10010010,   // S
  0b10000111,   // t
  0b11000001,   // U
  0b11100011,   // v
  0b10000001,   // W
  0b10110110,   // ksi
  0b10010001,   // Y
  0b10100100,   // Z
};

constexpr int positionsCount = 4;
constexpr int maxDigit = 10;
constexpr byte initialPosition = 0;
constexpr byte initialNumber = 0;

constexpr byte emptyGlyphMask =  0b11111111;
constexpr byte allPositionsMask = (1 << positionsCount) - 1;
constexpr byte decimalPointMask = 0b01111111;

constexpr unsigned long runningMessagesInterval = 300;

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

  void updateTimeStamp(unsigned long newTimeStamp) {
    previousMillis = newTimeStamp;
  }
};

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

  byte getPositionMask(int position) {
    return 1 << (positionsCount - 1 - position);
  }

  void showEmptyGlyph(int position) {
    byte positionMask = getPositionMask(position);

    showGlyph(emptyGlyphMask, positionMask);
  }

  void showDigit(int digit, int position, bool showDecimalPoint = false) {
    byte glyphMask = digitGlyphs[digit];
    byte positionMask = getPositionMask(position);

    if (showDecimalPoint) {
      glyphMask &= decimalPointMask;
    }

    showGlyph(glyphMask, positionMask);
  }

  void showLetter(char letter, int position) {
    int letterIndex;

    if (isUpperCase(letter)) {
      letterIndex = letter - 'A';
    }
    else {
      letterIndex = letter - 'a';
    }

    byte letterMask = letterGlyphs[letterIndex]; 
    byte positionMask = getPositionMask(position);

    showGlyph(letterMask, positionMask);
  }

  void showChar(char symbol, int position) {
    if (isAlpha(symbol)) {
      showLetter(symbol, position);
    }

    else if (isDigit(symbol)) {
      int digit = symbol - '0';
      showDigit(digit, position);
    }

    else if (isSpace(symbol)) {
      showGlyph(emptyGlyphMask, getPositionMask(position));
    }
  }
};

class TextDisplay : public Display {

private:
  char text_[positionsCount];
  int currentPosition_;
  bool isON_;

public:
  void initialize() {
    Display::initialize();
    currentPosition_ = positionsCount;
  }

  void turnOff() {
    isON_ = false;
    showGlyph(emptyGlyphMask, emptyGlyphMask); // Turn off all segments
  }

  void setText(const char* string) {
    isON_ = true;
    const char* sourcePointer = string;
    char* targetPointer = text_;
  
    for (int i = positionsCount - 1; i >= 0; --i) {
      if (*sourcePointer != '\0') {
        *targetPointer++ = *sourcePointer++;
      }
      else {
        *targetPointer++ = ' ';
      }
    }
  }

  void update() {
    if (isON_) {
      currentPosition_ = (currentPosition_ - 1 + positionsCount) % positionsCount;
      showChar(text_[positionsCount - 1 - currentPosition_], currentPosition_);
    }
  }
};

TextDisplay textDisplay;

class RunningMessages {
private:
  const char* string_;
  char runningMessage_[positionsCount];
  int initialBlanks_;
  bool isFinished_;
  Timer timer;

  void scrollMessage(unsigned long currentMillis) {
    if (timer.timeUp(currentMillis)) {
      timer.updateTimeStamp(currentMillis);
      if (initialBlanks_ >= 0) {
        --initialBlanks_;
      }
      else {
        string_++;
      }
    }
  }

  void calculateRunningMessage() {
    const char* sourcePointer = string_;
    char* targetPointer = runningMessage_;

    for (int i = 0; i < positionsCount; ++i) {
      if (initialBlanks_ < i) {
        *targetPointer++ = *sourcePointer++;
      }
      else {
        *targetPointer++ = ' ';
      }
    }
  }

  void finished() {
    if (*string_ == '\0' && initialBlanks_ <= 0) {
      isFinished_ = true;
    }
  }

public:
  void initialize(unsigned long runningInterval) {
    string_ = "";
    timer.changeInterval(runningInterval);
  }

  void sendMessage(const char* string) {
    initialBlanks_ = positionsCount - 1;
    isFinished_ = false;
    string_ = string;
  }

  void displayMessage(unsigned long currentMillis) {

    if (!isFinished_) {
      calculateRunningMessage();
      textDisplay.setText(runningMessage_);
      scrollMessage(currentMillis);
      finished();
    }
  }

  bool getfinishedState() {
    return isFinished_;
  }
};

RunningMessages runningMessages;
SerialInputHandler serialInputHandler;

void setup() {
  serialInputHandler.initialize();

  textDisplay.initialize();

  runningMessages.initialize(runningMessagesInterval);
  runningMessages.sendMessage(serialInputHandler.getMessage());
}

void loop() {
  unsigned long currentMillis = millis();

  serialInputHandler.updateInLoop();

  if (runningMessages.getfinishedState()) {
    const char* message = serialInputHandler.getMessage();
    runningMessages.sendMessage(message);
  }

  runningMessages.displayMessage(currentMillis);
  textDisplay.update();
}

