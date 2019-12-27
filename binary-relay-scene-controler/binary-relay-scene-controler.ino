//   Sketch dla Arduino MEGA 2560
#define VER 0.5.1

/*Sterownik wyjść binarnych, z obsługą MySensors. Dedykowany do obsługi oświetlenia w instalacji inteligentnego budynku.
  
  *** UPDATE 07.01.2018 **** 
  Dodalem zapisywanie stanow do pamieci stalej EEPROM: Stany lamp sa zpisywane i wczytywane z pamieci po restarcie sterownika.
  Jezeli (tak jak ja) z jakiegos powodu nie chcesz zachowywac stanow zakomentuj definicje saveToEEPROM
  UWAGA! Wylaczenie zapisywania do EEPROM nie wylacza odczytu: Jesli w pamieci ukladu zapisane sa stany z poprzedniej kompilacji, zostana odczytane i nie beda nadpisane.
  W takiej sytuacji nalezy aktywowac zapis do EEPROM, wylaczyc wszystkie lampy i dezaktywaowac zapis.

  *********
  
  Opis konfiguracji wejść/wyjść znajduje się w dalszej części pliku.
  
  ZASADA DZIAŁANIA:
  Krótkie wciścnięcie - przełączenie lampy.
  Dwuklik             - wyzwalanie sceny (innej dla kazdego wlacznika).
  Longpress           - dezaktywacja sceny przypisanej do przelacznika. Moze byc uzyty do wyzwalania sceny (innej niz dwuklikiem) lub grupy lamp.
                      Jesli przypiszemy do grupy lamp, wowczas Dwuklikiem bedziemy mogli je wlaczyc, longpresem wylaczyc.
  Klik + longpress    - wyzwalanie sceny (innej niz dwuklikiem i innej dla kazdego wlacznika).

  Jak latwo policzyc z jednego wlacznika mozna wyzwolic 3 sceny czyli calym sterownikiem 96.
  
  PMIĘTAJ! 230V MOŻE CIE ZABIĆ LUB SPALIĆ TWÓJ DOM !!!
*/
//#define saveToEEPROM      //zakomentuj aby wylaczyc zapisywanie stanow do EEPROM

#define MY_GATEWAY_SERIAL

#define MAX_LAMPS   1    //ilość lamp obsługiwanych przez sterownik (max 32)

#define shortPress  120   //zwloka klikniecia [ms]
#define longPress   800  //zwloka przytrzymania przycisku [ms]

#define BD          10    //bounceDelay - debouncer [ms]

#include <MySensors.h>
#include <JC_Button.h>       //https://github.com/JChristensen/Button,

/* SETUP ==================================================================================== */

MyMessage sceneOnMsg(33, V_SCENE_ON);
MyMessage sceneOffMsg(33, V_SCENE_OFF);

//struktura danych lamp niesciemnianych
struct NLAMP
{
  Button    button;
  byte      lampPin;
  //
  MyMessage lightMsg;
  byte      multiclick;
  
} nlamp[] =
/*
  DEFINICJA WEJŚĆ/WYJŚĆ
  Opis poszczególnych kolumn struktury:

  1) Pin włącznika
  2) Pullup: 1 = INPUT_PULLUP 0 = INPUT
  3) 1 = sterowanie stanem niskim (LOW); 0 = sterowanie stanem wysokim (HIGH)
  4) Czas debouncera w milisekundach.
  5) Pin wyjścia do przekaźnika
  6) Stan lampy po restarcie sterownika 0 = wyłączona; 1 = włączona.
*/
{
//-------- 1| 2|3|4 ---- 5| 6| 
//  {Button(21, 1,1,BD), A15}, //0
//  {Button(20, 1,1,BD), A14}, //1 
//  {Button(19, 1,1,BD), A13}, //2
//  {Button(18, 1,1,BD), A12}, //3
//  {Button(17, 1,1,BD), A11}, //4
//  {Button(16, 1,1,BD), A10}, //5
//  {Button(15, 1,1,BD),  A9}, //6
  {Button(14, 1,1,BD),  13}, //7
//  {Button( 4, 1,1,BD),  A7}, //8
//  {Button( 5, 1,1,BD),  A6}, //9
//  {Button( 6, 1,1,BD),  A5}, //10
//  {Button( 7, 1,1,BD),  A4}, //11
//  {Button( 8, 1,1,BD),  A3}, //12
//  {Button( 9, 1,1,BD),  A2}, //13
//  {Button(10, 1,1,BD),  A1}, //14
//  {Button(11, 1,1,BD),  A0}, //15
//  {Button(22, 1,1,BD),  23}, //16
//  {Button(24, 1,1,BD),  25}, //17
//  {Button(26, 1,1,BD),  27}, //18
//  {Button(28, 1,1,BD),  29}, //19
//  {Button(30, 1,1,BD),  31}, //20
//  {Button(32, 1,1,BD),  33}, //21
//  {Button(34, 1,1,BD),  35}, //22
//  {Button(36, 1,1,BD),  37}, //23
//  {Button(38, 1,1,BD),  39}, //24
//  {Button(40, 1,1,BD),  41}, //25
//  {Button(42, 1,1,BD),  43}, //26
//  {Button(44, 1,1,BD),  45}, //27
//  {Button(46, 1,1,BD),  47}, //28
//  {Button(48, 1,1,BD),  49}, //29
//  {Button(50, 1,1,BD),  51}, //30
//  {Button(52, 1,1,BD),  53}  //31
};

