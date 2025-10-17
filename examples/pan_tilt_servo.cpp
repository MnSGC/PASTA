#include <atomic>
#include <bits/chrono.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <signal.h>
#include <lgpio.h>
#include "libpixyusb2.h"
#include <PIDLoop.h>
#include <pthread.h>
#include <fstream>
#include <stdexcept>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <thread>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <chrono>

// definition of pins
#define AIN2 27
#define AIN1 17
#define PWMA 12
#define STBY 7

#define SERVO_GPIO 13                   
//

//definition of servo constants
#define PWM_PERIOD_US 20000   // 20 ms = 50 Hz PWM period
#define MIN_PW 1000           // full reverse
#define MAX_PW 2000           // full forward
#define STOP_PW 1500          // stop
//

std::atomic<int> currentSpeed(0);      // shared speed for servo thread
std::atomic<bool> servoRunning(true);  // thread control flag


// for the adc
typedef struct mcp3008_s
{
   int sbc;     // sbc connection
   int device;  // SPI device
   int channel; // SPI channel
   int speed;   // SPI bps
   int flags;   // SPI flags
   int spih;    // SPI handle
   callbk_t enable;
} mcp3008_t, *mcp3008_p;


// arguments for the threaded functions
struct Thread_args{
  int handle;
  int8_t is_pan;
  int8_t is_tilt;
  int32_t pan_offset; // 1 for cw, 0 for ccw
  int32_t tilt_offset;
  mcp3008_p adc;
};


struct Thread_block{
  uint16_t m_signature;
  uint16_t m_x;
  uint16_t m_y;
  uint16_t m_width;
  uint16_t m_height;
  uint8_t is_tracking;
};

auto start = std::chrono::high_resolution_clock::now();
double duration = 0;
double run_time = 30;
time_t timestamp;
pthread_mutex_t mutex;
pthread_mutex_t block_mutex;
Pixy2 pixy;
// PIDLoop panLoop(400, 0, 400, true);
// PIDLoop tiltLoop(500, 0, 500, true);
static bool  run_flag = true;
std::fstream fout;

Thread_args args;
Thread_block thread_block;


void handle_SIGINT(int unused)
{
  // On CTRL+C - abort! //

  run_flag = false;
}

mcp3008_p MCP3008_open(int sbc, int device, int channel, int speed, int flags)
{
   mcp3008_p s;

   s = (mcp3008_p) calloc(1, sizeof(mcp3008_t));

   if (s == NULL) {
      printf("calloc failed");
      return NULL;
   }

   s->sbc = sbc;         // sbc connection
   s->device = device;   // SPI device
   s->channel = channel; // SPI channel
   s->speed = speed;     // SPI speed
   s->flags = flags;     // SPI flags

   s->spih = lgSpiOpen(device, channel, speed, flags);

   if (s->spih < 0)
   {
      printf("lgSpiOpen failed\n");
      free(s);
      s = NULL;
   }

   return s;
}

mcp3008_p MCP3008_close(mcp3008_p s)
{
   if (s != NULL)
   {
      lgSpiClose(s->spih);
      free(s);
      s = NULL;
   }
   return s;
}

int MCP3008_read_single_ended(mcp3008_p s, int channel)
{
   int value;
   char buf[16];

   if (s == NULL) return -1;

   if ((channel < 0) || (channel > 7)) return -2;

   if (s->enable != NULL) s->enable();

   buf[0] = 1;
   buf[1] = 0x80 + (channel<<4);
   buf[2] = 0;

   lgSpiXfer(s->spih, buf, buf, 3);

   if (s->enable != NULL) s->enable();

   value = ((buf[1]&0x03)<<8) + buf[2];

   return value;
}

int MCP3008_set_enable(mcp3008_p s, callbk_t enable)
{
   s->enable = enable;

   return 0;
}



void setup (int handle){
    int pins[4] = {AIN2, AIN1, PWMA, STBY}; //Oct. 10th pins[5] -> pins[4] 
    int levels[4] = {0, 0, 1, 1};
    lgGroupClaimOutput(handle, 0, 4, pins, levels);
    lgGpioWrite(handle, PWMA, 1);
    lgTxPwm(handle, PWMA, 500, 50, 0, 0);
    if (lgGpioClaimOutput(handle, 0, SERVO_GPIO, 0) != 0) { //claims pin 13
        std::cerr << "Failed to claim SERVO_GPIO " << SERVO_GPIO << std::endl;
    }
 }



