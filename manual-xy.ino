#include <Wire.h>


const int dirx = 4;
const int stepx = 5;
const int diry = 2;
const int stepy = 3;
const int stepDelay = 1500;  // microseconds between steps (lower = faster)
const int stepsPerRevolution = 200; // 200 == 1 rev

//global i2c variables
char mode = 0;
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
  Serial.println("Starting, commands issued via I2C comms");;
  state = IDLE;
  Mstate = M_IDLE;
}

void loop() {
  // Check if a character is available
  if(mode){
    //atick();
  }
  else{
    mtick();
  }
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
        digitalWrite(diry, HIGH);
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
      if(xcurr_pos < xpos){
        state = XPOSITIVE;
      }
      else if(xcurr_pos > xpos){
        state = XNEGATIVE;
      }
      else if(ycurr_pos < ypos){
        state = YPOSITIVE;
      }
      else if(ycurr_pos > ypos){
        state = YNEGATIVE;
      }
      else{
        state = IDLE;
        busy = 0;
      }
      break;

    case XPOSITIVE:
      //CCW movement
      digitalWrite(dirx, LOW);
      for (int i = 0; i < xpos / stepsPerRevolution; i++) {
        digitalWrite(stepx, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepx, LOW);
        delayMicroseconds(stepDelay);
      }
      xcurr_pos = xpos;
      state = IDLE;
      break;

    case YPOSITIVE:
      //CCW movement
      digitalWrite(diry, LOW);
      for (int i = 0; i < ypos /stepsPerRevolution; i++) {
        digitalWrite(stepy, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepy, LOW);
        delayMicroseconds(stepDelay);
      }
      ycurr_pos = ypos;
      state = IDLE;
      break;

    case XNEGATIVE:
      //CW movement
      digitalWrite(dirx, HIGH);
      for (int i = 0; i < xpos / stepsPerRevolution; i++) {
        digitalWrite(stepx, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepx, LOW);
        delayMicroseconds(stepDelay);
      }
      state = IDLE;
      xcurr_pos = xpos;
      break;
    case YNEGATIVE:
      //CW movement
      digitalWrite(diry, LOW);
      for (int i = 0; i < ypos / stepsPerRevolution; i++) {
        digitalWrite(stepy, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepy, LOW);
        delayMicroseconds(stepDelay);
      }
      ycurr_pos = ypos;
      state = IDLE;
      break;
  }
}


//i2c function to get info from master
void RecInfo(int bytes){
  mode = Wire.read();
  xpos = Wire.read();
  ypos = Wire.read();
  busy = Wire.read();
  Serial.println("xpos: " + String(xpos) + " ypos: " + String(ypos) );  
}

//i2c function to send info 
void ReqInfo(int bytes){
  Wire.write(busy);
}