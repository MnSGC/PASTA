/*
_____________________________________________________________
Code for the reaction control system controled by a continous rotational servo
Code by: Drew Miler
Modified by: Andrew Schmit
Date of last modification: 14 July 2025
_____________________________________________________________

*/
void Controlwheelsetup() {

  Serial.println("Reaction Control Wheel System Initializing...");
  
  // Initialize reaction wheel servo
  pinMode(SERVO_PIN, OUTPUT);
  myServo.attach(14);
  analogWriteFreq(50);
  analogWriteRange(20000);  // 20ms period (for 50Hz)  // Set PWM frequency to 50 Hz (20ms period)
  delay(1000);  // Allow servo to initialize
  
     printOLED("Starting sweep test...");

    // Full reverse (~1ms pulse → ~5% duty cycle)
    myServo.write(180);;
    digitalWrite(LED_R, HIGH);
    printOLED("Full reverse"); // change back to serial print
    delay(2000);

    // Stop (~1.5ms pulse → ~7.5% duty cycle)
    myServo.write(90);
    digitalWrite(LED_R, LOW);
    printOLED("Stop");
    delay(2000);

    // Full forward (~2ms pulse → ~10% duty cycle)
    myServo.write(0);
    digitalWrite(LED_L, HIGH);
    printOLED("Full forward");
    delay(2000);

    // Stop again
    myServo.write(90);
    digitalWrite(LED_L, LOW);
    printOLED("Stop");
    delay(2000);

    printOLED("Servo test complete.");
    
}

void setServoSpeed(int speed) {
  // speed should be in range -100 to 100 for PID control
  speed = constrain(speed, -MAX_SERVO_SPEED, MAX_SERVO_SPEED);
  int angle = map(speed, -MAX_SERVO_SPEED, MAX_SERVO_SPEED, 180, 0);  // 180 = full reverse, 0 = full forward
  myServo.write(angle);
}

float calculateHeadingError(float current, float target) {
  // Normalize the error to -180 to 180 range
  float error = target - current;
  while (error > 180) error -= 360;
  while (error < -180) error += 360;
  return error;
}

float calculatePID(float error) {
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - previousTime) / 1000.0;

  // Proportional term
  float proportional = KP * error;

  // Integral term (with anti-windup)
  integral += KI * error * deltaTime;
  integral = constrain(integral, -MAX_SERVO_SPEED, MAX_SERVO_SPEED);

  // Derivative term
  float derivative = 0;
  if (deltaTime > 0) {
    derivative = KD * (error - previousError) / deltaTime;
  }

  // Calculate total output
  float output = proportional + integral + derivative;

  // Update variables for next iteration
  previousError = error;
  previousTime = currentTime;

  return constrain(output, -MAX_SERVO_SPEED, MAX_SERVO_SPEED);
}