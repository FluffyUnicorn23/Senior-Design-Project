#ifndef STEPPER_H
#define STEPPER_H

#include<Arduino.h>


void MoveStepperMotor(int FromPosition, int ToPosition, int stepPin, int dirPin){
    
    if (FromPosition > ToPosition){
        digitalWrite(dirPin, HIGH);
        for (int i = 0; i < (FromPosition - ToPosition); i++) {
            digitalWrite(stepPin, HIGH);
            delayMicroseconds(100);
            digitalWrite(stepPin, LOW);
            delayMicroseconds(100);
      }

    } else{
        digitalWrite(dirPin, LOW);
        for (int i = 0; i < ToPosition - FromPosition; i++) {
            digitalWrite(stepPin, HIGH);
            delayMicroseconds(100);
            digitalWrite(stepPin, LOW);
            delayMicroseconds(100);
        }

    }

}

#endif