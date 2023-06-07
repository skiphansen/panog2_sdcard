#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "gpio_defs.h"
#include "timer.h"
#include "uart_lite.h"
#include "cmd_parser.h"
#include "pano_io.h"
// #define DEBUG_LOGGING         1
// #define VERBOSE_DEBUG_LOGGING 1
#include "log.h"

#define REG_WR(reg, wr_data)       *((volatile uint32_t *)(reg)) = (wr_data)
#define REG_RD(reg)                *((volatile uint32_t *)(reg))

bool ButtonJustPressed(void);

const CommandTable_t gCmdTable[] = {
   { "?", NULL, NULL, HelpCmd},
   { "help",NULL, NULL, HelpCmd},
   { NULL }  // end of table
};

#define RX_BUF_LEN            80
char gRxBuf[RX_BUF_LEN];
int gRxCount;

void SendPrompt(void);

//-----------------------------------------------------------------
// main
//-----------------------------------------------------------------
int main(int argc, char *argv[])
{
    int i;
    unsigned char Buf[256];
    int Id = 0;
    uint32_t Temp;
    uint32_t Led;
    int Fast = 0;
    char Char;

// Set LED GPIO's to output
    Temp = REG_RD(GPIO_BASE + GPIO_DIRECTION);
    Temp |= GPIO_BIT_RED_LED|GPIO_BIT_GREEN_LED|GPIO_BIT_BLUE_LED;
    REG_WR(GPIO_BASE + GPIO_DIRECTION,Temp);

    Led = GPIO_BIT_RED_LED;

    CmdParserInit(gCmdTable,printf);
    SendPrompt();

    for(; ; ) {
       if(uartlite_haschar()) {
          LOG("Char ready\n");
          Char = (char) uartlite_getchar();
          LOG("Char 0x%x\n",Char);
          if(Char == '\r' || Char == '\n') {
             uartlite_putc('\r');
             uartlite_putc('\n');
             if(gRxCount > 0) {
                ParseCmd(gRxBuf);
                gRxCount = 0;
             }
             SendPrompt();
          }
          else if(Char == '\b') {
             if(gRxCount > 0) {
                gRxBuf[--gRxCount] = 0;
                uartlite_putc(Char);
                uartlite_putc(' ');
                uartlite_putc(Char);
             }
          }
          else if(gRxCount < RX_BUF_LEN - 1) {
             gRxBuf[gRxCount++] = Char;
             gRxBuf[gRxCount] = 0;
             uartlite_putc(Char);
          }
          else {
             uartlite_putc('\a');
          }
       }
       timer_sleep(1);
#if 0
       REG_WR(GPIO_BASE + GPIO_OUTPUT,Led);
       for(i = 0; i < (Fast ? 3 : 10); i++) {
          timer_sleep(50);
          if(ButtonJustPressed()) {
             Fast = !Fast;
             break;
          }
       }
       REG_WR(GPIO_BASE + GPIO_OUTPUT,0);
       for(i = 0; i < (Fast ? 3 : 10); i++) {
          timer_sleep(50);
          if(ButtonJustPressed()) {
             Fast = !Fast;
             break;
          }
       }
       switch(Led) {
          case GPIO_BIT_RED_LED:
             Led = GPIO_BIT_GREEN_LED;
             break;

          case GPIO_BIT_GREEN_LED:
             Led = GPIO_BIT_BLUE_LED;
             break;

          case GPIO_BIT_BLUE_LED:
             Led = GPIO_BIT_RED_LED;
             break;
       }
#endif
    }

    return 0;
}

bool ButtonJustPressed()
{
   static uint32_t ButtonLast = 3;
   uint32_t Temp;
   int Ret = 0;

   Temp = REG_RD(GPIO_BASE + GPIO_INPUT) & GPIO_BIT_PANO_BUTTON;
   if(ButtonLast != 3 && ButtonLast != Temp) {
      if(Temp == 0) {
         printf("Pano button pressed\n");
         Ret = 1;
      }
   }
   ButtonLast = Temp;

   return Ret;
}

void SendPrompt()
{
   printf("sdcard> ");
}

