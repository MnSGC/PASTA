                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
// begin license header
//
// This file is part of Pixy CMUcam5 or "Pixy" for short
//
// All Pixy source code is provided under the terms of the
// GNU General Public License v2 (http://www.gnu.org/licenses/gpl-2.0.html).
// Those wishing to use Pixy source code, software and/or
// technologies under different licensing terms should contact us at
// cmucam@cs.cmu.edu. Such licensing terms are available for
// all portions of the Pixy codebase presented here.
// comments/fw4zdi/british_prime_minister_who_is_suffering_from//
// end license header
//

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <signal.h>
#include <lgpio.h>
#include "libpixyusb2.h"
#include <PIDLoop.h>
#include <pthread.h>
#include <fstream>
#include <string>
#include <iostream>
#include <thread>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>
// #include "lg_mcp3008.h"

#define AIN2 27
#define AIN1 17
#define PWMA 12
#define STBY 7

#define STP 23
#define DIR 24
#define ENA 25

using namespace std;
//struct used for creating arguments for the threaded functions
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



// /*
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

void setup (int handle){
  int pins[7] = {AIN2, AIN1, PWMA, STBY, STP, DIR, ENA};
  int levels[7] = {0, 0, 1, 1, 0, 0, 1};
  lgGroupClaimOutput(handle, 0, 7, pins, levels);
  lgGpioWrite(handle, PWMA, 1);
	lgTxPwm(handle, PWMA, 200, 20, 0, 0); // sets pwm cycle but it's not working
}



void handle_SIGINT(int unused)
{
  // On CTRL+C - abort! //

  run_flag = false;
}


int panToStep(int32_t panOffset){
  
  //return static_cast<int>()
  return (int)(panOffset * 1/10.8);
}

void* step (void *args){

  while(run_flag){
    pthread_mutex_lock(&mutex);
    Thread_args arguments =*((Thread_args *) args);
   	pthread_mutex_unlock(&mutex);
    int handle = arguments.handle;
    // int32_t panOffset = arguments.panOffset;
    int8_t is_pan = arguments.is_pan;
    int32_t pan_offset= arguments.pan_offset;
    // int steps = panToStep(panOffset);
    int32_t pan_direction = pan_offset >> 31;
  	lgGpioWrite(handle, DIR, abs(pan_direction));
  	pan_offset = ((pan_direction) + pan_offset) ^ pan_direction;
    if(is_pan){
    	lgGpioWrite(handle, STP, 1);
    	lguSleep(0.1/pan_offset);
    	lgGpioWrite(handle, STP, 0);
    	lguSleep(0.1/pan_offset);
    } 
  }
	return NULL;
}

