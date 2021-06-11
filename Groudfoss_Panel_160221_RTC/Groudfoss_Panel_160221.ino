//////////////////////////////////////////////////////
//    Project : Grundfoss Motor Control Panel       //
//    No of Motors : 4                              //
//    HP : 7                                        //
//    VFD : 5.5KW    MODEL:2800                     //
//    AUTHOR : KALAIVANAN                           //
//    DATE : 28/01/21                               //
//    VERSION : Initial                             //
//////////////////////////////////////////////////////
//#include <Keypad.h>
#include <LiquidCrystal_I2C.h>//Display
#include <avr/wdt.h>
//#include <DS3231.h>//for RTC Module 3231

//DS3231  rtc(43, 44);
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

byte VFD_REALY = LOW;
byte Press_switch_status = HIGH; 

const int analogInPin = A1;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 3; // Analog output pin that the LED is attached to
int MOTOR1_STATUS,MOTOR2_STATUS,MOTOR3_STATUS,MOTOR4_STATUS = LOW;
int MOTOR1 = 4;//
int MOTOR2 = 5;
int MOTOR3 = 6;
int MOTOR4 = 7;
//int VFD_MOTOR[4] = {4, 5, 6, 7};
//int AC_MOTOR[4] = {8, 9, 10, 11};
int VFD_MOTOR[3] = {4, 5, 7};
int AC_MOTOR[3] = {8, 9, 11};
float ReadADC = 0;
float ReadPSValue = 0;        // value read from the pot/Pressure Transmitter
int outputValue = 0;        // value output to the PWM (analog out)
float SetPSValue = 5.3;//4.5;//3.5; //3.5 bar

String DisplayString = "Welcome to VEPL";

float VFD_DAC = 500;
int DAC_INC_VALUE = 10;
int DAC_DEC_VALUE = 10;
int DAC_INIT_VALUE = 500;
int val = 0;  // variable to store the value read
char customKey = 0;
float UpOffSet = 0.7;
float DownOffSet = 0.3;
float VFD_PERCENT = 0;


const int PIN_02 = 2;
int read_switch_status = 0;

byte Overload_relay1 = 0;
byte Overload_relay2 = 0;
byte Overload_relay3 = 0;
byte Overload_relay4 = 0;

int timer_count  = 0;

const int OVERLOADRELAY1 = A9; // potentiometer wiper (middle terminal) connected to analog pin 3
const int OVERLOADRELAY2 = A8;
const int OVERLOADRELAY3 = A10;
const int OVERLOADRELAY4 = A11;

float Read_Current_ADC;
float Convert_Current;

int int_hour,int_min,int_sec,int_millis;

int Minutes;// = ( millis()/1000 ) / 60;
int Hours;// = ( ( millis()/1000 ) / 60 ) / 60;
int Interval = 15;//5;
int temp=0;
int Select_Master = 1;
float difference = 0;