// Map speed (-100 to 100) to pulse width (1000 to 2000 µs)
int speedToPulseWidth(int speed) {
    if (speed < -100) speed = -100;
    if (speed > 100) speed = 100;
    return STOP_PW + ((MAX_PW - STOP_PW) * speed) / 100; // returns num between -2500 & 2500
}

// Set servo speed (continuous until stopped)
void setServoSpeed(int speed) {
    currentSpeed = speed; 
}
// Send one PWM pulse
void sendServoPulse(int h, int highTime_us) { // hightime_us is hte pulsewidth
    int lowTime_us = PWM_PERIOD_US - highTime_us;
    int result = lgTxPulse(h, SERVO_GPIO, highTime_us, lowTime_us, 0, 1); 
    if (result < 0) {
        std::cerr << "lgTxPulse failed: " << result << std::endl;
    }
    usleep(PWM_PERIOD_US);
}

// Servo thread: keeps running while servoRunning = true
void* servoThreadFunc(void *args) {
    pthread_mutex_lock(&mutex);
    Thread_args arguments =*((Thread_args *) args);//creates copy of the threaded argument
   	pthread_mutex_unlock(&mutex);
   	
    int handle = arguments.handle;
    int8_t is_pan = arguments.is_pan;
    int32_t pan_offset = arguments.pan_offset;
    int32_t tilt_offset = arguments.tilt_offset;
    while (run_flag && duration < run_time) {
        
        int speed = is_pan == 1 ? pan_offset : 0;
        setServoSpeed(speed);
        std::cout << "speed: " << speed << std::endl;
        std::cout <<"pan_ofset: " << pan_offset << std::endl;
        std::cout <<"tilt_offset: " << tilt_offset << std::endl;
        int pulseWidth = speedToPulseWidth(currentSpeed); //converts new speed using function, creates pulseWidth value
        sendServoPulse(handle, pulseWidth); 
    }
    return NULL;
}


//Threaded function for moving the linear actuator
void * actuate (void *args){
  int v0;
  while(run_flag && duration < run_time){//runs for a given set of time or if ctrl+c is pressed
    
    //block all threads to prevent race condition
    pthread_mutex_lock(&mutex);
    Thread_args arguments =*((Thread_args *) args);//creates copy of the threaded argument
   	pthread_mutex_unlock(&mutex);
   	
    //reads the position of the linear actuator
    v0 = MCP3008_read_single_ended(arguments.adc, 0);

    int handle = arguments.handle;
    int8_t is_tilt = arguments.is_tilt;
    int32_t tilt_offset = arguments.tilt_offset;

    int8_t tilt_direction = abs(tilt_offset >> 31);
    // printf("v0 = %d\n", v0);

    //run the actuator as long as is_tilt is true.
    // is_tilt will be true as long as the object being tracked
    // is not in the center of frame.
   	if(is_tilt){
   	  // printf("%d\n", !tilt_direction);
      lgGpioWrite(handle, AIN1, tilt_direction);
      lgGpioWrite(handle, AIN2, !tilt_direction);
   	}else{
   	  
      lgGpioWrite(handle, AIN1, tilt_direction);
      lgGpioWrite(handle, AIN2, tilt_direction);  	 
   	}
  }
  return NULL;
}
int16_t acquireBlock()
{
  if (pixy.ccc.numBlocks && pixy.ccc.blocks[0].m_age>30)
    return pixy.ccc.blocks[0].m_index;

  return -1;
}

