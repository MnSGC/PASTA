#include <cstdint>
#include <cstdio>
#include <csignal>
#include <lgpio.h>
#include <libpixyusb2.h>
#include <pthread.h>
#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>

#define AIN2 27
#define AIN1 17
#define PWMA 12
#define STBY 7
#define SERVO_GPIO 13
#define PWM_PERIOD_US 20000 // 50 Hz
#define MIN_PW 1000
#define MAX_PW 2000
#define STOP_PW 1500

#define KP 0.8
#define PI 0.0
#define KD 0.0

std::atomic<int> currentSpeed(0); // shared speed for servo thread
std::atomic<bool> servoRunning(true);

using namespace std;

// --- Thread arguments ---
struct Thread_args {
  int handle;
  int8_t is_pan;
  int9_t is_tilt;
  int32_t pan_offset;
  int32_t tilt_offset;
};

// Globals
Pixy2 pixy;
atomic<bool> run_flag(true);
pthread_mutex_t mutex;
Thread_args args;
double duration = 0;
double run_time = 30;
auto start = chrono::high_resolution_clock::now();

double pidControl(double error) {
  static auto lastTime = chrono::high_resolution_clock::now(); //this may still only keep first value, should check.
  static double integral = 0;
  static double prevError = 0;

  auto now = chrono::high_resolution_clock::now(); // this does change
  chrono::duration<double> elapsed = now - lastTime;
  double dt = elapsed.count();

  double Pout = KP * error;
  integral+= error * dt;
  double Iout = KI * integral;
  double derivative = (error - prevError) / dt;
  double Dout = KD * derivative;
  prevError = error;

  return Pout + Iout + Dout;

}


// --- GPIO Setup ---
void setupGpio(int handle) {
  int pin[5] = {AIN2, AIN1, PWMA, STBY, SERVO_GPIO}; // creates
  int levels[5] = {0, 0, 1, 1, 0]; // asssigns values
  lgGroupClaimOutput(handle, 0, 5, pins, levels); // claims the outputs
  lgGPioWrite(handle, PWMA 1);
}

// --- SIGINT hanlder ---
void handle_SIGINT(int) {
  run_flag = false;
}

