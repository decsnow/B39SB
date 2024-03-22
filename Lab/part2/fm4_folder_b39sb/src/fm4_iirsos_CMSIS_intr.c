// fm4_iirsos_CMSIS_intr.c

#include "fm4_wm8731_init.h"
#include "bp2000.h"

float32_t coeffs[5*NUM_SECTIONS] = {0};
float32_t state[4*NUM_SECTIONS] = {0};

arm_biquad_casd_df1_inst_f32 S;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;
  float32_t xn, yn;   // intermediate and output values

  gpio_set(DIAGNOSTIC_PIN, HIGH);
  sample.uint32bit = i2s_rx();
//  xn =(float32_t)(sample.uint16bit[LEFT]);
  xn =(float32_t)(prbs(8000));
  arm_biquad_cascade_df1_f32(&S, &xn, &yn, 1);
  sample.uint16bit[LEFT] = (int16_t)(yn);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  gpio_set(DIAGNOSTIC_PIN, LOW);
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  int i,k;
  
  k = 0;
  for (i=0; i<NUM_SECTIONS ; i++)
  { 
    coeffs[k++] = b[i][0];
    coeffs[k++] = b[i][1];
    coeffs[k++] = b[i][2];
    coeffs[k++] = -a[i][1];
    coeffs[k++] = -a[i][2];
  }
	arm_biquad_cascade_df1_init_f32(&S, NUM_SECTIONS, coeffs, state);
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