void setup()
{
  //LCD Initialisation
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print(DisplayString);
  //Serial start
  Serial.begin(9600);//Serial start with 9600
  
  digitalWrite(8, HIGH);
  delay(100);
  digitalWrite(9, HIGH);
  delay(100);
  digitalWrite(10, HIGH);
  delay(100);
  digitalWrite(11, HIGH);
  delay(100);
  
  digitalWrite(4, HIGH);
  delay(100);
  digitalWrite(5, HIGH);
  delay(100);
  digitalWrite(6, HIGH);
  delay(100);
  digitalWrite(7, HIGH);
  delay(100);
  
  pinMode(4, OUTPUT);  
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);  
    
  pinMode(PIN_02, INPUT);
  delay(100);

   //MOTOR1 = VFD_MOTOR[0];
   //MOTOR2 = AC_MOTOR[1];
   //MOTOR3 = AC_MOTOR[2];
   //MOTOR4 = AC_MOTOR[3];
   
   MasterChange(); 
   
  //Start Motor with 50% Load
   digitalWrite(MOTOR1, LOW);
   delay(1000);
   MOTOR1_STATUS = HIGH;
   outputValue = map(VFD_DAC, 0, 1023, 0, 255);
   analogWrite(analogOutPin, outputValue);
   delay(100);

   // make a delay before enable WDT
   // this delay help to complete all initial tasks
   delay(2000);

   /////////////////////////RTC////////////////////////
   //rtc.begin();
  
  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(WEDNESDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(12, 0, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(16, 2, 2021);   // Set the date to January 1st, 2014
   ////////////////////////RTC//////////////////////////
   wdt_enable(WDTO_4S);
}
  
void loop()
{
  wdt_reset();
  
  //Minutes = ( millis()/1000 ) / 60;
  //Hours = ( ( millis()/1000 ) / 60 ) / 60;
  
  temp = temp + 1;
  
  if(temp >= 60)
  {
    temp = 0;
    Minutes = Minutes - 1;
  }

  
  if (Minutes == 0) 
  {
    Minutes = Interval;
    Select_Master = Select_Master + 1;
    MasterChange();      
  }
  //Serial.println(millis());
  wdt_reset();
  
  Overload_relay1 = digitalRead(OVERLOADRELAY1);
  Overload_relay2 = digitalRead(OVERLOADRELAY2);
  Overload_relay3 = digitalRead(OVERLOADRELAY3);  
  Overload_relay4 = digitalRead(OVERLOADRELAY4);

  //check_relay_status();   
  wdt_reset();
  Read_Pressure();
  wdt_reset();
  CheckPressure();
  wdt_reset();
  Display();
  //time_display();
  //temp_display();
  delay(1000);

  Pressure_switch_open();

/*  Serial.println((Overload_relay1));  // debug value 
  Serial.println((Overload_relay2));  // debug value 
  Serial.println((Overload_relay3));  // debug value 
  Serial.println((Overload_relay4));  // debug value 
*/
  
  Print_function(); //Serial Output

  wdt_reset();
  
  }

void Read_Pressure()
{
  ReadADC = analogRead(analogInPin);    // read the input pin/ Read Pressure value
  delay(50);
  Read_Current_ADC = ((ReadADC * 4.84) / 1024);
  Convert_Current = ( Read_Current_ADC * 1000 ) / 220;
  ReadPSValue = (0.625*(Convert_Current-4));
  if (ReadPSValue < 0)
  {    
    ReadPSValue=0;
  }
  //Serial.println(ReadPSValue);  // debug value
 }

 void Pressure_switch_open()
 {
    Read_Pressure();
    if(ReadPSValue == LOW)
    {
      MotorAllOff();//Switch off All motors
      //Press_switch_status = LOW;    
      lcd.clear();
      lcd.setCursor(3, 0); 
      lcd.print("WARNING!");
      lcd.setCursor(1, 1);           
      lcd.print("CHK  TRANSMITER");      
      delay(1000);
      lcd.clear();
    }
  }

 void temp_display(){
  lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(temp);
    delay(1000);
  }
 
  void time_display(){
   /* lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DATE :");     
    lcd.print(rtc.getDateStr());    

    lcd.setCursor(0, 1);
    lcd.print("TIME :");     
    lcd.print(rtc.getTimeStr());
    delay(1000);*/
    }
void Display()
{
  lcd.clear();
  //lcd.scrollDisplayLeft();  
  
  if (Select_Master == 0)
  {
    lcd.setCursor(0, 0);     
    lcd.print(MOTOR1_STATUS);        
    lcd.setCursor(1, 0);     
    lcd.print("M");    
  
    lcd.setCursor(2, 0);     
    lcd.print(MOTOR2_STATUS);    
  
    lcd.setCursor(4, 0);     
    lcd.print(MOTOR3_STATUS);      
  }
  if (Select_Master == 1)
  {     
    lcd.setCursor(2, 0);     
    lcd.print(MOTOR1_STATUS);    
    lcd.setCursor(3, 0);     
    lcd.print("M");    
  
    lcd.setCursor(4, 0);     
    lcd.print(MOTOR2_STATUS);    
  
    lcd.setCursor(0, 0);     
    lcd.print(MOTOR3_STATUS);        
  }
  if (Select_Master == 2)
  {
    lcd.setCursor(2, 0);     
    lcd.print(MOTOR3_STATUS);         
    
    lcd.setCursor(4, 0);     
    lcd.print(MOTOR1_STATUS);    
    
    lcd.setCursor(5, 0); 
    lcd.print("M");    
  
    lcd.setCursor(0, 0);     
    lcd.print(MOTOR2_STATUS);         
  }
  

  
  lcd.setCursor(6, 0);  
  lcd.print(Minutes);
  
  VFD_PERCENT = (VFD_DAC/1024)*100;
  lcd.setCursor(10, 0);   
  lcd.print(VFD_PERCENT);  

  lcd.setCursor(15, 0);   
  lcd.print("%");  
  
  lcd.setCursor(0, 1);     
  lcd.print(SetPSValue);  

  lcd.setCursor(5, 1);     
  lcd.print("b");  

  lcd.setCursor(9, 1);   
  lcd.print(ReadPSValue);  

  lcd.setCursor(14, 1);     
  lcd.print("b"); 
  //delay(1000);
}


void MasterChange()
{    
    MotorAllOff();//Switch off All motors  
    lcd.clear();   
    if (Select_Master >= 3)
    {
      Select_Master = 0;        
    }
    if (Select_Master == 0)
    {
      MOTOR1 = VFD_MOTOR[0];
      MOTOR2 = AC_MOTOR[1];
      MOTOR3 = AC_MOTOR[2];
      //MOTOR4 = AC_MOTOR[3];
    }
    else if(Select_Master == 1)
    {
      MOTOR1 = VFD_MOTOR[1];
      MOTOR2 = AC_MOTOR[2];
      MOTOR3 = AC_MOTOR[0];
      //MOTOR4 = AC_MOTOR[0];
    }
    else if(Select_Master == 2)
    {
      MOTOR1 = VFD_MOTOR[2];
      MOTOR2 = AC_MOTOR[0];
      MOTOR3 = AC_MOTOR[1];
      //MOTOR4 = AC_MOTOR[1];
    }   
    else
    {
      MOTOR1 = VFD_MOTOR[0];
      MOTOR2 = AC_MOTOR[1];
      MOTOR3 = AC_MOTOR[2];
      //MOTOR4 = AC_MOTOR[3];
    }  
    
    VFD_DAC = 0;     
  }

  void MotorAllOff()
  {
    wdt_reset();
    VFD_DAC = 0;
    outputValue = map(VFD_DAC, 0, 1023, 0, 255);
    analogWrite(analogOutPin, outputValue); 
    delay(1000);
    wdt_reset();  
    digitalWrite(MOTOR1, HIGH);    
    digitalWrite(MOTOR2, HIGH);    
    digitalWrite(MOTOR3, HIGH);
    delay(2000);    
    MOTOR1_STATUS = LOW;
    MOTOR2_STATUS = LOW;
    MOTOR3_STATUS = LOW; 
    wdt_reset();   
  }

  void CheckPressure()
  {
    
    if (ReadPSValue < (SetPSValue - DownOffSet))
    {       
    
    if (VFD_DAC < 1024)// and AC_RELAY1 == LOW and AC_RELAY2 == LOW and AC_RELAY3 == LOW)
    {
      if(MOTOR1_STATUS == LOW)
      {
        digitalWrite(MOTOR1, LOW);
        delay(2000);
        MOTOR1_STATUS = HIGH;
      }
      wdt_reset();
      difference = (SetPSValue - ReadPSValue);
      //VFD_DAC = VFD_DAC + DAC_INC_VALUE;
      if (difference >= 0.1 and difference < 0.2)
      {VFD_DAC = VFD_DAC + 10;}
      else if (difference >= 0.2 and difference < 0.5)
      {VFD_DAC = VFD_DAC + 20;}
      else if (difference >= 0.5 and difference < 1)
      {VFD_DAC = VFD_DAC + 50;}
      else if (difference >= 1 and difference < 2)
      {VFD_DAC = VFD_DAC + 100;}
      else if (difference >= 2 and difference < 5)
      {VFD_DAC = VFD_DAC + 150;}
      else if (difference >= 5 and difference < 10)
      {VFD_DAC = VFD_DAC + 200;}
      else
      {VFD_DAC = VFD_DAC + 10;}        
      
      outputValue = map(VFD_DAC, 0, 1023, 0, 255);
      analogWrite(analogOutPin, outputValue);
      delay(500);      
    }
    else if (VFD_DAC >= 1024 and MOTOR2_STATUS == LOW)// and MOTOR3_STATUS == LOW and MOTOR4_STATUS == LOW)
    {          
      digitalWrite(MOTOR2, LOW);
      delay(500);
      MOTOR2_STATUS = HIGH;

      VFD_DAC = 800;//DAC_INIT_VALUE;
      outputValue = map(VFD_DAC, 0, 1023, 0, 255);
      analogWrite(analogOutPin, outputValue); 
      delay(500);              
     }
     else if (VFD_DAC >= 1024 and MOTOR3_STATUS == LOW)// and MOTOR3_STATUS == LOW and MOTOR4_STATUS == LOW)
    {      
      digitalWrite(MOTOR3, LOW);
      delay(500);
      MOTOR3_STATUS = HIGH;

      VFD_DAC = 800;//DAC_INIT_VALUE;
      outputValue = map(VFD_DAC, 0, 1023, 0, 255);
      analogWrite(analogOutPin, outputValue);
      delay(500);                
     }
   /*  else if (VFD_DAC >= 1000 and MOTOR4_STATUS == LOW)// and MOTOR3_STATUS == HIGH and MOTOR4_STATUS == LOW)
    {
      VFD_DAC = DAC_INIT_VALUE;
      digitalWrite(MOTOR4, LOW);
      delay(1000);
      MOTOR4_STATUS = HIGH;
      digitalWrite(MOTOR3, LOW);
      delay(500);
      MOTOR3_STATUS = HIGH;
      digitalWrite(MOTOR4, LOW);
      delay(500);
      MOTOR4_STATUS = HIGH;          
     }*/     
  }
  else if(ReadPSValue > (SetPSValue + UpOffSet))
  {       
    if (VFD_DAC > 250)// and VFD_DAC <= 1000)// and AC_RELAY1 == LOW and AC_RELAY2 == LOW and AC_RELAY3 == LOW)
    {
      if(MOTOR1_STATUS == LOW)
      {
        digitalWrite(MOTOR1, LOW);
        delay(500);
        MOTOR1_STATUS = HIGH;
      }

      //VFD_DAC = VFD_DAC - DAC_DEC_VALUE;
      difference = (ReadPSValue - SetPSValue);
      //VFD_DAC = VFD_DAC + DAC_INC_VALUE;
      if (difference >= 0.1 and difference < 0.2)
      {VFD_DAC = VFD_DAC - 10;}
      else if (difference >= 0.2 and difference < 0.5)
      {VFD_DAC = VFD_DAC - 20;}
      else if (difference >= 0.5 and difference < 1)
      {VFD_DAC = VFD_DAC - 50;}
      else if (difference >= 1 and difference < 2)
      {VFD_DAC = VFD_DAC - 100;}
      else if (difference >= 2 and difference < 5)
      {VFD_DAC = VFD_DAC - 150;}
      else if (difference >= 5 and difference < 10)
      {VFD_DAC = VFD_DAC - 200;}
      else
      {VFD_DAC = VFD_DAC - 1;}
      
      outputValue = map(VFD_DAC, 0, 1023, 0, 255);
      analogWrite(analogOutPin, outputValue);
      delay(500);      
    }
    else if (VFD_DAC <= 500 and MOTOR2_STATUS == HIGH)// and MOTOR3_STATUS == HIGH and MOTOR4_STATUS == HIGH)
    {      
      digitalWrite(MOTOR2, HIGH);
      delay(500);
      MOTOR2_STATUS = LOW;                

      VFD_DAC = 500;
      outputValue = map(VFD_DAC, 0, 1023, 0, 255);
      analogWrite(analogOutPin, outputValue);
      delay(500);
     }
     else if (VFD_DAC <= 500 and MOTOR3_STATUS == HIGH)// and MOTOR3_STATUS == HIGH and MOTOR4_STATUS == HIGH)
    {      
      digitalWrite(MOTOR3, HIGH);
      delay(500);
      MOTOR3_STATUS = LOW;   

      VFD_DAC = 500;
      outputValue = map(VFD_DAC, 0, 1023, 0, 255);
      analogWrite(analogOutPin, outputValue); 
      delay(500);      
     }
     /*else if (VFD_DAC <= 1000 and MOTOR4_STATUS == HIGH)// and MOTOR3_STATUS == LOW and MOTOR4_STATUS == HIGH)
    {
      VFD_DAC = 1000;
      digitalWrite(MOTOR4, HIGH);
      delay(1000);
      MOTOR4_STATUS = LOW;              
    }*/
     /*else
     {
      Serial.print("LOAD IS VERY LOW!");
      lcd.clear();
      lcd.setCursor(3, 0); 
      lcd.print("WARNING!");
      lcd.setCursor(1, 1); 
      lcd.print("VERY LESS LOAD");
      delay(1000);
     }*/
  }
  }

  void Print_function()
  {
    Serial.print("SetPSValue = ");
  
    Serial.print(SetPSValue);
    //Serial.print(Select_Master);
      
    Serial.print("\t ReadPSValue = ");  
  
    Serial.print(ReadPSValue); 
  
    Serial.print("\t VFD % = ");
  
    Serial.println(VFD_DAC);
  
    Serial.print("Motor1 = ");
  
    Serial.print(MOTOR1_STATUS);
  
    Serial.print("\t Motor2 = ");
  
    Serial.print(MOTOR2_STATUS);
  
    Serial.print("\t Motor3 = ");
  
    Serial.print(MOTOR3_STATUS);
  
    Serial.print("\t Motor4 = ");  
  
    Serial.println(MOTOR4_STATUS);

    /*// Send Day-of-Week
    Serial.print(rtc.getDOWStr());
    Serial.print(" ");
    
    // Send date
    Serial.print(rtc.getDateStr());
    Serial.print(" -- ");
    
    // Send time
    Serial.println(rtc.getTimeStr());*/
    
    // Wait one second before repeating :)
    delay (1000);
  }

   void check_relay_status()
  {
    if (Overload_relay1 == LOW )
    {
      lcd.clear();
      lcd.setCursor(3, 0); 
      lcd.print("WARNING!");
      lcd.setCursor(1, 1); 
      lcd.print("RELAY 1 FAILURE");      
      delay(1000);
      lcd.clear();
    }
    else if(Overload_relay2 == LOW)
    {
      lcd.clear();
      lcd.setCursor(3, 0); 
      lcd.print("WARNING!");
      lcd.setCursor(1, 1); 
      lcd.print("RELAY 2 FAILURE");      
      delay(1000);
      lcd.clear();
    }
    else if(Overload_relay3 == LOW)
    {
      lcd.clear();
      lcd.setCursor(3, 0); 
      lcd.print("WARNING!");
      lcd.setCursor(1, 1); 
      lcd.print("RELAY 3 FAILURE");      
      delay(1000);
      lcd.clear();
    }
    else if(Overload_relay4 == LOW)
    {
      lcd.clear();
      lcd.setCursor(3, 0); 
      lcd.print("WARNING!");
      lcd.setCursor(1, 1); 
      lcd.print("RELAY 4 FAILURE");      
      delay(1000);
      lcd.clear();
    }          
    }
