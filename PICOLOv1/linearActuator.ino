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
  
  resetActuator();
  // 20% duty
  
  moveActuator(false);
  for (int i = 0; i < 50; i++){
    delay(100);
    pos = analogRead(feedbackPin);
    printOLED(String(pos));
  }

  resetActuator();

  printOLED("Actuator test complete.");
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
  pos = analogRead(feedbackPin);
  if(pos > 280){
    actuatorReset = true;
  }
  else{
    actuatorReset = false;
  }
  if (!actuatorReset) {
    moveActuator(true);
    for (int i = 0; i < 10; i++) { 
      delay(500);
      pos = analogRead(feedbackPin);
      if (pos > 280) {
        actuatorReset = true;
        break;  // stop checking early if reset reached!
      }
    }
    stopActuator();
  }
}



errorZ = tiltOffset;

float calculateP(errorZ) {
  proportionalZ = KPZ * errorZ;
  outputZ = proportionalZ;

  previousErrorZ = errorZ;
  return constrain(outputZ, 10, 280);
}

void updateLinearActuator(proportionalZ) {  
  if (controlOutputZ < 0){
    moveActuator(true);
  }
  if (controlOutputZ > 0) {
    moveActuator(false);
  }
  else {
    stopActuator();
  }
}
// Reads and prints position every 100 ms for `duration` milliseconds

