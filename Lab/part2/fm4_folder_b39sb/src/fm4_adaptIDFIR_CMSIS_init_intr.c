// fm4_adaptIDFIR_CMSIS_init_intr.c

#include "fm4_wm8731_init.h"
#include "adaptIDFIR_CMSIS_init_coeffs.h"

#define BETA 5E-13              // adaptive learning rate      
#define NUM_COEFFS 60
#define BLOCK_SIZE 1

float32_t firStateF32[BLOCK_SIZE + NUM_COEFFS -1];
float32_t firCoeffs32[NUM_COEFFS] = {0.0};
arm_lms_instance_f32 S_lms;

float32_t state[N];
arm_fir_instance_f32 S_fir;

float32_t x_lms, y_lms, error;
float32_t x_fir, y_fir;

volatile int16_t out_type = 0; 


static void switch_delay( __IO uint32_t nCount)
{ for (; nCount > 0; nCount--);
}

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  sample.uint32bit = i2s_rx();
  gpio_set(DIAGNOSTIC_PIN, HIGH);
     
			x_fir = (float32_t)(prbs(8000));
	    x_lms = x_fir;
      arm_fir_f32(&S_fir, &x_fir, &y_fir, 1);
 	    arm_lms_f32(&S_lms, &x_lms, &y_fir, &y_lms, &error, 1);
	      
 gpio_set(DIAGNOSTIC_PIN, LOW);
 
      if (out_type == 0)
  		  sample.uint16bit[LEFT] = ((int16_t)(error));  
	    else
 		    sample.uint16bit[LEFT] = ((int16_t)(y_fir));  
			
  
  i2s_tx(sample.uint32bit);

  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn); // probably not needed
}

int main(void)
{
  int16_t i;

	arm_fir_init_f32(&S_fir, N, h, state, 1);
  arm_lms_init_f32(&S_lms, NUM_COEFFS, firCoeffs32, firStateF32, BETA, 1);
  for (i=1; i<60 ; i++)
    firCoeffs32[i] = w[i];
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
	{
    switch_delay(10000);
    if (!gpio_get(USER_SWITCH))  // if user switch is closed/pressed
    {
      switch_delay(10000);
      out_type = (out_type+1)%2;
      switch (out_type)
      {
        case 0:
          gpio_set(LED_R, 0x01);
          break;
        case 1:
          gpio_set(LED_R, 0x00);
          break;
      }
      while (!gpio_get(USER_SWITCH)){} // wait until user switch not pressed
    }
  }
}
