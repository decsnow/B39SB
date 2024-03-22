// fm4_am_intr.c

#include "fm4_wm8731_init.h"

#define LOOPLENGTH 20
#define AMPLITUDE 10
int16_t baseband[LOOPLENGTH] = {1000,951,809,587,309,0,-309,
  	                      -587,-809,-951,-1000,-951,-809,
  	                      -587,-309,0,309,587,809,951};   // 400 Hz
int16_t carrier[LOOPLENGTH] = {1000,0,-1000,0,1000,0,-1000,
  	                     0,1000,0,-1000,0,1000,0,-1000,
  	                     0,1000,0,-1000,0};               // 2 kHz 
int16_t output[LOOPLENGTH];
int16_t k = 0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  gpio_toggle(DIAGNOSTIC_PIN);
  sample.uint32bit = i2s_rx();
  output[k]= carrier[k] + ((baseband[k]*carrier[k])>>12);
  k = (k+1)%LOOPLENGTH;
  sample.uint16bit[LEFT] = (AMPLITUDE*output[k]);
	sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}


int main(void)
{
  fm4_wm8731_init (FS_8000_HZ, WM8731_MIC_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
