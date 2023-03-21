#include <SoftwareSerial.h>
#include <EEPROM.h> // EEPROM lib.
#include "HX711.h" // HX711 lib. https://github.com/bogde/HX711 or
#include "LiquidCrystal_PCF8574.h"
#include <Wire.h>
SoftwareSerial mySerial(4, 3); // Выводы SIM800L Tx & Rx подключены к выводам Arduino 3 и 2
LiquidCrystal_PCF8574 lcd(0x27); // set the LCD address to 0x27 for a 16 chars and 2 line display
 
String UserPhone="+79039931289";//618643419
String SystemPhone="";
 
char ch = 0;
 
char csq = 0;
 
char GSMPow1[5]={0,0,0,0,0};
char BatPow1[5]={0,0,0,0,0};
String GSMPow;
String BatPow;
byte pin2sleep=5;
byte pinScreen=16;
byte pinZero=15;
byte MaxScreen=2;

unsigned int ScaleStartAdress=100;
unsigned int YesterdayWeightAdress=200;
 
#define SENSORCNT 8
HX711 scale[SENSORCNT];
float delta0[SENSORCNT]; // delta weight from start
float delta1[SENSORCNT]; // delta weight from yesterday
float delta2[SENSORCNT]; // delta weight from 2xyesterday
float deltaLast1[SENSORCNT]; //
float deltaLast2[SENSORCNT]; //
 
float raw0[SENSORCNT]; //raw data from sensors on first start
float raw1[SENSORCNT]; //raw data from sensors on yesterday
float raw11[SENSORCNT]; //raw data from sensors on 2x yesterday
float raw2[SENSORCNT]; //actual raw data from sensors
//float swarmw[SENSORCNT]; //weight data for swarm warning
//float w[SENSORCNT]; //weight data for swarm warning
//unsigned long swarm0t[SENSORCNT]; //time for swarm warning
word calibrate[SENSORCNT];; //calibration factor for each sensor
bool ActiveSensors[SENSORCNT];
int AnyActiveScale=-1;
//word daynum=0; //numbers of day after start
//int notsunset=0;
//int Hwarning=0;
//int Halarm=0;
byte ScreenNumber=0;
unsigned int LoopCnt=0;
bool GSMExist=0;

 
boolean setZero=false;

boolean Button[8];
boolean ButtonOld[8];
boolean Photo;
boolean PhotoOld;
void MakeMeasureStart()
{
if(AnyActiveScale==-1)return;
lcd.home();
lcd.clear();
lcd.print("PowerUp sensors");
for(int i=0;i<SENSORCNT;i++)
    if(ActiveSensors[i])
      scale[i].power_up(); //power up all scales
lcd.home();
lcd.clear();
lcd.print("WarmUp sensors");
  raw2[AnyActiveScale]=scale[AnyActiveScale].get_units(240); //warm-up Wheatstone bridges and hx711-s
}

void MakeMeasure(int Cycles)
{
if(AnyActiveScale==-1)return;
for(int i=0;i<SENSORCNT;i++)
  if(ActiveSensors[i])
    raw2[i]=scale[i].get_units(Cycles);
for(int i=0;i<SENSORCNT;i++)if(ActiveSensors[i])
  {
  delta0[i]=(raw2[i]-raw0[i])/calibrate[i]; // calculate weight changes from start
  delta1[i]=(raw2[i]-raw1[i])/calibrate[i];// calculate weight changes from yesterday
  delta2[i]=(raw2[i]-raw11[i])/calibrate[i];// calculate weight changes from 2xyesterday
  deltaLast1[i]=delta0[i]-delta1[i];
  deltaLast2[i]=delta1[i]-delta2[i];
  }
}
void MakeMeasureStop()
{
if(AnyActiveScale==-1)return;
for(int i=0;i<SENSORCNT;i++)
  if(ActiveSensors[i])
    scale[i].power_down();
}

