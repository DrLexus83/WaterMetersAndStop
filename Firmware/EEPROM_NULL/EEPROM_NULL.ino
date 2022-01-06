//Обнулятор EEPROM
#include "EEPROM.h"
#include <Wire.h>                     //  Подключаем библиотеку для работы с шиной I2C
#include <LiquidCrystal_I2C.h>        //  Подключаем библиотеку для работы с LCD дисплеем по шине I2C
LiquidCrystal_I2C lcd(0x27,20,4);     //  Объявляем  объект библиотеки, указывая параметры дисплея (адрес I2C = 0x27, количество столбцов = 20, количество строк = 4)
void setup()
{
  pinMode(13, OUTPUT);
  // Проход всех ячеек(байтов) и запись в них нулей.
  for (int i = 0; i < EEPROM.length(); i++) {
   EEPROM.update(i, 0);
  }
  lcd.init();                            // Инициализация lcd             
  lcd.backlight();                       // Включаем подсветку
  lcd.setCursor(0, 0);     // устанавливаем курсор в 0-ом столбце, 1 строке (начинается с 0)
  lcd.print("ALL EEPROM NULL");
  lcd.setCursor(0, 1);     // устанавливаем курсор в 0-ом столбце, 2 строке (начинается с 0)
  lcd.print("PLEASE PUT FIRMWARE");
  digitalWrite(13, HIGH); //включаем светодиод на плате
}

void loop()
{
   
             
}
