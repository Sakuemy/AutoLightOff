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
bool Sleap = true;   //Переменная для понимания когда можно переходить в сон, а когда нельзя
byte tap = 100;      //Переменная для переключения экранов
bool tapB = false;   //Положение кнопки (false == кнопка отпущена)
byte button = 4;     //Кнопка
byte rele = 8;       //Реле
byte fRez = 17;      //Фоторезистор
int MinLight = 500;  //Освешенности ниже которой нужно включить реле
int MaxLight = 600;  //Освещенность выше которой нужно выключить реле
bool releB = false;  //Переменная говорящая в каком положении реле
int Light1 = 1024;  //Переменная для проверки освещенности №1
int Light2 = 1024;  //Переменная для проверки освещенности №2
int Light3 = 1024;  //Переменная для проверки освещенности №3
byte HourNight = 0;  //Час выключения света
byte MinuteNight = 0;//Минута выключения света
bool Settings = false;
bool LongPressB = false;
byte s = 0;            //Для энкодера
int x = 0;             //Ответ от энкодера
int nkoder1 = 5;      //Энкодер пин 1
int nkoder2 = 11;      //Энкодер пин 2
byte blinking = 0;     //Счетчик задержки моргания

void setup()
{
  Serial.begin(9600);
  pinMode(button, INPUT);
  pinMode(fRez, INPUT);
  pinMode(rele, OUTPUT);
  pinMode(nkoder1, INPUT);
  pinMode(nkoder2, INPUT);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();

//Экран загрузки
  lcd.setCursor(6, 0);  
  lcd.print("Load");
  for (int i = 0; i < 18; i++){
    delay(100);
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

  if (Settings == true){
    Enkoder();  //Энкодер
  }

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
          tap = 100;
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
        if (tap < 7){
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
  if (millis() - timingRele > 3000){
    timingRele = millis();
    if ((analogRead(fRez) < MinLight)and(Light1 < MinLight)and(Light2 < MinLight)and(Light3 < MinLight)and(releB == false)){
      digitalWrite(rele, HIGH);
      releB = true;
    }

    if (((analogRead(fRez) > MaxLight)and(Light1 > MaxLight)and(Light2 > MaxLight)and(Light3 > MaxLight)and(releB == true))or(TimeCheck(HourNight, MinuteNight) == true)){
      digitalWrite(rele, LOW);
      releB = false;
    }
/*    Serial.println(Light1);
    Serial.println(Light2);
    Serial.println(Light3);
    Serial.println(analogRead(fRez));
    Serial.println(releB);
    Serial.println(TimeCheck(HourNight, MinuteNight));
    Serial.println(" ");*/
    Light3 = Light2;
    Light2 = Light1;
    Light1 = analogRead(fRez);
  }
/////////////////

//Смена экранов
  if (millis() - timing > 250){
   timing = millis();
   if (Settings == true){
     if (blinking < 4){
       blinking++;
     }else{
       blinking = 0;
     }
   }
   tmElements_t tm; 
    switch (tap) {
      case 1:
        if (RTC.read(tm)) {
          if (blinking <= 2){
            st = time0(String(tm.Hour), 2) + ":" + time0(String(tm.Minute), 2) + ":" + time0(String(tm.Second), 2);
          }else{
            st = "  :" + time0(String(tm.Minute), 2) + ":" + time0(String(tm.Second), 2);
          }
          lcd.setCursor(0, 0);
          lcd.print("Time:");
          lcd.setCursor(0, 1);  
          lcd.print(st);
        } else {
          lcd.setCursor(0, 1);
          lcd.print("Eror time");
        }
        break;
      case 2:
        if (RTC.read(tm)) {
          if (blinking <= 2){
            st = time0(String(tm.Hour), 2) + ":" + time0(String(tm.Minute), 2) + ":" + time0(String(tm.Second), 2);
          }else{
            st = time0(String(tm.Hour), 2) + ":  " + ":" + time0(String(tm.Second), 2);
          }
          lcd.setCursor(0, 0);
          lcd.print("Time:");
          lcd.setCursor(0, 1);  
          lcd.print(st);
        } else {
          lcd.setCursor(0, 1);
          lcd.print("Eror time");
        }
        break;
      case 3:
        if (RTC.read(tm)) {
          if (blinking <= 2){
            st = time0(String(tm.Hour), 2) + ":" + time0(String(tm.Minute), 2) + ":" + time0(String(tm.Second), 2);
          }else{
            st = time0(String(tm.Hour), 2) + ":" + time0(String(tm.Minute), 2) + ":  ";
          }
          lcd.setCursor(0, 0);
          lcd.print("Time:");
          lcd.setCursor(0, 1);  
          lcd.print(st);
        } else {
          lcd.setCursor(0, 1);
          lcd.print("Eror time");
        }
        break;
      case 4:
        lcd.setCursor(0, 0);  
        lcd.print("TimeNight:");
        lcd.setCursor(0, 1);
        if (blinking <= 2){
          lcd.print(time0(String(HourNight), 2) + ":" + time0(String(MinuteNight), 2));
        }else{
          lcd.print("  :" + time0(String(MinuteNight), 2));
        }
        break;
      case 5:
        lcd.setCursor(0, 0);  
        lcd.print("TimeNight:");
        lcd.setCursor(0, 1);
        if (blinking <= 2){
          lcd.print(time0(String(HourNight), 2) + ":" + time0(String(MinuteNight), 2));
        }else{
          lcd.print(time0(String(HourNight), 2) + ":  ");
        }
        break;
      case 6:
        lcd.setCursor(0, 0);  
        lcd.print("Min.Light:");
        lcd.setCursor(0, 1);
        if (blinking <= 2){
          lcd.print(time0(String(MinLight), 4));
        }else{
          lcd.print("    ");
        }
        break;
      case 7:
        lcd.setCursor(0, 0);  
        lcd.print("Max.Light:");
        lcd.setCursor(0, 1);  
        if (blinking <= 2){
          lcd.print(time0(String(MaxLight), 4));
        }else{
          lcd.print("    ");
        }
        break;
        
      case 100:
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
  if (s1.toInt() == s2.toInt()){
    return (true);
  } else {
    return (false);
  }
}
/////////////////

//Энкодер
void Enkoder(){
  switch (s) {
      case 0:
        if ((digitalRead(nkoder2) == 1)and(digitalRead(nkoder1) == 0)) s = 1;
        if ((digitalRead(nkoder1) == 1)and(digitalRead(nkoder2) == 0)) s = 10;
        break;
      case 1:
        if ((digitalRead(nkoder2) == 1)and(digitalRead(nkoder1) == 0)) {s = 1;}else{
          if ((digitalRead(nkoder2) == 1)and(digitalRead(nkoder1) == 1)) s = 2;}
        break;
      case 10:
        if ((digitalRead(nkoder1) == 1)and(digitalRead(nkoder2) == 0)) {s = 10;}else{
          if ((digitalRead(nkoder2) == 1)and(digitalRead(nkoder1) == 1)) s = 11;}
        break;
      case 2:
        if ((digitalRead(nkoder2) == 1)and(digitalRead(nkoder1) == 1)) {s = 2;}else{
          if ((digitalRead(nkoder2) == 0)and(digitalRead(nkoder1) == 1)) s = 3;}
        break;
      case 11:
        if ((digitalRead(nkoder2) == 1)and(digitalRead(nkoder1) == 1)) {s = 11;}else{
          if ((digitalRead(nkoder2) == 0)and(digitalRead(nkoder1) == 1)) s = 12;}
        break;
      case 3:
        if ((digitalRead(nkoder2) == 0)and(digitalRead(nkoder1) == 1)) {s = 3;}else{
          if ((digitalRead(nkoder2) == 0)and(digitalRead(nkoder1) == 0)) {s = 0;x = 1;}}
        break;
      case 12:
        if ((digitalRead(nkoder2) == 0)and(digitalRead(nkoder1) == 1)) {s = 12;}else{
          if ((digitalRead(nkoder2) == 0)and(digitalRead(nkoder1) == 0)) {s = 0;x = -1;}}
        break;
  }
}
/////////////////
