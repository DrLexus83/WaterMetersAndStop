#include <EEPROM.h>                   //  Подключаем библиотеку для работы с EEPROM
#include <Wire.h>                     //  Подключаем библиотеку для работы с шиной I2C
#include <LiquidCrystal_I2C.h>        //  Подключаем библиотеку для работы с LCD дисплеем по шине I2C
LiquidCrystal_I2C lcd(0x27,20,4);     //  Объявляем  объект библиотеки, указывая параметры дисплея (адрес I2C = 0x27, количество столбцов = 20, количество строк = 4)
#include <GyverButton.h>              //  Подключаем библиотеку для работы с кнопкой
GButton butt1(16);                    //  Пин кнопки подсветка/сброс

float counterstart_1 = 0 ; // Начальные показания ХВС 1
float counterstart_2 = 19.10; // Начальные показания ГВС 1
float counterstart_3 = 0; // Начальные показания ХВС 2
float counterstart_4 = 8.37; // Начальные показания ГВС 2
float counter_1; //текущие показания ХВС 1
float counter_2; //текущие показания ГВС 1
float counter_3; //текущие показания ХВС 2
float counter_4; //текущие показания ГВС 2
float counter_11; //показания ХВС 1 в EEPROM
float counter_22; //показания ГВС 1 в EEPROM
float counter_33; //показания ХВС 2 в EEPROM
float counter_44; //показания ГВС 2 в EEPROM
float counter_111; //показания ХВС 1 за период
float counter_222; //показания ГВС 1 за период
float counter_333; //показания ХВС 2 за период
float counter_444; //показания ГВС 2 за период
float counter_1111; //показания ХВС 1 в EEPROM за период
float counter_2222; //показания ГВС 1 в EEPROM за период
float counter_3333; //показания ХВС 2 в EEPROM за период
float counter_4444; //показания ГВС 2 в EEPROM за период
bool meter1_flag = 0; //служебный флаг счетчика ХВС 1
bool meter2_flag = 0; //служебный флаг счетчика ГВС 1
bool meter3_flag = 0; //служебный флаг счетчика ХВС 2
bool meter4_flag = 0; //служебный флаг счетчика ГВС 2
bool Light = 0; //флаг подсветки дисплея
uint32_t last_press = 0; //переменная таймера антидребезга
uint32_t timer1 = 0; //переменная таймера автопроворота
uint32_t leak1_timer; //переменная таймера срабатывания первой группы датчиков
uint32_t leak2_timer; //переменная таймера срабатывания второй группы датчиков
uint32_t tone_timer; //переменная таймера сирены
uint32_t display_timer; //переменная таймера обновления информации на дисплее
bool manual_flag = 0; //служебный флаг нажатия кнопки ручного режима
bool manual_mode = 0; //флаг включения ручного режима
bool water1_leak = false; //флаг авария первая группа
bool water2_leak = false; //флаг авария вторая группа
bool leak1_flag = false; //флаг срабатывания первой группы датчиков
bool leak2_flag = false; //флаг срабатывания второй группы датчиков
byte water1_open; //флаг открытия крана туалет, 0 - закрыто, 1 - открыто
byte water2_open; //флаг открытия крана ванная, 0 - закрыто, 1 - открыто
bool water1; //служебный флаг нажатия кнопки управления краном 1
bool water2; //служебный флаг нажатия кнопки управления краном 1
bool autoturn = false; //флаг включения функции автопроворота кранов
bool tone_flag; //флаг сирены

