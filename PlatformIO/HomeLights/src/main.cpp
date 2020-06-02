#include <Arduino.h>

#include <DmxSimple.h>
#include <JC_Button.h>

#define DEBUG 0

const byte STANDBY = 0;
const byte PRESSED = 1;
const byte DIMMING = 2;

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

// MyLamp kuchniaListwa = {DMX, 13};

MyLamp *lamps[] = {
  &tobi1, &tobi2, &sypialnia, &salon1, &salon2, &stol, &drzwi, &piwnica,
  &korytarz1, &korytarz2, &korytarz3, &garderoba1, &garderoba2,
  &lazienkaUmywalka, &lazienkaWanna, &lazienkaReszta, &lazienkaLustro,
  &kuchnia1, &kuchnia2, &kuchnia3
  // , &kuchniaListwa
};

const byte lampsCount = (sizeof(lamps) / sizeof(MyLamp*));

byte dmxDefault = 255;
byte dmxNight = 3;

const unsigned long dbTime = 25;
// const unsigned long pressTime = 40;

const byte dimStep = 30;
const unsigned long dimmInitialTime = 1000;
const unsigned long dimmStepTime = 500;


struct MyButton {
  const char name[10];
  Button button;
  byte lampsCount;
  MyLamp *lamps[10];
  byte state;
  unsigned long pressedFor;
  bool dimmingUp;

} buttonsArr[] = {

  {"Wspolny  ", Button(29, dbTime), 9, {&korytarz1, &korytarz2, &korytarz3, &kuchnia1, &kuchnia2, &kuchnia3, &salon1, &salon2, &stol} },
  
  {"Tobi1    ", Button(4 , dbTime), 1, {&tobi1} },
  {"Tobi2    ", Button(5 , dbTime), 1, {&tobi2} },

  {"Korytarz ", Button(6, dbTime), 3, {&korytarz1, &korytarz2, &korytarz3} },

  {"Korytarz1", Button(20, dbTime), 1, {&korytarz1} },
  {"Korytarz2", Button(22, dbTime), 2, {&korytarz2, &korytarz3} },

  {"Garderoba", Button(7 , dbTime), 2, {&garderoba1, &garderoba2} },
  {"Lazienka ", Button(9 , dbTime), 3, {&lazienkaUmywalka, &lazienkaWanna, &lazienkaReszta} },
  {"LazienkaL", Button(11, dbTime), 1, {&lazienkaLustro} },

  {"Kuchnia  ", Button(17, dbTime), 3, {&kuchnia1, &kuchnia2, &kuchnia3} },

  {"Sypialnia", Button(14, dbTime), 1, {&sypialnia} },

  {"Salon    ", Button(16, dbTime), 2, {&salon1, &salon2} },
  {"Salon1   ", Button(21, dbTime), 1, {&salon1} },
  {"Salon2   ", Button(23, dbTime), 1, {&salon2} },
  {"Stol     ", Button(15, dbTime), 1, {&stol} },

  {"Drzwi    ", Button(A1), 1, {&drzwi} },
};

const byte buttonsCount = (sizeof(buttonsArr) / sizeof(MyButton));

MyButton *buttons[buttonsCount];

void print(String text, long val) {
  if (DEBUG) {
    Serial.print(text);
    Serial.println(val);
  }
}

void print(String text, String val) {
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
      digitalWrite(lamp->channel, brightness <= dmxNight ? LOW : HIGH);
      break;
    case PIN_LOW:
      digitalWrite(lamp->channel, brightness <= dmxNight ? HIGH : LOW);
      break;
    case DMX:
      DmxSimple.write(lamp->channel, brightness);
      break;
  }
}

void setAll(MyButton *button, byte brightness) {
  print("set all", button->name);
  print("br = ", brightness);
  for (int i = 0; i < button->lampsCount; i++) {
    MyLamp *lamp = *(button->lamps + i);
    setBrightness(lamp, brightness);
  }
}

bool allOff(MyButton *button) {
  for (int i = 0; i < button->lampsCount; i++) {
    MyLamp *lamp = *(button->lamps + i);
    if (lamp->brightness > dmxNight) {
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

byte getMinBrightness(MyButton *button) {
  byte min = 255;
  for (int i = 0; i < button->lampsCount; i++) {
    MyLamp *lamp = *(button->lamps + i);
    if (lamp->brightness < min) {
      min = lamp->brightness;
    }
  }
  return min;
}

void toggleOnOff(MyButton *button) {
  if (allOff(button)) {
    setAll(button, dmxDefault);
  } else {
    setAll(button, dmxNight);
  }
}

void changeBrightness(MyButton *button) {
  byte minBrightness = getMinBrightness(button);
  byte brightness;
  if (button->dimmingUp) {
    brightness = min(minBrightness + dimStep, dmxDefault);
    if (brightness == dmxDefault) {
      button->dimmingUp = false;
    }
  } else {
    brightness = max(minBrightness - dimStep, 0);
    if (brightness == 0) {
      button->dimmingUp = true;
    }
  }
  setAll(button, brightness);
}

void setup() {
  pinMode(13, OUTPUT);
  //analogWrite(13,40);
  if (DEBUG) {
    Serial.begin(9600);
    Serial.println("SETUP");
  }
  for (int i = 0; i < buttonsCount; i++) {
    print("Setup button: ", i);
    buttons[i] = &buttonsArr[i];
    MyButton *button = buttons[i];
    print(button->name);
    button->button.begin();
    button->state = STANDBY;
  }
  for (int i = 0; i < lampsCount; i++) {
    MyLamp *lamp = lamps[i];
    print("Setup lamp: ", i);
    print(" type: ", lamp->type);
    if (lamp->type == PIN_HIGH or lamp->type == PIN_LOW) {
      print(" outputPin: ", lamp->channel);
      pinMode(lamp->channel, OUTPUT);
    }
    lamp->brightness = 0;
    setBrightness(lamp, 0);
  }
  DmxSimple.usePin(3);
  DmxSimple.write(1, 0);
}

void setState(MyButton *button, byte state) {
    print(button->name, state);
    button->state = state;
}

void loop() {
  delay(200);
  for (int i = 0; i < buttonsCount; i++) {
    MyButton *myButton = buttons[i];
    Button *button = &(myButton->button);
    button->read();

    switch (myButton->state) {
      case STANDBY:
        if (button->wasPressed()) {
          setState(myButton, PRESSED);
        }
        break;
      case PRESSED:
        if (button->wasReleased()) {
          toggleOnOff(myButton);
          setState(myButton, STANDBY);
        }
        if (button->pressedFor(dimmInitialTime) and isDimmable(myButton)) {
          myButton->pressedFor = dimmInitialTime;
          myButton->dimmingUp = false;
          setState(myButton, DIMMING);
        }
        break;
      case DIMMING:
        if (button->wasReleased()) {
            setState(myButton, STANDBY);
        }
        if (button->pressedFor(myButton->pressedFor + dimmStepTime)) {
          changeBrightness(myButton);
          myButton->pressedFor = myButton->pressedFor + dimmStepTime;
        }
        break;
    }
  }
}
