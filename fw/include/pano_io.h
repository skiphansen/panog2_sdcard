#ifndef _PANIO_IO_H_
#define _PANIO_IO_H_

#define REG_WR(reg, wr_data)       *((volatile uint32_t *)(reg)) = (wr_data)
#define REG_RD(reg)                *((volatile uint32_t *)(reg))

#define GPIO_BASE             0x94000000
#define ETH_BASE              0x95000000

#define GPIO_BIT_PANO_BUTTON  0x02
#define GPIO_BIT_RED_LED      0x04
#define GPIO_BIT_GREEN_LED    0x08
#define GPIO_BIT_BLUE_LED     0x10
#define GPIO_BIT_CODEC_SDA    0x20
#define GPIO_BIT_CODEC_SCL    0x40

#define GPIO_LED_BITS         (GPIO_BIT_RED_LED  | \
                              GPIO_BIT_GREEN_LED | \
                              GPIO_BIT_BLUE_LED)

#define EXTERNAL_GPIO_SHIFT      8
#define EXTERNAL_GPIO_MASK       0x1fff

// Names from card view, ie SI = input to Card, output from CPU
#define SD_CLK_BIT               (1 << (EXTERNAL_GPIO_SHIFT + 0))
#define SD_SO_BIT                (1 << (EXTERNAL_GPIO_SHIFT + 1))
#define SD_SI_BIT                (1 << (EXTERNAL_GPIO_SHIFT + 2))
#define SD_CS_BIT                (1 << (EXTERNAL_GPIO_SHIFT + 3))
#define SD_D1_BIT                (1 << (EXTERNAL_GPIO_SHIFT + 4))
#define SD_D2_BIT                (1 << (EXTERNAL_GPIO_SHIFT + 5))
#define SD_DET_BIT               (1 << (EXTERNAL_GPIO_SHIFT + 6))

#endif   // _PANIO_IO_H_

