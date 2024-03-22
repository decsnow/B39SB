// fm4_sine8_buf_intr.c

#include "fm4_wm8731_init.h"
#define LOOPLENGTH 8
int16_t sine_table[LOOPLENGTH] = {0, 7071, 10000, 7071, 0, -7071, -10000, -7071};
static int sine_ptr = 0;

#define BUFFER_SIZE 100
float32_t buffer[BUFFER_SIZE];
static int buf_ptr=0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  gpio_toggle(DIAGNOSTIC_PIN);
  sample.uint16bit[LEFT] = sine_table[sine_ptr];
  sample.uint16bit[RIGHT] = sine_table[sine_ptr];
  sine_ptr = (sine_ptr+1) % LOOPLENGTH;
  buffer[buf_ptr] = sample.uint16bit[LEFT]; 
  buf_ptr = (buf_ptr+1) % BUFFER_SIZE;

  i2s_tx(sample.uint32bit);

  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init (FS_8000_HZ, WM8731_MIC_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