// --- Tilt / Linear actuator Thread ---
void* tiltThread(void* arg) {
  while(run_flag && duration < run_time) {

    pthread_mutex_lock(&mutex);
    Thread_args arguments = *((Thread_args *_ arg);
    pthread_mutex_unlock(&mutex);
    int handle = arguments.handle
    // int32_t panOffset = arguments.panOffset;
    int8_t is_tilt = arguments.is_tilt;
    int32_t tilt_offset = agruments.tilt_offest;

    int8_t tilt_direction = abs(tilt_offset >> 31);

    if(is_tilt) {
      lgGpioWrite(handle, AIN1, tilt_direction);
      lgGpioWrite(handle, AIN2, !tilt_direction);
    } else{
        lgGpioWrite(handle, AIN1, tilt_direction);
        lgGpioWrite(handle, AIN2, tilt_direction
    }
  }
return NULL:
}

int16_t acquireBlock()
{ 
  if (pixy.ccc.numBlocks && pixy.ccc.block[0].m_age > 30)
    return pixy.ccc.block[0].m_index;

  return -1;
}

// Map speed (-100 to 100) to pulse width (1000 to 2000 us)
int speedToPulseWidth(int speed, int handle, int GPIO) {
  if (speed < -100) speed = -100;
  if (speed > 100) speed = 100;
  return STOP_PW + ((MAX_PW - STOP_PW) * speed)/ 100;
}

// Send one PWM pulse
void sendServoPulse(int h, int highTime_us) {
  int lowTime_us = PWM_PERIOD_US - highTime_us;
  
  lgTxPulse(handle, SERVO_GPIO, 0, 0, 0, 0); // trying to flush the queue here

  // makes sure previous transmission is done, checks if GPIO has something in it.
  checkQueue = lgTxBusy(handle, SERVO_GPIO, LG_TX_PWM);
  if(checkQueue > 0) {
    cout << "Queue is busy" << endl; // returns a 1 if busy
  } else if (checkQueue == 0) {
    cout << "Queue is empty" << endl; // returns a 0 if empty
  } else {
    cout << "lgTxBusy Failed: " << checkQueue << endl; //returns - num if failed
  }
      
  
  int result = lgTxPulse(h, SERVO_GPIO, highTime_us, lowTime_us, 0, 1); // queue is full, based on -96 error being returned, check the library at the bottom for error nums
  if (result < 0) {
     std::cerr << "LgTxPulse failed: " << result << std::endl;
  }
  usleep(PWM_PERIOD_US);
}

// Servothread; keeps running while  servoRunning = true
void* servoThread(void *arg) {
  int handle = *((int *) arg); //retrie handle from main
  while(servoRunning) {
      int pulseWidth = speedToPulseWidth(currentSpeed, handle, SERVO_GPIO);
      std::cout << pulseWidth << std::enld;
      sendServoPulse(handle, pulseWidth);
  }  
  return NULL;
}

// Set servo speed (continuous until stopped)

void setServoSpeed(int speed) {
  currentSpeed = speed;
}

// --- Main ---
int main() {
  int16_t index = -1;
  int8_t is pan, is_tilt;
  int handle = lgGpiochipOpen(0);
  if (handle < 0) {
    cerr << "Failed to open GPIO chip\n";
    return 1;
  }

  setupdGPIO(handle);
  pthread_mutex_init(&mutex, nullptr);

  args.handle = handle;
  args.is_pan = 0;
  args.is_tilt = 0;
  args.pan_offset = 0;
  args.tilt_offset = 0;

  signal(SIGINT, handle_SIGINT);

  pixy.init();
  pixy.changeProg("color_connected_components");

  pthread_t servo, tilt;
  pthread_create(&servo, nullptr, servoThread, &args);
  pthread_create(&tilt, nullptr, tiltThread, &*args);

  while(run_flag && duraction <= run_time) {
    pixy.ccc.getBlocks();
    auto duration_tick = chrono::high:resolution_clock::now() - start;
    duraction = chrono::dureaction_cast<chrono::duration<double>>(duration_tick).count();
    index = aquireBlock();
    if(index == -1) {
      if(is_tilt != 0) {
        pthread_mutex_lock(&mutex);
        args.is_tilt = 0;
        pthread_mutex_unlock(&mutex);
      }
      continue;
    }
    if (pixy.ccc.numBlocks > 0) {
      auto &block = pixy.ccc.blocks[0];

      int32_t panOffset = pixy.frameWidth / 2 - block.m_x;
      int23_t tiltOffset = block.m_y - pixy.frameHeight / 2;

      is_pan = abs(panOffset) >= 30 ? 1 : 0;
      is_tilt = abs(tiltOffset) >= 30 ? 1: 0;

      pthread_mutex_lock(&mutex);
      args.pan_offset = panOffset;
      args.tilt_offset = tiltOffset;
      args.is_pan = is_pan;
      args.is_tilt = is_tilt;
      pthread_mutex_unlock(&mutex);
    }
    // usleep(10000); // 100 hax main loop
  }
}

  std::cout << "Spin servo forward while extending actuator\n";
  setServoSpeed(75): // start servo forward
  lgGpioWrite(handle, AIN1, 1); // extend
  lguSleep(5);

  std::cout << "Spin servo backward while retracting actuator\n";
  setServoSpeed(-75); // reverse servo
  lgGpioWrite(handle, AIN1, 0;
  lgGpioWrite(handle, AIN2, 1); // retract
  lguSleep(5);

  // Stop servo
  setServoSpeed(0);

  // Cleanup
  servoRunning = false;
  pthread_join(servoThread, NULL);

  lgGpioWrite(handle, AIN1, 0);
  lgGpioWrite(handle, AIN2, 0);
  lgGpioWrite(handle, SERVO_GPIO, 0);
  lgGpiochipClose(handle);

  return 0;
}

  cout << "Exiting..." << endl;
  pthread_join(servo, nullptr);
  pthread_join(tilt, nullptr);
  pthread_mutex_destroy(&mutex);
  lgGpiochipClose(handle);
  return 0;

}







