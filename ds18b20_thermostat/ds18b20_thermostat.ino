/*
*  LCM1602 & Arduino Uno
*  VCC - > 5 V
*  GND - GND
*  SCL -> A5
*  SDA -> A4
*/
 
#include <Wire.h>   // standardowa biblioteka Arduino
#include <LiquidCrystal_I2C.h> // dolaczenie pobranej biblioteki I2C dla LCD
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire ds(2);
DallasTemperature sensors(&ds);
 
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27

int less = 7;
int more = 5;

float temp;
float temp1;
float temp2;
float tempSet = 26;
float tempMin = 15;
float tempMax = 32;
 
void setup() {
  sensors.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(less, INPUT);
  pinMode(more, INPUT);
  lcd.begin(16,2);   // Inicjalizacja LCD 2x16
  while(!Serial);
  Serial.begin(9600);
  
  lcd.backlight(); // zalaczenie podwietlenia 
}
 


void loop() {
  delay(500);
   sensors.requestTemperatures();
  temp1 = sensors.getTempCByIndex(0);
  temp2 = sensors.getTempCByIndex(1);
  temp = getCurrentTemp(temp1, temp1);
  if (temp > 0 && temp < tempSet) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  if (digitalRead(less) == HIGH && tempSet > tempMin) {
    tempSet = tempSet - 0.5;
    Serial.println("less");
  }
  if (digitalRead(more) == HIGH && tempSet < tempMax) {
    tempSet = tempSet + 0.5;
    Serial.println("more");
  }

  lcd.clear();  
  lcd.setCursor(0, 0);

  String actual = String(temp1, 2) + " " + String(temp2, 2);
  lcd.print(actual);
  Serial.println(actual);

  lcd.setCursor(0,1);
  String set = String(temp, 2) + "/" + String(tempSet, 2);
  lcd.print(set);
  Serial.println(set);
}

float getCurrentTemp(float temp1, float temp2) {
  if (temp2 < 0 || temp2 > 40) {
    return temp1;
  }
  return max(temp1, temp2);
}
