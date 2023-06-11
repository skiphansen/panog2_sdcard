/*------------------------------------------------------------------------/
/  Foolproof MMCv3/SDv1/SDv2 (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2019, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

#ifndef _SDMM_H_
#define _SDMM_H_
/* MMC/SD command (SPI mode) */
#define CMD0   (0)         /* GO_IDLE_STATE */
#define CMD1   (1)         /* SEND_OP_COND */
#define  ACMD41   (0x80+41)   /* SEND_OP_COND (SDC) */
#define CMD8   (8)         /* SEND_IF_COND */
#define CMD9   (9)         /* SEND_CSD */
#define CMD10  (10)     /* SEND_CID */
#define CMD12  (12)     /* STOP_TRANSMISSION */
#define CMD13  (13)     /* SEND_STATUS */
#define ACMD13 (0x80+13)   /* SD_STATUS (SDC) */
#define CMD16  (16)     /* SET_BLOCKLEN */
#define CMD17  (17)     /* READ_SINGLE_BLOCK */
#define CMD18  (18)     /* READ_MULTIPLE_BLOCK */
#define CMD23  (23)     /* SET_BLOCK_COUNT */
#define  ACMD23   (0x80+23)   /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)     /* WRITE_BLOCK */
#define CMD25  (25)     /* WRITE_MULTIPLE_BLOCK */
#define CMD32  (32)     /* ERASE_ER_BLK_START */
#define CMD33  (33)     /* ERASE_ER_BLK_END */
#define CMD38  (38)     /* ERASE */
#define CMD55  (55)     /* APP_CMD */
#define CMD58  (58)     /* READ_OCR */


BYTE send_cmd(BYTE cmd,DWORD arg);
int rcvr_datablock(BYTE *buff,UINT btr);
#endif   // _SDMM_H_

