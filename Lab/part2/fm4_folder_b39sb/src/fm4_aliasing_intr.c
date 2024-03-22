// fm4_aliasing_intr.c 

#include "fm4_wm8731_init.h"
#include "lp6545.h"	

#define DISCARD 0
#define SAMPLE 1
#define BLOCKSIZE 1

volatile int16_t flag = DISCARD;
int16_t antialiasing = 1; 
float32_t xin[BLOCKSIZE], yin[BLOCKSIZE];
float32_t xout[BLOCKSIZE], yout[BLOCKSIZE];
float32_t stateout[N+BLOCKSIZE-1];
float32_t statein[N+BLOCKSIZE-1];

arm_fir_instance_f32 Sin, Sout;

static void switch_delay( __IO uint32_t nCount)
{ for (; nCount > 0; nCount--);
}

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  sample.uint32bit = i2s_rx();
  xin[0] = (float32_t)(sample.uint16bit[LEFT]);
  arm_fir_f32(&Sin, xin, yin, BLOCKSIZE);
  if (flag == DISCARD) 
  { 
    flag = SAMPLE; 
    xout[0] = 0.0f;
  }
  else
  { 
    flag = DISCARD;
    if (antialiasing == 0) 
	    xout[0] = yin[0]; 
    else
      xout[0] = (float32_t)(sample.uint16bit[LEFT]);
  }
  arm_fir_f32(&Sout, xout, yout, BLOCKSIZE);
  sample.uint16bit[LEFT] = (int16_t)(yout[0]);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{

  arm_fir_init_f32(&Sin, N, h, statein, BLOCKSIZE);
  arm_fir_init_f32(&Sout, N, h, stateout, BLOCKSIZE);
  fm4_wm8731_init (FS_16000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
	{
    switch_delay(10000);
    if (!gpio_get(USER_SWITCH))  // if user switch is closed/pressed
    {
      switch_delay(10000);
      antialiasing = (antialiasing+1) % 2; 
      while (!gpio_get(USER_SWITCH)){} // wait until user switch not pressed
    }
  }
}
