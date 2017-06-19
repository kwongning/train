#include <Wiichuck.h>
#include <Wire.h>
#include <core.h>


#define IR4     10 //define IN2 interface motor A
#define IR3     11 //define IN3 interface
#define IR2     12 //define IN4 interface
#define IR1     13 //speed motor B
#define PWMA 3  //speed motor A
#define PWMB 6  //speed motor A
#define DIRA 4  //speed motor A
#define DIRB 7  //speed motor A
#define maxSpeed     70 //define the spead of motor
#define startUpSpeed 35 // min speed just start
#define stationSpeed 40 // int station Speed min 60

Wiichuck wii;
boolean isInit = true;
int trainSpeed = 0;
int wiiZero = 0;
int ir1 = 0;
int ir2 = 0;
int ir3 = 0;
int ir4 = 0;
boolean isAuto = false;
boolean isArriveA = false;
boolean isLeaveA = false;
boolean isArriveB = false;
boolean isLeaveB = false;

void forwardA()
{
     digitalWrite(DIRA,HIGH);
     forwardB();
}

void forwardB()
{
     digitalWrite(DIRB,HIGH);
}

void backwardA()
{
      digitalWrite(DIRA,LOW);
      backwardB();
}

void backwardB()
{
     digitalWrite(DIRB,LOW);
}


void trainStop()
{
     analogWrite(PWMA,0);// Unenble the pin, to stop the motor. this should be done to avid damaging the motor.
     analogWrite(PWMB,0);
}

void readIR()
{
  //Serial.println("readIR()");
  ir1 = digitalRead(IR1);
  ir2 = digitalRead(IR2);
  ir3 = digitalRead(IR3);
  ir4 = digitalRead(IR4);
  
  //Serial.println(ir1);
  //Serial.println(ir2);
  //Serial.println(ir3);
  //Serial.println(ir4);
}

void initWiiZero()
{
  int pVal = 0;
  for(int calCount = 0; calCount < 10; calCount++){
    if (wii.poll() && pVal!=wii.joyY()) {
      pVal = wii.joyY();
    }
    delay(100);
  }
  wiiZero = pVal;
  isInit = false;
}

void wiiControl()
{

  trainSpeed = wii.joyY() - wiiZero;
  if (trainSpeed >0){
      forwardA();
  } else if(trainSpeed <0){
      backwardA();
  } else {
      trainStop();
  }
  trainSpeed = abs(trainSpeed);
  if (trainSpeed > 5){
    Serial.println(trainSpeed / 100.0);
    Serial.println((int) ((trainSpeed / 100.0) * 255.0));
    trainSpeed = (int) ((trainSpeed / 100.0) * 255.0);
  }
  analogWrite(PWMA,abs(trainSpeed));
  printCurrentOutput();
}

void autoControl()
{
  readIR();
  if (ir2 == 0){
    if (!isArriveA && !isLeaveA)
    {
    isArriveA = true;
    Serial.println("IR2(Arrive)");
    trainSpeed = stationSpeed;
    analogWrite(PWMA,abs(trainSpeed));
    }
    else if (isArriveA && isLeaveA)
    {
    isArriveA = false;
    Serial.println("IR2(Leave)");
    trainSpeed = maxSpeed;
    analogWrite(PWMA,abs(trainSpeed));
    }
  }
  if (ir1 == 0){
    if (isArriveA && !isLeaveA)
    {
    Serial.println("IR1(Arrive)");
    trainStop();
    Serial.println("IR1(10)");
    delay(10000);
    backwardA();
    //forwardA();
    Serial.println("IR1(Leave)");
    trainSpeed = stationSpeed;
    analogWrite(PWMA,abs(trainSpeed));
    isLeaveA = true;
    }
  }
  if (ir3 == 0){
    if (!isArriveB && !isLeaveB)
    {
    isArriveB = true;
    Serial.println("IR3(Arrive)");
    trainSpeed = stationSpeed;
    analogWrite(PWMA,abs(trainSpeed));
    }
    else if (isArriveB && isLeaveB)
    {
    isArriveB = false;
    Serial.println("IR3(Leave)");
    trainSpeed = maxSpeed;
    analogWrite(PWMA,abs(trainSpeed));
    }
  }
  if (ir4 == 0){
    if (isArriveB && !isLeaveB)
    {
    Serial.println("IR4(Arrive)");
    trainStop();
    Serial.println("IR4(10)");
    delay(10000);
    //backwardA();
    forwardA();
    Serial.println("IR4(Leave)");
    trainSpeed = stationSpeed;
    analogWrite(PWMA,abs(trainSpeed));
    isLeaveB = true;
    }
  }
  
  if ((ir1 == 1) && (ir2 == 1) && !isArriveA && isLeaveA){
    isLeaveA = false;
    Serial.println("IR1,2(Reset)");
  }
  if ((ir3 == 1) && (ir4 == 1) && !isArriveB && isLeaveB) {
    isLeaveB = false;
    Serial.println("IR3,4(Reset)");
  }
}
void autoStart(){
  trainStop();
  forwardA();
  backwardB();
  isArriveA = false;
  isArriveB = false;
  isLeaveA = false;
  isLeaveB = false;
  trainSpeed = stationSpeed;
  analogWrite(PWMA,abs(trainSpeed));
  delay(1000);
}

void setup()
{
  Serial.begin(9600);
  pinMode(IR1,INPUT);
  pinMode(IR2,INPUT);
  pinMode(IR3,INPUT);
  pinMode(IR4,INPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(DIRB,OUTPUT);

  wii.init();
  wii.calibrate();  // calibration
  delay(5000);
}

void loop()
{
  wii.poll();
  if (isInit) {
    initWiiZero();
    if(isAuto){
    autoStart();
    }
  }
  
  if (wii.buttonC()==1){
    if(!isAuto){
      autoStart();
    }
    isAuto = !isAuto; 
  }
  
  if (isAuto) {
      autoControl();
  } else {
      wiiControl();
  }
   //Serial.print("Speed:");
   //Serial.print(trainSpeed);
   //Serial.print("\t");
   //Serial.println("");
  analogWrite(PWMA,abs(trainSpeed));
  delay(100);
}

void printCurrentOutput()
{
    Serial.print("joy:");
    Serial.print(wii.joyX());
    Serial.print(", ");
    Serial.print(wii.joyY());
    Serial.print("  \t");
    
    Serial.print("accle:");
    Serial.print(wii.accelX());
    Serial.print(", ");
    Serial.print(wii.accelY());
    Serial.print(", ");
    Serial.print(wii.accelZ());
    Serial.print("  \t");
    
    Serial.print("button:");
    Serial.print(wii.buttonC());
    Serial.print(", ");
    Serial.print(wii.buttonZ());
    Serial.print("  \t");
    Serial.println("");
    
    Serial.print("Speed:");
    Serial.print(wiiZero);
    Serial.print(", ");
    Serial.print(trainSpeed);
    Serial.print("  \t");
    Serial.println("");
}

