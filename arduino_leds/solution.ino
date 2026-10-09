#include "funshield.h"

// global constants
constexpr int diodePins[] = { led1_pin, led2_pin, led3_pin, led4_pin };  // indexing diodes
constexpr int diodesCount = sizeof(diodePins) / sizeof(diodePins[0]) ;
constexpr int interval = 300;

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

  void updateTimeStamp(unsigned long newTimeStamp) {
    previousMillis = newTimeStamp;
  }
};

// defining class objects
Diode diodes[diodesCount];
Timer timer;

// defining global variables for animation function
int activeDiode = 0;
bool movingDown = true;

void animation(unsigned long currentTime) {

  if (timer.timeUp(currentTime)) {
    diodes[activeDiode].change(false);

    if (movingDown) {
      ++activeDiode;  // Go one diode down and increment the activeDiode value simultaneously
      if (activeDiode == diodesCount - 1) {
        movingDown = false;
      }
    }
    else {
      activeDiode--;  // Go one diode up and decrement the activeDiode value simultaneously
      if (activeDiode == 0) {
        movingDown = true;
      }
    }
    diodes[activeDiode].change(true);
  }
}

void setup() {
  // initializing diode class objects
  for (int i=0; i < diodesCount; i++) {
    diodes[i].initialize(i);
  }
  
  // initializing Timer class object
  timer.changeInterval(interval);
  timer.updateTimeStamp(millis());
}

void loop() {
  animation(millis());
}
