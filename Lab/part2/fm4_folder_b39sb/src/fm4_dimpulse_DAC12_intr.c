// fm4_dimpulse_DAC12_intr.c

#include "fm4_wm8731_init.h"

#define IMPULSE_PERIOD 256
#define IMPULSE_AMPLITUDE 10000
int impulse_count = 0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  gpio_toggle(DIAGNOSTIC_PIN);
  if (impulse_count++ >= IMPULSE_PERIOD)
    {
      impulse_count = 0;
      sample.uint16bit[LEFT] = IMPULSE_AMPLITUDE;
    }
  else
    sample.uint16bit[LEFT] = 0;
         
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];

  FM4_DAC0->DADR =(sample.uint16bit[LEFT]+10000)/8;
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}


int main(void)
{
  fm4_wm8731_init (FS_8000_HZ, WM8731_MIC_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  bFM4_DAC0_DACR_DAE = 0x01; // enable DAC0
  while(1){}
}

