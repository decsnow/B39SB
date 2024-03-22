// fm4_fir4types_intr.c 

#include "fm4_wm8731_init.h"
#include "L138_fir4types_coeffs.h"

float32_t x[N];
float32_t h[4][N];

int16_t FIR_number = 0;

static void switch_delay( __IO uint32_t nCount)
{ for (; nCount > 0; nCount--);
}

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;
  int16_t i;
  float32_t yn = 0.0;

  sample.uint32bit = i2s_rx();
  x[0] = (float32_t)(prbs(8000));
//  x[0] = (float32_t)(sample.uint16bit[LEFT]);
  for (i=0 ; i<N ; i++) yn += h[FIR_number][i]*x[i];
  for (i=(N-1) ; i>0 ; i--) x[i] = x[i-1];
  sample.uint16bit[LEFT] = (int16_t)(yn);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  int16_t i;

  for (i=0; i<N; i++)
  {	
    x[i] = 0.0;
    h[0][i] = hlp[i];  	    
    h[1][i] = hhp[i];
    h[2][i] = hbp[i];
    h[3][i] = hbs[i];
  }

  fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
	{
    if (!gpio_get(USER_SWITCH))  // if user switch is closed/pressed
    {
      switch_delay(10000);
      FIR_number = (FIR_number+1) % 4; 
      while (!gpio_get(USER_SWITCH)){} // wait until user switch not pressed
    }
  }
}
