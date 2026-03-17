#include <Wire.h>


const int dirx = 4;
const int stepx = 5;
const int diry = 2;
const int stepy = 3;
const int stepDelay = 1500;  // microseconds between steps (lower = faster)
const int stepsPerRevolution = 200; // 200 == 1 rev
const int XStepsPerRevolution = 25;
const int YStepsPerRevolution = 10;

//global i2c variables
char mode = 0;
char manualx = 0;
char manualy = 0;
char xpos = 50;
char ypos = 50;
volatile char busy = 1;

//adding these to track curr position starts in middle
char xcurr_pos = 50; 
char ycurr_pos = 50;

enum states{IDLE, XMOVE, YMOVE} state;
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
  if(mode){
    atick();
  }
  else{
    mtick();
    state = IDLE;
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

      //handles x-movement
      if(xcurr_pos >= 100){
        xcurr_pos = 100;
      }
      else if(manualx < 0){
        xcurr_pos = MotorMove(xcurr_pos+1, xcurr_pos, dirx, stepx, XStepsPerRevolution, stepDelay);
      }

      if(xcurr_pos <= 0){
        xcurr_pos = 0;
      }
      else if(manualx > 0){
        xcurr_pos = MotorMove(xcurr_pos-1, xcurr_pos, dirx, stepx, XStepsPerRevolution, stepDelay);
      }

      //handles y-movement
      if(ycurr_pos >= 95){
        ycurr_pos = 95;
      }
      else if(manualy > 0){
        ycurr_pos = MotorMove(ycurr_pos+1, ycurr_pos, diry, stepy, YStepsPerRevolution, stepDelay);
      }

      if(ycurr_pos <= 0){
        ycurr_pos = 0;
      }
      else if(manualy < 0){
        ycurr_pos = MotorMove(ycurr_pos-1, ycurr_pos, diry, stepy, YStepsPerRevolution, stepDelay);
      }
  }
}

void atick(){
  switch(state){
    case IDLE:
      if(xcurr_pos != xpos){
        state = XMOVE;
      }
      else{
        state = IDLE;
      }
      break;

    case XMOVE:
      xcurr_pos = MotorMove(xpos, xcurr_pos, dirx, stepx, XStepsPerRevolution, stepDelay);
      state = YMOVE;
      break;

    case YMOVE:
      ycurr_pos = MotorMove(ypos, ycurr_pos, diry, stepy, YStepsPerRevolution, stepDelay);
      busy = 0;
      state = IDLE;
      break;
  }
}


//i2c function to get info from master
void RecInfo(int bytes){
  mode = Wire.read();
  if(mode){
    xpos = Wire.read();
    ypos = Wire.read();
    //makes sure we never go out of bounds
    if(xpos > 100){
      xpos = 100;
    }
    if(xpos < 0){
      xpos = 0;
    }
    if(ypos > 95){
      ypos = 95;
    }
    if(ypos < 0){
      ypos = 0;
    }
    busy = 1;
  }
  else{
    manualx = Wire.read();
    manualy = Wire.read();
    busy = 0;
  }
  Wire.read();  
}

//i2c function to send info 
void ReqInfo(int bytes){
  Wire.write(busy);
}

int MotorMove(int move, int curr, int dir, int step, int Steps, int delay){
  if(curr > move){
    digitalWrite(dir, LOW);
    for(int j = 0; j < curr - move; ++j){
      for (int i = 0; i < Steps; i++) {
        digitalWrite(step, HIGH);
        delayMicroseconds(delay);
        digitalWrite(step, LOW);
        delayMicroseconds(delay);
      }
    }
  }
  else if(curr < move){
    digitalWrite(dir, HIGH);
    for(int j = 0; j < move - curr; ++j){
      for (int i = 0; i < Steps; i++) {
        digitalWrite(step, HIGH);
        delayMicroseconds(delay);
        digitalWrite(step, LOW);
        delayMicroseconds(delay);
      }
    }
  }
  return move;
}