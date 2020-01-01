#include <DmxSimple.h>
#include <JC_Button.h>

#define DEBUG 1

struct BUTTON {
  const char name[10];
  Button button;
  byte lampPins[5];
  byte dmx[5];
  boolean state;

} buttons[] = {
  {"Tobi 1    ", Button(4 ), {40}, {}, false},
  {"Tobi 2    ", Button(5 ), {41}, {}, false},

  {"Korytarz  ", Button(6 ), {}, {1, 2, 3} , false},
  {"Garderoba ", Button(7 ), {}, {4, 5} , false},
  {"Łazienka  ", Button(9 ), {}, {7, 8, 9} , false},
  {"Kuchnia   ", Button(17), {}, {10, 11, 12} , false},

  {"Sypialnia ", Button(14), {42}, {}, false},
  {"Salon     ", Button(16), {43, 44}, {}, false},
  {"Stół      ", Button(15), {45}, {}, false},


  {"Drzwi     ", Button(A1), {50}, {}, false},

};

const byte buttonsCount = (sizeof(buttons) / sizeof(BUTTON));

int dmxDefault = 255;

Button onOff = Button(A0);

void setup() {
  if (DEBUG) {
    Serial.begin(9600);
  }
  onOff.begin();
  for (int i = 0; i < buttonsCount; i++) {
    buttons[i].button.begin();
    byte lampPins = pinsCount(i);
    for (int j = 0; j < lampPins; j++) {
      pinMode(buttons[i].lampPins[j], OUTPUT);
      digitalWrite(buttons[i].lampPins[j], buttons[i].state); //TODO dmx initialization
    }
  }
  DmxSimple.usePin(3);
  DmxSimple.write(1, 0);
}

void loop() {
  for (int i = 0; i < buttonsCount; i++) {
    buttons[i].button.read();
    if (buttons[i].button.wasReleased()) {
      buttonPressed(i);
    }
  }
  onOff.read();
  if (onOff.wasReleased()) {
    if (DEBUG) Serial.println("onOff");
    if (anyOn()) {
      setAll(false);
    } else {
      setAll(true);
    }
  }
}

void buttonPressed(int i) {
  if (DEBUG) Serial.println(buttons[i].name);
  buttons[i].state = !buttons[i].state;
  updateState(i);
}

void updateState(int i) {
  byte lampPins = pinsCount(i);
  for (int j = 0; j < lampPins; j++) {
    digitalWrite(buttons[i].lampPins[j], buttons[i].state);
  }

  byte dmxPins = dmxCount(i);
  for (int j = 0; j < dmxPins; j++) {
    if (buttons[i].state) {
      DmxSimple.write(buttons[i].dmx[j], dmxDefault);
    } else {
      DmxSimple.write(buttons[i].dmx[j], 0);
    }
  }
}

bool anyOn() {
  for (int i = 0; i < buttonsCount; i++) {
    if (buttons[i].state) {
      return true;
    }
  }
  return false;
}

void setAll(boolean state) {
  for (int i = 0; i < buttonsCount; i++) {
    buttons[i].state = state;
    updateState(i);
  }
}

byte pinsCount(int i) {
  return sizeof(buttons[i].lampPins) / sizeof(byte);
}

byte dmxCount(int i) {
  return sizeof(buttons[i].lampPins) / sizeof(byte);
}
