// fm4_sine_intr.c

#include "fm4_wm8731_init.h"

#define SAMPLING_FREQ 8000

float32_t frequency = 2345.0;
float32_t amplitude = 10000.0;
float32_t theta_increment;
float32_t theta = 0.0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  sample.uint32bit = i2s_rx();
  gpio_set(DIAGNOSTIC_PIN, HIGH);
  theta_increment = 2*PI*frequency/SAMPLING_FREQ;
  theta += theta_increment;
  if (theta > 2*PI) theta -= 2*PI;
//	sample.uint16bit[LEFT] = (int16_t)(amplitude*sin(theta));
//  sample.uint16bit[LEFT] = (int16_t)(amplitude*sinf(theta));
  sample.uint16bit[LEFT] = (int16_t)(amplitude*arm_sin_f32(theta));
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  gpio_set(DIAGNOSTIC_PIN, LOW);
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init (FS_8000_HZ, WM8731_MIC_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