void SetZero(int SensorNum)
{
  int adress=ScaleStartAdress+SensorNum*4;
  raw0[SensorNum]=raw2[SensorNum];
  EEPROM.put(adress, raw0[SensorNum]);
}
void GetEPROMData()
{
  int adress;
  for(int i=0;i<SENSORCNT;i++)
    {
    adress=ScaleStartAdress+i*4;
    EEPROM.get(adress, raw0[i]);
    }
  for(int i=0;i<SENSORCNT;i++)
    {
    adress=YesterdayWeightAdress+i*4;
    EEPROM.get(adress, raw11[i]);
    adress=YesterdayWeightAdress+i*4+SENSORCNT*4;
    EEPROM.get(adress, raw1[i]);
    }
}
void NightUpdateAndSave()
{
  int adress;
  for(int i=0;i<SENSORCNT;i++)
    {
    raw11[i]=raw1[i];
    raw1[i]=raw2[i];
    adress=YesterdayWeightAdress+i*4;
    EEPROM.put(adress, raw11[i]);
    adress=YesterdayWeightAdress+i*4+SENSORCNT*4;
    EEPROM.put(adress, raw1[i]);
    }
}
 
void setup()
{
 //L2C
  Wire.begin();
  Wire.beginTransmission(0x27);
  int error = Wire.endTransmission();
  //Serial.print("Error: ");
  //Serial.print(error);
 
  if (error == 0) {
  //  Serial.println(": LCD found.");
    lcd.begin(16, 2); // initialize the lcd
//lcd.print("jklj");
  } else {
   // Serial.println(": LCD not found.");
  } // if
lcd.setBacklight(40);
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Initialising...");

scale[0].begin(6, 14);
scale[1].begin(7, 14);
scale[2].begin(8, 14);
scale[3].begin(9, 14);
scale[4].begin(10, 14);
scale[5].begin(11, 14);
scale[6].begin(12, 14);
scale[7].begin(13, 14);


  /*
pinMode(14, OUTPUT);
pinMode(6, INPUT);
pinMode(7, INPUT);
pinMode(8, INPUT);
pinMode(9, INPUT);
pinMode(10, INPUT);
pinMode(11, INPUT);
pinMode(12, INPUT);

scale[0].begin(14, 6);
scale[0].begin(14, 7);
scale[0].begin(14, 8);
scale[0].begin(14, 9);
scale[0].begin(14, 10);
scale[0].begin(14, 11);
scale[0].begin(14, 12);
scale[0].begin(14, 13);
*/



calibrate[0]=20880;
calibrate[1]=20880;
calibrate[2]=20880;
calibrate[3]=20880;
calibrate[4]=20880;
calibrate[5]=20880;
calibrate[6]=20880;
calibrate[7]=20880;
for(int i=0;i<SENSORCNT;i++)
  {
  delta0[i]=0; // delta weight from start
  delta1[i]=0; // delta weight from yesterday
  delta2[i]=0; // delta weight from 2xyesterday
  raw0[i]=0; //raw data from sensors on first start
  raw1[i]=0; //raw data from sensors on yesterday
  raw11[i]=0; //raw data from sensors on 2xyesterday
  raw2[i]=0; //actual raw data from sensors
//  swarmw[i]=0; //weight data for swarm warning
//  w[i]=0; //weight data for swarm warning
//  swarm0t[i]=0; //time for swarm warning
  }
ScreenNumber=0;
   pinMode(pin2sleep, OUTPUT);
   pinMode(2, INPUT_PULLUP);
   pinMode(pinScreen, INPUT_PULLUP);
   pinMode(pinZero, INPUT_PULLUP);


   
//   Serial.begin(9600);
switchto9600(); //for AiThinker GSM A6
mySerial.begin(9600);
int Cnt=0;
while(!mySerial.available() && Cnt<10){ // Зацикливаем и ждем инициализацию SIM800L
    mySerial.println("AT"); // Отправка команды AT
    delay(1000); // Пауза
    //Serial.println("Connecting…"); // Печатаем текст
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting GSM");
    Cnt++;
    }
GSMExist=Cnt<10;
lcd.clear();
lcd.setCursor(0, 0);
if(GSMExist)
{
lcd.print("GSM exist");
}
else
{
lcd.print("GSM not fond");

}
delay(1000);
if(GSMExist)
{
   // Serial.println("Connected!"); // Печатаем текст
    mySerial.println("AT+CMGF=1"); // Отправка команды AT+CMGF=1
    delay(1000); // Пауза
    mySerial.println("AT+CNMI=1,2,0,0,0"); // Отправка команды AT+CNMI=1,2,0,0,0
    delay(1000); // Пауза
    mySerial.println("AT+CMGL=\"REC UNREAD\"");
}
GetEPROMData();


for(int i=0;i<SENSORCNT;i++)
  {
  if(scale[i].wait_ready_timeout(1000))
    {
      if(AnyActiveScale==-1)AnyActiveScale=i;
      ActiveSensors[i]=1;
    }
  else ActiveSensors[i]=0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scale platform "+String(i+1));
  lcd.setCursor(0, 1);
  if(ActiveSensors[i])lcd.print("fond.");
  else lcd.print("not fond.");
  delay(1000);
  }



if(GSMExist)
{
readVcc();
SignalQ();
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Batt=");//ery
lcd.print(BatPow);

lcd.print("%");
lcd.setCursor(0, 1);
lcd.print("GSM sgn=");
lcd.print(GSMPow);
/*
lcd.print(GSMPow[0]);
if (GSMPow[1] != ',') 
  {
  lcd.print(GSMPow[1]);
  if (GSMPow[2] != ',') 
    {
    lcd.print(GSMPow[2]);
    if (GSMPow[3] != ',') {mySerial.print(GSMPow[3]);}
    }
  }
*/
//lcd.print(GSMPow);
lcd.println("of 31");
delay(5000);
digitalWrite(pin2sleep, HIGH); // Turn OFF modem
}

MakeMeasureStart();
//lcd.setCursor(0, 1);
//lcd.print("1");




    
//lcd.setCursor(0, 1);
//lcd.print("2");
//attachInterrupt(0, Int0 , RISING); // Interrupt by HIGH level
}
 
