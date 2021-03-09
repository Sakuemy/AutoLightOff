#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <avr/eeprom.h>

LiquidCrystal_I2C lcd(0x27,16,2);
String st;
unsigned long timing;
unsigned long timingTap;
unsigned long timingSleap;
unsigned long timingRele;
unsigned long LongPress;
bool Sleap = true;
byte tap = 10;
bool tapB = false;
byte button = 4;       //Кнопка
byte rele = 8;         //Реле
byte fRez = 17;        //Фоторезистор
int MinLight = 300;
int MaxLight = 400;
bool releB = false;
byte Light1 = 1024;
byte Light2 = 1024;
byte Light3 = 1024;
byte HourOn;
byte MinuteOn;
byte HourNight = 0;
byte MinuteNight = 0;
bool Settings = false;
bool LongPressB = false;

void setup()
{
  Serial.begin(9600);
  pinMode(button, INPUT);
  pinMode(fRez, INPUT);
  pinMode(rele, OUTPUT);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();

//Экран загрузки
  lcd.setCursor(6, 0);  
  lcd.print("Load");
  for (int i = 0; i < 18; i++){
    delay(150);
    lcd.setCursor(i, 1);  
    lcd.print("#");
  }
  lcd.clear();
  timingSleap = millis();
/////////////////
}

void loop()
{ 
//  Serial.println(tap);
//Сброс  перемолненных переменных
  if (millis() - timingTap < 0){
    timingTap = millis();
    timing = millis();
    timingSleap = millis();
    timingRele = millis();
  }
/////////////////

//Переход на начальный экран при бездействии
/*
  if ((millis() - timingSleap > 20000)and(Sleap == false)){
    Sleap = true;
    tap = 10;
    lcd.clear();
  }*/
/////////////////

//Отключение подсветки
  if ((millis() - timingSleap > 30000)and(Sleap == true)){
    lcd.noBacklight();
  }
/////////////////

//Нажание на кнопку
  if (millis() - timingTap > 50){
    timingTap = millis();
    if (Sleap == false)timingSleap = millis(); //Сон
    //Долгое нажание на кнопку
    if ((tapB == true)and(millis() - LongPress > 3000)){
      if (LongPressB == false){
        if (Settings == true){
          Settings = false;
          LongPressB = true;
          tap = 10;
          Sleap = true;
        } else {
          Settings = true;
          LongPressB = true;
          tap = 1;
          Sleap = false;
        }
      }
      if (digitalRead(button) == 0){
        tapB = false;
        LongPressB = false;
      }
      lcd.clear();
    }
    /////////////////
    if ((digitalRead(button) == 1)and(tapB == false)and(LongPressB == false)){
      tapB = true;
      LongPress = millis();
      lcd.backlight();
      timingSleap = millis();
    }else{
      if ((digitalRead(button) == 0)and(tapB == true)and(Settings == true)){
        timingSleap = millis();
        if (tap < 5){
          tap++;
        }else{
          tap = 1;
        }
        tapB = false;
        lcd.clear();
      }
    }
  }
/////////////////

//Переключение реле
  if (millis() - timingRele > 500){
    timingRele = millis();
    if ((analogRead(fRez) < MinLight)and(Light1 < MinLight)and(Light2 < MinLight)and(Light3 < MinLight)and(releB == false)){
      digitalWrite(rele, HIGH);
      releB = true;
    }

    if (((analogRead(fRez) > MaxLight)and(Light1 > MaxLight)and(Light2 > MaxLight)and(Light3 > MaxLight)and(releB == true))or(TimeCheck(HourOn, MinuteOn) == true)){
      digitalWrite(rele, LOW);
      releB = false;
    }
    Light3 = Light2;
    Light2 = Light1;
    Light1 = analogRead(fRez);
  }
/////////////////

//Смена экранов
  if (millis() - timing > 250){
   timing = millis();
   tmElements_t tm; 
    switch (tap) {
      case 1:
        lcd.setCursor(0, 0);  
        lcd.print("Light sensor:");
        lcd.setCursor(0, 1);  
        lcd.print(time0(String(analogRead(fRez)), 4));
        break;
      case 2:
        if (RTC.read(tm)) {
          st = time0(String(tm.Hour), 2) + ":" + time0(String(tm.Minute), 2) + ":" + time0(String(tm.Second), 2);
          lcd.setCursor(0, 0);
          lcd.print("Time:");
          lcd.setCursor(0, 1);  
          lcd.print(st);
        } else {
          lcd.setCursor(6, 1);
          lcd.print("Eror time");
        }
        break;
      case 3:
        lcd.setCursor(0, 0);  
        lcd.print("TimeNight:");
        lcd.setCursor(0, 1);  
        lcd.print(time0(String(HourNight), 2) + ":" + time0(String(MinuteNight), 2));
        break;
      case 4:
        lcd.setCursor(0, 0);  
        lcd.print("Min.Light:");
        lcd.setCursor(0, 1);  
        lcd.print(time0(String(MinLight), 4));
        break;
      case 5:
        lcd.setCursor(0, 0);  
        lcd.print("Max.Light:");
        lcd.setCursor(0, 1);  
        lcd.print(time0(String(MaxLight), 4));
        break;
      case 10:
        lcd.setCursor(0, 0);  
        lcd.print("Illum.:");
        lcd.setCursor(12, 0);  
        lcd.print(time0(String(analogRead(fRez)), 4));
        lcd.setCursor(0, 1);  
        lcd.print("Min.illum.:");
        lcd.setCursor(12, 1);  
        lcd.print(time0(String(MinLight), 4));
        break;
    }
  }
/////////////////
}

//Добавление нулей в начало
String time0(String te, byte max)
{
  if (max < 2) return te;
  for (int i = 0; i < 20; i++){
    if (te.length() < max) {
      te = "0" + te;
    }else{
      return (te);
    }
  }
}
/////////////////

//Сравнение времени
bool TimeCheck(byte h, byte m){
  tmElements_t tm;
  String s1, s2;
  s1 = String(tm.Hour)+String(tm.Minute);
  s2 = String(h)+String(m);
  if (s1.toInt() >= s2.toInt()){
    return (true);
  } else {
    return (false);
  }
}
/////////////////