void setup() {
  
  butt1.setDebounce(100);        // настройка антидребезга кнопки дисплея (по умолчанию 80 мс)
  butt1.setTimeout(5000);        // настройка таймаута на удержание (по умолчанию 500 мс)
  butt1.setClickTimeout(300);   // настройка таймаута между кликами (по умолчанию 300 мс)
  // HIGH_PULL - кнопка подключена к GND, пин подтянут к VCC (PIN --- КНОПКА --- GND)
  // LOW_PULL  - кнопка подключена к VCC, пин подтянут к GND
  butt1.setType(HIGH_PULL);
  // NORM_OPEN - нормально-разомкнутая кнопка
  // NORM_CLOSE - нормально-замкнутая кнопка
  butt1.setDirection(NORM_OPEN);
  
 pinMode(2, INPUT_PULLUP); //вход счетчика ХВС 1
 pinMode(3, INPUT_PULLUP); //вход счетчика ГВС 1
 pinMode(4, INPUT_PULLUP); //вход счетчика ХВС 2
 pinMode(5, INPUT_PULLUP); //вход счетчика ГВС 2
 pinMode (6, INPUT_PULLUP); //вход датчика туалет
 pinMode (7, INPUT_PULLUP); //вход датчика ванная
 pinMode (8, INPUT_PULLUP); //вход кнопки кран туалет
 pinMode (9, INPUT_PULLUP); //вход кнопки кран ванная
 pinMode (10, INPUT_PULLUP); //вход кнопки активации ручного режима
 pinMode (11, OUTPUT); //выход реле кран туалет
 pinMode (12, OUTPUT); //выход реле кран ванная
 pinMode (13, OUTPUT); //выход на светодиод индикации включения ручного режима
 pinMode (14, OUTPUT); //выход на светодиод индикации аварии
 pinMode (15, OUTPUT); //выход на динамик
  
//проверка состояния кранов до отключения питания
 EEPROM.get(150, water1_open);  //читаем данные из EEPROM - был ли открыт кран 1
 EEPROM.get(151, water2_open); //читаем данные из EEPROM - был ли открыт кран 2

 //Установка начала отсчета при запуске
 //Счетчик ХВС 1  
 EEPROM.get(0, counter_1); // читаем данные из EEPROM
 EEPROM.get(64, counter_111); // читаем данные из EEPROM 
 if (counter_1 < counterstart_1)  //сравниваем данные из EEPROM с начальными показаниями 
 {
 counter_1 = counterstart_1; //если значение в памяти меньше, отсчет начинается со стартового значения
 EEPROM.put(0, counterstart_1); //запись начального значения в память
 }
//Счетчик ГВС 1
 EEPROM.get(16, counter_2); // читаем данные из EEPROM 
 EEPROM.get(80, counter_222); // читаем данные из EEPROM
 if (counter_2 < counterstart_2)  //сравниваем данные из EEPROM с начальными показаниями 
 {
 counter_2 = counterstart_2; //если значение в памяти меньше, отсчет начинается со стартового значения
 EEPROM.put(16, counterstart_2); //запись начального значения в память
 }
 //Счетчик ХВС 2
 EEPROM.get(32, counter_3); // читаем данные из EEPROM
 EEPROM.get(96, counter_333); // читаем данные из EEPROM  
 if (counter_3 < counterstart_3)  //сравниваем данные из EEPROM с начальными показаниями 
 {
 counter_3 = counterstart_3; //если значение в памяти меньше, отсчет начинается со стартового значения
 EEPROM.put(32, counterstart_3); //запись начального значения в память
 }
 //Счетчик ГВС 2
 EEPROM.get(48, counter_4); // читаем данные из EEPROM 
 EEPROM.get(112, counter_444); // читаем данные из EEPROM 
 if (counter_4 < counterstart_4)  //сравниваем данные из EEPROM с начальными показаниями 
 {
 counter_4 = counterstart_4; //если значение в памяти меньше, отсчет начинается со стартового значения
 EEPROM.put(48, counterstart_4); //запись начального значения в память
 }
 
 lcd.init();    // Инициализация lcd             
 }