void Int0() //send weight
{
 
  detachInterrupt(digitalPinToInterrupt(0));
lcd.home();
lcd.clear();
lcd.print("Int 0");
delay(10000);
  attachInterrupt(0, Int0 , RISING);
}
void RePaint()
{
lcd.home();
//lcd.clear();
if(ScreenNumber==0)
{
    lcd.setCursor(0, 0);
    lcd.print("1-");
    lcd.print(String(delta0[0],(delta0[0]>=100)?1:2));
    lcd.setCursor(8, 0);
    lcd.print("2-");
    lcd.print(String(delta0[1],(delta0[1]>=100)?1:2));
    lcd.setCursor(0, 1);
    lcd.print("3-");
    lcd.print(String(delta0[2],(delta0[2]>=100)?1:2));
    lcd.setCursor(8, 1);
    lcd.print("4-");
    lcd.print(String(delta0[3],(delta0[3]>=100)?1:2));
}
else if(ScreenNumber==1)
{
    lcd.setCursor(0, 0);
    lcd.print("5-");
    lcd.print(String(delta0[4],(delta0[4]>=100)?1:2));
    lcd.setCursor(8, 0);
    lcd.print("6-");
    lcd.print(String(delta0[5],(delta0[5]>=100)?1:2));
    lcd.setCursor(0, 1);
    lcd.print("7-");
    lcd.print(String(delta0[6],(delta0[6]>=100)?1:2));
    lcd.setCursor(8, 1);
    lcd.print("8-");
    lcd.print(String(delta0[7],(delta0[7]>=100)?1:2));
}
else if(ScreenNumber==2)
{
    lcd.setCursor(0, 0);
    lcd.print("S1_w");
    lcd.setCursor(0, 1);
    lcd.print(String(raw2[0]/calibrate[0],(raw2[0]/calibrate[0]>=100)?1:2));
    lcd.setCursor(6, 0);
    lcd.print("S1_d1");
    lcd.setCursor(6, 1);
    lcd.print(String(deltaLast1[0],(deltaLast1[0]>=100)?1:2));
    lcd.setCursor(12, 0);
    lcd.print("S1_d2");
    lcd.setCursor(12, 1);
    lcd.print(String(deltaLast2[0],(deltaLast2[0]>=100)?1:2));
}
else if(ScreenNumber==3)
{
    lcd.setCursor(0, 0);
    lcd.print("Bat=");
    lcd.print(String(Button[0]));
    lcd.print(" ");
    lcd.print(String(Button[1]));
    lcd.print(" ");
    lcd.print(String(digitalRead(2)));
    
}
}
 
