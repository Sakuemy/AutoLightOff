#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <DS1307RTC.h>          //Библиотека для работы с модулем времени
#include <avr/eeprom.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27,16,2);
String st;
unsigned long timing;
unsigned long timingTap;
unsigned long timingSleap;
unsigned long timingRele;
unsigned long LongPress;
unsigned long ChangeSetting;
int Sleap = true;   //Переменная для понимания когда можно переходить в сон, а когда нельзя
byte tap = 100;      //Переменная для переключения экранов
bool tapB = false;   //Положение кнопки (false == кнопка отпущена)
byte button = 4;     //Кнопка
byte rele = 8;       //Реле
byte fRez = 17;      //Фоторезистор
int MinLight;  //Освешенности ниже которой нужно включить реле
int MaxLight;  //Освещенность выше которой нужно выключить реле
bool releB = false;  //Переменная говорящая в каком положении реле
int Light1 = 1024;  //Переменная для проверки освещенности №1
int Light2 = 1024;  //Переменная для проверки освещенности №2
int Light3 = 1024;  //Переменная для проверки освещенности №3
int HourNight;  //Час выключения света
int MinuteNight;//Минута выключения света
int HourMorning;  //Час когда свет может включаться
int MinuteMorning;//Минута когда свет может включаться
bool Settings = false;
bool LongPressB = false;
byte s = 0;            //Для энкодера
int x = 0;            //Ответ от энкодера
byte nkoder1 = 5;      //Энкодер пин 1
byte nkoder2 = 11;     //Энкодер пин 2
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
  lcd.backlight();                 //Включение подсветки дисплея
  
//Загрузка занных из энергонезависимой памяти
  MaxLight = EEPROM_int_read(0);
  if ((MaxLight < 0)or(MaxLight > 1024)) MaxLight = 500;
  MinLight = EEPROM_int_read(2);
  if ((MinLight < 0)or(MinLight > 1024)) MinLight = 300;
  HourNight = EEPROM_int_read(4);
  if ((HourNight < 0)or(HourNight > 23)) HourNight = 0;
  MinuteNight = EEPROM_int_read(6);
  if ((MinuteNight < 0)or(MinuteNight > 59)) MinuteNight = 0;
  HourMorning = EEPROM_int_read(8);
  if ((HourMorning < 0)or(HourMorning > 23)) HourMorning = 6;
  MinuteMorning = EEPROM_int_read(10);
  if ((MinuteMorning < 0)or(MinuteMorning > 59)) MinuteMorning = 0;
