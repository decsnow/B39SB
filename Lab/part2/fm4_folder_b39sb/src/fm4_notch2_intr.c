// fm4_notch2_intr.c 

#include "fm4_wm8731_init.h"
#include "notch2_coeffs.h"

float32_t x[N][2];          // filter delay lines

int16_t out_type = 0;


static void switch_delay( __IO uint32_t nCount)
{ for (; nCount > 0; nCount--);
}

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;
  int16_t i;
  float32_t yn[2];

  sample.uint32bit = i2s_rx();
  gpio_set(DIAGNOSTIC_PIN, HIGH);
//  x[0][0] = (float)(prbs(8000));
  x[0][0] = (float32_t)(sample.uint16bit[LEFT]);
  yn[0] = 0.0;              // compute filter 1 output
  for (i = 0; i< N; i++) yn[0] += h900[i]*x[i][0];
  x[0][1] = (yn[0]);
  yn[1] = 0.0;              // compute filter 2 output
  for (i = 0; i< N; i++) yn[1] += h2700[i]*x[i][1];  
  for (i = N-1; i > 0; i--) // shift delay contents
  {
    x[i][0] = x[i-1][0];
    x[i][1] = x[i-1][1];
  }
  sample.uint16bit[LEFT] = (int16_t)(yn[out_type]);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
	{
    if (!gpio_get(USER_SWITCH))  // if user switch is closed/pressed
    {
      switch_delay(10000);
      out_type = (out_type+1) % 2; 
      while (!gpio_get(USER_SWITCH)){} // wait until user switch not pressed
    }
  }
}
