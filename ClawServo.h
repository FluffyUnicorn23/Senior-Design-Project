#ifndef CLAW_SERVO_H
#define CLAW_SERVO_H

#include<Arduino.h>
#include<Servo.h>

void MoveServo(int fromPos, int toPos, Servo GripServo) {
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

#endif 