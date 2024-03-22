// fm4_sine_lut_intr.c with switchable square, dimpulse, prbs

#include "fm4_wm8731_init.h"
#define DIMPULSE_LOOP_SIZE 128
#define SINE_LOOP_SIZE 8
int16_t sine_table[SINE_LOOP_SIZE] = {0, 7071, 10000, 7071, 0, -7071, -10000, -7071};
static int sine_ptr = 0;
#define SQUARE_LOOP_SIZE 32
int16_t square_table[SQUARE_LOOP_SIZE] = {
10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
//10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
//10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
//-10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000,
//-10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000,
-10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000,
-10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000};
static int square_ptr = 0;
static int dimpulse_ptr = 0;

int16_t flag = 0;
int16_t random_value;

static void switch_delay( __IO uint32_t nCount)
{ for (; nCount > 0; nCount--);
}

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  gpio_toggle(DIAGNOSTIC_PIN);

	switch(flag)
	{
		case 0:
      sine_ptr = (sine_ptr+1) % SINE_LOOP_SIZE;
      sample.uint16bit[0] = sine_table[sine_ptr];
      sample.uint16bit[1] = sine_table[sine_ptr];
  FM4_DAC0->DADR =(sample.uint16bit[0]+10000)/8;
      break;
	  case 1:
      square_ptr = (square_ptr+1) % SQUARE_LOOP_SIZE;
      sample.uint16bit[0] = square_table[square_ptr];
      sample.uint16bit[1] = square_table[square_ptr];
  FM4_DAC0->DADR =(sample.uint16bit[0]+10000)/8;
      break;
    case 2:
      dimpulse_ptr = (dimpulse_ptr+1) % DIMPULSE_LOOP_SIZE;			
      if (dimpulse_ptr == 0)
			{
				sample.uint16bit[0] = 20000;
        sample.uint16bit[1] = 20000;
  FM4_DAC0->DADR =(10000)/8;
			}
			else
			{
				sample.uint16bit[0] = 0;
        sample.uint16bit[1] = 0;
  FM4_DAC0->DADR = 0;
			}
      break;
		case 3:
		{
		random_value = prbs(10000);
				sample.uint16bit[0] = random_value;
        sample.uint16bit[1] = random_value;
  FM4_DAC0->DADR =(sample.uint16bit[0]+10000)/8;
			break;
		}
	}
		
// while(bFM4_DAC0_DACR_DRDY == 0){} 
 // FM4_DAC0->DADR =(sample.uint16bit[0]+10000)/8;

  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  audio_init (FS_8000_HZ, WM8731_MIC_IN, IO_METHOD_INTR); 
  bFM4_DAC0_DACR_DAE = 0x01; // enable DAC0
 
  while(1)
	{
	  switch_delay(10000);
    if (!gpio_get(USER_SWITCH))  // if user switch is closed/pressed
		{
			switch_delay(10000);
			flag = (flag+1)%4;
      while (!gpio_get(USER_SWITCH)){} // wait until user switch not pressed
	 }
  }
}

