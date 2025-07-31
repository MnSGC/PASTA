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
  return (float)(pos - min) * (5.0 / (max - min)); // only change that might be meaningful
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
      printOLED("Actuator reset properly"); // aded to make sure the actuator is resettin adn we will know that
      delay(100);
      return;  // Exit loop when within range
    }
    delay(50);  // Give time for motor to move
  }
  // idk if this should be in an if statement for when this should happen
  //if (millis() - currentTime < 200) { needs to be fixed, but the idea is if the while loop above runs for too long the actuator will stop instaed of continuoulsy running.
    stopActuator();
    printOLED("Reset completed"); // adds a stop if the loop fails
  //}
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

