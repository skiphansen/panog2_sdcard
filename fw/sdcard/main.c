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
#include "ff.h"
#include "diskio.h"
#define DEBUG_LOGGING         1
// #define VERBOSE_DEBUG_LOGGING 1
#include "log.h"

bool ButtonJustPressed(void);
int TestSignalCmd(char *CmdLine);
int DirCmd(char *CmdLine);
int DiskInitCmd(char *CmdLine);

const CommandTable_t gCmdTable[] = {
   { "dir", "directory <path>",NULL, DirCmd},
   { "disk_init","Initialize SDCARD interface", NULL, DiskInitCmd},
   { "test_signal","Send binary count to SDCARD GPIO pins", NULL, TestSignalCmd},
   { "?", NULL, NULL, HelpCmd},
   { "help",NULL, NULL, HelpCmd},
   { NULL }  // end of table
};

const char gVerStr[] = "Pano SD Card test program compiled " __DATE__ " " __TIME__ "\r\n";

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

    ALOG_R(gVerStr);
    CmdParserInit(gCmdTable,printf);
    SendPrompt();

    for(; ; ) {
       if(uartlite_haschar()) {
          Char = (char) uartlite_getchar();
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

int TestSignalCmd(char *CmdLine)
{
   uint32_t Temp;
   int i;

   printf("Press any key to exit\n");
// Set external GPIO's to output
   Temp = REG_RD(GPIO_BASE + GPIO_DIRECTION);
   Temp |= EXTERNAL_GPIO_MASK << EXTERNAL_GPIO_SHIFT;
   REG_WR(GPIO_BASE + GPIO_DIRECTION,Temp);
   Temp = REG_RD(GPIO_BASE + GPIO_INPUT);

   do {
      for(i = 0; i <= EXTERNAL_GPIO_MASK; i++) {
         Temp &= ~(EXTERNAL_GPIO_MASK << EXTERNAL_GPIO_SHIFT);
         Temp |= i << EXTERNAL_GPIO_SHIFT;
         REG_WR(GPIO_BASE + GPIO_OUTPUT,Temp);
      }
   } while(!uartlite_haschar());
   uartlite_getchar();

   return RESULT_OK;
}

int DiskInitCmd(char *CmdLine)
{
   DSTATUS Status;

   printf("Press any key to exit\n");
   do {
      Status = disk_initialize(0);
      break;
   } while(Status !=0 && !uartlite_haschar());
   uartlite_getchar();
   LOG("disk_initialize returned %d\n",Status);

   return RESULT_OK;
}

DIR Dir;             /* Directory object */
FILINFO Finfo;

int DirCmd(char *CmdLine)
{
   int res;
   QWORD acc_size = 0;
   DWORD acc_dirs = 0;
   DWORD acc_files = 0;
   DWORD dw;
   FATFS *fs;
   FATFS FatFs;           /* File system object for each logical drive */

   do {
      res = f_mount(&FatFs, "", 1);
      if(res != FR_OK) {
         ELOG("Unable to mount filesystem: %d\n",res);
         break;
      }

      res = f_opendir(&Dir,CmdLine);
      if(res) {
         ELOG("f_opendir failed: %d\n",res);
         break;
      }

      while(true) {
         res = f_readdir(&Dir, &Finfo);
         if(res != FR_OK){
            ELOG("f_readdir failed: %d\n",res);
            break;
         }
         if(!Finfo.fname[0]) {
            break;
         }
         if(Finfo.fattrib & AM_DIR) {
            acc_dirs++;
         } 
         else {
            acc_files++; 
            acc_size += Finfo.fsize;
         }
         printf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9llu  %s\n",
                (Finfo.fattrib & AM_DIR) ? 'D' : '-',
                (Finfo.fattrib & AM_RDO) ? 'R' : '-',
                (Finfo.fattrib & AM_HID) ? 'H' : '-',
                (Finfo.fattrib & AM_SYS) ? 'S' : '-',
                (Finfo.fattrib & AM_ARC) ? 'A' : '-',
                (Finfo.fdate >> 9) + 1980, 
                (Finfo.fdate >> 5) & 15, 
                Finfo.fdate & 31,
                (Finfo.ftime >> 11), 
                (Finfo.ftime >> 5) & 63,
                (QWORD)Finfo.fsize, Finfo.fname);
      }
      if(res != FR_OK){
         break;
      }
      printf("%4u File(s),%10llu bytes total\n%4u Dir(s)", acc_files, acc_size, acc_dirs);
      res = f_getfree(CmdLine, &dw, &fs);
      if(res == FR_OK) {
         printf(", %10llu bytes free\n", (QWORD)dw * fs->csize * 512);
      }
      else {
         ELOG("f_getfree failed: %d\n",res);
         break;
      }
   } while(false);
   return RESULT_OK;
}
