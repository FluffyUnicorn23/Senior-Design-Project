#include <Servo.h>
#include <Wire.h>

Servo GripServo;
Servo RotateServo;
const int dirx = 5;
const int stepx = 4;
const int stepDelay = 1500;  // microseconds between steps (lower = faster)
const int stepsPerRevolution = 1000; // adjust to your motor

//global i2c variables
char mode = 1;
char zmove = 0;
char block_angle = 0;
char claw = 0;
char busy = 0;

enum {IDLE, ZDOWN, ANGLE, CLAWC, ZUP, CLAWO} state; 
enum {M_IDLE, M_MOVE} Mstate;



int ClawState = 0;     // Track current position (0 or 90)
bool at90 = false;      // Toggle state

void setup() {
  pinMode(dirx, OUTPUT);
  pinMode(stepx, OUTPUT);

  Wire.begin(9);
  Wire.onReceive(RecInfo);
  Wire.onRequest(ReqInfo);

  GripServo.attach(9);
  RotateServo.attach(10);
  GripServo.write(0); // start at 0 degrees
  RotateServo.write(0);     // start at 0 degrees
  Serial.begin(9600);
  delay(1000);
  Serial.println("Press 's' have claw grip state");
  Serial.println("Press 'a' and 'd' to rotate claw from 0° to 90°" );
  Serial.println("Press 'z' and 'c' to rotate stepper motor from CW or  CCW" );
  state = IDLE;
  Mstate = M_IDLE;
}

void loop() {
  if(mode){
    atick();
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

      if(zmove > 0){
        digitalWrite(dirx, HIGH);
        for (int i = 0; i < stepsPerRevolution; i++) {
          digitalWrite(stepx, HIGH);
          delayMicroseconds(stepDelay);
          digitalWrite(stepx, LOW);
          delayMicroseconds(stepDelay);
        }  
      }
      else if(zmove < 0)
      {
        digitalWrite(dirx, LOW);
        for (int i = 0; i < stepsPerRevolution; i++) {
          digitalWrite(stepx, HIGH);
          delayMicroseconds(stepDelay);
          digitalWrite(stepx, LOW);
          delayMicroseconds(stepDelay);
        }
      }
      if(block_angle > 0){
        if(ClawState <= 90){
          ClawState += 1;
          RotateServo.write(ClawState);
          delay(15);
        }
      }
      else if(block_angle < 0){
        if(ClawState >= 0){
          ClawState -= 1;
          RotateServo.write(ClawState);
          delay(15);
        }
      }
      if(claw){
        if(at90){
          moveGripServoSmooth(90, 0);
          at90 = false;
        }
      }
      else if(!claw){
        if(!at90){
          moveGripServoSmooth(0, 90);
          at90 = true;
        }
      } 
  }
}

void atick(){
  //transitions
  switch (state){
    case IDLE:
      if(busy){
        state = ZDOWN;
      }
      else{
        state = IDLE;
      }
      break;

    case ZDOWN:
      //CW movement
      digitalWrite(dirx, HIGH);
      for (int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(stepx, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepx, LOW);
        delayMicroseconds(stepDelay);
      }
      state = ANGLE;
      break;

    case ANGLE:
      while(block_angle != ClawState){
        if(block_angle < ClawState){
          if(ClawState <= 0){
            block_angle = ClawState;
          }
          else{
            ClawState -= 1;
            RotateServo.write(ClawState);
            delay(15);
          }
        }
        else{
          if(ClawState >= 90){
            block_angle = ClawState;
          }
          else{
            ClawState += 1;
            RotateServo.write(ClawState);
            delay(15);
          }
        }
      }
      state = CLAWC;
      break;

    case CLAWC:
      if(!at90){
        moveGripServoSmooth(0, 90);
        at90 = true;
      }
      state = ZUP;
      break;

    case ZUP:
      //CCW movement
      digitalWrite(dirx, LOW);
      for (int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(stepx, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepx, LOW);
        delayMicroseconds(stepDelay);
      }
      state = CLAWO;
      break;

    case CLAWO:
      if(at90){
        moveGripServoSmooth(90, 0);
        at90 = false;
      }
      state = IDLE;
      busy = 0;
      break;
  }
}

// Smooth servo movement function
void moveGripServoSmooth(int fromPos, int toPos) {
  if (fromPos < toPos) {
    for (int p = fromPos; p <= toPos; p++) {
      GripServo.write(p);
      delay(15);
    }
  } else {
    for (int p = fromPos; p >= toPos; p--) {
      GripServo.write(p);
      delay(15);
    }
  }
}

//i2c function to get info from master
void RecInfo(int bytes){
  mode = Wire.read();
  zmove = Wire.read();
  block_angle = Wire.read();
  claw =  Wire.read();

  Serial.println("mode: " + String(int(mode)) + " zmove: " + String(int(zmove)) + " angle: " + String(int(block_angle)));
  if(mode){
    busy = 1;
  }
}

//i2c function to send info 
void ReqInfo(int bytes){
  Wire.write(busy);
}