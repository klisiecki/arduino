#include <DmxSimple.h>
#include <JC_Button.h>

#define DEBUG 0

struct BUTTON {
  const char name[10];
  Button button;
  short lampPin;
  short dmx;
  boolean state;

} buttons[] = {
  {"Tobi 1    ", Button(4 ), 40, -1, false},
  {"Tobi 2    ", Button(5 ), 41, -1, false},
  {"Korytarz  ", Button(6 ), -1, 1 , false},

  {"Sypialnia ", Button(14), 42, -1, false},
  {"Salon 1   ", Button(16), 43, -1, false},


};

const byte buttonsCount = (sizeof(buttons) / sizeof(BUTTON));

void setup() {
  if (DEBUG) {
    Serial.begin(9600);
  }
  for (int i = 0; i < buttonsCount; i++) {
    buttons[i].button.begin();
    if (buttons[i].lampPin > 0) {
      pinMode(buttons[i].lampPin, OUTPUT);
    }
    digitalWrite(buttons[i].lampPin, true);
  }
  DmxSimple.usePin(3);
}

void loop() {
  for (int i = 0; i < buttonsCount; i++) {
    buttons[i].button.read();
    if (buttons[i].button.wasReleased()) {
      if (DEBUG) Serial.println(buttons[i].name);
      buttons[i].state = !buttons[i].state;
      if (buttons[i].lampPin > 0) {
        digitalWrite(buttons[i].lampPin, buttons[i].state);
      } else {
        if (buttons[i].state) {
          DmxSimple.write(buttons[i].dmx, 255);
        } else {
          DmxSimple.write(buttons[i].dmx, 0);
        }
      }
    }
  }

}
