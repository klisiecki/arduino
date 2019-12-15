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

struct HEATING_AREA {
  const char areaName[10];
  float currentTemp;
  float setTempOffset;
  int pin;
  DeviceAddress deviceAddress;
} areas[] =
{
  {"salon    ", -1, 0, A1, { 0x28, 0xAA, 0x67, 0xB3, 0x53, 0x14, 0x01, 0xC2 } },
  {"kuchnia  ", -1, 0, A2},
  {"sypialnia", -1, 0, A3  },
  {"pokoj    ", -1, 0, A4},
  {"lazienka ", -1, 0, A5},
  {"korytarz1", -1, 0, 3},
  {"korytarz2", -1, 0, 11, { 0x28, 0xAA, 0x38, 0xD2, 0x53, 0x14, 0x01, 0x27 }},
//  {"garderoba", -1, 0, 12},
};

const byte areasCount = (sizeof(areas) / sizeof(HEATING_AREA));

float setTempBase = 24.0;
int selectedArea = 0;
byte lastPressed = -1;
long lastRefreshTime;
static long REFRESH_INTERVAL = 10000;
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
  if (millis() - lastRefreshTime > REFRESH_INTERVAL) {
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
    HEATING_AREA area = areas[selectedArea];
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

void updatePinsState() {
  if (DEBUG) Serial.println("Updating temps");
  sensors.requestTemperatures();
  for (int i = 0; i < areasCount; i++) {
    areas[i].currentTemp = sensors.getTempC(areas[i].deviceAddress);
    if (DEBUG) Serial.println("Temp for " + ((String) areas[i].areaName) + " is " + areas[i].currentTemp);
    if (areas[i].currentTemp > 0 && areas[i].currentTemp < getTempSet(i)) {
      digitalWrite(areas[i].pin, HIGH);
    } else {
      digitalWrite(areas[i].pin, LOW);
    }
  }
}
