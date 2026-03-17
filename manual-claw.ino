#include <Servo.h>
#include <Wire.h>

Servo GripServo;
Servo RotateServo;
const int dirx = 2;
const int stepx = 3;
const int stepDelay = 1500;  // microseconds between steps (lower = faster)
const int stepsPerRevolution = 200; // 200 = 1 rev
const int weight = 10; //used to adjust how far down claw goes

int zcurr_pos = 0;

int resistance = 0;
const int threshold = 60;
const int close_pos = 35;
const int open_pos = 100;

//global i2c variables
char mode = 0;
char zmove = 0;
int block_angle = 0;
volatile char busy = 1;

char manualz = 0;
char manual_angle = 0;
char claw = 0;

//may need to switch up so we angle our claw first before we go down
enum {IDLE, ZDOWN, ANGLE, CLAWC, ZUP, CLAWO} state; 
enum {M_IDLE, M_MOVE} Mstate;



int ClawState = 60;     // Track current position (60 or 150)
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
  GripServo.write(open_pos); // start at open_pos degrees
  RotateServo.write(ClawState);     // start at 60 degrees
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

      if(zcurr_pos <= 0){
        zcurr_pos = 0;
      }
      else if(manualz < 0){
        zcurr_pos = MotorMove(zcurr_pos -1, zcurr_pos);  
      }

      if(zcurr_pos >= 20){
        zcurr_pos = 20;
      }
      else if(manualz > 0){
        zcurr_pos = MotorMove(zcurr_pos +1, zcurr_pos);
      }
      
      if(manual_angle > 0){
        if(ClawState <= 240){
          ClawState += 1;
          RotateServo.write(ClawState);
          delay(15);
        }
      }
      else if(manual_angle < 0){
        if(ClawState >= 0){
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
        state = ANGLE;
      }
      else if(zmove == 20){
        state = ANGLE;
      }
      else{
        state = IDLE;
      }
      break;

    case ZDOWN:
      //CW movement should be fixed length
      if(zmove == 20){
        zcurr_pos = MotorMove(20, zcurr_pos);
        state = CLAWO;
      }
      else if(zmove == 10){
        if(!at90){//opens claw if originally closed
          moveGripServoSmooth(close_pos, open_pos);
          at90 = true;
        }
        zcurr_pos = MotorMove(20, zcurr_pos);
        state = CLAWC;
      }
      break;

    case ANGLE:
      while(block_angle != ClawState){ // angles may need to go beyound [60,240]
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
          if(ClawState >= 240){
            block_angle = ClawState;
          }
          else{
            ClawState += 1;
            RotateServo.write(ClawState);
            delay(15);
          }
        }
      }
      state = ZDOWN;
      break;

    case CLAWC:
      moveGripServoSmooth(open_pos, close_pos);
      at90 = false;
      state = ZUP;
      break;

    case ZUP:
      //CCW movement
      zcurr_pos = MotorMove(0, zcurr_pos);
      
      zmove = 0;
      busy = 0;
      state = IDLE;
      break;

    case CLAWO:
      moveGripServoSmooth(close_pos, open_pos);
      
      at90 = true;
      state = ZUP;
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
        GripServo.write(p + 5);
        p = toPos - 1;
        break;
      }
      GripServo.write(p);
      delay(15);
    }
  }
}

//i2c function to get info from master
void RecInfo(int bytes){
  mode = Wire.read();
  if(mode){
    zmove = Wire.read();
    block_angle = Wire.read();
    Wire.read();
    busy = 1;
  }
  else{
    manualz = Wire.read();
    manual_angle = Wire.read();
    claw =  Wire.read();
    busy = 0;
  }
  // Serial.println("mode: " + String(int(mode)) + " zmove: " + String(int(zmove)) + " angle: " + String(int(block_angle)));
}

//i2c function to send info 
void ReqInfo(int bytes){
  Wire.write(busy);
}


int MotorMove(int move, int curr){
  if(curr > move){
    digitalWrite(dirx, HIGH);
    for(int j = 0; j < curr - move; ++j){
      for (int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(stepx, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepx, LOW);
        delayMicroseconds(stepDelay);
      }
    }
  }
  else if(curr < move){
    digitalWrite(dirx, LOW);
    for(int j = 0; j < move - curr; ++j){
      for (int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(stepx, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepx, LOW);
        delayMicroseconds(stepDelay);
      }
    }
  }
  return move;
}