// fm4_loop_buf_intr.c

#include "fm4_wm8731_init.h"

#define BUFFER_SIZE 256

float32_t rbuffer[BUFFER_SIZE];
int16_t rbufptr = 0;
float32_t lbuffer[BUFFER_SIZE];
int16_t lbufptr = 0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  gpio_toggle(DIAGNOSTIC_PIN);
  sample.uint32bit = i2s_rx();
  lbuffer[lbufptr] = (float32_t)(sample.uint16bit[LEFT]);
  lbufptr = (lbufptr+1) % BUFFER_SIZE;
  rbuffer[rbufptr] = (float32_t)(sample.uint16bit[RIGHT]);
  rbufptr = (rbufptr+1) % BUFFER_SIZE;
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}

