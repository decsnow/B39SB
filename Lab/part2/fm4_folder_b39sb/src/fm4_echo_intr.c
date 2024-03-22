// fm4_echo_intr.c

#include "fm4_wm8731_init.h"

#define DELAY_BUF_SIZE 16000
#define GAIN 0.7f

int16_t buffer[DELAY_BUF_SIZE];
int16_t buf_ptr = 0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;
  int16_t delayed_sample, output_sample, input_sample;	

  gpio_toggle(DIAGNOSTIC_PIN);
  sample.uint32bit = i2s_rx();
  input_sample = sample.uint16bit[LEFT];
  delayed_sample = buffer[buf_ptr];
  output_sample = input_sample + delayed_sample;
  buffer[buf_ptr] = input_sample + delayed_sample*GAIN;
  buf_ptr = (buf_ptr+1)%DELAY_BUF_SIZE;
  sample.uint16bit[LEFT] = output_sample;
  sample.uint16bit[RIGHT] = output_sample;
  i2s_tx(sample.uint32bit);

  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init (FS_48000_HZ, WM8731_MIC_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