void loop()
{
if(AnyActiveScale==-1)
  {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("No platform");
  lcd.setCursor(0, 1);
  lcd.print("Measurement stopped");
  return;

    
  }

//lcd.setCursor(0, 1);
//lcd.print("3");
Button[0]=digitalRead(pinScreen);
Button[1]=digitalRead(pinZero);
Photo=digitalRead(2);
if(!Button[0] && ButtonOld[0])
{
ScreenNumber++;
if(ScreenNumber>MaxScreen)ScreenNumber=0;
}
//Set 0
if(!Button[1] && ButtonOld[1])
{
//MakeMeasureStart();
MakeMeasure(20);
//MakeMeasureStop();
for(int i=0;i<SENSORCNT;i++)SetZero(i);
}
if(Photo && !PhotoOld)
{
MakeMeasure(20);
if(GSMExist)SendScaleData();
NightUpdateAndSave();
}

//ScreenNumber=3;
//lcd.setCursor(0, 1);
//lcd.print("4");


 
if(LoopCnt%20==0)
  {
  MakeMeasure(5);
  lcd.clear();
  }
//lcd.print("5");

RePaint();



//lcd.print("6");

 /*
if(LoopCnt%1000==0)
  {
    //SendScaleData();
readVcc();
SignalQ();
  }
  
   if(mySerial.available() && LoopCnt%100==0)
        { // Проверяем, если есть доступные данные
   // delay(100); // Пауза
   // while(mySerial.available()){ // Проверяем, есть ли еще данные.
   // incomingByte = mySerial.read(); // Считываем байт и записываем в переменную incomingByte
   // inputString += incomingByte; // Записываем считанный байт в массив inputString
    //}
                      // Пауза
 
    
readVcc();
SignalQ();
mySerial.print("Battery capacity is:");
  Serial.print(BatPow1);
  Serial.println("%");
  Serial.print("GSM signal strenght:");
  Serial.print(GSMPow1);
  Serial.println(" of 31");
 
   
    // sms(String("Send"), String("+79618643419")); // Отправка SMS
 
        }
 */
for(int i=0;i<8;i++)ButtonOld[i]=Button[i];
PhotoOld=Photo;
LoopCnt++;
delay(20);
}
 
