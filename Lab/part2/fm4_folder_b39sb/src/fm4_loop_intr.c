// fm4_loop_intr.c

#include "fm4_wm8731_init.h"

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  gpio_toggle(DIAGNOSTIC_PIN);
  sample.uint32bit = i2s_rx();
  i2s_tx(sample.uint32bit);

  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init (FS_48000_HZ, WM8731_MIC_IN,
                   IO_METHOD_INTR, 
	                 WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
