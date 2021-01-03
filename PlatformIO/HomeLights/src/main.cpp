#include <Arduino.h>

#define VER "0.6.1"
#define MY_GATEWAY_SERIAL

#include <MySensors.h>
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
  MyMessage message;
};

MyLamp tobi1 = {PIN_HIGH, 40}; //0
MyLamp tobi2 = {PIN_HIGH, 41}; //1

MyLamp sypialnia = {PIN_HIGH, 42}; //2
MyLamp lozkoL = {DMX, 16}; //3
MyLamp lozkoP = {DMX, 17}; //4

MyLamp salon1 = {PIN_HIGH, 43}; //5
MyLamp salon2 = {PIN_HIGH, 44};//6
MyLamp stol = {PIN_HIGH, 46}; //7

MyLamp drzwi = {PIN_LOW, 51}; //8
MyLamp piwnica = {PIN_LOW, 50}; //9, spalony

MyLamp korytarz1 = {DMX, 1}; //10
MyLamp korytarz2 = {DMX, 2}; //11
MyLamp korytarz3 = {DMX, 3}; //12

MyLamp garderoba1 = {DMX, 4}; //13
MyLamp garderoba2 = {DMX, 5}; //14

MyLamp lazienkaUmywalka = {DMX, 7}; //15
MyLamp lazienkaWanna =    {DMX, 8}; //16
MyLamp lazienkaReszta =   {DMX, 9}; //17
MyLamp lazienkaLustro = {PIN_LOW, 52}; //18

MyLamp kuchnia1 = {DMX, 10}; //19
MyLamp kuchnia2 = {DMX, 11}; //20
MyLamp kuchnia3 = {DMX, 12}; //21

MyLamp kinkietL = {DMX, 19}; //22
MyLamp kinkietP = {DMX, 20}; //23

MyLamp bojler = {PIN_HIGH, 24}; //24
MyLamp ogrzewanie = {PIN_HIGH, 26}; //25

MyLamp taras = {PIN_LOW, 53}; //26


// MyLamp kuchniaListwa = {DMX, 13};

MyLamp *lamps[] = {
  &tobi1, &tobi2, 
  &sypialnia, &lozkoL, &lozkoP,
  &salon1, &salon2, &stol, &drzwi, &piwnica,
  &korytarz1, &korytarz2, &korytarz3, &garderoba1, &garderoba2,
  &lazienkaUmywalka, &lazienkaWanna, &lazienkaReszta, &lazienkaLustro,
  &kuchnia1, &kuchnia2, &kuchnia3, &kinkietL, &kinkietP,
  &bojler, &ogrzewanie,
  // , &kuchniaListwa,
  &taras
};

const byte lampsCount = (sizeof(lamps) / sizeof(MyLamp*));

byte onBrightness = 255;
byte offBrightness = 0;

const byte ON_BRIGTHTNESS_ID = 50;
const byte OFF_BRIGTHTNESS_ID = 51;

const unsigned long dbTime = 25;
const unsigned long pressTime = 40;

const byte dimStep = 30;
const unsigned long dimmInitialTime = 600;
const unsigned long dimmStepTime = 500;


struct MyButton {
  const char name[10];
  Button button;
  byte lampsCount;
  MyLamp *lamps[12];
  byte state;
  unsigned long pressedFor;
  bool dimmingUp;

} buttonsArr[] = {

  {"Wspolny  ", Button(29, dbTime), 11, {&korytarz1, &korytarz2, &korytarz3, &kuchnia1, &kuchnia2, &kuchnia3, &salon1, &salon2, &kinkietL, &kinkietP, &stol} },
  
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
  {"Lozko wsp", Button(12, dbTime), 2, {&lozkoL, &lozkoP} },
  {"Lozko L  ", Button(19, dbTime), 1, {&lozkoL} },
  {"Lozko P  ", Button(18, dbTime), 1, {&lozkoP} },

  {"Salon    ", Button(16, dbTime), 2, {&salon1, &salon2} },
  {"Salon1   ", Button(21, dbTime), 1, {&salon1} },
  {"Salon2   ", Button(23, dbTime), 1, {&salon2} },
  {"Stol     ", Button(15, dbTime), 1, {&stol} },

  {"Drzwi    ", Button(25), 1, {&drzwi} },

  {"Taras    ", Button(27), 1, {&taras} },
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
      digitalWrite(lamp->channel, brightness <= offBrightness ? LOW : HIGH);
      send(lamp->message.set(brightness <= offBrightness ? 0 : 1));
      break;
    case PIN_LOW:
      digitalWrite(lamp->channel, brightness <= offBrightness ? HIGH : LOW);
      send(lamp->message.set(brightness <= offBrightness ? 0 : 1));
      break;
    case DMX:
      DmxSimple.write(lamp->channel, brightness);
      int value = floor(brightness / 2.55);
      send(lamp->message.set(value));
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
    if (lamp->brightness > offBrightness) {
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
    setAll(button, onBrightness);
  } else {
    setAll(button, offBrightness);
  }
}

void changeBrightness(MyButton *button) {
  byte minBrightness = getMinBrightness(button);
  byte brightness;
  if (button->dimmingUp) {
    brightness = min(minBrightness + dimStep, onBrightness);
    if (brightness == onBrightness) {
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

void presentation() {
  sendSketchInfo("HomeLights", VER);
  for (int i = 0; i < lampsCount; i++) {
    MyLamp *lamp = lamps[i];
    if (lamp->type == PIN_HIGH or lamp->type == PIN_LOW) {
      present(i, S_LIGHT);
    } else {
      present(i, S_DIMMER);
    }
  }
  present(ON_BRIGTHTNESS_ID, S_DIMMER);
  present(OFF_BRIGTHTNESS_ID, S_DIMMER);
}

void setup() {
  pinMode(13, OUTPUT);
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
      lamp->message = MyMessage(i, V_LIGHT);
    } else {
      lamp->message = MyMessage(i, V_DIMMER);
    }
    lamp->brightness = 0;
    setBrightness(lamp, 0);
  }
  DmxSimple.usePin(3);
  DmxSimple.write(1, 0);
  delay(5000);
}

void setState(MyButton *button, byte state) {
    print(button->name, state);
    button->state = state;
}

void loop() {
  for (int i = 0; i < buttonsCount; i++) {
    MyButton *myButton = buttons[i];
    Button *button = &(myButton->button);
    button->read();

    switch (myButton->state) {
      case STANDBY:
        if (button->pressedFor(pressTime)) {
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

void receive(const MyMessage &message) {
  if (message.getType() == V_LIGHT) {
   	int state= atoi(message.data);
    if (state == 0) {
      setBrightness(lamps[message.sensor], offBrightness);
    } else {
      setBrightness(lamps[message.sensor], 255);
    }
	} else if (message.getType() == V_DIMMER) {
		int value = atoi(message.data);
    byte value255 = ceil(value * 2.55);
    if (message.sensor == ON_BRIGTHTNESS_ID) {
      onBrightness = value255;
    } else if (message.sensor == OFF_BRIGTHTNESS_ID) {
      offBrightness = value255;
    } else {
      setBrightness(lamps[message.sensor], value255);
    }
  }
}