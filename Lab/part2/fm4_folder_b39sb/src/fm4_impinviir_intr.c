// fm4_impinviir_intr.c

#include "fm4_wm8731_init.h"
#include "elliptic.h"

float w[NUM_SECTIONS][2] = {0};

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;
  float32_t xn, yn;   

  sample.uint32bit = i2s_rx();
	xn = (float32_t)(sample.uint16bit[LEFT]);

//***********************************************************************
// insert code to compute new output sample here, i.e.
// y(n) = 0.48255x(n-1) + 0.71624315y(n-1) - 0.38791310y(n-2)
// also update stored previous sample values, i.e.
// y(n-2), y(n-1), and x(n-1)
//***********************************************************************

  sample.uint16bit[LEFT] = (int16_t)(yn);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
