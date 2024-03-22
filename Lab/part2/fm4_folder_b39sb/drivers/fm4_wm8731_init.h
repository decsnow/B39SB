// fm4_wm8731_init.h
//
// support for Cypress FM4 version of example programs
//

#define ARM_MATH_CM4

#include <s6e2cc.h>
#include "arm_math.h"
#include <stdint.h>
#include "arm_const_structs.h"

#define bFM4_DAC0_DACR_DAE  *((volatile unsigned char*)(0x42660000UL))
#define bFM4_DAC0_DACR_DRDY *((volatile unsigned char*)(0x42660004UL))
#define FM4_DAC0            ((FM4_DAC_TypeDef *)FM4_DAC0_BASE)

// offsets in bit-banded memory for GPIO pins used by example programs
#define LED_B          0x00A0
#define LED_R          0x00A8
#define DIAGNOSTIC_PIN 0x0080
#define LED_G          0x0588
#define USER_SWITCH    0x0100

#define PFR_BASE  (0x42DE0000)
#define PCR_BASE  (0x42DE2000)
#define DDR_BASE  (0x42DE4000)
#define PDIR_BASE (0x42DE6000)
#define PDOR_BASE (0x42DE8000)
#define PZR_BASE  (0x42DEE000)

#define PING 0x00
#define PONG 0x01
#define DMA_BUFFER_SIZE 128 // this is the number of sampling instants

extern uint32_t dma_tx_buffer_pong[DMA_BUFFER_SIZE], 
dma_tx_buffer_ping[DMA_BUFFER_SIZE], 
dma_rx_buffer_ping[DMA_BUFFER_SIZE], 
dma_rx_buffer_pong[DMA_BUFFER_SIZE];
extern uint32_t rx_proc_buffer;
extern uint32_t tx_proc_buffer;
extern volatile int tx_buffer_empty;
extern volatile int rx_buffer_full;

static void DonaldDelay(__IO uint32_t nCount);

void i2c_init(void);

void i2c_write(uint8_t device_address, uint8_t register_address, uint8_t cmd); 

union WM8731_data
{
  int32_t uint32bit;    
  int16_t uint16bit[2];
};

void i2s_init(char fs, char io_method);

void i2s_tx(unsigned int c);

unsigned int i2s_rx(void);

void gpio_toggle(int pin_ofs);

void gpio_set(int pin_ofs, int value);

int gpio_get(int pin_ofs);

void gpio_set_mode(int pin_ofs, int gpio_mode); 

typedef struct stc_dstc_des0
{
  uint32_t DV       : 2u;
  uint32_t ST       : 2u;
  uint32_t MODE     : 1u;
  uint32_t ORL      : 3u;
  uint32_t TW       : 2u;
  uint32_t SAC      : 3u;
  uint32_t DAC      : 3u;
  uint32_t CHRS     : 6u;
  uint32_t DMSET    : 1u;
  uint32_t CHLK     : 1u;
  uint32_t ACK      : 2u;
  uint32_t RESERVED : 2u;
  uint32_t PCHK     : 4u;
} stc_dstc_des0_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor 1 (DES1 - Number of Transfers) in Mode 0
 ******************************************************************************/
typedef struct stc_dstc_des1_mode0
{
  uint32_t IIN      : 16u;
  uint32_t ORM      : 16u;
} stc_dstc_des1_mode0_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor 1 (DES1 - Number of Transfers) in Mode 1
 ******************************************************************************/
typedef struct stc_dstc_des1_mode1
{
  uint32_t IIN      : 8u;
  uint32_t IRM      : 8u;
  uint32_t ORM      : 16u;
} stc_dstc_des1_mode1_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor Group DES0 - DES5
 ******************************************************************************/
