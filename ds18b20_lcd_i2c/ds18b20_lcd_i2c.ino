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

OneWire ds(10);
DallasTemperature sensors(&ds);
 
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27
 
 
void setup() {
  sensors.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  lcd.begin(16,2);   // Inicjalizacja LCD 2x16
  
  lcd.backlight(); // zalaczenie podwietlenia 
//  lcd.setCursor(0,0); // Ustawienie kursora w pozycji 0,0 (pierwszy wiersz, pierwsza kolumna)
//  lcd.print("Hello, world!");
//  lcd.setCursor(0,1); //Ustawienie kursora w pozycji 0,0 (drugi wiersz, pierwsza kolumna)
//  lcd.print("Temperatury");

}
 
boolean led = true;

void loop() {
  delay(1000);
  if (led) {
    digitalWrite(LED_BUILTIN, HIGH);
   led = false;
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    led = true;
  }
  sensors.requestTemperatures();
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print(sensors.getTempCByIndex(0));
  
  lcd.setCursor(0, 1);
  lcd.print(sensors.getTempCByIndex(1));

  lcd.setCursor(9, 0);
  lcd.print(sensors.getTempCByIndex(3));

  lcd.setCursor(9, 1);
  lcd.print(sensors.getTempCByIndex(4));

  delay(1000);
}
