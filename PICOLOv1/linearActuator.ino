/*
_____________________________________________________________
Code for the reaction control system controlled by a Linear Actuator
Using SparkFun TB6612FNG driver and BNO055 IMU
Code by: Andrew Schmit
Last modified: 6/30/2025
_____________________________________________________________
*/

void Actuatorsetup() {

  analogWrite(PWMB, moveDuty); 

  printOLED("Starting Actuator test.");
  
  moveActuator(false);
  for (int i = 0; i < 50; i++){
    delay(100);
    pos = analogRead(feedbackPin);
    printOLED(String(pos));
  }
  maxPos = pos;

  moveActuator(true);
  for (int i = 0; i < 50; i++){
    delay(100);
    pos = analogRead(feedbackPin);
    printOLED(String(pos));
  }
  minPos = pos;
  
  moveActuator(false);
  for (int i = 0; i < 50; i++){
    delay(100);
    pos = analogRead(feedbackPin);
    truePos = posMap(pos,minPos,maxPos);
    printOLED(String(truePos));
  }
  
  resetActuator();
  printOLED(String(truePos));

  printOLED("Actuator test complete.");
}

float posMap(int pos,int min,int max){
  return (float)(pos - min) * (5.0 / (max - min)); // only change that might be meaningful
}
// Moves actuator in a direction at duty cycle

// Stops the actuator
void stopActuator() {
  analogWrite(PWMB, 0);
}

void resetActuator() {
  // Continuously move actuator until it's within the desired range
  while (true) {
    pos = analogRead(feedbackPin);
    truePos = posMap(pos, minPos, maxPos);

    if (truePos > targetPos + tolerance) {
      moveActuator(true);   // Move toward retracted
    }
    else if (truePos < targetPos - tolerance) {
      moveActuator(false);  // Move toward extended
    }
    else {
      stopActuator(); 
      delay(100);
      Serial.println("centered");
      break;  // Exit loop when within range
    }
    delay(50);  // Give time for motor to move
  }
}

void updateLinearActuator() {  
  if(pixy.ccc.numBlocks > 0){
     proportionalZ = KPZ * errorZ; // find proportional term
     Serial.println("linear Actuator stuff: " + String(proportionalZ) + ", " + String(errorZ) + ", " + String(tiltOffset));
      if (proportionalZ < -5){
        Serial.println("move true");
        moveActuator(true);
      }
      else if (proportionalZ > 5) {
        Serial.println("move false");
        moveActuator(false);
      }
      else {
        Serial.println("move stop");
        stopActuator();
      }
    }
  else{
    stopActuator();
  }

}
// Reads and prints position every 100 ms for `duration` milliseconds
void moveActuator(bool extend) {
  digitalWrite(BIN1, extend ? HIGH : LOW);
  digitalWrite(BIN2, extend ? LOW : HIGH);
  analogWrite(PWMB, moveDuty);  // 20% 
  Serial.println("movibg");
}
