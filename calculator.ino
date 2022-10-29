#include <Keypad.h>
#include "HD44780_LCD_PCF8574.h"
HD44780LCD lcd(2,16,0x20);//LCD对象
const byte ROWS=4;
const byte COLS=4;
const int pointPin=3;//与小数点关联的引脚
const int deletePin=2;//与删除关联的引脚
char hexaKeys[ROWS][COLS]={
  {'7','8','9','/'},
  {'4','5','6','*'},
  {'1','2','3','-'},
  {'c','0','=','+'},
};
byte rowPins[ROWS]={11,10,9,8};
byte colPins[COLS]={4,5,6,7};
int lastCondition[2]={HIGH,HIGH};//检测按键状态,0是删除，1是小数点
Keypad customKeypad=Keypad(makeKeymap(hexaKeys),rowPins,colPins,ROWS,COLS);//构造函数
static String str;//用于储存字符
float Number1=0,Number2=0;
char action=0,key=' ';
void setup() {
  Serial.begin(9600);
  Serial.println("this calculator is based on a 4*4 keyPad and two buttons");
  lcd.PCF8574_LCDInit(LCDCursorTypeOff);
  lcd.PCF8574_LCDClearScreen();
  lcd.PCF8574_LCDBackLightSet(true);
  lcd.PCF8574_LCDGOTO(1,0);
  lcd.PCF8574_LCDSendString("Calculator");
  delay(1000);
  lcd.PCF8574_LCDClearScreen();
  pinMode(pointPin,INPUT);
  pinMode(deletePin,INPUT);
  digitalWrite(pointPin,LOW);
  digitalWrite(deletePin,LOW);
  lcd.PCF8574_LCDGOTO(1,0);
}
void loop() {
  key=customKeypad.getKey();
  if(key!=NO_KEY)
  {
    if (key=='c') Serial.println('c');
    else Serial.print(key);
    if(key=='=') getAnswer(str);//计算结果
    else if(key=='c') cleanCal(str);//清屏重置
    else addStr(str,key);
  }
  buttonCheck(str);//检测按键指令
}
void addStr(String&str,char key)//字符串加长
{
  str=str+key;
  lcd.PCF8574_LCDSendChar(key);
}
void getAnswer(String&str)//计算结果，先分离，再计算
{
  float answer=0;
  int process1=0,process2=0,process3=0;//分别代表小数点、运算符、小数点是否存在
  for(int i=0;i<str.length();i++)
  {
    if(str[i]=='+'||str[i]=='-'||str[i]=='*'||str[i]=='/')
    {
      action=str[i];
      process2=1;
    }
    else if(str[i]=='.') (process2?process3:process1)=1;//判断是第一个还是第二个数的小数点
    else if(process1==0&&process2==0&&process3==0) Number1=Number1*10+str[i]-'0';
    else if(process1==1&&process2==0&&process3==0) Number1+=(str[i]-'0')*0.1;
    else if(process2==1&&process3==0) Number2=Number2*10+str[i]-'0';
    else if(process3==1) Number2+=(str[i]-'0')*0.1;
    else 
    {
      Number1=0;Number2=0,action=0;
      lcd.PCF8574_LCDGOTO(2,0);
      lcd.PCF8574_LCDSendString("Errors happend");
      return;
    }
  }
  switch(action)
  {
    case '+':answer=Number1+Number2;break;
    case '-':answer=Number1-Number2;break;
    case '*':answer=Number1*Number2;break;
    case '/':
    {
      if(Number2==0) 
      {
        Number1=0;Number2=0,action=0;
        lcd.PCF8574_LCDGOTO(2,0);
        lcd.PCF8574_LCDSendString("Errors happend");
        return;
      }
      answer=Number1/Number2;break;
    }
    default:break;
  }
  lcd.PCF8574_LCDGOTO(2,0);
  char myAnswer[20];
  dtostrf(answer,6,2,myAnswer);
  lcd.PCF8574_LCDSendString(myAnswer);
  Serial.print('\n');
  Serial.println(answer);
  Number1=0;
  Number2=0;
  action=0;
}
void buttonCheck(String&str)//小数点的添加以及数据的删除
{
  int thisCondition[2]={digitalRead(deletePin),digitalRead(pointPin)};
  if(thisCondition[0]==HIGH&&lastCondition[0]==LOW&&str.length()>0)
  {
    char tmp[20]="0";
    str.getBytes(tmp,str.length());
    str=tmp;
    lcd.PCF8574_LCDClearScreen();
    lcd.PCF8574_LCDGOTO(1,0);
    lcd.PCF8574_LCDSendString(str.c_str());
    Serial.print("delete\n");
    Serial.println(str);
  }
  if(thisCondition[1]==HIGH&&lastCondition[1]==LOW)
  {
    str+='.';
    lcd.PCF8574_LCDSendChar('.');
    Serial.print('.');
  }
  lastCondition[0]=thisCondition[0];
  lastCondition[1]=thisCondition[1];
}
void cleanCal(String&str)//按C清屏重置
{
  lcd.PCF8574_LCDClearScreen();
  lcd.PCF8574_LCDGOTO(1,0);
  String tmp;
  str=tmp;
  Number1=0,Number2=0,action=0;
}
