#include <Wire.h>


const int dirx = 4;
const int stepx = 5;
const int diry = 2;
const int stepy = 3;
const int stepDelay = 1500;  // microseconds between steps (lower = faster)
const int stepsPerRevolution = 1000; // adjust to your motor

//global i2c variables
char mode = 1;
char xpos = 0;
char ypos = 0;
char busy = 0;

//adding these to track curr position when needed
char xcurr_pos = 0;
char ycurr_pos = 0;

enum states{IDLE, XPOSITIVE, YPOSITIVE, XNEGATIVE, YNEGATIVE} state;
enum manual{M_IDLE, M_MOVE} Mstate;

void setup() {
  pinMode(dirx, OUTPUT);
  pinMode(stepx, OUTPUT);
  pinMode(diry, OUTPUT);
  pinMode(stepy, OUTPUT);

  Wire.begin(10);
  Wire.onReceive(RecInfo);
  Wire.onRequest(ReqInfo);

  Serial.begin(9600);
  delay(1000);
  Serial.println("Press 's' have claw grip state");
  Serial.println("Press 'a' and 'd' to rotate claw from 0° to 90°" );
  Serial.println("Press 'z' and 'c' to rotate stepper motor from CW or  CCW" );
  state = IDLE;
  Mstate = M_IDLE;
}

void loop() {
  // Check if a character is available
  if(mode){
    atick();
  }
  else{
    mtick();
  }
  delay(500);
}

void mtick(){
  switch(Mstate){
    case M_IDLE:
      if(mode) {
        Mstate = M_IDLE;
      }
      else{
        Mstate = M_MOVE;
      }
      break;

    case M_MOVE:
      if(mode) {
        Mstate = M_IDLE;
      }
      else{
        Mstate = M_MOVE;
      }

      if(xpos > 0){
        digitalWrite(dirx, LOW);
        for (int i = 0; i < stepsPerRevolution; i++) {
          digitalWrite(stepx, HIGH);
          delayMicroseconds(stepDelay);
          digitalWrite(stepx, LOW);
          delayMicroseconds(stepDelay);
        }
      }
      else if(xpos < 0){
        digitalWrite(dirx, HIGH);
        for (int i = 0; i < stepsPerRevolution; i++) {
          digitalWrite(stepx, HIGH);
          delayMicroseconds(stepDelay);
          digitalWrite(stepx, LOW);
          delayMicroseconds(stepDelay);
        }
      }
      if(ypos > 0){
        digitalWrite(diry, LOW);
          for (int i = 0; i < stepsPerRevolution; i++) {
          digitalWrite(stepy, HIGH);
          delayMicroseconds(stepDelay);
          digitalWrite(stepy, LOW);
          delayMicroseconds(stepDelay);
        }
      }
      else if(ypos < 0){
        digitalWrite(diry, LOW);
        for (int i = 0; i < stepsPerRevolution; i++) {
          digitalWrite(stepy, HIGH);
          delayMicroseconds(stepDelay);
          digitalWrite(stepy, LOW);
          delayMicroseconds(stepDelay);
        }
      }

  }
}

void atick(){
  switch(state){
    case IDLE:
      if(busy){
        state = XPOSITIVE;
      }
      else{
        state = IDLE;
      }
      break;

    case XPOSITIVE:
      //CCW movement
      digitalWrite(dirx, LOW);
      for (int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(stepx, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepx, LOW);
        delayMicroseconds(stepDelay);
      }
      state = YPOSITIVE;
      break;

    case YPOSITIVE:
      //CCW movement
      digitalWrite(diry, LOW);
      for (int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(stepy, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepy, LOW);
        delayMicroseconds(stepDelay);
      }
      state = XNEGATIVE;
      break;

    case XNEGATIVE:
      //CW movement
      digitalWrite(dirx, HIGH);
      for (int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(stepx, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepx, LOW);
        delayMicroseconds(stepDelay);
      }
      state = YNEGATIVE;
      break;
    case YNEGATIVE:
      //CW movement
      digitalWrite(diry, LOW);
      for (int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(stepy, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepy, LOW);
        delayMicroseconds(stepDelay);
      }
      state = IDLE;
      busy = 0;
      break;
  }
}


//i2c function to get info from master
void RecInfo(int bytes){
  mode = Wire.read();
  xpos = Wire.read();
  ypos = Wire.read();
  char t = Wire.read();
  Serial.println("xpos: " + String(xpos) + " ypos: " + String(ypos) );  
  if(mode){
    busy = 1;
  }
}

//i2c function to send info 
void ReqInfo(int bytes){
  Wire.write(busy);
}