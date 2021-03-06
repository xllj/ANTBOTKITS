#include <ABTKITS.h>
#include <Servo.h>
#include "SoftwareSerial.h"
#include <EEPROM.h>
//#define DRV_9110

#ifdef DRV_9110
#define E1  5  //PWMA 
#define M1  6  //DIRA 
#define E2  3  //PWMB
#define M2  10  //DIRB
#define BUZZERPIN 7//蜂鸣器
#define LEDPIN    4//LED(L9110S板)
#else
#define E1  5  //PWMA 
#define M1  4  //DIRA 
#define E2  6  //PWMB
#define M2  7  //DIRB
#endif
#define NOTE_CS5 554 //3
#define NOTE_D5  587 //4
#define NOTE_E5  659 //5
ABTKITS abtKits;
Servo mServo[6];//声明舵机数组
unsigned char mServoPin[6]={9,11,A0,A1,A2,A3};//舵机引脚
unsigned char abtAttack1[6]={0,0,0,0,0,0};//攻击动作1
unsigned char abtAttack2[6]={0,0,0,0,0,0};//攻击动作2
unsigned char abtDefense[6]={0,0,0,0,0,0};//防守动作
int ang01 = 0;
int ang02 = 100;
int cnt=0;
int carType=0;
int dir01 = 0;
int dir02 = 0;
void setup() {
  // put your setup code here, to run once:
  int i;
abtKits.ABTINIT();
delay(200);
pinMode(2,INPUT);
pinMode(8,INPUT);
pinMode(E1,OUTPUT);//LED ctrl
pinMode(E2,OUTPUT);//Buzzer ctrl
pinMode(M1,OUTPUT);
pinMode(M2,OUTPUT);
#ifdef DRV_9110
pinMode(BUZZERPIN,OUTPUT);
pinMode(LEDPIN,OUTPUT);
#endif
for(i=0;i<6;i++)
{
  mServo[i].attach(mServoPin[i]);
  }

delay(10);
rdCfg();
delay(50);
ABTMotorstop();

}

void loop() {
  // put your main code here, to run repeatedly:
  //abtKits.ABTSimple();return;
  int i=0;
  int j=0;
  i = abtKits.ABTGetBleCmd();
  delay(20);  
  if(i>6)//接收字节数大于6执行处理命令函数
  {
    abtKits.ABTHandleBleCmd();delay(20);
    if(abtKits.curInfo.cRW=='W')
    {
      abtKits.curInfo.cRW = 'N';//避免下一次重复进入
      #ifdef DRV_9110
      digitalWrite(4,0);//熄灭LED
      #endif
      if(abtKits.curInfo.sID<5)
      {
        carType = abtKits.curInfo.sID;
        if(abtKits.curInfo.sVal==0)//停止
      {        
        ABTMotorstop();
        } else if(abtKits.curInfo.sVal==1)//前
      {        
       ABTMotorfrwd();
        }else if(abtKits.curInfo.sVal==2)//后
      {
         ABTMotorback();
        }else if(abtKits.curInfo.sVal==3)//左
      {
        ABTMotorleft();
        }else if(abtKits.curInfo.sVal==4)//右
      {
        ABTMotorright();
        }       
      
      }else if(abtKits.curInfo.sID==5)
      {
        abtKits.speedL = 120+abtKits.curInfo.sVal;
        }else if(abtKits.curInfo.sID==6)
      {
        abtKits.speedR = 120+abtKits.curInfo.sVal;
        }else if(abtKits.curInfo.sID>6&&abtKits.curInfo.sID<13)
      {
        ang01 = abtKits.curInfo.sVal;
        if(ang01>=0&&ang01<=180)
        {
          abtAttack1[abtKits.curInfo.sID-7]=ang01;
          mServo[abtKits.curInfo.sID-7].write(ang01);
          Serial.println(ang01);delay(10);
          }        
        }else if(abtKits.curInfo.sID==32)
        {
          if(abtKits.curInfo.sVal==0)//保存动作
          {
            wtCfg();
            }else if(abtKits.curInfo.sVal==1)
            {
              for(j=0;j<6;j++)
              {
                mServo[j].write(abtAttack1[j]);
                Serial.println(abtAttack1[j]);delay(10);
                }
              }else if(abtKits.curInfo.sVal==2)
            {
              for(j=0;j<6;j++)
              {
                mServo[j].write(abtAttack2[j]);
                }
              }else if(abtKits.curInfo.sVal==3)
            {
              for(j=0;j<6;j++)
              {
                mServo[j].write(abtDefense[j]);
                }
              }
          }
   }
  }
return;

}
void ABTMotorstop()//停止
{
  digitalWrite(E1,0);
  digitalWrite(E2,0);
  digitalWrite(M1,0);
  digitalWrite(M2,0);
#ifdef DRV_9110
  digitalWrite(4,1);//点亮LED
  StopSound();
#endif
 }