Block *trackBlock(uint8_t index)
{
  uint8_t i;

  for (i=0; i<pixy.ccc.numBlocks; i++)
  {
    if (index==pixy.ccc.blocks[i].m_index)
      return &pixy.ccc.blocks[i];
  }

  return NULL;
}
int main() {
    // Open the GPIO chip
    int32_t panOffset, tiltOffset;
    Block *block = NULL;
    int16_t index = -1;
    int8_t is_pan, is_tilt;
    int handle = lgGpiochipOpen(0);
    if (handle < 0) {
        std::cerr << "Failed to open GPIO chip\n";
        return 1;
    }


    //setting up the analog to digital converter 
    int sbc=-1;
    mcp3008_p adc=NULL;
    // int v0;
    adc = MCP3008_open(sbc, 0, 0, 9600, 0);
  
    //setting the shared variable
    args.handle = handle;
    args.is_pan= 0;
    args.pan_offset = 0;
    args.is_tilt = 0;
    args.tilt_offset = 0;  
    args.adc = adc;

    //setup handle_SIGINT as a signal handler
    signal(SIGINT, handle_SIGINT);
    
    
    //setup the mutually exclusive variables
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&block_mutex, NULL);
    setup(handle);
    
    
    // Start threads
    pthread_t servo_thread;
    pthread_t actuator_thread;
    pthread_create(&servo_thread, NULL, servoThreadFunc, (void *) (&args));
    pthread_create(&actuator_thread, NULL, actuate, (void *) (&args));

    //initialize the pixy camera
    pixy.init();
    
    while(run_flag && duration < run_time){
      auto duration_tick = std::chrono::high_resolution_clock::now() - start;
      duration = std::chrono::duration_cast<std::chrono::duration<double>>(duration_tick).count();
        pixy.ccc.getBlocks();

        index=acquireBlock();
        if(index==-1){
          if(is_tilt != 0){
        
            pthread_mutex_lock(&mutex);
            args.is_tilt = 0;
            pthread_mutex_unlock(&mutex);
          }
          continue;
      
        }
        block = trackBlock(index);

        if(block){
          // pthread_mutex_lock(&block_mutex);
          // thread_block.m_height = block->m_height;
          // thread_block.m_width = block->m_width;
          // thread_block.m_signature = block->m_signature;
          // thread_block.m_x = block->m_x;
          // thread_block.m_y = block->m_y;
          // thread_block.is_tracking = 1;
          // pthread_mutex_unlock(&block_mutex);
              // std::cout << 
              // block->m_signature << "," << block->m_x << "," 
              // << block->m_y << "," << block->m_width << "," << 
              // block->m_height << "\n";
          // printf("entered her");
          panOffset = (int32_t)pixy.frameWidth/2 - (int32_t)block->m_x;
          tiltOffset = (int32_t)block->m_y - (int32_t)pixy.frameHeight/2;  
          // v0 = MCP3008_read_singlauto e_ended(_tickadc, 0);
          // printf("adc report: %d\n", v0);
          is_pan = 1;
          is_tilt = 1;
          if(abs(panOffset) < 30){
            is_pan = 0;
          }
          if(abs(tiltOffset) < 30){
            is_tilt = 0;
          }
          pthread_mutex_lock(&mutex);
          args.is_pan= is_pan;
          args.is_tilt= is_tilt;
          args.pan_offset = panOffset;
          args.tilt_offset = tiltOffset;
          pthread_mutex_unlock(&mutex);

        }else {
          pthread_mutex_lock(&block_mutex);
          thread_block.is_tracking = 0;
          pthread_mutex_unlock(&block_mutex);
        }
      
    }


    // lgGpioWrite(handle, AIN1, 1);
    // lguSleep(3);
    // lgGpioWrite(handle, AIN1, 0);
    // lguSleep(1);
    // lgGpioWrite(handle, AIN2, 1);
    // lguSleep(3);
    // lgGpioWrite(handle, AIN2, 0);
    // lguSleep(1);
  
  
    //uncommented out the below on oct. 10th 2025
    // ==== Example motion sequence ====
    // std::cout << "Spin servo forward while extending actuator\n";
    // setServoSpeed(75);  // start servo forward
    // lgGpioWrite(handle, AIN1, 1); // extend
    // lguSleep(5);

    // std::cout << "Spin servo backward while retracting actuator\n";
    // setServoSpeed(-75); // reverse servo
    // lgGpioWrite(handle, AIN1, 0);
    // lgGpioWrite(handle, AIN2, 1); // retract
    // lguSleep(5);
    
    
    // Stop servo
    setServoSpeed(0);

    // Cleanup
    servoRunning = false;
    pthread_join(servo_thread, NULL);
    pthread_join(actuator_thread, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&block_mutex);

    // lgGpioWrite(handle, AIN1, 0);
    // lgGpioWrite(handle, AIN2, 0);
    // lgGpioWrite(handle, SERVO_GPIO, 0);
    lgGpiochipClose(handle);

    return 0;
}
