#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int lcdKey     = 0;
int adc_key_in  = 0;

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

struct HEATING_AREA {
  const char areaName[9];
  float currentTemp;
  float setTemp;
  int pin;
} areas[] =
{
  {"salon    ", -1, 21.5, A1},
  {"kuchnia  ", -1, 22,   A2},
  {"sypialnia", -1, 21,   A3},
  {"pokoj    ", -1, 20.5, A4},
  {"lazienka ", -1, 20.5, 0},
  {"korytarz1", -1, 20.5, 1},
  {"korytarz2", -1, 20.5, 11},
  {"garderoba", -1, 20.5, 12},
};

const byte areasCount = (sizeof(areas) / sizeof(HEATING_AREA));

byte selectedArea = 0;
byte lastPressed = -1;

int read_LCD_buttons() {
  adc_key_in = analogRead(0);
  if (adc_key_in > 1000) return btnNONE;
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 650)  return btnLEFT;
  if (adc_key_in < 850)  return btnSELECT;
  return btnNONE;
}

void setup() {
  lcd.begin(16, 2);
  for (int i = 0; i < areasCount; i++) {
    pinMode(areas[i].pin, OUTPUT);
  }
}

void loop() {
  lcdKey = read_LCD_buttons();
  if (lastPressed != lcdKey) {
    keyPressed(lcdKey);
  }
  lastPressed = lcdKey;
  refreshScreen();
  updatePinsState();
}

void keyPressed (int lcdKey) {
  switch (lcdKey) {
    case btnRIGHT: {
        break;
      }
    case btnLEFT: {
        break;
      }
    case btnUP: {
        if (selectedArea == 0) {
          selectedArea = areasCount - 1;
        } else {
          selectedArea--;
        }
        break;
      }
    case btnDOWN: {
        if (selectedArea == areasCount - 1) {
          selectedArea = 0;
        } else {
          selectedArea++;
        }
        break;
      }
    case btnSELECT: {
        break;
      }
    case btnNONE: {
        break;
      }
  }
}

void refreshScreen() {
  lcd.setCursor(0, 0);
  lcd.print(">");
  lcd.setCursor(1, 0);
  lcd.print(areas[selectedArea].areaName);
}

void updatePinsState() {
  for (int i = 0; i < areasCount; i++) {
    if (i == selectedArea) {
      digitalWrite(areas[i].pin, HIGH);
    } else {
      digitalWrite(areas[i].pin, LOW);
    }
  }

}
