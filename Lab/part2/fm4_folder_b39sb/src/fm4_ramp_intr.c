// fm4_ramp_intr.c 

#include "fm4_wm8731_init.h"

#define RAMP_INCREMENT 2000
#define RAMP_AMPLITUDE 24000

int16_t output_sample = 0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  sample.uint32bit = i2s_rx();
  output_sample += RAMP_INCREMENT;		   // increment output value
  if (output_sample >= RAMP_AMPLITUDE)   // if peak is reached  
    output_sample = -RAMP_AMPLITUDE;     // then reinitialize

	sample.uint16bit[LEFT] = output_sample;
	sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}


int main(void)
{
  fm4_wm8731_init (FS_8000_HZ, WM8731_MIC_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}