void SendScaleData()
{
lcd.home();
lcd.clear();
lcd.print("Sending SMS.");
digitalWrite(pin2sleep, LOW); // Turn ON modem
delay(5000); // Wait for modem booting
MakeMeasure(20);
String Txt="";
for(int i=0;i<SENSORCNT;i++)
  {
  if(delta0[i]>0)
    {
      Txt=Txt+"Hive"+String(i+1)+"="+String(delta0[i],2)+" ("+String(delta1[i],2)+")"+String(char(10));
    }
  }
if(Txt=="")Txt=Txt+"No sensors"+String(char(10));
 
readVcc();
SignalQ();
 
Txt=Txt+"Battery capacity is:"+BatPow+"%"+String(char(10));
Txt=Txt+"GSM signal strenght:"+GSMPow+" of 31"+String(char(10));
if(UserPhone!="")sms(Txt,UserPhone);
if(SystemPhone!="")sms(Txt,SystemPhone);
 
digitalWrite(pin2sleep, HIGH); // Turn OFF modem
lcd.home();
lcd.print("Sending SMS");
lcd.setCursor(0, 1);
lcd.print("complite.");
delay(3000);
}
 
 void sms(String text, String phone) // Процедура Отправка SMS
 {
   //Serial.println("SMS send started");
   mySerial.println("AT+CMGS=\"" + phone + "\"");
   delay(500);
   mySerial.print(text);
   delay(500);
   mySerial.print((char)26);
   delay(500);
   //Serial.println("SMS send complete");
   //delay(2000);
 }
 
void readVcc() // Функция чтения заряда батареи из GSM-модуля
{
for (int i = 0; i <= 5 ; i++) BatPow1[i]=0;
  
  ch = mySerial.read();
   while (mySerial.available() > 0) { ch = mySerial.read(); } // очищаем входной буфер порта
 
 mySerial.println("AT+CBC?"); //запрашивем статус батареи у GSM-модуля
// delay(200);
for (int i = 0; i <= 20 ; i++) { delayMicroseconds(10000);}
 while (mySerial.available() > 0) { // считываем входную строку после запятой
 ch = mySerial.read();
 //Serial.println(ch); // Печатаем текст
    
     if (ch ==','){
      BatPow1[0]=mySerial.read();
      BatPow1[1]=mySerial.read();
      BatPow1[2]=mySerial.read();
      BatPow1[3]=mySerial.read();
      BatPow1[4]=0;
     }
   }

for(int i=0;i<4;i++)
if(BatPow1[i]<'0' || BatPow1[i]>'9')
  {BatPow1[i]=' ';
  //  for(int j=i;i<4;j++){BatPow1[j]=BatPow1[j+1];}
  }

BatPow=BatPow1;
}
 
void SignalQ() // Считывание мощности сигнала GSM
{
for (int i = 0; i <= 5 ; i++) GSMPow1[i]=0;
  csq = mySerial.read();
   while (mySerial.available() > 0) { csq = mySerial.read(); } // очищаем входной буфер порта
 
 mySerial.println("AT+CSQ"); //запрашиваем у GSM-модуля мощность сигнала
// delay(200);
for (int i = 0; i <= 20 ; i++) { delayMicroseconds(10000);}
 while (mySerial.available() > 0) { //считываем входную строку после двоеточия
 csq = mySerial.read();
 //Serial.println(csq); // Печатаем текст
 
     if (csq ==':'){
       GSMPow1[0]=mySerial.read();
       GSMPow1[1]=mySerial.read();
       GSMPow1[2]=mySerial.read();
       GSMPow1[3]=mySerial.read();
       GSMPow1[4]=0;
       }
   }
if (GSMPow1[1] == ',') {GSMPow1[1]=0;GSMPow1[2]=0;GSMPow1[3]=0;}
else if (GSMPow1[2] == ',') {GSMPow1[2]=0;GSMPow1[3]=0;}
else if (GSMPow1[3] == ',') {GSMPow1[3]=0;}
GSMPow=GSMPow1;
}
void switchto9600()
{
mySerial.begin(115200); // Open software serial port
delay(16000); // wait for boot
mySerial.println("AT");
delay(200);
mySerial.println("AT");
delay(200);
mySerial.println("AT+IPR=9600");    //  Change Serial Speed 
delay(200);
mySerial.begin(9600);
mySerial.println("AT&W0");
delay(200);
mySerial.println("AT&W");
}
