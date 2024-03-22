// fm4_delay_intr.c

#include "fm4_wm8731_init.h"

#define DELAY_BUF_SIZE 16000

uint32_t buffer[DELAY_BUF_SIZE];
int16_t buf_ptr = 0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data input_sample, delayed_sample;

  gpio_toggle(DIAGNOSTIC_PIN);
  input_sample.uint32bit = i2s_rx();
  delayed_sample.uint32bit = buffer[buf_ptr];
  buffer[buf_ptr] = input_sample.uint32bit;
  buf_ptr = (buf_ptr+1)%DELAY_BUF_SIZE;
  i2s_tx(delayed_sample.uint32bit);

  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init (FS_48000_HZ, WM8731_MIC_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