/////////////////

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
    ChangeSetting = millis();
  }
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
          EEPROM_int_write(0, MaxLight);
          EEPROM_int_write(2, MinLight);
          EEPROM_int_write(4, HourNight);
          EEPROM_int_write(6, MinuteNight);
          EEPROM_int_write(8, HourMorning);
          EEPROM_int_write(10, MinuteMorning);
        } else {
          if ((Settings == false)and(millis() - LongPress > 3000)){
            Settings = true;
            LongPressB = true;
            tap = 1;
            Sleap = false;
          }
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
        if (tap < 9){
          tap++;
        }else{
          tap = 1;
        }
        if ((tap > 3)and(tap < 100)){
          EEPROM_int_write(0, MaxLight);
          EEPROM_int_write(2, MinLight);
          EEPROM_int_write(4, HourNight);
          EEPROM_int_write(6, MinuteNight);
          EEPROM_int_write(8, HourMorning);
          EEPROM_int_write(10, MinuteMorning);
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
    Serial.println(TimeCheck());
    if ((analogRead(fRez) < MinLight)and(Light1 < MinLight)and(Light2 < MinLight)and(Light3 < MinLight)and(releB == false)and(TimeCheck() == false)){
      digitalWrite(rele, HIGH);
      releB = true;
    }else{
      if (((analogRead(fRez) > MaxLight)and(Light1 > MaxLight)and(Light2 > MaxLight)and(Light3 > MaxLight)and(releB == true))or(TimeCheck() == true)){
        digitalWrite(rele, LOW);
        releB = false;
      }
    }
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
    if (millis() - ChangeSetting < 1000){
      blinking = 1;
    }
    switch (tap) {
      case 1:
        if (RTC.read(tm)) {
          if (x != 0){
            if ((tm.Hour == 23)and(x > 0)) {tm.Hour = 0; x = 0;}
            if ((tm.Hour == 0)and(x < 0)) {tm.Hour = 23; x = 0;}
            tm.Hour = tm.Hour + x;
            RTC.write(tm);
            x = 0;
            ChangeSetting = millis();
          }
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
          tm.Hour = 0;
          tm.Minute = 0;
          tm.Second = 0;
          RTC.write(tm);
        }
        break;
      case 2:
        if (RTC.read(tm)) {
          if (x != 0){
            if ((tm.Minute == 59)and(x > 0)) {tm.Minute = 0; x = 0;}
            if ((tm.Minute == 0)and(x < 0)) {tm.Minute = 59; x = 0;}
            tm.Minute = tm.Minute + x;
            RTC.write(tm);
            x = 0;
            ChangeSetting = millis();
          }
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
          tm.Hour = 0;
          tm.Minute = 0;
          tm.Second = 0;
          RTC.write(tm);
        }
        break;
      case 3:
        if (RTC.read(tm)) {
          if (x != 0){
            if ((tm.Second == 59)and(x > 0)) {tm.Second = 0; x = 0;}
            if ((tm.Second == 0)and(x < 0)) {tm.Second = 59; x = 0;}
            tm.Second = tm.Second + x;
            RTC.write(tm);
            x = 0;
            ChangeSetting = millis();
          }
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
          tm.Hour = 0;
          tm.Minute = 0;
          tm.Second = 0;
          RTC.write(tm);
        }
        break;
      case 4:
        lcd.setCursor(0, 0);  
        lcd.print("TimeNight:");
        lcd.setCursor(0, 1);
        if (x != 0){
          HourNight = HourNight + x;
          if (HourNight > 23) HourNight = 0;
          if (HourNight < 0) HourNight = 23;
          x = 0;
          ChangeSetting = millis();
        }
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
        if (x != 0){
          MinuteNight = MinuteNight + x;
          if (MinuteNight > 59) MinuteNight = 0;
          if (MinuteNight < 0) MinuteNight = 59;
          x = 0;
          ChangeSetting = millis();
        }
        if (blinking <= 2){
          lcd.print(time0(String(HourNight), 2) + ":" + time0(String(MinuteNight), 2));
        }else{
          lcd.print(time0(String(HourNight), 2) + ":  ");
        }
        break;
      case 6:
        lcd.setCursor(0, 0);  
        lcd.print("TimeMorning:");
        lcd.setCursor(0, 1);
        if (x != 0){
          HourMorning = HourMorning + x;
          if (HourMorning > 23) HourMorning = 0;
          if (HourMorning < 0) HourMorning = 23;
          x = 0;
          ChangeSetting = millis();
        }
        if (blinking <= 2){
          lcd.print(time0(String(HourMorning), 2) + ":" + time0(String(MinuteMorning), 2));
        }else{
          lcd.print("  :" + time0(String(MinuteMorning), 2));
        }
        break;
      case 7:
        lcd.setCursor(0, 0);  
        lcd.print("TimeMorning:");
        lcd.setCursor(0, 1);
        if (x != 0){
          MinuteMorning = MinuteMorning + x;
          if (MinuteMorning > 59) MinuteMorning = 0;
          if (MinuteMorning < 0) MinuteMorning = 59;
          x = 0;
          ChangeSetting = millis();
        }
        if (blinking <= 2){
          lcd.print(time0(String(HourMorning), 2) + ":" + time0(String(MinuteMorning), 2));
        }else{
          lcd.print(time0(String(HourMorning), 2) + ":  ");
        }
        break;
      case 8:
        lcd.setCursor(0, 0);  
        lcd.print("Min.Light:");
        lcd.setCursor(0, 1);
        if (x != 0){
          MinLight = MinLight + (x * 10);
          if (MinLight > 1024) MinLight = 1024;
          if (MinLight < 0) MinLight = 0;
          x = 0;
          ChangeSetting = millis();
        }
        if (blinking <= 2){
          lcd.print(time0(String(MinLight), 4));
        }else{
          lcd.print("    ");
        }
        break;
      case 9:
        lcd.setCursor(0, 0);  
        lcd.print("Max.Light:");
        lcd.setCursor(0, 1);  
        if (x != 0){
          MaxLight = MaxLight + (x * 10);
          if (MaxLight > 1024) MaxLight = 1024;
          if (MaxLight < 0) MaxLight = 0;
          x = 0;
          ChangeSetting = millis();
        }
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
}
/////////////////

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

//Сравнение времени (true = время входит в диопозон)
bool TimeCheck(){
  tmElements_t tmp;
  RTC.read(tmp);
  String Nig, Mor, Ti;
  Nig = String(HourNight) + time0(String(MinuteNight), 2);
  Mor = String(HourMorning) + time0(String(MinuteMorning), 2);
  Ti = time0(String(tmp.Hour), 2) + time0(String(tmp.Minute), 2);
  if (((Nig.toInt() < Ti.toInt())and(Ti.toInt() < Mor.toInt())) or (((Nig.toInt() > Mor.toInt())and(Ti.toInt() > Nig.toInt()))or((Ti.toInt() < Nig.toInt())and(Ti.toInt() < Mor.toInt())))){
    return (true);
  }else{
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
          if ((digitalRead(nkoder2) == 1)and(digitalRead(nkoder1) == 1)) {s = 2;}else{s = 0;}}
        break;
      case 10:
        if ((digitalRead(nkoder1) == 1)and(digitalRead(nkoder2) == 0)) {s = 10;}else{
          if ((digitalRead(nkoder1) == 1)and(digitalRead(nkoder2) == 1)) {s = 11;}else{s = 0;}}
        break;
      case 2:
        if ((digitalRead(nkoder2) == 1)and(digitalRead(nkoder1) == 1)) {s = 2;}else{
          if ((digitalRead(nkoder2) == 0)and(digitalRead(nkoder1) == 1)) {s = 3;}else{s = 0;}}
        break;
      case 11:
        if ((digitalRead(nkoder1) == 1)and(digitalRead(nkoder2) == 1)) {s = 11;}else{
          if ((digitalRead(nkoder1) == 0)and(digitalRead(nkoder2) == 1)) {s = 12;}else{s = 0;}}
        break;
      case 3:
        if ((digitalRead(nkoder2) == 0)and(digitalRead(nkoder1) == 1)) {s = 3;}else{
          if ((digitalRead(nkoder2) == 0)and(digitalRead(nkoder1) == 0)) {s = 0;x = 1;}else{s = 0;}}
        break;
      case 12:
        if ((digitalRead(nkoder1) == 0)and(digitalRead(nkoder2) == 1)) {s = 12;}else{
          if ((digitalRead(nkoder1) == 0)and(digitalRead(nkoder2) == 0)) {s = 0;x = -1;}else{s = 0;}}
        break;
  }
}
/////////////////

//Чтение переменной int из памяти EEPROM
int EEPROM_int_read(int addr) {
  byte raw[2];
  for(byte i = 0; i < 2; i++) raw[i] = EEPROM.read(addr+i);
  int &num = (int&)raw;
  return num;
}
/////////////////

//Запись переменной int из памяти EEPROM
void EEPROM_int_write(int addr, int num) {
  if (EEPROM_int_read(addr)!= num){//если сохраняемое отличается
    byte raw[2];
    (int&)raw = num;
    for(byte i = 0; i < 2; i++) EEPROM.write(addr+i, raw[i]);
  }
}
/////////////////
