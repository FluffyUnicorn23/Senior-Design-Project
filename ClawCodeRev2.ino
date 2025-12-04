#include <Servo.h>

Servo GripServo;
Servo RotateServo;

int ClawState = 0;     // Track current position (0 or 90)
bool at90 = false;      // Toggle state

void setup() {
  GripServo.attach(9);
  RotateServo.attach(11);
  GripServo.write(0); // start at 0 degrees
  RotateServo.write(0);     // start at 0 degrees
  Serial.begin(9600);
  delay(1000);
  Serial.println("Press 's' have claw grip state");
  Serial.println("Press 'a' and 'd' to rotate claw from 0° to 90°" );
}

void loop() {
  // Check if a character is available
  if (Serial.available() > 0) {
    char c = Serial.read();

    if (c == 's') {   // if the user pressed "s"
      if (!at90) {
        moveGripServoSmooth(0, 90);
        at90 = true;
      } else {
        moveGripServoSmooth(90, 0);
        at90 = false;
      }
    }
    else if(c == 'a'){
      if(ClawState <= 0){
        Serial.println("Cannot rotate further in that direction");
      }
      else{
        ClawState -= 10;
        RotateServo.write(ClawState);
        delay(15);

      }

    }
    else if(c == 'd'){
      if(ClawState == 90){
        Serial.println("Cannot rotate further in that direction");
      }
      else{
        ClawState += 10;
        RotateServo.write(ClawState);
        delay(15);
      }
    }
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