const byte sizeNlamp = (sizeof(nlamp)/sizeof(NLAMP)) <= MAX_LAMPS? (sizeof(nlamp)/sizeof(NLAMP)) : MAX_LAMPS;

/* SETUP ====================================================================== */

void presentation()
{
  sendSketchInfo("BULB", 'VER');
  delay(1000);
  for (byte i=0; i < sizeNlamp; i++)
  {
      present(i, S_LIGHT); 
  }
  present(33, S_SCENE_CONTROLLER);
}

void setup()
{
  for (byte i=0; i < sizeNlamp; i++)
  {
      pinMode(nlamp[i].lampPin, OUTPUT);
      digitalWrite(nlamp[i].lampPin, loadState(i));
      nlamp[i].lightMsg = MyMessage(i, V_LIGHT);
      delay(5);
  }
  
  delay(5000);
} //koniec setup

/* FUNKCJE ===================================================================== */

//przelacznik lamp
void toggleNormal(byte i, bool state)
{
  #ifdef saveToEEPROM
      if (state != loadState(i)) saveState(i, state);
  #endif
  digitalWrite(nlamp[i].lampPin, state);
  send(nlamp[i].lightMsg.set(state));
}

//wyslij scene On
void sendSceneOn(byte i)
{
    send(sceneOnMsg.set(i));
}

//wyslij scene Off
void sendSceneOff(byte i)
{
    send(sceneOffMsg.set(i));
}

/* MAIN ======================================================================== */

void loop()
{
unsigned long currentMillis = millis();
//lampa niesciemniana
for (byte i=0; i < sizeNlamp; i++)  
{
    nlamp[i].button.read();
    //pierwsze wcisniecie
    if (nlamp[i].multiclick != 1)
    {
        //pierwsze wcisnieczie lub przytrzymanie
        if (nlamp[i].button.wasReleased())
        {
            if (nlamp[i].multiclick == 2)
            {
                toggleNormal(i, !digitalRead(nlamp[i].lampPin)? true:false);
            }
            nlamp[i].multiclick = 1;
        }
         //krotki klik - przelacz lampe
        else if ((nlamp[i].button.pressedFor(shortPress)) && (!nlamp[i].multiclick))
        {
            nlamp[i].multiclick = 2;
        }
        //przytrzymany
        else if ((nlamp[i].button.pressedFor(longPress)) && (nlamp[i].multiclick < 3))
        {
            nlamp[i].multiclick = 3;
            sendSceneOff(i);
        }
    }
    //drugie wcisniecie
    else
    {
        if (nlamp[i].button.releasedFor(shortPress))
        {
            nlamp[i].multiclick = 0;
        }
        else if (nlamp[i].button.wasReleased() && (nlamp[i].multiclick == 1)) 
        {
            sendSceneOn(i);
            nlamp[i].multiclick = 0;
        }
        else if (nlamp[i].button.pressedFor(shortPress + 100) && (nlamp[i].multiclick < 3))
        {
            sendSceneOn(i + 32);
            nlamp[i].multiclick = 3;
        }
    }
} //koniec for
} //koniec loop

/* MySENSORS =================================================================== */

void receive(const MyMessage &message)
{
  if (message.type == V_LIGHT) //odebrano instrukcje wlacz/wylacz
  {
      toggleNormal(message.sensor, (message.getBool()? true:false));
  }
} //koniec void receive

/* EOF ========================================================================= */
