#include <Pixy2.h>
Pixy2 pixy;
//problems to be adressed
//sharp angles >90 aren't really detected, work on it, possible issue illumination
//not using the whole block to determine x,y pos, not as important, but work on it
//not 100% detecting the color codes, must fix.
//can work in dark just make sure to set pixy.setLamp(1,0);

//83 is 3 by 2 by 2.5
//19 is 2 by 2 by 2
//10 is 2 by 1.75 by 1.5
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("starting");
  pixy.init();
  pixy.setLamp(1,0);
  // pixy.setCameraBrightness(50);
}

void loop() {
  // put your main code here, to run repeatedly:
  //char tmp = final_object_detect();
  delay(1000);
  pixy.ccc.getBlocks(); 
  int arr[pixy.ccc.numBlocks/2][4]; //at [0][0] has first blocks type, at [0][1] has first blocks angle at 
  //[0][2] has first blocks x pos at [0][3] has first blocks y pos
  int num = 0;
  for (int i = 0; i < pixy.ccc.numBlocks; ++i){
    int type = pixy.ccc.blocks[i].m_signature;
    
    //grab unique color codes associated with blocks
    //Serial.println("block number =" + String(i) + " identify code =" + String(type)); 

    //finds closest block to identification tags, and matches into arr
    if(type == 10 || type == 83 || type == 19) {//unique color code for identification
      int ind = 0;
      int small = sqr(pixy.ccc.blocks[i].m_x - pixy.ccc.blocks[0].m_x) + sqr(pixy.ccc.blocks[i].m_y - pixy.ccc.blocks[0].m_y); 
      for (int j = 0; j < pixy.ccc.numBlocks; ++j){
        if(pixy.ccc.blocks[j].m_signature < 10){ //make sure we target objects and not identification tags
          int check = sqr(pixy.ccc.blocks[i].m_x - pixy.ccc.blocks[j].m_x) + sqr(pixy.ccc.blocks[i].m_y - pixy.ccc.blocks[j].m_y);
          if (check < small){
            small = check;
            ind = j;
          }
        }
      }
      arr[num][0] = type;
      arr[num][1] = pixy.ccc.blocks[i].m_angle;
      arr[num][2] = pixy.ccc.blocks[ind].m_x;
      arr[num][3] = pixy.ccc.blocks[ind].m_y;
      ++num;
    }
  }
  for (int i = 0; i < pixy.ccc.numBlocks/2; ++i){
    Serial.println("Block of type " + String(arr[i][0]) + " found at (" + String(arr[i][2]) + "," + String(arr[i][3]) + ") angled " 
    + String(arr[i][1]) + " degrees.");
  }
}
//helper functions
int sqr(int x){
  return x*x;
}
