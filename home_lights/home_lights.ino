#include <DmxSimple.h>
#include <JC_Button.h>

#define DEBUG 1

const byte ON = 255;
const byte OFF = 0;


enum LampType {
  PIN_LOW,
  PIN_HIGH,
  DMX
};

struct MyLamp {
  LampType type;
  byte channel;
  byte state;
};

MyLamp tobi1 = {PIN_HIGH, 40, OFF};
MyLamp tobi2 = {PIN_HIGH, 41, OFF};

MyLamp sypialnia = {PIN_HIGH, 42, OFF};
MyLamp salon1 = {PIN_HIGH, 43, OFF};
MyLamp salon2 = {PIN_HIGH, 44, OFF};
MyLamp stol = {PIN_HIGH, 45, OFF};

MyLamp drzwi = {PIN_LOW, 51, OFF};
MyLamp piwnica = {PIN_LOW, 52, OFF};

MyLamp korytarz1 = {DMX, 1, ON};
MyLamp korytarz2 = {DMX, 2, ON};
MyLamp korytarz3 = {DMX, 3, ON};

MyLamp garderoba1 = {DMX, 4, OFF};
MyLamp garderoba2 = {DMX, 5, OFF};

MyLamp lazienkaUmywalka = {DMX, 7, OFF};
MyLamp lazienkaWanna =    {DMX, 8, OFF};
MyLamp lazienkaReszta =   {DMX, 9, OFF};

MyLamp kuchnia1 = {DMX, 10};
MyLamp kuchnia2 = {DMX, 11};
MyLamp kuchnia3 = {DMX, 12};

MyLamp kuchniaListwa = {DMX, 13};

MyLamp *lamps[] = {
  &tobi1, &tobi2, &sypialnia, &salon1, &salon2, &stol, &drzwi, &piwnica,
  &korytarz1, &korytarz2, &korytarz3, &garderoba1, &garderoba2,
  &lazienkaUmywalka, &lazienkaWanna, &lazienkaReszta,
  &kuchnia1, &kuchnia2, &kuchnia3, &kuchniaListwa
};

const byte lampsCount = (sizeof(lamps) / sizeof(MyLamp));

struct MyButton {
  const char name[10];
  Button button;
  MyLamp *lamps[5];

} buttons[] = {
  {"Tobi 1    ", Button(4 ), {&tobi1} },
  {"Tobi 2    ", Button(5 ), {&tobi2} },

  {"Korytarz  ", Button(6 ), {&korytarz1, &korytarz2, &korytarz3} },
  {"Garderoba ", Button(7 ), {&garderoba1, &garderoba1} },
  {"Łazienka  ", Button(9 ), {&lazienkaUmywalka, &lazienkaWanna, &lazienkaReszta,} },
  {"Kuchnia   ", Button(17), {&kuchnia1, &kuchnia2, &kuchnia3} },

  {"Sypialnia ", Button(14), {&sypialnia} },
  {"Salon     ", Button(16), {&salon1, &salon2} },
  {"Stół      ", Button(15), {&stol} },

  {"Drzwi     ", Button(A1), {&drzwi} },

};

const byte buttonsCount = (sizeof(buttons) / sizeof(MyButton));

int dmxDefault = 255;

Button onOff = Button(A0);

void setup() {
  if (DEBUG) {
    Serial.begin(9600);
  }
  onOff.begin();
  for (int i = 0; i < buttonsCount; i++) {
    buttons[i].button.begin();
  }
  for (int i = 0; i < lampsCount; i++) {
    MyLamp lamp = *lamps[i];
    if (lamp.type == PIN_HIGH or lamp.type == PIN_LOW) {
      pinMode(lamp.channel, OUTPUT);
    }
  }
  DmxSimple.usePin(3);
  DmxSimple.write(1, 0);
}

void loop() {
  for (int i = 0; i < buttonsCount; i++) {
    buttons[i].button.read();
    if (buttons[i].button.wasReleased()) {
      buttonPressed(&buttons[i]);
    }
  }
  onOff.read();
  if (onOff.wasReleased()) {
    if (DEBUG) Serial.println("onOff");
    if (anyLamp(ON)) {
      setAll(false);
    } else {
      setAll(true);
    }
  }
}

void buttonPressed(MyButton *button) {
  if (DEBUG) Serial.println("Button pressed");
  if (DEBUG) Serial.println(button->name);
  byte lCount = countLamps(button);
  if (DEBUG) Serial.println("lamps: ");
  if (DEBUG) Serial.println(lCount);

  for (int i = 0; i < lCount; i++) {
    toggle(*(button->lamps) + i);
  }
}

void toggle(MyLamp *lamp) {
  if (lamp->state == ON) {
    setState(lamp, OFF);
  } else {
    setState(lamp, ON);
  }
}

void setState(MyLamp *lamp, byte state) {
  lamp->state = state;
  switch (lamp->type) {
    case PIN_HIGH:
      digitalWrite(lamp->channel, state == ON ? HIGH : LOW);
      break;
    case PIN_LOW:
      digitalWrite(lamp->channel, state == ON ? LOW : HIGH);
      break;
    case DMX:
      DmxSimple.write(lamp->channel, state == ON ? dmxDefault : 0);
      break;
  }
}


bool anyLamp(byte state) {
  for (int i = 0; i < lampsCount; i++) {
    if (DEBUG) Serial.println("Checking lamp:");
    if (DEBUG) Serial.println(lamps[i]->channel);
    if (lamps[i]->state == state) {
      if (DEBUG) Serial.println("Lamp on: " + i);
      return true;
    }
  }
  if (DEBUG) Serial.println("All lamps off");
  return false;
}

void setAll(byte state) {
  for (int i = 0; i < lampsCount; i++) {
    setState(lamps[i], state);
  }
}

byte countLamps(MyButton *button) {
  return sizeof(*button->lamps) / sizeof(*button->lamps[0]);
}
