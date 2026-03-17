#include <Wire.h>

char MotorOn = 0; // 0 = off, 1 = on
char mode = 0;
char speed = 0;
char busy = 0;
const int ENA_PIN = 10; //enable can be any pwm pin
const int IN1_PIN = 9; // IN1 and IN2 can be any digital pin
const int IN2_PIN = 6;
const int trig_pin = 8;
const int echo_pin = 7; // may have to change
float timing = 0.0;
float distance = 0.0;
bool BlockDetected  = false;


enum {MOTOR_OFF, MOTOR_ON} state; 


void setup() {
  // put your setup code here, to run once:
  pinMode(ENA_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(echo_pin, INPUT);
  pinMode(trig_pin, OUTPUT);

  Wire.begin(11);
  Wire.onReceive(RecInfo);
  Wire.onRequest(ReqInfo);

  Serial.begin(9600);
  
}

  //analogwrite(ENA_PIN, speed of motor) (change from 0-255)
  //digitalWrite(IN1_PIN, HIGH); // HI HI = LO LO = OFF, HI LOW = CW, LOW HI = CCW
  //digitalWrite(IN2_PIN, LOW); 

void loop() {

  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);
  timing = pulseIn(echo_pin, HIGH, 20000);
  distance = (timing * 0.034) / 2;
    
  if (distance > 20) { // in cm 
  	BlockDetected = false;
  }
  else{
      BlockDetected = true;
  }

  if(mode){
    atick();
  }
  else{
    mtick();
  }
}

void mtick(){
  (ENA_PIN, 0); //(change from 0-255 to change speed)
  analogWrite(IN1_PIN, LOW); // HI HI = LO LO = OFF, HI LOW = CW, LOW HI = CCW
  digitalWrite(IN2_PIN, LOW); 
}


void atick(){
  switch (state){
    case MOTOR_OFF:

    if(MotorOn && !BlockDetected){
      analogWrite(ENA_PIN, speed); //highest possible speed
      digitalWrite(IN1_PIN, HIGH); // HI HI = LO LO = OFF, HI LOW = CW, LOW HI = CCW
      digitalWrite(IN2_PIN, LOW); 
      state = MOTOR_ON;
    }
        
      break;
    
    case MOTOR_ON:

      if(!MotorOn || BlockDetected){
        mtick();
        state = MOTOR_OFF;
      }
      break;
  }
}


//i2c function to get info from master
void RecInfo(int bytes){
  mode = Wire.read();
  MotorOn = Wire.read();
  speed = Wire.read();
  Wire.read();
  //since master function always sends 4 bits of data

}

//i2c function to send info 
void ReqInfo(int bytes){
  
  char c = distance;
  Wire.write(c);
}
    

  
