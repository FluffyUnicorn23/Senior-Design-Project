#include <Wire.h>
#include <Pixy2.h>
//Pixy2 pixy;

int joyx = 0; //pin A0
int joyy = 0; //pin A1
int ManualOveride = 0; //pin 10
int joyb = 0; //pin a2
int clawState = 0; //pin 9
int DC_motor = 7; //pin 6
//reserve pins 9 and 10 for sonar sensor

//i2c global variables
char info = 0;

bool buttInput[3] = {false,false,false};
// first is switch from manual to auto, 2nd is switch what joystick does, 3rd is to opn/close claw 

enum Buttons{OFF, PRESS_ON, ON, PRESS_OFF} Bstate;
enum Manual{M_IDLE, M_CLAW, XY} Mstate;
enum AUTO {A_IDLE, MOVE_BLOCK, WAIT1, A_CLAWC, WAIT2, MOVE_DROP, WAIT3, A_CLAWO} Astate;
//should work if detection is consistent enough. If not could have some issues when in the wait states

void setup() {
  // put your setup code here, to run once:
  Wire.begin();

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(6, INPUT);
  pinMode(10, INPUT);
  pinMode(9, INPUT);

  pinMode(5, OUTPUT);
  pinMode(DC_motor, OUTPUT);

//  pixy.init();
//  pixy.setLamp(0,0);

  Serial.begin(9600);
  Serial.println("Starting test");
  Bstate = OFF;
  Mstate = M_IDLE;
  Astate = A_IDLE;
}

void loop() {
  // read inputs
  joyx = analogRead(A0);
  joyy = analogRead(A1);
  ManualOveride = digitalRead(6);                  
  joyb = digitalRead(10);
  clawState = digitalRead(9);

  // //read camera
  // pixy.ccc.getBlocks(); 
  // int arr[pixy.ccc.numBlocks/2][4]; //at [0][0] has first blocks type, at [0][1] has first blocks angle at 
  // //[0][2] has first blocks x pos at [0][3] has first blocks y pos
  // int num = 0;
  // for (int i = 0; i < pixy.ccc.numBlocks; ++i){
  //   int type = pixy.ccc.blocks[i].m_signature;
    
  //   //grab unique color codes associated with blocks
  //   //Serial.println("block number =" + String(i) + " identify code =" + String(type)); 

  //   //finds closest block to identification tags, and matches into arr
  //   if(type == 10 || type == 83 || type == 19) {//unique color code for identification
  //     int ind = 0;
  //     int small = sqr(pixy.ccc.blocks[i].m_x - pixy.ccc.blocks[0].m_x) + sqr(pixy.ccc.blocks[i].m_y - pixy.ccc.blocks[0].m_y); 
  //     for (int j = 0; j < pixy.ccc.numBlocks; ++j){
  //       if(pixy.ccc.blocks[j].m_signature < 10){ //make sure we target objects and not identification tags
  //         int check = sqr(pixy.ccc.blocks[i].m_x - pixy.ccc.blocks[j].m_x) + sqr(pixy.ccc.blocks[i].m_y - pixy.ccc.blocks[j].m_y);
  //         if (check < small){
  //           small = check;
  //           ind = j;
  //         }
  //       }
  //     }
  //     arr[num][0] = type;
  //     arr[num][1] = pixy.ccc.blocks[i].m_angle;
  //     arr[num][2] = pixy.ccc.blocks[ind].m_x;
  //     arr[num][3] = pixy.ccc.blocks[ind].m_y;
  //     ++num;
  //   }
  // }
  char c = 'k';  
  if(Serial.available()){
    c = Serial.read();
  }

  if(c == 'a'){
    buttInput[0] = !buttInput[0];
  }
  if(c == 's'){
    buttInput[1] = !buttInput[1]; 
  }
  if(c == 'd'){
    buttInput[2] = !buttInput[2];
  }
  //tick based on inputs
  delay(50);
  btick();
  //redundancy if check
  if(buttInput[0]){
    //Atick(pixy.ccc.numBlocks /2, arr[0][0], arr[0][1], arr[0][2], arr[0][3]);
    Serial.println("atick");
  }
  else{
    digitalWrite(DC_motor, LOW);
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

//tick statement for manual mode
void Mtick(){
  char sendx = 0;
  char sendy = 0;
  switch(Mstate){
    case M_IDLE:
      Serial.println("M_IDLE");
      if(buttInput[0]){
        Serial.println((buttInput[0]));
        Mstate = M_IDLE;
      }
      else if(!buttInput[0] && buttInput[1]){
        Mstate = XY;
      }
      else{
        Mstate = M_CLAW;
      }
      break;

    case M_CLAW:
    Serial.println("M_CLAW");
      if(buttInput[0]){
        Mstate = M_IDLE;
      }
      else if(!buttInput[0] && buttInput[1]){
        Mstate = XY;
      }
      else{
        Mstate = M_CLAW;
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
        Mstate = M_CLAW;
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
void Atick(char amount, char type, char angle, char xpos, char ypos){
//delay(500);
//actions
switch(Astate){
    case A_IDLE:
      Serial.println("IDLE");
      if(buttInput[0] && amount >= 1){
        Astate = MOVE_BLOCK;
        digitalWrite(DC_motor, LOW);
      }
      else{
        Astate = A_IDLE;
        digitalWrite(DC_motor, HIGH);
      }
      break;
    case MOVE_BLOCK:
      Serial.println("MOVE");
      send_data(10, buttInput[0], xpos, ypos, 1);
      
      Astate = WAIT1;
      break;

    case WAIT1:
      Serial.println("WAIT1");
      Wire.requestFrom(10,1);
      if(Wire.available()){
        info = Wire.read();
      }
      if(!buttInput[0]){
        Astate = A_IDLE;
      }
      else if(info){
        Astate = WAIT1;
      }
      else{
        Astate = A_CLAWC;
      }
      break;

    case A_CLAWC:
      Serial.println("A_CLAWC");
      send_data(9, buttInput[0], 10, angle, 1);
      
      Astate = WAIT2;
      break;

    case WAIT2:
      Serial.println("WAIT2");
      Wire.requestFrom(9,1);
      if(Wire.available()){
        info = Wire.read();
      }
      if(!buttInput[0]){
        Astate = A_IDLE;
      }
      else if(info){
        Astate = WAIT1;
      }
      else{
        Astate = MOVE_DROP;
      }
      break;

    case MOVE_DROP:
      Serial.println("MOVE_DROP");
      //designated drop off points. with an extra for erronious objects
      if(type == 10){
        send_data(10, buttInput[0], 5, 5, 1);
      }
      else if(type == 83){
        send_data(10, buttInput[0], 5, 5, 1);
      }
      else if(type == 19){
        send_data(10, buttInput[0], 5, 5, 1);
      }
      else{
        send_data(10, buttInput[0], 5, 5, 1);
      }
      Astate = WAIT3;
      break;
    
    case WAIT3:
      Serial.println("WAIT3");
      Wire.requestFrom(10,1);
      if(Wire.available()){
        info = Wire.read();
      }
      if(!buttInput[0]){
        Astate = A_IDLE;
      }
      else if(info){
        Astate = WAIT1;
      }
      else{
        Astate = A_CLAWO;
      }
      break;

    case A_CLAWO:
      Serial.println("A_CLAWO");
      send_data(9, buttInput[0], 20, angle, 1);
      
      Astate = A_IDLE;
      break;
  }
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

//return the x^2
int sqr(int x){
  return x*x;
}