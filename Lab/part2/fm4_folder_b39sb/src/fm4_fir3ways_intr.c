// fm4_fir3ways_intr.c 

#include "fm4_wm8731_init.h"
#include "bp1750.h"

#define NUM_METHODS 3

float32_t x[2*N];
int16_t k = 0;
int16_t method = 0;

static void switch_delay( __IO uint32_t nCount)
{ for (; nCount > 0; nCount--);
}

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;
  int16_t i;
  float32_t yn = 0.0;

  sample.uint32bit = i2s_rx();
  gpio_set(DIAGNOSTIC_PIN, HIGH);
  switch(method)
  {
    case 0: 
      x[0] = (float32_t)(prbs(8000));
      for (i=0 ; i<N ; i++)	yn += h[i]*x[i];
      for (i=N-1 ; i>0 ; i--)	x[i] = x[i-1];
      break;
    case 1:
      x[k++] = (float32_t)(prbs(8000));
      if (k >= N) k = 0;
      for (i=0 ; i<N ; i++)
      {
        yn += h[i]*x[k++];
        if (k >= N) k = 0;
      }
      break;
    case 2: 
      x[k] = (float32_t)(prbs(8000));
      x[k+N] = x[k];
      k = (k+1)% N;
      for (i=0 ; i<N ; i++)	yn += h[i]*x[k+i];
      break;
  }
  gpio_set(DIAGNOSTIC_PIN, LOW);
  sample.uint16bit[LEFT] = (int16_t)(yn);
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
      method = (method+1) % 3; 
      while (!gpio_get(USER_SWITCH)){} // wait until user switch not pressed
    }
  }
}
