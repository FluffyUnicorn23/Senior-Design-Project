#include <Servo.h>
#include <Wire.h>

Servo GripServo;
Servo RotateServo;
const int dirx = 2;
const int stepx = 3;
const int stepDelay = 1500;  // microseconds between steps (lower = faster)
const int stepsPerRevolution = 200; // 200 = 1 rev
const int weight = 5; //used to adjust how far down claw goes

int resistance = 0;
const int threshold = 25;
const int close_pos = 25;
const int open_pos = 100;

//global i2c variables
char mode = 0;
char zmove = 0;
char block_angle = 0;
char claw = 0;

//may need to switch up so we angle our claw first before we go down
enum {IDLE, ZDOWN, ANGLE, CLAWC, ZUP, CLAWO} state; 
enum {M_IDLE, M_MOVE} Mstate;



int ClawState = 0;     // Track current position (0 or 90)
bool at90 = false;      // Toggle state

void setup() {
  pinMode(dirx, OUTPUT);
  pinMode(stepx, OUTPUT);
  pinMode(A0, INPUT);

  Wire.begin(9);
  Wire.onReceive(RecInfo);
  Wire.onRequest(ReqInfo);

  GripServo.attach(9);
  RotateServo.attach(10);
  GripServo.write(close_pos); // start at 0 degrees
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
        if(ClawState <= 91){
          ClawState += 1;
          RotateServo.write(ClawState);
          delay(15);
        }
      }
      else if(block_angle < 0){
        if(ClawState >= 1){
          ClawState -= 1;
          RotateServo.write(ClawState);
          delay(15);
        }
      }
      if(claw){
        if(at90){
          moveGripServoSmooth(open_pos, close_pos);
          at90 = false;
        }
      }
      else if(!claw){
        if(!at90){
          moveGripServoSmooth(close_pos, open_pos);
          at90 = true;
        }
      } 
  }
}

void atick(){
  //transitions
  switch (state){
    case IDLE:
      if(zmove == 10){
        state = ZDOWN;
      }
      else if(zmove == 20){
        state = CLAWO;
      }
      else{
        state = IDLE;
        claw = 0;
      }
      break;

    case ZDOWN:
      //CW movement should be fixed length
      if(!at90){//opens claw if originally closed
        moveGripServoSmooth(close_pos, open_pos);
        at90 = true;
      }
      digitalWrite(dirx, HIGH);
      for (int i = 0; i < weight * stepsPerRevolution; i++) {
        digitalWrite(stepx, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepx, LOW);
        delayMicroseconds(stepDelay);
      }
      state = ANGLE;
      break;

    case ANGLE:
      while(block_angle != ClawState){ // angles may need to go beyound [0,90]
        if(block_angle < ClawState){
          if(ClawState <= 1){
            block_angle = ClawState;
          }
          else{
            ClawState -= 1;
            RotateServo.write(ClawState);
            delay(15);
          }
        }
        else{
          if(ClawState >= 91){
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
      moveGripServoSmooth(open_pos, close_pos);
      at90 = false;
      state = ZUP;
      break;

    case ZUP:
      //CCW movement
      digitalWrite(dirx, LOW);
      for (int i = 0; i < weight * stepsPerRevolution; i++) {
        digitalWrite(stepx, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepx, LOW);
        delayMicroseconds(stepDelay);
      }
      zmove = 0;
      state = IDLE;
      break;

    case CLAWO:
      moveGripServoSmooth(close_pos, open_pos);
      at90 = true;
      zmove = 0;
      state = IDLE;
      
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
      resistance = analogRead(A0);
      if(resistance > threshold){
        GripServo.write(p + 1);
        Serial.print("p = ");
        Serial.println(p);  
        p = toPos - 1;
      }
      Serial.println(resistance);
      GripServo.write(p);
      delay(20);
      
    }
  }
}

//i2c function to get info from master
void RecInfo(int bytes){
  mode = Wire.read();
  zmove = Wire.read();//in manual tells z axis how to move, in auto tells master when we are done
  block_angle = Wire.read();
  claw =  Wire.read();

  Serial.println("mode: " + String(int(mode)) + " zmove: " + String(int(zmove)) + " angle: " + String(int(block_angle)));
}

//i2c function to send info 
void ReqInfo(int bytes){
  Wire.write(claw);
}