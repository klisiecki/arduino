#include <Wire.h>   // standardowa biblioteka Arduino
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
OneWire ds(2);
DallasTemperature sensors(&ds);

int lcdKey     = 0;
int adc_key_in  = 0;

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

#define DEBUG 0
#define DEBUG_KEYS 0

struct HeatingArea {
  const char areaName[10];
  float currentTemp;
  float setTempOffset;
  int pin;
  DeviceAddress deviceAddress;
  byte phase;
} areas[] =
{
  {"korytarz1", -1, 0, 3,  {0x28, 0x11, 0x7F, 0x78, 0x0B, 0x00, 0x00, 0x05}, 1},
  {"korytarz2", -1, 0, 11, {0x28, 0x83, 0x2D, 0x78, 0x0B, 0x00, 0x00, 0xCC}, 1},
  {"lazienka ", -1, 0, A5, {0x28, 0xFF, 0x22, 0x48, 0x33, 0x18, 0x02, 0xD3}, 2},
  {"kuchnia  ", -1, 0, A2, {0x28, 0x9F, 0x1F, 0x6D, 0x0B, 0x00, 0x00, 0x54}, 2},
  {"sypialnia", -1, 0, A3, {0x28, 0x58, 0xCF, 0x6C, 0x0B, 0x00, 0x00, 0x9F}, 3},
  {"pokoj    ", -1, 0, A4, {0x28, 0x57, 0x11, 0x78, 0x0B, 0x00, 0x00, 0x71}, 3},
  {"salon    ", -1, 0, A1, {0x28, 0x2F, 0x0D, 0x78, 0x0B, 0x00, 0x00, 0x15}, 3}
//  {"garderoba", -1, 0, 12, {}, 3},
};

const byte areasCount = (sizeof(areas) / sizeof(HeatingArea));

float setTempBase = 24.0;
int selectedArea = 0;
byte lastPressed = -1;
long lastRefreshTime;
static long TEMP_REFRESH_INTERVAL = 10000;
static float TEMP_STEP = 0.1;

int read_LCD_buttons() {
  adc_key_in = analogRead(0);
  if (DEBUG_KEYS) Serial.println(adc_key_in);
  if (adc_key_in > 1000) return btnNONE;
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 150)  return btnUP;
  if (adc_key_in < 300)  return btnDOWN;
  if (adc_key_in < 450)  return btnLEFT;
  if (adc_key_in < 700)  return btnSELECT;
  return btnNONE;
}

void setup() {
  if (DEBUG || DEBUG_KEYS) {
      Serial.begin(9600);
  }
  sensors.begin();
  lcd.begin(16, 2);
  for (int i = 0; i < areasCount; i++) {
    pinMode(areas[i].pin, OUTPUT);
  }
  updatePinsState();
  refreshScreen();
}

void loop() {
  lcdKey = read_LCD_buttons();
  if (lastPressed != lcdKey || lcdKey == btnRIGHT || lcdKey == btnLEFT) {
    if (DEBUG) {
      Serial.println(lcdKey);
    }
    keyPressed(lcdKey);
    refreshScreen();
  }
  lastPressed = lcdKey;
  if (millis() - lastRefreshTime > TEMP_REFRESH_INTERVAL) {
    updateTemps();
    updatePinsState();
    refreshScreen();
  }
  delay(100);
}

void keyPressed (int lcdKey) {
  switch (lcdKey) {
    case btnRIGHT: {
        tempUp();
        break;
      }
    case btnLEFT: {
        tempDown();   
        break;
      }
    case btnUP: {
        if (selectedArea == 0) {
          selectedArea = -1;
        } else {
          selectedArea--;
        }
        break;
      }
    case btnDOWN: {
        if (selectedArea == areasCount - 1) {
          selectedArea = -1;
        } else {
          selectedArea++;
        }
        break;
      }
    case btnSELECT: {
        selectedArea = -1;
        break;
      }
    case btnNONE: {
        break;
      }
  }
}

void tempUp() {
  if (selectedArea == -1) {
    setTempBase = setTempBase + TEMP_STEP;
  }
  if (selectedArea >= 0) {
    areas[selectedArea].setTempOffset = areas[selectedArea].setTempOffset + TEMP_STEP;
  }
}

void tempDown() {
  if (selectedArea == -1) {
    setTempBase = setTempBase - TEMP_STEP;
  }
  if (selectedArea >= 0) {
    areas[selectedArea].setTempOffset = areas[selectedArea].setTempOffset - TEMP_STEP;
  }
}

float getTempSet(int areaIndex) {
  return setTempBase + areas[areaIndex].setTempOffset; 
}

void refreshScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (selectedArea >= 0) {
    HeatingArea area = areas[selectedArea];
    lcd.print(area.areaName);
    lcd.print(" " + String(area.currentTemp, 1));
    lcd.setCursor(0, 1);
    lcd.print(":" + String(setTempBase, 1) + getSign(area.setTempOffset) + String(area.setTempOffset, 1) +"=" + String(getTempSet(selectedArea), 1));
  } else {
    lcd.print("stan: TODO" );
    lcd.setCursor(0, 1);
    lcd.print("Bazowa: " + String(setTempBase, 1));
  }
  lastRefreshTime = millis();
}

String getSign(float offset) {
  if (offset >= 0) return "+";
  return "";
}

void updateTemps() {
  if (DEBUG) Serial.println("Updating temps");
  sensors.requestTemperatures();
  for (int i = 0; i < areasCount; i++) {
    areas[i].currentTemp = sensors.getTempC(areas[i].deviceAddress);
    if (DEBUG) Serial.println("Temp for " + ((String) areas[i].areaName) + " is " + areas[i].currentTemp);
  }
}

void updatePinsState() {
  for (int i = 0; i < areasCount; i++) {
    if (areas[i].currentTemp > 0 && areas[i].currentTemp < getTempSet(i)) {
      digitalWrite(areas[i].pin, HIGH);
    } else {
      digitalWrite(areas[i].pin, LOW);
    }
  }
}