void loop() {

//БЛОК АКВАСТОРОЖА

 if (autoturn == false) { 
 digitalWrite(11, water1_open); //открываем-закрываем кран в зависимости от состояния флага open
 digitalWrite(12, water2_open); //открываем-закрываем кран в зависимости от состояния флага open
 }
 
 digitalWrite(13, manual_mode); //включаем-выключаем индикатор ручного режима
 
 if (water1_leak == true || water2_leak == true) {  //если случилась авария
  digitalWrite(14, 1);  //включаем сигнализатор аварии
  if (tone_flag == false && millis() - tone_timer > 500) {  //запускаем сирену
  tone(15, 400); 
  tone_flag = true;
  tone_timer = millis();
  } 
  if (tone_flag == true && millis() - tone_timer > 500) {
    tone(15, 800);  
    tone_flag = false;
    tone_timer = millis();
  }
 }
 if (water1_leak == false && water2_leak == false) {
  digitalWrite(14, 0); //выключаем сигнализатор аварии
  noTone(15);
 }

 //включаем-выключаем ручной режим
   if (!digitalRead(10) == 1 && manual_flag == 0 && millis() - last_press > 100) { 
    manual_flag = 1;
    manual_mode = !manual_mode;
    last_press = millis();
    }
    if (!digitalRead(10) == 0 && manual_flag == 1) {
    manual_flag = 0;
    }
 
 //автоматический режим 
  if (manual_mode == false) {   
    if (digitalRead(6) == 0 && leak1_flag == false) {   //опрос датчиков 1 группы
      leak1_timer = millis();
      leak1_flag = true;
    }
      if (leak1_flag == true && millis() - leak1_timer > 5000) { 
      water1_leak = true;
      water1_open = 0;
      EEPROM.put(150, water1_open);
      }
    if (digitalRead(6) == 1 && leak1_flag == true) {   
       leak1_flag = false;
    }
    
     if (digitalRead(7) == 0 && leak2_flag == false) {   //опрос датчиков 1 группы
      leak2_timer = millis();
      leak2_flag = true;
    }
      if (leak2_flag == true && millis() - leak2_timer > 5000) { 
      water2_leak = true;
      water2_open = 0;
      EEPROM.put(151, water2_open);
      }
    if (digitalRead(7) == 1 && leak2_flag == true) {   
       leak2_flag = false;
    }
  }
  
   //ручной режим
  if (manual_mode == true) {             
    water1_leak = false; //отключаем режим аварии
    water2_leak = false;
    noTone(15);
   
    if (!digitalRead(8) == 1 && water1 == 0 && millis() - last_press > 100) {  //управление краном 1
    water1 = 1;
      if (water1_open == 1) {
      water1_open = 0;
    }
      else {
      water1_open = 1;
    }
      EEPROM.put(150, water1_open);
      last_press = millis();
    }
   if (!digitalRead(8) == 0 && water1 == 1) {
    water1 = 0;
    }
   
    if (!digitalRead(9) == 1 && water2 == 0 && millis() - last_press > 100) {   //управление краном 2
    water2 = 1;
      if (water2_open == 1) {
      water2_open = 0;
    }
      else {
      water2_open = 1;
    }
      EEPROM.put(151, water2_open);
      last_press = millis();
    }
    if (!digitalRead(9) == 0 && water2 == 1) {
    water2 = 0;
    }
  }

//автоматический проворот крана 1 раз в ~3 недели. Статус флагов открытия не меняем, чтобы бывшие открытыми краны затем автоматически открылись.
//Функция активна только при открытых кранах.
  if ((millis() - timer1) > 2147483647) {  
    if (water1_open == 1) {  
      digitalWrite(11, 0);  //закрываем первый кран если он открыт
      autoturn = true;
    }
    if (water2_open == 1) {  
      digitalWrite(12, 0);  //закрываем второй кран если он открыт
      autoturn = true;
    }
    timer1 = millis();
  }
  if (autoturn == true && (millis() - timer1) > 30000) {
    autoturn = false;
    }

//сброс значений, зависящих от millis при переполнении и обнулении таймера. Необходимо для корректной работы антидребезга и автопрокрутки.
  if (last_press > millis()) {
    last_press = millis();
  }
  if (timer1 > millis()) {
    timer1 = millis();
  }

//БЛОК РЕГИСТРАТОРА СЧЕТЧИКОВ
      
 //Счетчик ХВС 1
  if (!digitalRead(2) == 1 && meter1_flag == 0 && millis() - last_press > 100) { 
    meter1_flag = 1;
    counter_1 = counter_1 + 0.01;
    counter_111 = counter_111 + 0.01;
    last_press = millis();
    }
    if (!digitalRead(2) == 0 && meter1_flag == 1) {
    meter1_flag = 0;
    }
  EEPROM.get(0, counter_11); //вызываем записанное число из EEPROM
  if (counter_1 - counter_11 > 0.09) //сравниваем текущее значение счетчика с данными в EEPROM
  {
  EEPROM.put(0, counter_1); //записываем новые данные в память через каждые 10 импульсов - экономим ресурс EEPROM
  }
  EEPROM.get(64, counter_1111); //вызываем записанное число из EEPROM
    if (counter_111 - counter_1111 > 0.09) //сравниваем текущее значение счетчика с данными в EEPROM
  {
  EEPROM.put(64, counter_111); //записываем новые данные в память через каждые 10 импульсов - экономим ресурс EEPROM
  }
  
 //Счетчик ГВС 1
 if (!digitalRead(3) == 1 && meter2_flag == 0 && millis() - last_press > 100) { 
    meter2_flag = 1;
    counter_2 = counter_2 + 0.01;
    counter_222 = counter_222 + 0.01;
    last_press = millis();
    }
    if (!digitalRead(3) == 0 && meter2_flag == 1) {
    meter2_flag = 0;
    }
  EEPROM.get(16, counter_22); //вызываем записанное число из EEPROM
  if (counter_2 - counter_22 > 0.09) //сравниваем текущее значение счетчика с данными в EEPROM
  {
  EEPROM.put(16, counter_2); //записываем новые данные в память через каждые 10 импульсов - экономим ресурс EEPROM
  }
  EEPROM.get(80, counter_2222); //вызываем записанное число из EEPROM
  if (counter_222 - counter_2222 > 0.09) //сравниваем текущее значение счетчика с данными в EEPROM
  {
  EEPROM.put(80, counter_222); //записываем новые данные в память через каждые 10 импульсов - экономим ресурс EEPROM
  }
  
  //Счетчик ХВС 2
  if (!digitalRead(4) == 1 && meter3_flag == 0 && millis() - last_press > 100) { 
    meter3_flag = 1;
    counter_3 = counter_3 + 0.01;
    counter_333 = counter_333 + 0.01;
    last_press = millis();
    }
    if (!digitalRead(4) == 0 && meter3_flag == 1) {
    meter3_flag = 0;
    }
  EEPROM.get(32, counter_33); //вызываем записанное число из EEPROM
  if (counter_3 - counter_33 > 0.09) //сравниваем текущее значение счетчика с данными в EEPROM
  {
  EEPROM.put(32, counter_3); //записываем новые данные в память через каждые 10 импульсов - экономим ресурс EEPROM
  }
  EEPROM.get(96, counter_3333); //вызываем записанное число из EEPROM
  if (counter_333 - counter_3333 > 0.09) //сравниваем текущее значение счетчика с данными в EEPROM
  {
  EEPROM.put(96, counter_333); //записываем новые данные в память через каждые 10 импульсов - экономим ресурс EEPROM
  }
  
  //Счетчик ГВС 2
  if (!digitalRead(5) == 1 && meter4_flag == 0 && millis() - last_press > 100) { 
    meter4_flag = 1;
    counter_4 = counter_4 + 0.01;
      counter_444 = counter_444 + 0.01;
    last_press = millis();
    }
    if (!digitalRead(5) == 0 && meter4_flag == 1) {
    meter4_flag = 0;
    }
  EEPROM.get(48, counter_44); //вызываем записанное число из EEPROM
  if (counter_4 - counter_44 > 0.09) //сравниваем текущее значение счетчика с данными в EEPROM
  {
  EEPROM.put(48, counter_4); //записываем новые данные в память через каждые 10 импульсов - экономим ресурс EEPROM
  }
  EEPROM.get(112, counter_4444); //вызываем записанное число из EEPROM
  if (counter_444 - counter_4444 > 0.09) //сравниваем текущее значение счетчика с данными в EEPROM
  {
  EEPROM.put(112, counter_444); //записываем новые данные в память через каждые 10 импульсов - экономим ресурс EEPROM
  }
 
 //Включаем/выключаем подсветку дисплея и делаем сброс по длительному нажатию

  butt1.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
 if (butt1.isHolded())
  { 
    counter_111 = 0;
    counter_222 = 0; 
    counter_333 = 0;
    counter_444 = 0;
    EEPROM.put(64, counter_111);
    EEPROM.put(80, counter_222);
    EEPROM.put(96, counter_333);
    EEPROM.put(112, counter_444);
  }
   if (butt1.isSingle()) 
{      if (Light == false)
  {
   Light = true; 
   lcd.clear();  //очистка экрана для профилактики либо убирания артефактов
   display_timer = millis() - 60001; //устанавливаем переменную таймера дисплея на 1 милисекунду больше чтобы не ждать пока обновится информация
   } else
 {
  Light = false;
 }
}  
  if (butt1.isTriple()) //запись в EEPROM по тройному клику (перед перезагрузкой чтобы не потерять данные)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    // пишем всё в EEPROM
    EEPROM.put(0, counter_1);
    EEPROM.put(16, counter_2);
    EEPROM.put(32, counter_3);
    EEPROM.put(48, counter_4);
    EEPROM.put(64, counter_111);
    EEPROM.put(80, counter_222);
    EEPROM.put(96, counter_333);
    EEPROM.put(112, counter_444);
    lcd.print("Stored");
    delay(500);
    lcd.clear();
  }
  
   //Отображаем информацию на дисплее
   lcd.setBacklight(Light);
   if (millis() - display_timer >= 60000) { //обновляем 1 раз в минуту - для счетчиков вполне достаточно. При изменении этой цифры меняй число в блоке управления подсветкой!
    display_timer = millis(); 
   lcd.setCursor(0, 0);     // устанавливаем курсор в 0-ом столбце, 1 строке (начинается с 0)
   lcd.print("Cold1 ");
   lcd.print(counter_1);
   lcd.setCursor(14, 0);
   lcd.print(counter_1111);
   lcd.setCursor(0, 1);     // устанавливаем курсор в 0-ом столбце, 2 строке (начинается с 0)
   lcd.print("Hot1  ");
   lcd.print(counter_2);
   lcd.setCursor(14, 1);
   lcd.print(counter_2222);
   lcd.setCursor(0, 2);     // устанавливаем курсор в 0-ом столбце, 3 строке (начинается с 0)
   lcd.print("Cold2 ");
   lcd.print(counter_3);
   lcd.setCursor(14, 2);
   lcd.print(counter_3333);
   lcd.setCursor(0, 3);     // устанавливаем курсор в 0-ом столбце, 4 строке (начинается с 0)
   lcd.print("Hot2  ");
   lcd.print(counter_4);
   lcd.setCursor(14, 3);
   lcd.print(counter_4444);
   } 
}
