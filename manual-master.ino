#include <Servo.h>
#include <Wire.h>

Servo RotateServo;
int joyx = 0; //pin A0
int joyy = 0; //pin A1
int ManualOveride = 0; //pin 2
int joyb = 0; //pin 3
int clawState = 0; //pin 4

bool buttInput[3] = {false,false,false};
// first is switch from manual to auto, 2nd is switch what joystick does, 3rd is to opn/close claw 

enum Buttons{OFF, PRESS_ON, ON, PRESS_OFF} Bstate;
enum Manual{M_IDLE, CLAW, XY} Mstate;

void setup() {
  // put your setup code here, to run once:
  Wire.begin();

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);

  pinMode(5, OUTPUT);
  RotateServo.attach(9);
  Serial.begin(9600);
  Serial.println("Starting test");
  Bstate = OFF;
  Mstate = M_IDLE;
}

void loop() {
  // read inputs
  joyx = analogRead(A0);
  joyy = analogRead(A1);
  ManualOveride = digitalRead(2);                  
  joyb = digitalRead(3);
  clawState = digitalRead(4);
  

  delay(50);
  //tick based on inputs
  btick();
  //redundancy if check
  if(buttInput[0]){
    Atick();
  }
  else{
    Mtick();
  }
}

//toggle states 
void toggle(){
  if(ManualOveride){
    buttInput[0] = !buttInput[0];

  }
  if(joyb){
    buttInput[1] = !buttInput[1];
  }
  if(clawState){
    buttInput[2] = !buttInput[2];
  }
}

//tick statement for button inputs 
void btick(){
  switch(Bstate){
    case OFF:
      if(ManualOveride || joyb || clawState){
        toggle();
        Bstate = PRESS_ON;
      }
      else{
        Bstate = OFF;
      }
      break;

    case PRESS_ON:
      if(ManualOveride || joyb || clawState){
        Bstate = PRESS_ON;
      }
      else{
        Bstate = ON;
      }
      break;

    case ON:
      if(ManualOveride || joyb || clawState){
        toggle();
        Bstate = PRESS_OFF;
      }
      else{
        Bstate = ON;
      }
      break;
    
    case PRESS_OFF:
      if(ManualOveride || joyb || clawState){
        Bstate = PRESS_OFF;
      }
      else{
        Bstate = OFF;
      }
      break;
  }
}

//tick stateement for manual mode
void Mtick(){
  char sendx = 0;
  char sendy = 0;
  switch(Mstate){
    case M_IDLE:
      Serial.println("IDLE");
      if(buttInput[0]){
        Serial.println((buttInput[0]));
        Mstate = M_IDLE;
      }
      else if(!buttInput[0] && buttInput[1]){
        Mstate = XY;
      }
      else{
        Mstate = CLAW;
      }
      break;

    case CLAW:
      if(buttInput[0]){
        Mstate = M_IDLE;
      }
      else if(!buttInput[0] && buttInput[1]){
        Mstate = XY;
      }
      else{
        Mstate = CLAW;
      }

      if(joyx > 756){
        sendx = 1;
      }
      else if(joyx < 256){
        sendx = -1;
      }

      if(joyy > 756){
        sendy = 1;
      }
      else if(joyy < 256){
        sendy = -1;
      }
 
      send_data(9, buttInput[0], sendx, sendy, buttInput[2]);
      break;

    case XY:
      Serial.println("XY");
      if(buttInput[0]){
        Mstate = M_IDLE;
      }
      else if(!buttInput[0] && buttInput[1]){
        Mstate = XY;
      }
      else{
        Mstate = CLAW;
      }

    
      if(joyx > 756){
        sendx = 1;
      }
      else if(joyx < 256){
        sendx = -1;
      }

      if(joyy > 756){
        sendy = 1;
      }
      else if(joyy < 256){
        sendy = -1;
      }
 
      send_data(10, buttInput[0], sendx, sendy, buttInput[2]);
      break;
  }
}

//tick statement for auto mode
void Atick(){
  Serial.println("Atick implement later");
}

//function to send data
void send_data(int adrr, char send1, char send2, char send3, char send4){
  Wire.beginTransmission(adrr);

  char send[4] = {send1, send2, send3, send4};
  Wire.write(send, 4);

  int i = Wire.endTransmission();
  Serial.println("adr: " + String(adrr) + " " + String(i));
  Serial.println(String(int(send2)) + " " + String(int(send3)));
}
