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
#include "sdmm.h"
#define DEBUG_LOGGING         1
// #define VERBOSE_DEBUG_LOGGING 1
#include "log.h"

bool ButtonJustPressed(void);
int TestSignalCmd(char *CmdLine);
int DirCmd(char *CmdLine);
int DumpCmd(char *CmdLine);
uint32_t __attribute__((noinline)) GetSp(void);
int TestWriteCmd(char *CmdLine);
int TypeCmd(char *CmdLine);
int DelCmd(char *CmdLine);
int DiskInitCmd(char *CmdLine);
int CSD_Cmd(char *CmdLine);
int CID_Cmd(char *CmdLine);
int ReadFile(char *Path,bool bDump);

const CommandTable_t gCmdTable[] = {
   { "csd", "dump CSD register", NULL, CSD_Cmd},
   { "cid", "dump CID register", NULL, CID_Cmd},
   { "del", "del <path> - delete a file",NULL, DelCmd},
   { "dir", "directory <path>",NULL, DirCmd},
   { "dump", "dump <path>",NULL, DumpCmd},
   { "disk_init","Initialize SDCARD interface", NULL, DiskInitCmd},
   { "test_write","<path> create a file", NULL, TestWriteCmd},
   { "type","<path> display file on console", NULL, TypeCmd},
   { "?", NULL, NULL, HelpCmd},
   { "help",NULL, NULL, HelpCmd},
   { NULL }  // end of table
};

const char gVerStr[] = "Pano SD Card test program compiled " __DATE__ " " __TIME__ "\r\n";

