#include <Wire.h>
#include <Pixy2.h>
#include <KickSort.h>
Pixy2 pixy;

//check data names, and make sure nothing is resetting
struct cam_vals{
  int sig = 0;
  int angle = 0;
  int xpos = 0;
  int ypos = 0;
};

int dist = 0;
char x = 0;
char y = 0;
int type = 0;
int t = 0;

int joyx = 0; //pin A0
int joyy = 0; //pin A1
int ManualOveride = 0; //pin 6
int joyb = 0; //pin 10
int clawState = 0; //pin 9
//reserve pins 2 pins for sonar sensor

//i2c global variables
char info = 0;
char resistance = 0;
int arrX[30];
int arrY[30];
int arrANGLE[30];
int count = 0;

bool buttInput[3] = {false,false,false};
// first is switch from manual to auto, 2nd is switch what joystick does, 3rd is to opn/close claw 

enum Buttons{OFF, PRESS_ON, ON, PRESS_OFF} Bstate;
enum Manual{M_IDLE, M_CLAW, XY} Mstate;
enum AUTO {A_IDLE, AVG, MOVE_BLOCK, WAIT1, A_CLAWC, WAIT2, MOVE_DROP, WAIT3, A_CLAWO, WAIT4} Astate;
//should work if detection is consistent enough. If not could have some issues when in the wait states

void setup() {
  // put your setup code here, to run once:
  Wire.begin();

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(6, INPUT);
  pinMode(10, INPUT);
  pinMode(9, INPUT);

  pixy.init();
  pixy.setLamp(0,0);

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

  char c = 'l';
  if(Serial.available()){
    c = Serial.read();
  }
  if(c == 's'){
    Astate = A_IDLE;
  }

  //tick based on inputs
  delay(50);
  btick();
  //redundancy if check
  if(buttInput[0]){
    pixy.setLamp(0, 0);
    //read camera
    pixy.ccc.getBlocks();    
    cam_vals arrCAM[pixy.ccc.numBlocks];
    int cnt = 0;

    for(int i =0; i < pixy.ccc.numBlocks; ++i){
      if(pixy.ccc.blocks[i].m_x < 240 && pixy.ccc.blocks[i].m_x > 50){
        if(pixy.ccc.blocks[i].m_y < 205 && pixy.ccc.blocks[i].m_y > 130){
          arrCAM[cnt].angle = pixy.ccc.blocks[i].m_angle;
          arrCAM[cnt].sig = pixy.ccc.blocks[i].m_signature;
          arrCAM[cnt].xpos = pixy.ccc.blocks[i].m_x;
          arrCAM[cnt].ypos = pixy.ccc.blocks[i].m_y;
          ++cnt;
        }
      }
    }

    Atick(cnt, arrCAM[0].sig, arrCAM[0].angle, arrCAM[0].xpos, arrCAM[0].ypos);
    // Serial.println(Astate);   
  }
  else{
    send_data(11, buttInput[0], 0, 0, 11);
    Mtick();
    t = (t + 1) % 5;
    pixy.setLED(R(t), G(t), B(t));
    Astate = A_IDLE;
  }
  delay(50);
}

