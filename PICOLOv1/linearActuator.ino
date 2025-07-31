/*
_____________________________________________________________
Code for the reaction control system controlled by a Linear Actuator
Using SparkFun TB6612FNG driver and BNO055 IMU
Code by: Andrew Schmit
Last modified: 6/30/2025
_____________________________________________________________
*/

void Actuatorsetup() {
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);
  
  digitalWrite(STBY, HIGH); // Wake up motor driver
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
  truePos = (pos - min) * (5.0 / (max - min));
  return truePos;
}
// Moves actuator in a direction at duty cycle
void moveActuator(bool extend) {
  digitalWrite(BIN1, extend ? HIGH : LOW);
  digitalWrite(BIN2, extend ? LOW : HIGH);
  analogWrite(PWMB, moveDuty);  // 20% duty
}
// Stops the actuator
void stopActuator() {
  analogWrite(PWMB, 0);
}

// I need to ask Andrew about position scale and why 280 is the assumed to be reset.
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
      break;  // Exit loop when within range
    }

    delay(50);  // Give time for motor to move
  }
}

void updateLinearActuator() {  
  proportionalZ = KPZ * errorZ; // find proportional term
  
  if(pixy.ccc.numBlocks > 0){
    while(true){
      if (proportionalZ < -5){
        moveActuator(true);
      }
      else if (proportionalZ > 5) {
        moveActuator(false);
      }
      else {
        stopActuator();
        break;
      }
      delay(50);
    }
  }else{
    resetActuator();
  }

}
// Reads and prints position every 100 ms for `duration` milliseconds

