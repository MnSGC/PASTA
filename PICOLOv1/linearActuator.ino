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
/*
// Normalize heading error to range [-180, 180]
float calculateHeadingErrorZ(float currentZ, float targetZ) {
  float errorZ = targetZ - currentZ;
  while (errorZ > 180) error -= 360;
  while (errorZ < -180) error += 360;
  return errorZ;
}

float calculateP(float errorZ) {
  float proportionalZ = KPZ * errorZ;
 
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - previousTime) / 1000.0;
 
  float outputZ = proportionalZ

  previousErrorZ = errorZ;
  previousTime = currentTime;

  return constrain(outputZ, -255, 255);
}

void updateLinearActuator(float targetHeadingZ) {
  float currentHeadingZ = orientation[2];  
  float errorZ = calculateHeadingErrorZ(currentHeadingZ, targetHeadingZ);
  float pOutput = calculateP(errorZ);
  moveActuator(////// )
*/
// Reads and prints position every 100 ms for `duration` milliseconds