//toggle states 
void toggle(){
  if(ManualOveride){
    buttInput[0] = !buttInput[0];

  }
  if(!joyb){
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
      if(ManualOveride || !joyb || clawState){
        toggle();
        Bstate = PRESS_ON;
      }
      else{
        Bstate = OFF;
      }
      break;

    case PRESS_ON:
      if(ManualOveride || !joyb || clawState){
        Bstate = PRESS_ON;
      }
      else{
        Bstate = ON;
      }
      break;

    case ON:
      if(ManualOveride || !joyb || clawState){
        toggle();
        Bstate = PRESS_OFF;
      }
      else{
        Bstate = ON;
      }
      break;
    
    case PRESS_OFF:
      if(ManualOveride || !joyb || clawState){
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

      if(joyy > 756){
        sendx = 1;
      }
      else if(joyy < 256){
        sendx = -1;
      }

      if(joyx > 756){
        sendy = 1;
      }
      else if(joyx < 256){
        sendy = -1;
      }

      send_data(9, buttInput[0], sendx, sendy, buttInput[2]);
      //used to read resistance from claw
    
      // resistance = receive_data(9);
      // Serial.print("resistance = ");
      // Serial.println(int(resistance));
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
 
      send_data(10, buttInput[0], sendx, sendy, 0);
      send_data(9, buttInput[0], 0, 0, buttInput[2]);
      break;
  }
}

//tick statement for auto mode
void Atick(char amount, char tmp_type, int angle, int xpos, int ypos){
  switch(Astate){
    case A_IDLE:
      Serial.println("A_IDLE");
    
      dist = receive_data(11);
      Serial.print("dist = ");
      Serial.println(dist);
      
      if(buttInput[0] && dist < 20 && amount > 0){
        count = 0;
        send_data(11, buttInput[0], 0, 0, 11);
        Astate = AVG;
      }
      else{
        send_data(10, buttInput[0], 10, 10, 1);
        delay(1000);
        send_data(11, buttInput[0], 1, 90, 11);
        Astate = A_IDLE;
      }
      break;

    case AVG:
      Serial.println("AVG");
      if(count < 30){
        arrX[count] = xpos;
        arrY[count] = ypos;
        arrANGLE[count] = angle;
        ++count;
        Astate = AVG;
      }
      else{
        KickSort<int>::insertionSort(arrX, 30);
        KickSort<int>::insertionSort(arrY, 30);
        KickSort<int>::insertionSort(arrANGLE, 30);
        Astate = MOVE_BLOCK;
      }
      break;

    case MOVE_BLOCK:
      Serial.println("MOVE");
      arrX[0] = 0;
      arrY[0] = 0;
      arrANGLE[0] = 0;
      for (int i = 5; i < 15; ++i){
        arrX[0] += arrX[i];
        arrY[0] += arrY[i];
        arrANGLE[0] += arrANGLE[i];
      }
      arrX[0] /= 10;
      arrY[0] /= 10;
      arrANGLE[0] /= 10;
      type = tmp_type;
      x = X(arrX[0], arrY[0]);
      y = Y(arrY[0]);
      send_data(10, buttInput[0], x, y, 1);
      Serial.print(String(int(x)) + "," + String(int(y)) + "," + String(type));

      delay(1000);
      info = 1;
      Astate = WAIT1;
      break;

    case WAIT1:
      Serial.println("WAIT1");

      info = receive_data(10);
  
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
      Serial.println(Angle(arrANGLE[0]));

      send_data(9, buttInput[0], 10, Angle(arrANGLE[0]), 1);
      
      delay(1000);
      info = 1;
      Astate = WAIT2;
      break;

    case WAIT2:
      Serial.println("WAIT2");
      
      info = receive_data(9);
    
      if(!buttInput[0]){
        Astate = A_IDLE;
      }
      else if(info){
        Astate = WAIT2;
        Serial.println(type);
      }
      else{
        Astate = MOVE_DROP;
      }
      break;

    case MOVE_DROP:
    //maybe start here, maybe??
      Serial.println("MOVE_DROP");
      if(type == 10){
        send_data(10, buttInput[0], 10, 10, 1);
      }
      else if(type == 83){
        send_data(10, buttInput[0], 10, 50, 1);
      }
      else if(type == 19){
        send_data(10, buttInput[0], 10, 90, 1);
      }
      else{
        send_data(10, buttInput[0], 20, 20, 1);
      }

      delay(1000);
      info = 1;
      Astate = WAIT3;
      break;

    case WAIT3:
      Serial.println("WAIT3");
      
      info = receive_data(10);
      
      if(!buttInput[0]){
        Astate = A_IDLE;
      }
      else if(info){
        Astate = WAIT3;
      }
      else{
        Astate = A_CLAWO;
      }
      break;

    case A_CLAWO:
      Serial.println("A_CLAWO");
      send_data(9, buttInput[0], 20, 150, 1);
      
      info = 1;
      Astate = WAIT4;
      break;
    
    case WAIT4:
      info = receive_data(9);
      
      if(!buttInput[0]){
        Astate = A_IDLE;
      }
      else if(info){
        Astate = WAIT4;
      }
      else{
        Astate = A_IDLE;
      }
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

char receive_data(int adrr){
  
  Wire.requestFrom(adrr, 1);
  //loop Wire will send stuff;
  int i = 0;
  if(Wire.available()){
    i = Wire.read();
  }  
  
  Serial.print("i = ");
  Serial.println(i);
  return i;
}

//converst to usable angls
//i have no idea, it's workin tho
unsigned char Angle(int angle){
  Serial.print("angle = ");
  Serial.println(angle);
  if(angle <= 90 && angle >= 0){
    return 150 - angle;
  }
  else if(angle > 90){
    if(angle >= 150){
      return 0;
    }
    return 60 - (angle - 90);
  }
  else if(angle < -90){
    return 150 - (180 - abs(angle));
  }
  else if(angle < 0){
    if(angle >= -30){
      return 0;
    }
    return abs(angle);
  }
}
//changes x and y camera inputs
//middle of image is (130,210)
//returns values that can be used for xy control
int X(int x, int y){
  x = x - 135; //center pixel
  Serial.println(1.0*((y -160) / (105-160) * (1.26 - 5.25) + 5.25));
  if(x<0){
    return 50 + x/5 - 1.0*((y -160) / (105-160) * (1.26 - 5.25) + 5.25);
  }
  return 50 + x/4 - 1.0*((y -160) / (105-160) * (1.26 - 5.25) + 5.25); // 160 -105, offsets of 1 in to 1/4 inch 
}
//zero in by doing 
int Y(int y){
  y = y - 180;
  return 50 + y/2;
}

char rgb = 0;
char R_arr[3] = {1,0,0};
char G_arr[3] = {0,1,0};
char B_arr[3] = {0,0,1};

char R(int cnt){
  if(cnt >= 4){
    rgb = (rgb + 1) % 3; 
  }
  return R_arr[rgb];
}

char G(int cnt){
  return G_arr[rgb];
}

char B(int cnt){
  return B_arr[rgb];
}