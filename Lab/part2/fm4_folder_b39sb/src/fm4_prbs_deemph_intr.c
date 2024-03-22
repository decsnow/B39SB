// fm4_prbs_deemph_intr.c 

#include "fm4_wm8731_init.h"

int16_t deemph = 0;

static void switch_delay( __IO uint32_t nCount)
{ for (; nCount > 0; nCount--);
}

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  sample.uint32bit = i2s_rx();
  sample.uint16bit[LEFT] = prbs(8000);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
	{
    switch_delay(10000);
    if (!gpio_get(USER_SWITCH))  // if user switch is closed/pressed
    {
      switch_delay(10000);
      deemph = (deemph+1)%2;
      switch (deemph)
      {
        case 0:
          Codec_WriteRegister ( WM8731_DIGITAL_PATH, 0x00); 
          gpio_set(LED_R, 0x01);
          break;
        case 1:
          Codec_WriteRegister ( WM8731_DIGITAL_PATH, 0x06); 
          gpio_set(LED_R, 0x00);
          break;
      }
      while (!gpio_get(USER_SWITCH)){} // wait until user switch not pressed
    }
  }
}
