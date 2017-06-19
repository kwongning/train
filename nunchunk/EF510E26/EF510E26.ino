#include <Wiichuck.h>
#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


#define IR4     0 //define IN2 interface motor A
#define IR3     2 //define IN3 interface
#define IR2     5 //define IN4 interface
#define IR1     8 //speed motor B
#define PWMA 3  //speed motor A
#define PWMB 6  //speed motor A
#define DIRA 4  //speed motor A
#define DIRB 7  //speed motor A
#define MAXSPEED     65 //define the speed of motor
#define STEPSPEED     5 //define the spead of motor
#define startUpSpeed 45 // min speed just start
#define lightOnSpeed 2 // min speed just start
#define stationSpeed 55 // int station Speed min 60

RF24 radio(9,10);
const uint64_t talking_pipes[5] = { 0xF0F0F0F0D2LL, 0xF0F0F0F0C3LL, 0xF0F0F0F0B4LL, 0xF0F0F0F0A5LL, 0xF0F0F0F096LL };
const uint64_t listening_pipes[5] = { 0x3A3A3A3AD2LL, 0x3A3A3A3AC3LL, 0x3A3A3A3AB4LL, 0x3A3A3A3AA5LL, 0x3A3A3A3A96LL };
typedef enum { role_invalid = 0, role_client = 1, role_server = 2 } role_e;
const char* role_friendly_name[] = { "invalid", "Client", "Server"};
role_e role = role_client;
const uint8_t address_at_eeprom_location = 0;
uint8_t node_address;

int intCmd = 0;
int prevIntCmd = 0;

Wiichuck wii;
boolean isInit = true;
int trainSpeed = 0;
int prevTrainSpeed = 0;
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
boolean isResetA = false;
boolean isResetB = false;
int maxSpeed = MAXSPEED;
int stepSpeed = STEPSPEED;
boolean isForward = false;
boolean isPrevForward = false;
boolean isCurved = false;

void forwardA()
{
     digitalWrite(DIRA,HIGH);
}

void switchCurved()
{
     digitalWrite(DIRB,HIGH);
     analogWrite(PWMB,255);
     delay(100);
     analogWrite(PWMB,0);
}

void switchStraight()
{
     digitalWrite(DIRB,LOW);
     analogWrite(PWMB,255);
     delay(100);
     analogWrite(PWMB,0);
}

void backwardA()
{
      digitalWrite(DIRA,LOW);
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

  if ( role == role_server )
  {
   ;
    
  } else if ( role == role_client )
  {
    prevTrainSpeed = trainSpeed;
    trainSpeed = wii.joyY() - wiiZero;
    if (prevTrainSpeed != trainSpeed)
      intCmd = 11500 + trainSpeed;
  }
  //printCurrentOutput();
}

void autoControl()
{
  readIR();
  if (ir2 == 0){
    if (!isArriveA && !isLeaveA && !isResetA)
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
    isArriveB = false;
    isLeaveB = false;
    isResetB = false;
    delay(10000);
    //forwardA();
    backwardA();
    Serial.println("IR1(Leave)");
    trainSpeed = stationSpeed;
    analogWrite(PWMA,abs(trainSpeed));
    isLeaveA = true;
    }
  }
  if (ir3 == 0){
    if (!isArriveB && !isLeaveB && !isResetB)
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
    if (isArriveB && !isLeaveB )
    {
    Serial.println("IR4(Arrive)");
    trainStop();
    Serial.println("IR4(10)");
    isArriveA = false;
    isLeaveA = false;
    isResetA = false;
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
    isResetA = true;
    if (isCurved){
      isCurved=false;
      switchStraight();
    } else {
      isCurved=true;
      switchCurved();
    }
    Serial.println("IR1,2(Reset)");
  }
  if ((ir3 == 1) && (ir4 == 1) && !isArriveB && isLeaveB) {
    isLeaveB = false;
    isResetB = true;
    Serial.println("IR3,4(Reset)");
  }
}
void autoStart(){
  switchStraight();
  trainStop();
  forwardA();
  isArriveA = false;
  isArriveB = false;
  isLeaveA = false;
  isLeaveB = false;
  isResetA = false;
  isResetB = false;
  trainSpeed = stationSpeed;
  analogWrite(PWMA,abs(trainSpeed));
  delay(1000);
}

