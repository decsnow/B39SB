// fm4_square_intr.c

#include "fm4_wm8731_init.h"

#define LOOPLENGTH 64
int16_t square_table[LOOPLENGTH] = {
10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
-10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000,
-10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000,
-10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000,
-10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000};
static int square_ptr = 0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  gpio_toggle(DIAGNOSTIC_PIN);
  sample.uint16bit[LEFT] = square_table[square_ptr];
  sample.uint16bit[RIGHT] = square_table[square_ptr];
  square_ptr = (square_ptr+1) % LOOPLENGTH;
  i2s_tx(sample.uint32bit);

  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init (FS_8000_HZ, WM8731_MIC_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
