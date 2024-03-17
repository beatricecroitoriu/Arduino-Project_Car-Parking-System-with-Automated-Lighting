#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 
#include <Servo.h> 

Servo myservo1;

#define sensorPin A0 
#define light 13
#define light2 10
#define ledVerde 12
#define ledRosu 11


int IR1 = 2;
int IR2 = 4;

int Slot = 4;           

int flag1 = 0;
int flag2 = 0;

int sensorValue;

void setup()
 {
  pinMode(light,OUTPUT);
  pinMode(light2,OUTPUT);
  pinMode(ledVerde,OUTPUT);
  pinMode(ledRosu,OUTPUT);
  pinMode(sensorPin,INPUT);
  lcd.begin(16,2);
  lcd.backlight();
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  
  myservo1.attach(3);
  myservo1.write(100);

  lcd.setCursor (0,0);
  lcd.print("PROIECT PARCARE");
  lcd.setCursor (0,1);
  lcd.print("  INTELIGENTA  ");
  delay (3000);
  lcd.clear();  

 
  lcd.setCursor (0,0);
  lcd.print("   AFARA ESTE   ");  
  lcd.setCursor (0,1);
  lcd.print("     LUMINA     "); 
  delay (3000);
  lcd.clear();   

      
}

void loop()
{ 
  if(digitalRead (IR1) == LOW && flag1==0)
  {
    if(Slot>0)
    {
      flag1=1;
      if(flag2==0)
      {
        myservo1.write(0); 
        Slot = Slot-1;
      }
     
    }
    else
    {
      lcd.setCursor (0,0);
      lcd.print("  NE PARE RAU!  ");  
      lcd.setCursor (0,1);
      lcd.print("PARCAREA OCUPATA"); 
      delay (3000);
      lcd.clear();     
    }
  }

  if(digitalRead (IR2) == LOW && flag2==0)
  {
    flag2=1;
    if(flag1==0)
      {
        myservo1.write(0); 
        Slot = Slot+1;
        if(Slot==1)
          digitalWrite(ledVerde,LOW);
      }
  }

  if(flag1==1 && flag2==1)
  {
    delay (1000);
    myservo1.write(100);
    flag1=0;
    flag2=0;
    if(Slot==0)
      digitalWrite(ledRosu,HIGH);
    else
      digitalWrite(ledVerde,HIGH);
  }

  if(flag1==0 && flag2==0)
    if(Slot!=0)
      digitalWrite(ledVerde,HIGH);
      
  lcd.setCursor (0,0);
  lcd.print("   BUN VENIT!   ");
  lcd.setCursor (0,1);
  lcd.print("LOCURI LIBERE: ");
  lcd.print(Slot);

  if(Slot==0)
    digitalWrite(ledVerde,LOW);
  else
    digitalWrite(ledRosu,LOW);

  sensorValue = analogRead(sensorPin);  

  if (sensorValue <=300)
  {
    digitalWrite(light,HIGH);
    digitalWrite(light2,HIGH);
    lcd.setCursor (0,0);
    lcd.print("   AFARA ESTE   ");  
    lcd.setCursor (0,1);
    lcd.print("   INTUNERIC    "); 
    delay (3000);
    lcd.clear();
    
    
  }
  else
  {
    digitalWrite(light,LOW);
    digitalWrite(light2,LOW);
    lcd.setCursor (0,0);
    lcd.print("   AFARA ESTE   ");  
    lcd.setCursor (0,1);
    lcd.print("     LUMINA     "); 
    delay (3000);
    lcd.clear();
  }
}