#include <DmxSimple.h>
#include <JC_Button.h>

#define DEBUG 1

const byte ON = 1;
const byte OFF = 0;

int dmxDefault = 200;
int dmxNight = 3;

enum LampType {
  PIN_LOW,
  PIN_HIGH,
  DMX,
  DMX_NIGHT //set to 3/255 when turned off
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
MyLamp stol = {PIN_HIGH, 46, OFF};

MyLamp drzwi = {PIN_LOW, 51, OFF};
MyLamp piwnica = {PIN_LOW, 50, OFF}; //spalony

MyLamp korytarz1 = {DMX_NIGHT, 1, ON};
MyLamp korytarz2 = {DMX_NIGHT, 2, ON};
MyLamp korytarz3 = {DMX_NIGHT, 3, ON};

MyLamp garderoba1 = {DMX, 4, OFF};
MyLamp garderoba2 = {DMX, 5, OFF};

MyLamp lazienkaUmywalka = {DMX_NIGHT, 7, OFF};
MyLamp lazienkaWanna =    {DMX_NIGHT, 8, OFF};
MyLamp lazienkaReszta =   {DMX_NIGHT, 9, OFF};
MyLamp lazienkaLustro = {PIN_LOW, 52, OFF};


MyLamp kuchnia1 = {DMX_NIGHT, 10};
MyLamp kuchnia2 = {DMX_NIGHT, 11};
MyLamp kuchnia3 = {DMX_NIGHT, 12};

MyLamp kuchniaListwa = {DMX, 13};

MyLamp *lamps[] = {
  &tobi1, &tobi2, &sypialnia, &salon1, &salon2, &stol, &drzwi, &piwnica,
  &korytarz1, &korytarz2, &korytarz3, &garderoba1, &garderoba2,
  &lazienkaUmywalka, &lazienkaWanna, &lazienkaReszta, &lazienkaLustro,
  &kuchnia1, &kuchnia2, &kuchnia3, &kuchniaListwa
};

const byte lampsCount = (sizeof(lamps) / sizeof(MyLamp*));
const unsigned long dbTime = 50;

struct MyButton {
  const char name[10];
  Button button;
  byte lampsCount;
  MyLamp *lamps[10];
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


Button onOff = Button(A0);

void setup() {
  if (DEBUG) {
    Serial.begin(9600);
    Serial.println("SETUP");
  }
  onOff.begin();
  for (int i = 0; i < buttonsCount; i++) {
    buttons[i].button.begin();
  }
  for (int i = 0; i < lampsCount; i++) {
    MyLamp lamp = *lamps[i];
    print("Setup lamp: ", i);
    print(" type: ", lamp.type);
    if (lamp.type == PIN_HIGH or lamp.type == PIN_LOW) {
      print(" outputPin: ", lamp.channel);
      pinMode(lamp.channel, OUTPUT);
    }
    setState(&lamp, lamp.state);
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

void setAll(MyButton *button, byte state) {
  for (int i = 0; i < button->lampsCount; i++) {
    MyLamp *lamp = *(button->lamps + i);
    setState(lamp, state);
  }
}

bool anyLamp(MyButton *button, byte state) {
  for (int i = 0; i < button->lampsCount; i++) {
    MyLamp *lamp = *(button->lamps + i);
    if (lamp->state == state) {
      return true;
    }
  }
  return false;
}

void toggle(MyLamp *lamp) {
  print("toggle lamp on channel ", lamp->channel);
  print("current state is ", lamp->state);
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
    case DMX_NIGHT:
      DmxSimple.write(lamp->channel, state == ON ? dmxDefault : dmxNight);
      break;
  }
}

bool anyLamp(byte state) {
  print("Checking all lamps");
  for (int i = 0; i < lampsCount; i++) {
    if (lamps[i]->state == state) {
      print("Lamp on: ", i);
      return true;
    }
  }
  print("All lamps off");
  return false;
}

void setAll(byte state) {
  for (int i = 0; i < lampsCount; i++) {
    setState(lamps[i], state);
  }
}

void print(String text) {
  if (DEBUG) Serial.println(text);
}
void print(String text, byte val) {
  if (DEBUG) {
    Serial.print(text);
    Serial.println(val);
  }
}

void print(String text, int val) {
  if (DEBUG) {
    Serial.print(text);
    Serial.println(val);
  }
}