void * actuate (void *args){
  int v0;
  while(run_flag){
    
    pthread_mutex_lock(&mutex);
    Thread_args arguments =*((Thread_args *) args);
   	pthread_mutex_unlock(&mutex);
    v0 = MCP3008_read_single_ended(arguments.adc, 0);
    int handle = arguments.handle;
    // int32_t panOffset = arguments.panOffset;
    int8_t is_tilt = arguments.is_tilt;
    int32_t tilt_offset = arguments.tilt_offset;

    int8_t tilt_direction = abs(tilt_offset >> 31);
    // printf("v0 = %d\n", v0);
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
// // Take the biggest block (blocks[0]) that's been around for at least 30 frames (1/2 second)
// and return its index, otherwise return -1
int16_t acquireBlock()
{
  if (pixy.ccc.numBlocks && pixy.ccc.blocks[0].m_age>30)
    return pixy.ccc.blocks[0].m_index;

  return -1;
}


// Find the block with the given index.  In other words, find the same object in the current
// frame -- not the biggest object, but he object we've locked onto in acquireBlock()
// If it's not in the current frame, return NULL
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


  void* write_to_csv (void * args) {
    // char buf[64];
    fout.open("pixyData.csv", std::ios::out | std::ios::trunc);
    fout << "H:M:S,block_index,signature,x location,y location,width,height \n";
    std::cout << "create csv file" << std::endl;
    int index;
    while(run_flag){
      timestamp = time(0);
    
      pthread_mutex_lock(&block_mutex);
      Thread_block cur_block = *((Thread_block *) args);
    
      pthread_mutex_unlock(&block_mutex);
      struct tm *loc_time = localtime(&timestamp);
      // std::cout << loc_time->tm_hour << ": " << loc_time->tm_min << ": " << loc_time->tm_sec << std::endl;
      // Were blocks detected? //
      if (cur_block.is_tracking)
      {
          // pthread_mutex_lock(&block_mutex);
          // Block block = pixy.ccc.blocks[0];
          // pthread_mutex_unlock(&block_mutex);
          // std::cout << "writiting" << cur_block.m_signature << std::endl;
          fout << loc_time->tm_hour << ":"<< loc_time->tm_min << ":" << loc_time->tm_sec << "," << 
          cur_block.m_signature << "," << cur_block.m_x << "," 
          << cur_block.m_y << "," << cur_block.m_width << "," << 
          cur_block.m_height << "\n";
          // std::cout << loc_time->tm_hour << ":"<< loc_time->tm_min << ":" << loc_time->tm_sec << "," << 0 << "," << 
          // cur_block.m_signature << "," << cur_block.m_x << "," 
          // << cur_block.m_y << "," << cur_block.m_width << "," << 
          // cur_block.m_height << "\n";
         // fout << loc_time->tm_hour << ":"<< loc_time->tm_min << ":" << loc_time->tm_sec << "," << 0 << "," << "\n"; 
      }else{
        
          fout << loc_time->tm_hour << ":"<< loc_time->tm_min << ":" << loc_time->tm_sec << "\n ";
      }
    }
    fout.close();
    return NULL;
      
  }
  
  
int main()
{  
  // int i=0;
  int16_t index=-1;
  // char buf[64]; 
  int32_t panOffset, tiltOffset;
  Block *block=NULL;
  int8_t is_pan, is_tilt;
  int handle = lgGpiochipOpen(4);
  setup(handle);
  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_init(&block_mutex, NULL);
  args.handle =handle;
  pthread_t stepper;
  pthread_t linear;
  int sbc=-1;
  mcp3008_p adc=NULL;
  // int v0;
  adc = MCP3008_open(sbc, 0, 0, 9600, 0);
  pthread_t csv;
  // pthread_t *pixy_thread;
  
  // // start writing to a csv
  
  // Catch CTRL+C (SIGINT) signals, otherwise the Pixy object
  // won't be cleaned up correctly, leaving Pixy and possibly USB
  // driver in a defunct state.
//   is_tilt = 0;
  
  signal (SIGINT, handle_SIGINT);

  // need to initialize pixy!
  pixy.init();

  // initialize args
  args.is_pan= 0;
  args.pan_offset = 0;
  args.is_tilt = 0;
  args.tilt_offset = 0;  
  args.adc = adc;


  // initialize thread_block
  thread_block.m_signature = 0;
  thread_block.m_height = 0;
  thread_block.m_width = 0;
  thread_block.m_x = 0;
  thread_block.m_y = 0;
  thread_block.is_tracking = 0;
  
  pthread_create(&stepper, NULL, step, (void *) &args);
  pthread_create(&linear,  NULL, actuate, (void *) &args);
  pthread_create(&csv, NULL, write_to_csv, (void *) &thread_block);
  // use ccc program to track objects

  pixy.changeProg("color_connected_components");
  while(run_flag){
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
      pthread_mutex_lock(&block_mutex);
      thread_block.m_height = block->m_height;
      thread_block.m_width = block->m_width;
      thread_block.m_signature = block->m_signature;
      thread_block.m_x = block->m_x;
      thread_block.m_y = block->m_y;
      thread_block.is_tracking = 1;
      pthread_mutex_unlock(&block_mutex);
          // std::cout << 
          // block->m_signature << "," << block->m_x << "," 
          // << block->m_y << "," << block->m_width << "," << 
          // block->m_height << "\n";
      // printf("entered her");
      panOffset = (int32_t)pixy.frameWidth/2 - (int32_t)block->m_x;
      tiltOffset = (int32_t)block->m_y - (int32_t)pixy.frameHeight/2;  
      // v0 = MCP3008_read_single_ended(adc, 0);
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
  printf("exiting...\n");
  pthread_join(stepper, NULL);
  pthread_join(linear, NULL);
  pthread_join(csv, NULL);
  pthread_mutex_destroy(&mutex);
  pthread_mutex_destroy(&block_mutex);
  MCP3008_close(adc);
  lgGpiochipClose(handle);
  return 0;
}

