# Learning Arduino

Assignments from an Arduino course, built for an Arduino UNO with an attached Funshield. Each assignment lives in its own folder, which contains the source file (`.ino` or `.cpp`) together with any header the assignment uses.

## Assignments

| Folder | Description |
|---|---|
| `celmomether` | A plain C++ program (no Arduino needed) that prints an array of temperature readings as a horizontal bar thermometer, with `*` bars around a `\|` column for 0 degrees. Missing readings (`no_value`) repeat the last valid value. Reads its data from `temps.h`. |
| `arduino_leds` | Animates one lit LED bouncing up and down the four vertical LEDs, moving one step every 300 ms. |
| `arduino_buttons` | A 0 to 15 counter shown in binary on the four LEDs. Buttons 1 and 2 increment and decrement it, with auto-repeat when held (first after 1000 ms, then every 300 ms). |
| `arduino_7seg_display` | A 0 to 9999 counter shown one digit at a time on the 7-segment display. Button 3 selects the digit order, and buttons 1 and 2 add or subtract at that order. |
| `arduino_stopwatch` | A stopwatch with 0.1 s precision that supports start/stop, lap freeze, and reset, controlled by the three buttons. It shows all digits at once using time multiplexing. |
| `arduino_running_message` | Receives text over the USB serial link and scrolls it across the 7-segment display from right to left, one shift every 300 ms. Strings are handled with pointers. |

## Requirements

- Arduino UNO with a Funshield and the [Arduino IDE](https://www.arduino.cc/en/software)
- A standard C++ compiler (for `celmomether`)

## Notes

- Timing in all Arduino programs uses `millis()`. `delay()` and `delayMicroseconds()` are not used, so the main loop is never blocked.
- To test `arduino_running_message`, open the Serial Monitor in the Arduino IDE (Tools > Serial Monitor), type a message, and press Send.
