// fm4_sine48_loop_intr.c

#include "fm4_wm8731_init.h"

#define BUFLENGTH 128
#define LOOPLENGTH 48
float32_t buffer[BUFLENGTH];
int16_t sine_table[LOOPLENGTH] = {0, 1305, 2588, 3827,
  5000, 6088, 7071, 7934, 8660, 9239, 9659, 9914, 10000,
  9914, 9659, 9239, 8660, 7934, 7071, 6088, 5000, 3827,
  2588, 1305, 0, -1305, -2588, -3827, -5000, -6088, -7071,
  -7934, -8660, -9239, -9659, -9914, -10000, -9914, -9659,
  -9239, -8660, -7934, -7071, -6088, -5000, -3827, -2588,
  -1305};
static int sine_ptr = 0;
static int buf_ptr = 0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  gpio_toggle(DIAGNOSTIC_PIN);
  sample.uint16bit[LEFT] = sine_table[sine_ptr];
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  buffer[buf_ptr] = (float32_t)(sample.uint16bit[LEFT]);
  sine_ptr = (sine_ptr+1) % LOOPLENGTH;
  buf_ptr = (buf_ptr+1) % BUFLENGTH;
  FM4_DAC0->DADR =(sample.uint16bit[LEFT]+10000)/8;
  i2s_tx(sample.uint32bit);

  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init (FS_48000_HZ, WM8731_MIC_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  bFM4_DAC0_DACR_DAE = 0x01; // enable DAC0
  while(1){}
}