#define RX_BUF_LEN            80
char gRxBuf[RX_BUF_LEN];
int gRxCount;
bool gDiskInitialized = false;
FATFS gFatFs;

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

    LOG("SP 0x%x\n",GetSp());

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
   bool bMsgSent = false;
   int res;
   int Ret = RESULT_ERR;

   do {
      do {
         if((Status = disk_initialize(0)) == 0) {
            gDiskInitialized = true;
            break;
         }
         if(!bMsgSent) {
            bMsgSent = true;
            printf("Inital call failed, retrying.\n");
            printf("Press any key to give up\n");
         }
      } while(Status != 0 && !uartlite_haschar());

      if(uartlite_haschar()) {
         uartlite_getchar();
      }

      if(bMsgSent) {
         LOG("disk_initialize returned %d\n",Status);
      }

      if(Status != 0) {
         break;
      }
         
      if((res = f_mount(&gFatFs,"", 1)) != FR_OK) {
         ELOG("Unable to mount filesystem: %d\n",res);
         break;
      }
      Ret = RESULT_OK;
   } while(false);

   return Ret;
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

   do {
      if(!gDiskInitialized) {
         DiskInitCmd(NULL);
      }
      if(!gDiskInitialized) {
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

// Calculate CRC7
// It's a 7 bit CRC with polynomial x^7 + x^3 + 1
// input:
//   crcIn - the CRC before (0 for first step)
//   data - byte for CRC calculation
// return: the new CRC7
uint8_t CRC7_one(uint8_t crcIn, uint8_t data) {
   const uint8_t g = 0x89;
   uint8_t i;

   crcIn ^= data;
   for (i = 0; i < 8; i++) {
      if (crcIn & 0x80) crcIn ^= g;
      crcIn <<= 1;
   }

   return crcIn;
}

// Calculate CRC7 value of the buffer
// input:
//   pBuf - pointer to the buffer
//   len - length of the buffer
// return: the CRC7 value
uint8_t CRC7_buf(uint8_t *pBuf, uint8_t len) {
   uint8_t crc = 0;

   while (len--) crc = CRC7_one(crc,*pBuf++);

   return crc;
}

const int LenMask[] = {
   0x001,
   0x003,
   0x007,
   0x00f,
   0x01f,
   0x03f,
   0x07f,
   0x0ff,
   0x1ff,
   0x3ff,
   0x7ff,
   0xfff
};

/* Bit numbers are from 127 (first bit of 16 bytes) to zero (last bit of 16 bytes)
Data is in little endan format and our processor is little endian
 
Example parsing of raw data:
40 0e 00 32 5b 59 00 00 77 5d 7f 80 0a 40 00 a3 
 
  01xx xxxx = version 2             offset 0, shift 6
  xx00 0000 reserved                offset 0, shift 0
  0e TAAC                           offset 1, shift 0
  00 NSAC                           offset 2, shift 0
  32 TRAN_SPEED                     offset 3, shift 0
  5b 5x  0101 1011 0101 <- CCC      offset 4, shift 0
         x1x1 1011 01x1 <- Required for version 2
  x9 READ_BL_LEN                    offset 6, shift 0
 
  xxx0 xxxx READ_BL_PARTIAL         offset 6, shift 4
  xx0x xxxx WRITE_BLK_MISALIGH      offset 6, shift 5
  x0xx xxxx WRITE_BLK_MISALIGH      offset 6, shift 6
  0xxx xxxx READ_BLK_MISALIGH       offset 6, shift 7
  xxxx xxx0 DSR_IMP                 offset 7, shift 0
  x000 000x reserved                offset 7, shift 1
  0xxx 77 5d C_SIZE                 offset 7, shift 7
 
  7f 80 0a 40 00 a3
*/

// This routine is based on UNSTUFF_BITS() from Linux driver.
/*
 *  linux/drivers/mmc/core/sd.c
 *
 *  Copyright (C) 2003-2004 Russell King, All Rights Reserved.
 *  SD support Copyright (C) 2004 Ian Molton, All Rights Reserved.
 *  Copyright (C) 2005-2007 Pierre Ossman, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
int CSD_SLICE(uint32_t *csd,int EndBit,int StartBit)
{
   uint32_t Bits = EndBit - StartBit + 1;
   uint32_t Mask = (Bits < 32 ? 1 << Bits : 0) - 1;
   int Offset = 3 - (StartBit / 32);
   int Shift = StartBit & 31;
   uint32_t Ret = csd[Offset] >> Shift;

   if((Bits + Shift) > 32) {
      Ret |= csd[Offset-1] << ((32 - Shift) % 32);
   }
   Ret &= Mask;

#if 0
   LOG("%d:%d - Bits %d, Shift %d, Offset %d ",EndBit,StartBit,Bits,Shift,Offset);
   LOG("Returning %d,0x%x\n",Ret,Ret);
#endif

   return Ret;
}

typedef struct  {
   const char *Desc;
   uint8_t EndBit;
   uint8_t StartBit;
   uint8_t ParseFlags;
} ParseCSDTable;

#define CSD_FLAG_V1           0x80
#define CSD_FLAG_V2           0x40
#define CSD_FLAG_V3           0x20
#define CSD_FLAG_ALL          (CSD_FLAG_V1 | CSD_FLAG_V2 | CSD_FLAG_V3)
#define CSD_FLAG_V2_V3_FIXED  0x10
#define CSD_FLAG_VER_MASK     (CSD_FLAG_ALL | CSD_FLAG_V2_V3_FIXED)
#define CSD_FLAG_CSIZE        1

const ParseCSDTable gCsdVer1[] = {
   {"TAAC",119,112,CSD_FLAG_V2_V3_FIXED},
   {"NSAC",111,104,CSD_FLAG_V2_V3_FIXED},
   {"TRAN_SPEED",103,96,CSD_FLAG_V2_V3_FIXED},
   {"CCC",95,84,CSD_FLAG_ALL},                // 
   {"READ_BL_LEN",83,80,CSD_FLAG_V2_V3_FIXED},
   {"READ_BL_PARTIAL",79,79,CSD_FLAG_V2_V3_FIXED},
   {"WRITE_BLK_MISALIGH",78,78,CSD_FLAG_V2_V3_FIXED},
   {"READ_BLK_MISALIGH",77,77,CSD_FLAG_V2_V3_FIXED},
   {"DSR_IMP",76,76,CSD_FLAG_ALL},
   {"C_SIZE",73,62,CSD_FLAG_V1 | CSD_FLAG_CSIZE},
   {"C_SIZE",69,48,CSD_FLAG_V2 | CSD_FLAG_CSIZE},
   {"C_SIZE",75,48,CSD_FLAG_V3 | CSD_FLAG_CSIZE},
   {"VDD_R_CURR_MIN",61,59,CSD_FLAG_V1},
   {"VDD_R_CURR_MAX",58,56,CSD_FLAG_V1},
   {"VDD_W_CURR_MIN",55,53,CSD_FLAG_V1},
   {"VDD_W_CURR_MAX",52,50,CSD_FLAG_V1},
   {"C_SIZE_MULT",49,47,CSD_FLAG_V1},
   {"ERASE_BLK_EN",46,46,CSD_FLAG_V2_V3_FIXED},
   {"SECTOR_SIZE",45,39,CSD_FLAG_V2_V3_FIXED},
   {"WP_GRP_SIZE",38,32,CSD_FLAG_V2_V3_FIXED},
   {"WP_GRP_ENABLE",31,31,CSD_FLAG_V2_V3_FIXED},
   {"R2W_FACTOR",28,26,CSD_FLAG_V2_V3_FIXED},
   {"WRITE_BL_LEN",25,22,CSD_FLAG_V2_V3_FIXED},
   {"WRITE_BL_PARTIAL",21,21,CSD_FLAG_V2_V3_FIXED},
   {"FILE_FORMAT_GRP",15,15,CSD_FLAG_V2_V3_FIXED},
   {"COPY",14,14,CSD_FLAG_ALL},
   {"PERM_WRITE_PROTECT",13,13,CSD_FLAG_ALL},
   {"TMP_WRITE_PROTECT",12,12,CSD_FLAG_ALL},
   {"FILE_FORMAT",11,10,CSD_FLAG_V2_V3_FIXED},
   {"WP_UPC",9,9,CSD_FLAG_ALL},
   {"CRC",7,1,CSD_FLAG_ALL},
   {"STOP",0,0,CSD_FLAG_ALL},
   {NULL}   // end of table
};

const ParseCSDTable gCid[] = {
   {"MID",127,120,0},
   {"OID",119,104,0},
   {"PNM",103,64,0},
   {"PRV",63,56,0},
   {"PSN",55,24,0},
   {"MDT",19,8,0},
   {"year",19,12,0},
   {"month",11,8,0},
   {"CRC",7,1,0},
   {"STOP",0,0,0},
   {NULL}   // end of table
};

int CSD_Cmd(char *CmdLine)
{
   BYTE raw_csd[16];
   BYTE csd[16];
   BYTE Err;
   int Line = 0;
   int x;
   const ParseCSDTable *p;
   uint8_t crc;
   int CsdVer;
   uint8_t VerMask;
   uint8_t Flags;
   int64_t C_Size;
   bool bDisplayAll = false;

   if(strcasecmp(CmdLine,"all") == 0) {
      bDisplayAll = true;
      LOG("bDisplayAll\n");
   }

   do {
      if((Err = send_cmd(CMD9,0) != 0)) {
         Line = __LINE__;
         break;
      }
      if((Err = rcvr_datablock(raw_csd,sizeof(raw_csd))) != 1) {
         Line = __LINE__;
         break;
      }
   // Convert raw CSD data to from big endian 32 bit words to little endian
   // (Linux kernal calls be32_to_cpu to do this)
      for(int i = 0; i < 4; i++) {
         for(int j = 0; j < 4; j++) {
            csd[(i * 4) + j] = raw_csd[(i * 4) +3-j];
         }
      }
      LOG_HEX(csd,sizeof(csd));

      x = CSD_SLICE((uint32_t *)csd,125,120);
      if(x != 0) {
      // Invalid, button 6 bits should be zero
         Line = __LINE__;
         break;
      }
      CsdVer = CSD_SLICE((uint32_t *)csd,127,126);
      printf("CSD Version: %d\n",CsdVer + 1);
      VerMask = CSD_FLAG_V1 >> CsdVer;
      p = gCsdVer1;

      while(p->Desc != NULL) {
         Flags = p->ParseFlags & CSD_FLAG_VER_MASK;
         if(Flags & VerMask || (bDisplayAll && Flags == CSD_FLAG_V2_V3_FIXED)) {
            x = CSD_SLICE((uint32_t *)csd,p->EndBit,p->StartBit);
            switch(p->ParseFlags & ~CSD_FLAG_VER_MASK) {
               case 0:
                  printf("%s: %d (0x%x)\n",p->Desc,x,x);
                  break;

               case CSD_FLAG_CSIZE:
                  switch(VerMask) {
                     case CSD_FLAG_V1:
                        printf("%s: %d (0x%x)\n",p->Desc,x,x);
                        break;

                     case CSD_FLAG_V2:
                     case CSD_FLAG_V3:
                        C_Size = ((int64_t) x + 1) * 512;
                        printf("C_SIZE %lld Kbytes, ",C_Size);
                        C_Size /= 1024;
                        printf("%lld megabytes, ",C_Size);
                        C_Size /= 1024;
                        printf("%lld gigabytes\n",C_Size);
                        break;

                     default:
                        ELOG("Internal error\n");
                        break;
                  }
                  break;
            }
         }
         p++;
      }
   } while(false);
   crc = CRC7_buf(csd,sizeof(csd) - 1);
   LOG("CRC 0x%x\n",crc);

   memset(csd,0,sizeof(csd));
   csd[0] = 0x40;
   crc = CRC7_buf(csd,5);
   LOG("CMD0 CRC 0x%x\n",crc);
   csd[0] = 0x41;
   crc = CRC7_buf(csd,5);
   LOG("CMD1 CRC 0x%x\n",crc);

#if 0
   csd[0] = 0x48;
   csd[3] = 1;
   csd[4] = 0xaa;
   crc = CRC7_buf(csd,5);
   LOG("CMD8 CRC 0x%x\n",crc);
#endif

   if(Line != 0) {
      LOG("Failure on line %d, %d\n",Line,Err);
   }
   return RESULT_OK;
}

int CID_Cmd(char *CmdLine)
{
   BYTE raw_cid[16];
   BYTE cid[16];
   BYTE Err;
   int Line = 0;
   int x;
   const ParseCSDTable *p;
   uint8_t crc;
   int CsdVer;
   uint8_t VerMask;
   uint8_t Flags;
   int64_t C_Size;
   bool bDisplayAll = false;

   if(strcasecmp(CmdLine,"all") == 0) {
      bDisplayAll = true;
      LOG("bDisplayAll\n");
   }

   do {
      if((Err = send_cmd(CMD10,0) != 0)) {
         Line = __LINE__;
         break;
      }
      if((Err = rcvr_datablock(raw_cid,sizeof(raw_cid))) != 1) {
         Line = __LINE__;
         break;
      }
   // Convert raw CSD data to from big endian 32 bit words to little endian
   // (Linux kernal calls be32_to_cpu to do this)
      for(int i = 0; i < 4; i++) {
         for(int j = 0; j < 4; j++) {
            cid[(i * 4) + j] = raw_cid[(i * 4) +3-j];
         }
      }
      LOG_HEX(raw_cid,sizeof(raw_cid));
      LOG_HEX(cid,sizeof(cid));

      p = gCid;
      while(p->Desc != NULL) {
         x = CSD_SLICE((uint32_t *)cid,p->EndBit,p->StartBit);
         printf("%s: %d (0x%x)\n",p->Desc,x,x);
         p++;
      }
   } while(false);

   if(Line != 0) {
      LOG("Failure on line %d, %d\n",Line,Err);
   }
   return RESULT_OK;
}

int ReadFile(char *Path,bool bDump)
{
   FRESULT Err;
   UINT bw;
   int Line = 0;
   FIL Fil;
   bool bFileOpen = false;
   uint8_t TestData[64];

   do {
      if(!Path[0]) {
         printf("Usage: test_write <path>\n");
         break;
      }
      if(!gDiskInitialized) {
         LOG("Calling DiskInitCmd\n");
         DiskInitCmd(NULL);
      }

      if(!gDiskInitialized) {
         break;
      }
      if(!bFileOpen) {
         if((Err = f_open(&Fil,Path,FA_READ)) != FR_OK) {
            Line = __LINE__;
            break;
         }
      }
      bFileOpen = true;
      if((Err = f_read(&Fil,TestData,sizeof(TestData),&bw)) != FR_OK) {
         Line = __LINE__;
         break;
      }
      if(bw > sizeof(TestData)) {
         LOG("unexpected read length, requested %d, f_write returned %d\n",
             sizeof(TestData),bw);
         break;
      }
      if(bw == 0) {
         break;
      }
      if(bDump) {
         LOG_HEX(TestData,bw);
      }
      else {
         UINT i;
         char c;

         for(i = 0; i < bw; i++) {
            c = TestData[i];
            if((c >= ' ' && c <= 0x7f) || c == '\n') {
               _putchar(c);
            }
         }
      }
      if(bw < sizeof(TestData)) {
         break;
      }
   } while(true);

   if(bFileOpen) {
      if((Err = f_close(&Fil)) != FR_OK) {
         Line = __LINE__;
      }
   }

   if(Line != 0) {
      LOG("Failure on line %d, %d\n",Line,Err);
   }
   return RESULT_OK;
}

int TypeCmd(char *CmdLine)
{
   return ReadFile(CmdLine,false);
}

int DumpCmd(char *CmdLine)
{
   return ReadFile(CmdLine,true);
}


int TestWriteCmd(char *CmdLine)
{
   FRESULT Err;
   UINT bw;
   int Line = 0;
   FIL Fil;
   const char TestData[] = "This is a test an only a test\r\n";

   do {
      if(!CmdLine[0]) {
         printf("Usage: test_write <path>\n");
         break;
      }
      if(!gDiskInitialized) {
         LOG("Calling DiskInitCmd\n");
         DiskInitCmd(NULL);
      }

      if(!gDiskInitialized) {
         break;
      }
      if((Err = f_open(&Fil,CmdLine,FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK) {
         Line = __LINE__;
         break;
      }
      if((Err = f_write(&Fil,TestData,sizeof(TestData),&bw)) != FR_OK) {
         Line = __LINE__;
         break;
      }
      if(bw != sizeof(TestData)) {
         LOG("unexpected write length, requested %d, f_write returned %d\n",
             sizeof(TestData),bw);
         break;
      }
      if((Err = f_close(&Fil)) != FR_OK) {
         Line = __LINE__;
         break;
      }
   } while(false);

   if(Line != 0) {
      LOG("Failure on line %d, %d\n",Line,Err);
   }
   return RESULT_OK;
}

int DelCmd(char *CmdLine)
{
   FRESULT Err = f_unlink(CmdLine);

   if(Err != FR_OK) {
      LOG("f_unlink failed, %d\n",Err);
   }

   return RESULT_OK;
}

uint32_t __attribute__((noinline)) GetSp()
{
   asm("mv a0, sp");
}


