/*
_____________________________________________________________
Code for the entire system of the Pi Pico W.
Code by: Radhakrishna Vojjala
Edited by: Andrew Schmit
Date of last modification: 7 July 2025
_____________________________________________________________
This file contains the Setup and Update functions for the entire system. The code is meant to be called in setup() and loop().
*/

// System wide setup function.

void systemSetup() {
  // pinMode(feedbackPin, INPUT);
  // pinMode(BIN1, OUTPUT);
  // pinMode(BIN2, OUTPUT);
  // pinMode(PWMB, OUTPUT);
  // pinMode(STBY, OUTPUT);
  // digitalWrite(STBY, HIGH); // Wake up motor driver
  Serial.begin(SERIAL_BAUD);
  Serial1.begin(19200);

  Wire.begin(); // default I2C clock
  Wire1.setSCL(I2C_1_SCL);
  Wire1.setSDA(I2C_1_SDA);
  Wire1.begin(); // default I2C clock

  beginOLED(); 

  Serial.println("Initialising....");
  printOLED("Initialising....", true);
  pinMode(ERR_LED_PIN, OUTPUT); // red LED
  pinMode(LOOP_LED_PIN, OUTPUT); // yellow LED
  digitalWrite(ERR_LED_PIN, HIGH);
  delay(100);
  digitalWrite(LOOP_LED_PIN, HIGH);
  delay(100);
  digitalWrite(ERR_LED_PIN, LOW);
  delay(100);
  digitalWrite(LOOP_LED_PIN, LOW);
  pinMode(LED_R, OUTPUT);//right LED
  pinMode(LED_L, OUTPUT);//left LED

  if (bno.begin()){
    Serial.println("BNO Online!");
    printOLED("BNO Online!", true);
  }
  else {
    Serial.println("BNO Offline! Check wiring.");
    printOLED("BNO Offline!\nCheck wiring.", true);
    digitalWrite(ERR_LED_PIN, HIGH);
    delay(500);
    digitalWrite(ERR_LED_PIN, LOW);
  }


  if (MSsetup()){
    Serial.println("MS5611 Online!");
    printOLED("MS5611 Online!", true);
  }
  else {
    Serial.println("MS5611 Offline! Check wiring.");
    printOLED("MS5611 Offline!\nCheck wiring.", true);
    digitalWrite(ERR_LED_PIN, HIGH);
    delay(500);
    digitalWrite(ERR_LED_PIN, LOW);
  }



  /*inStatus = inTher.begin(10);
  outStatus = outTher.begin(10);

  if (inStatus){
    Serial.println("Internal Thermistor Online!");
    printOLED("Internal Thermistor\nOnline!", true);
  }
  else {
    Serial.println("Internal Thermistor Offline. Check wiring.");
    printOLED("Internal Thermistor\nOffline.\nCheck wiring.", true);
    digitalWrite(ERR_LED_PIN, HIGH);
    delay(500);
    digitalWrite(ERR_LED_PIN, LOW);
  }
  
  if (outStatus){
    Serial.println("External Thermistor Online!");
    printOLED("External Thermistor\nOnline!", true);
  }
  else {
    Serial.println("External Thermistor Offline. Check wiring.");
    printOLED("External Thermistor\nOffline.\nCheck wiring.", true);
    digitalWrite(ERR_LED_PIN, HIGH);
    delay(500);
    digitalWrite(ERR_LED_PIN, LOW);
  }
  */
  //GPSsetup();
  /*
     Add setup code for additional sensors here
  */

    // == OPTIONAL =====
    // Can set non-default gain and integration time to
    // adjust for different lighting conditions.
    // ================

  SDsetup(dataFilename, dataFileN1, dataFileN2);
  logData(header, dataFilename);

  loopTime = 1000 / DATA_RATE;

  pinMode(inPin, INPUT);
  pinMode(tiltPin, INPUT);

  Pixysetup();
  Controlwheelsetup();
  delay(1000);
 // Actuatorsetup();
  Serial.println("Setup Finished");
  printOLED("Servo Setup Finished", true);
  Serial.println(header);

}


// System wide update function. Updates all sensors.

void systemUpdate(){

  // updating timers

  nowTimeMS = millis();
  digitalWrite(LOOP_LED_PIN, LOW);
  nowTimeS = nowTimeMS / 1000.0;
  nowTimeMin = nowTimeS / 60;
  freq = 1.0/((nowTimeMS-prevTime)/1000.0);
  HHMMSS = timeToHhmmss(nowTimeMS);

  // updating sensors

  //GPSupdate();
  unsigned long previousTime = millis();
  MSupdate();
  BNOupdate();
  Pixyupdate();

  

  //inTher.update();
  //inTempF = inTher.getTempF();
  //inTempC = inTher.getTempC();
  //outTher.update();
  //outTempF = outTher.getTempF();
  //outTempC = outTher.getTempC();


  //Addtional sensor update code here.
  if(vertVelFt < 0){
    unsigned long currentTime = millis();
    timeFalling += currentTime - previousTime;
    if(timeFalling/1000 >= 10){
      runState = 0;
    }
  }else{
    timeFalling = 0;
  }

  val = digitalRead(6);
  tiltVal = digitalRead(7);
  analogRead(feedbackPin);

  if (val == HIGH && runState){
    Serial.println("mode is now Gyro");
    mode = "Gyro";
    float pidOutput = calculatePID(error);
    angle = pidOutput;
    setServoSpeed(pidOutput);

    if(error < -10){
      digitalWrite(LED_R, HIGH);
      digitalWrite(LED_L, LOW);
    }
    else if(error > 10){
      digitalWrite(LED_L, HIGH);
      digitalWrite(LED_R, LOW);
    }
    else{
      digitalWrite(LED_L, LOW);
      digitalWrite(LED_R, LOW);
    }
    

  } else{
    Serial.println("mode is Idle");
    mode = "Idle"; //sets system mode to idle
    angle = 0; //sets initial servo speed to 0
  }
  
  // if (tiltVal == HIGH) {
  //     Serial.println("linear actuator engaged");
  //     tiltMode = "Tilt";
  //     updateLinearActuator();
  //     // digitalWrite(BIN1, HIGH);
  //     // digitalWrite(BIN2, LOW );
  //     // analogWrite(PWMB, moveDuty);  // 20% duty
  //     // delay(5000);
  //     // digitalWrite(BIN1, LOW);
  //     // digitalWrite(BIN2, HIGH);
  //     // analogWrite(PWMB, moveDuty);  // 20% duty
  //     // delay(5000);
  //   }
  //   else{
  //     tiltMode = "Idle";
  //     // resetActuator();
  //   }

  // Delay for next reading
  delay(BNO055_SAMPLERATE_DELAY_MS); // optional delay that decreases system Hz but reduces gyro drift

}


// Function to convert timer to HHMMSS format 

String timeToHhmmss(int milli) {

  int timerS = milli / 1000;
  int hours = timerS / 3600;
  int hoursRem = timerS % 3600;
  int mins = hoursRem / 60;
  int secs = hoursRem % 60;
  char timeStr[20];
  sprintf(timeStr, "%02d:%02d:%02d", hours, mins, secs);
  return String(timeStr);
}