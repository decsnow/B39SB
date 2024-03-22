// fm4_sysid_CMSIS_intr.c

#include "fm4_wm8731_init.h"

#define BLOCK_SIZE 1
#define NUM_TAPS 256
float32_t beta = 1E-12;
float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];
float32_t firCoeffs32[NUM_TAPS] = { 0.0f };
arm_lms_instance_f32 S;
float32_t yn, adapt_in, adapt_out, adapt_err,input,wn;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  sample.uint32bit = i2s_rx();
  input = (float32_t)(sample.uint16bit[LEFT]);
  gpio_set(DIAGNOSTIC_PIN, HIGH);
  adapt_in = (float32_t)(prbs(8000));
  arm_lms_f32(&S, &adapt_in, &input, &adapt_out, &adapt_err, BLOCK_SIZE);
  gpio_set(DIAGNOSTIC_PIN, LOW);
  sample.uint16bit[LEFT] = (int16_t)(adapt_in);
  sample.uint16bit[RIGHT] = (int16_t)(adapt_in);
  
  i2s_tx(sample.uint32bit);

  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  arm_lms_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], beta, BLOCK_SIZE);
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
