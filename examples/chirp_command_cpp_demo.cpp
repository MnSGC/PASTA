#include "libpixyusb2.h"

#include <signal.h>

#define COLOR_STEP_VALUE 5

Pixy2 pixy;
static bool run_flag = true;

void handle_SIGINT(int unused)
{
   run_flag = false;
}

int main()
{
   signal(SIGINT, handle_SIGINT);

   printf("=============================================================\n");
   printf("= PIXY2 Chirp Command Demo                                  =\n");
   printf("=============================================================\n");

   printf("Connecting to Pixy2...");

   int result = pixy.init();
   if (result < 0) {
      printf("Error\n");
      printf("pixy.init() returned %d\n", result);
      return result;
   }

   printf("Success\n");

   result = pixy.getVersion();
   if (result < 0) {
      printf("pixy.getVersion() returned %d\n", result);
      return result;
   }

   pixy.version->print();
   printf("Hit <CTRL+C> to exit...\n");

   {
      uint8_t Purple_Color = 0xFF;
      uint8_t Green_Color = 0x00;
      bool Green_Increasing = true;
      uint32_t Color;

      // Exit on <CTRL+C> signal //
      while (run_flag) {
         // Set LED using "led_set" chirp command
         //
         // Input is 32 bit unsigned: 00RRGGBB (hex)
         // RR : 8-bit Red component
         // GG : 8-bit Green component
         // BB : 8-bit Blue component
         pixy.m_link.callChirp("led_set", INT32(Color), END_OUT_ARGS, &result, END_IN_ARGS);

         if (result) {
            printf("callChirp() Error: %d\n", result);
            return result;
         }

         // Update color value //
         if (Green_Increasing) {
            // Green is increasing //

            if (Green_Color == 0xFF) {
               // Green is maxed out //
               Green_Increasing = false;
            } else {
               Green_Color += COLOR_STEP_VALUE;
               Purple_Color -= COLOR_STEP_VALUE;
            }
         } else {
            // Purple is increasing //

            if (Purple_Color == 0xFF) {
               // Purple is maxed out //
               Green_Increasing = true;
            } else {
               Green_Color -= COLOR_STEP_VALUE;
               Purple_Color += COLOR_STEP_VALUE;
            }
         }

         // Create 32-bit RGB color value //
         Color = (Purple_Color << 16) + (Green_Color << 8) + Purple_Color;
      }
   }

   printf("PIXY2 Chirp Command Demo Exit\n");
}