void changeSpeed(){
  int speedDir = wii.joyY() - wiiZero;
  //Serial.println(speedDir);
  if (speedDir < 0) {
    if ((maxSpeed - stepSpeed) >= stationSpeed) {
        maxSpeed = maxSpeed - stepSpeed;
    }
  }else{
        maxSpeed = maxSpeed + stepSpeed;
        if (maxSpeed >255){
          maxSpeed=255;
        }
  }
   if (wii.buttonC()==1){
     maxSpeed = MAXSPEED;
   }
   
   intCmd = 12000 + maxSpeed;
  // Serial.println("maxSpeed");
  //Serial.println(maxSpeed);
}
void radioLoop(){
  //
  // Client role.  Repeatedly send the current time
  //

  if (role == role_client && (prevIntCmd != intCmd))
  {
    // First, stop listening so we can talk.
    radio.stopListening();

    // Take the time, and send it.  This will block until complete
    printf("Now sending %d...",intCmd);
    radio.write( &intCmd, sizeof(int) );

    // Now, continue listening
    radio.startListening();

    // Wait here until we get a response, or timeout (250ms)
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 250 )
        timeout = true;

    // Describe the results
    if ( timeout )
    {
      printf("Failed, response timed out.\n\r");
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      unsigned long got_time;
      char* strAck;
      radio.read( &got_time, sizeof(unsigned long) );

      // Spew it
      //printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
    }
  }

  //
  // Server role.  Receive each packet, dump it out, and send it back
  //

  if ( role == role_server )
  {
    // if there is data ready
    uint8_t pipe_num;
    if ( radio.available(&pipe_num) )
    {
      // Dump the payloads until we've gotten everything
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &intCmd, sizeof(int) );

        // Spew it
        printf("Got payload %d from node %i...",intCmd,pipe_num+1);
      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Open the correct pipe for writing
      radio.openWritingPipe(listening_pipes[pipe_num-1]);

      // Retain the low 2 bytes to identify the pipe for the spew
      uint16_t pipe_id = listening_pipes[pipe_num-1] & 0xffff;

      // Send the final one back.
      unsigned long ackTime = millis();
      radio.write( &ackTime, sizeof(unsigned long) );
      printf("Sent response to %04x.\n\r",pipe_id);

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }

  //
  // Listen for serial input, which is how we set the address
  //
  if (role == 0){
  if (Serial.available())
  {
    // If the character on serial input is in a valid range...
    char c = Serial.read();
    if ( c >= '1' && c <= '6' )
    {
      // It is our address
      int val = c - '0';
      EEPROM.write(address_at_eeprom_location,val);

      // And we are done right now (no easy way to soft reset)
      printf("\n\rManually reset address to: %c\n\rPress RESET to continue!",c);
      while(1) ;
    }
  }
  }
}