typedef struct stc_dstc_des012345
{
  union
  {
    uint32_t        u32DES0;            // Needed for PCHK calculation
    stc_dstc_des0_t DES0;               // Transfer basic setting
  };
    union
  {
    stc_dstc_des1_mode0_t DES1_mode0;   // Number of Transfers in Mode 0
    stc_dstc_des1_mode1_t DES1_mode1;   // Number of Transfers in Mode 1
  };
  uint32_t        DES2;                 // Source Address
  uint32_t        DES3;                 // Destination Address
  union
  {
    stc_dstc_des1_mode0_t DES4_mode0;   // Number of Transfers in Mode 0 for OuterReload (copied in DES1 Mode 0)
    stc_dstc_des1_mode1_t DES4_mode1;   // Number of Transfers in Mode 1 for OuterReload (copied in DES1 Mode 1)
  };
  uint32_t        DES5;                 // Source Address in OuterReload (copied in DES2)
	uint32_t        DES6;                 // Source Address in OuterReload (copied in DES3)
} stc_dstc_des012345_t;

void dstc_init(void);
	 
#define WM8731_I2C_ADDRESS   0x1A    // 0011010

#define WM8731_LINE_IN_LEFT              ((unsigned char)0x00)
#define WM8731_LINE_IN_RIGHT              ((unsigned char)0x01)
#define WM8731_HP_OUT_LEFT             ((unsigned char)0x02)
#define WM8731_HP_OUT_RIGHT             ((unsigned char)0x03)
#define WM8731_ANALOG_PATH               ((unsigned char)0x04)
#define WM8731_DIGITAL_PATH             ((unsigned char)0x05)
#define WM8731_POWER_DOWN            ((unsigned char)0x06)
#define WM8731_INTERFACE            ((unsigned char)0x07)
#define WM8731_SAMPLING_RATE             ((unsigned char)0x08)
#define WM8731_CONTROL              ((unsigned char)0x09)
#define WM8731_RESET                ((unsigned char)0x0F)

#define FS_48000_HZ                 ((unsigned char)0x00)
#define FS_8000_HZ                  ((unsigned char)0x0C)
#define FS_16000_HZ                 ((unsigned char)0x58)
#define FS_24000_HZ                 ((unsigned char)0x40)
#define FS_32000_HZ                 ((unsigned char)0x18)
#define FS_96000_HZ                 ((unsigned char)0x1C)

#define WM8731_MIC_IN               ((unsigned char)0x14)
#define WM8731_MIC_IN_BOOST         ((unsigned char)0x15)
#define WM8731_LINE_IN              ((unsigned char)0x11)

#define IO_METHOD_INTR              ((unsigned char)0x00)
#define IO_METHOD_DMA               ((unsigned char)0x01)

#define WM8731_LINE_IN_GAIN_0_DB    ((unsigned char)0x17)
#define WM8731_LINE_IN_GAIN_3_DB    ((unsigned char)0x19)
#define WM8731_LINE_IN_GAIN_6_DB    ((unsigned char)0x1B)
#define WM8731_LINE_IN_GAIN_9_DB    ((unsigned char)0x1D)
#define WM8731_LINE_IN_GAIN_12_DB   ((unsigned char)0x1F)
#define WM8731_LINE_IN_ATTEN_3_DB   ((unsigned char)0x15)
#define WM8731_LINE_IN_ATTEN_6_DB   ((unsigned char)0x13)
#define WM8731_LINE_IN_ATTEN_9_DB   ((unsigned char)0x11)

#define WM8731_HP_OUT_GAIN_0_DB     ((unsigned char)0x79)
#define WM8731_HP_OUT_GAIN_3_DB     ((unsigned char)0x7C)
#define WM8731_HP_OUT_GAIN_6_DB     ((unsigned char)0x7F)
#define WM8731_HP_OUT_ATTEN_3_DB    ((unsigned char)0x76)
#define WM8731_HP_OUT_ATTEN_6_DB    ((unsigned char)0x73)

#define LEFT  1
#define RIGHT 0

#define HIGH 1
#define LOW  0

int16_t prbs(int16_t noise_level);
int16_t prand(void);
void fm4_wm8731_init(char fs, char select_input, char io_method, char hp_out_gain, char line_in_gain) ;
void Codec_WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue);