void LeftCtrl(int dir,int spdPwm)//左轮控制
{
#ifdef DRV_9110
  if(dir)//正转
  {   
    digitalWrite(M1,LOW); analogWrite(E1, spdPwm);   //PWM调速    
    }else{
    digitalWrite(E1,LOW); analogWrite(M1, spdPwm);   //PWM调速
      }
#else
  if(dir)//正转
  {   
    digitalWrite(M1,LOW); analogWrite(E1, spdPwm);   //PWM调速    
    }else{
    digitalWrite(M1,HIGH); analogWrite(E1, spdPwm);   //PWM调速
      }
#endif
}
void RightCtrl(int dir,int spdPwm)//右轮控制
{
#ifdef DRV_9110
  if(dir)//正转
  {
    digitalWrite(M2,LOW); analogWrite(E2, spdPwm);   //PWM调速
    }else{
    digitalWrite(E2,LOW); analogWrite(M2, spdPwm);   //PWM调速
      }
#else
  if(dir)//正转
  {
    digitalWrite(M2,HIGH); analogWrite(E2, spdPwm);   //PWM调速
    }else{
    digitalWrite(M2,LOW); analogWrite(E2, spdPwm);   //PWM调速
      }
#endif
}
void ABTMotorfrwd()//前进
{
  if(carType==0)
  {
      LeftCtrl(1,abtKits.speedL);  
      RightCtrl(1,abtKits.speedR);      
  }else if(carType==1)
    {
      
      }else if(carType==2)
        {
        
          }
  
   
}
void ABTMotorback()//后退
{
  if(carType==0)
  {
      LeftCtrl(0,abtKits.speedL);  
      RightCtrl(0,abtKits.speedR);  
  }else if(carType==1)
    {
     
      }else if(carType==2)
        {
          
          }
  
}
void ABTMotorleft()//左转
{
  if(carType==0)
  {
      LeftCtrl(0,abtKits.speedL);
      RightCtrl(1,abtKits.speedR);
  }else if(carType==1)
    {
     
      }else if(carType==2)
        {
         
          }
  
   
}
void ABTMotorright()//右转
{
  if(carType==0)
  {
      LeftCtrl(1,abtKits.speedL);
      RightCtrl(0,abtKits.speedR);
  }else if(carType==1)
    {
      
      }else if(carType==2)
        {
          
          }

  
}
void StopSound()
{
  int tonePin = 7;
  int melody[] = {
NOTE_CS5,//
NOTE_CS5,//
NOTE_E5,//
0,
};

int noteDurations[] = {
  8,8,2,
  4,  
};
for (int thisNote = 0; thisNote < 4; thisNote++) {

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/noteDurations[thisNote];
    tone(tonePin, melody[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(tonePin);
  }
  
  }
void rdCfg()//读取配置
{
  int i=0;
  int address = 0;
  byte value;
  for(i=0;i<6;i++)
  {
    value = EEPROM.read(address);
    if(value>=0&&value<=180)
    {
      abtAttack1[i]=value;
      mServo[i].write(value);
      }
    
    address++;
  }
}
 void wtCfg()//存储配置
{
  int i=0;
  int address = 0;  
  for(i=0;i<6;i++)
  {
    EEPROM.write(address, abtAttack1[i]);
    address++;
    }  
 }
