#include <Arduino.h>

#include <DmxSimple.h>
#include <JC_Button.h>

#define DEBUG 1

const byte STANDBY = 0;
const byte PRESSED = 1;
const byte DIMMING = 2;

byte dmxDefault = 200;
byte dmxNight = 3;

enum LampType {
  PIN_LOW,
  PIN_HIGH,
  DMX,
};

struct MyLamp {
  LampType type;
  byte channel;
  byte brightness;
};

MyLamp tobi1 = {PIN_HIGH, 40};
MyLamp tobi2 = {PIN_HIGH, 41};

MyLamp sypialnia = {PIN_HIGH, 42};
MyLamp salon1 = {PIN_HIGH, 43};
MyLamp salon2 = {PIN_HIGH, 44};
MyLamp stol = {PIN_HIGH, 46};

MyLamp drzwi = {PIN_LOW, 51};
MyLamp piwnica = {PIN_LOW, 50}; //spalony

MyLamp korytarz1 = {DMX, 1};
MyLamp korytarz2 = {DMX, 2};
MyLamp korytarz3 = {DMX, 3};

MyLamp garderoba1 = {DMX, 4};
MyLamp garderoba2 = {DMX, 5};

MyLamp lazienkaUmywalka = {DMX, 7};
MyLamp lazienkaWanna =    {DMX, 8};
MyLamp lazienkaReszta =   {DMX, 9};
MyLamp lazienkaLustro = {PIN_LOW, 52};

MyLamp kuchnia1 = {DMX, 10};
MyLamp kuchnia2 = {DMX, 11};
MyLamp kuchnia3 = {DMX, 12};

MyLamp kuchniaListwa = {DMX, 13};

MyLamp *lamps[] = {
  &tobi1, &tobi2, &sypialnia, &salon1, &salon2, &stol, &drzwi, &piwnica,
  &korytarz1, &korytarz2, &korytarz3, &garderoba1, &garderoba2,
  &lazienkaUmywalka, &lazienkaWanna, &lazienkaReszta, &lazienkaLustro,
  &kuchnia1, &kuchnia2, &kuchnia3, &kuchniaListwa
};

const byte lampsCount = (sizeof(lamps) / sizeof(MyLamp*));
const unsigned long dbTime = 50;
const unsigned long pressTime = 100;

struct MyButton {
  const char name[10];
  Button button;
  byte lampsCount;
  MyLamp *lamps[10];
  byte state;
} buttons[] = {

  {"Wspólny   ", Button(29, dbTime), 9, {&korytarz1, &korytarz2, &korytarz3, &kuchnia1, &kuchnia2, &kuchnia3, &salon1, &salon2, &stol} },
  
  {"Tobi 1    ", Button(4 , dbTime), 1, {&tobi1} },
  {"Tobi 2    ", Button(5 , dbTime), 1, {&tobi2} },

  {"Korytarz  ", Button(6 , dbTime), 3, {&korytarz1, &korytarz2, &korytarz3} },

  {"Korytarz 1", Button(20, dbTime), 1, {&korytarz1} },
  {"Korytarz 2", Button(22, dbTime), 2, {&korytarz2, &korytarz3} },

  {"Garderoba ", Button(7 , dbTime), 2, {&garderoba1, &garderoba2} },
  {"Łazienka  ", Button(9 , dbTime), 3, {&lazienkaUmywalka, &lazienkaWanna, &lazienkaReszta} },
  {"ŁazienkaL ", Button(11, dbTime), 1, {&lazienkaLustro} },

  {"Kuchnia   ", Button(17, dbTime), 3, {&kuchnia1, &kuchnia2, &kuchnia3} },

  {"Sypialnia ", Button(14, dbTime), 1, {&sypialnia} },

  {"Salon     ", Button(16, dbTime), 2, {&salon1, &salon2} },
  {"Salon 1   ", Button(21, dbTime), 1, {&salon1} },
  {"Salon 2   ", Button(23, dbTime), 1, {&salon2} },
  {"Stół      ", Button(15, dbTime), 1, {&stol} },

  {"Drzwi     ", Button(A1), 1, {&drzwi} },
};

const byte buttonsCount = (sizeof(buttons) / sizeof(MyButton));

void print(String text, int val) {
  if (DEBUG) {
    Serial.print(text);
    Serial.println(val);
  }
}

void print(String text) {
  if (DEBUG) Serial.println(text);
}

void setBrightness(MyLamp *lamp, byte brightness) {
  lamp->brightness = brightness;
  switch (lamp->type) {
    case PIN_HIGH:
      digitalWrite(lamp->channel, brightness == 0 ? LOW : HIGH);
      break;
    case PIN_LOW:
      digitalWrite(lamp->channel, brightness == 0 ? HIGH : LOW);
      break;
    case DMX:
      DmxSimple.write(lamp->channel, brightness);
      break;
  }
}

void setAll(MyButton *button, byte brightness) {
  for (int i = 0; i < button->lampsCount; i++) {
    MyLamp *lamp = *(button->lamps + i);
    setBrightness(lamp, brightness);
  }
}

bool allOff(MyButton *button) {
  for (int i = 0; i < button->lampsCount; i++) {
    MyLamp *lamp = *(button->lamps + i);
    if (lamp->brightness > 0) {
      return false;
    }
  }
  return true;
}

bool isDimmable(MyButton *button) {
  for (int i = 0; i < button->lampsCount; i++) {
    MyLamp *lamp = *(button->lamps + i);
    if (lamp->type != DMX) {
      return false;
    }
  }
  return true;
}

void buttonPressed(MyButton *button) {
  print("Button pressed: " + (String) button->name);
  //  print();
  //byte lCount = countLamps(button);
  byte lCount = button->lampsCount;
  print("lamps: ", lCount);

  if (anyLamp(button, ON)) {
    setAll(button, OFF);
  } else {
    setAll(button, ON);
  }
}

void setup() {
  if (DEBUG) {
    Serial.begin(9600);
    Serial.println("SETUP");
  }
  for (int i = 0; i < buttonsCount; i++) {
    MyButton button = *buttons[i]
    button->button.begin();
    button->state = STANDBY;
    
  }
  for (int i = 0; i < lampsCount; i++) {
    MyLamp lamp = *lamps[i];
    print("Setup lamp: ", i);
    print(" type: ", lamp.type);
    if (lamp.type == PIN_HIGH or lamp.type == PIN_LOW) {
      print(" outputPin: ", lamp.channel);
      pinMode(lamp.channel, OUTPUT);
    }
    lamp->brightness = 0;
    setBrightness(&lamp, OFF);
  }
  DmxSimple.usePin(3);
  DmxSimple.write(1, 0);
}

void loop() {
  for (int i = 0; i < buttonsCount; i++) {
    MyButton myButton = *buttons[i]
    Button button = myButton->button;
    button.read();
    switch (myButton.state) {
      case STANDBY:
        if (button.pressedFor(pressTime)) {
          myButton->state = PRESSED;
        }
        break;
      case PRESSED:
        if (button.wasReleased()) {
          //toggle
          myButton->state = STANDBY;
        }
        break;

      case DIMMING:

        break;

    }
    if (buttons[i].button.wasReleased()) {
      buttonPressed(&buttons[i]);
    }
  }

}