void setup()
{

    // Read the address from EEPROM
    uint8_t reading = EEPROM.read(address_at_eeprom_location);

    // If it is in a valid range for node addresses, it is our
    // address.
    if ( reading >= 2 && reading <= 6 ){
      node_address = reading;
      role = role_client;

    // Otherwise, it is invalid, so set our address AND ROLE to 'invalid'
    }else if ( reading == 1 )
    {
      node_address = 1;
      role = role_server;
    }
    else {
      node_address = 0;
      role = role_invalid;
    }
  
  
  Serial.begin(9600);
  printf_begin();
  Serial.println("\n\rRF24\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  pinMode(IR1,INPUT);
  pinMode(IR2,INPUT);
  pinMode(IR3,INPUT);
  pinMode(IR4,INPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(DIRB,OUTPUT);

  radio.begin();
  if ( role == role_server )
  {
    radio.openReadingPipe(1,talking_pipes[0]);
    radio.openReadingPipe(2,talking_pipes[1]);
    radio.openReadingPipe(3,talking_pipes[2]);
    radio.openReadingPipe(4,talking_pipes[3]);
    radio.openReadingPipe(5,talking_pipes[4]);
  }
  
  if ( role == role_client )
  {
    // Write on our talking pipe
    radio.openWritingPipe(talking_pipes[0]);
    // Listen on our listening pipe
    radio.openReadingPipe(1,listening_pipes[0]);
  }
  
  radio.startListening();
  radio.printDetails();
  
  if ( role == role_client )
  {
    wii.init();
    wii.calibrate();  // calibration;
  }
    
  
  delay(1000);
}

void lightOn(){
   forwardA();
   trainSpeed = lightOnSpeed;
}

void processRadioCmd(){
  if (intCmd >0){
    if(intCmd == 10000){
      lightOn();
    } else if(intCmd == 10001){
      if(isAuto){
        autoStart();
      }
    } else if(intCmd == 10002){
      if(!isAuto){
        autoStart();
      } else {
        trainStop();
        trainSpeed = 0;
      }
      isAuto = !isAuto; 
    } else if(intCmd == 10003){
      isAuto = false;
      isCurved=false;
      switchStraight();
      trainStop();
      forwardA();
      isArriveA = false;
      isArriveB = false;
      isLeaveA = false;
      isLeaveB = false;
      isResetA = false;
      isResetB = false;
      trainSpeed = 0;
      printf("\n\rSerial Control: Reset");
    } else if(intCmd == 10004){
      if(!isAuto){
        forwardA();
        printf("\n\rSerial Control: Forward");
      }
    } else if(intCmd == 10005){
      if(!isAuto){
        backwardA();
        printf("\n\rSerial Control: Backward");
      }
    } else if(intCmd == 10006){
      if(!isAuto){
        if(isCurved){
          isCurved=false;
          switchStraight();
        } else {
          isCurved=true;
          switchCurved();
        }
        printf("\n\rSerial Control: Switch");
      }
    } else if(intCmd >=10010 && intCmd <=10019){
      if(!isAuto){
      trainSpeed = (int) (((((intCmd-10010)*10.0) / 100.0) * (255.0-startUpSpeed)) + startUpSpeed);
      if (trainSpeed < startUpSpeed){
        trainSpeed = startUpSpeed;
      }else if (trainSpeed > 255){
        trainSpeed = 255;
      }
        printf("\n\rSerial Control: Train Speed %d", trainSpeed);
      }
    } else if(intCmd >=11000 && intCmd <=11755){
      trainSpeed = (intCmd - 11500);
      if (trainSpeed > 0){
        isForward = true;
      }else {
        isForward = false;
      }
      manualControl();
    } else if(intCmd >=12000 && intCmd <=12255){
      maxSpeed = (intCmd - 12000);
    } else {
      printf("\n\rSerial Control: Invalid Command");
    }
  }
  intCmd = 0;
}
void generateRadioCmd(){
  if (wii.buttonZ()==1){
      changeSpeed();
  }
  if (wii.buttonC()==1 && wii.buttonZ()==0){
     intCmd=10002;
     prevIntCmd=0;
  }
}
void manualControl() {
  
  if (isForward){
      forwardA();
      Serial.println("A");
    } else{
      backwardA();
      Serial.println("B");
    } 
    if (trainSpeed == 0) {
      trainStop();
    }  
    trainSpeed = abs(trainSpeed);
    
    if (trainSpeed > 5){
      //Serial.println(trainSpeed / 100.0);
      //Serial.println((int) ((trainSpeed / 100.0) * 255.0));
      trainSpeed = (int) (((trainSpeed / 100.0) * (255.0-startUpSpeed)) + startUpSpeed);
      //trainSpeed = trainSpeed - 500;
      if (trainSpeed < startUpSpeed){
        trainSpeed = startUpSpeed;
      }else if (trainSpeed > 255){
        trainSpeed = 255;
      }
    }
    
    //analogWrite(PWMA,abs(trainSpeed));
}
void serialControl(){
  
  if (role == role_server){
    if (Serial.available())
    {
      // If the character on serial input is in a valid range...
      char sc = Serial.read();
      if ( sc >= '0' && sc <= '9' )
      {
      int val = sc - '0';
      printf("\n\rSerial Control: %c\n\rAuto Control",sc);
      }
       
      if ( sc == 'A' || sc== 'a' ){
      intCmd=10002;
      printf("\n\rSerial Control: %c\tAuto Control",sc);
       }
       if ( sc == 'R' || sc== 'r' ){
      intCmd=10003;
      printf("\n\rSerial Control: %c\tReset",sc);
       }
       if ( sc == 'L' || sc== 'l' ){
      intCmd=10000;
      printf("\n\rSerial Control: %c\tLight",sc);
       }
       if ( sc == '+' ){
      intCmd=10004;
      printf("\n\rSerial Control: %c\tDirection",sc);
       }
        if ( sc == '-' ){
      intCmd=10005;
      printf("\n\rSerial Control: %c\tDirection",sc);
       }
       if ( sc == '=' ){
      intCmd=10006;
      printf("\n\rSerial Control: %c\tSwitch",sc);
       }
    }
  }
}
void loop()
{
  
  prevIntCmd = intCmd;
  if ( role == role_server )
  {
    processRadioCmd();
    if (isAuto) {
      autoControl();
    }
    analogWrite(PWMA,abs(trainSpeed));
  }
  else  if ( role == role_client )
  {
    wii.poll();
    if (isInit) {
      initWiiZero();
      if (isAuto) {
      intCmd=10001;
      }
    }  
    
    wiiControl();
    generateRadioCmd();
  }
  
  radioLoop();
  serialControl();
  delay(10);
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

