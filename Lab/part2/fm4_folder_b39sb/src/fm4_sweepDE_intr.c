// fm4_sweepDE_intr.c

#include "fm4_wm8731_init.h"

#define MIN_FREQ 200
#define MAX_FREQ 3800
#define STEP_FREQ 20
#define SWEEP_PERIOD 400
#define SAMPLING_FREQ 8000.0f
#define AMPLITUDE 4000.0f
#define PI 3.14159265358979f

float32_t y[3] = {0.0, 0.0, 0.0};
float32_t a1;
float32_t freq = MIN_FREQ;
int16_t sweep_count = 0;

void coeff_gen(float freq)
{
float32_t kk;
	
	kk =  2.0*PI*freq/SAMPLING_FREQ;
	a1 = -2.0*arm_cos_f32(kk);
  y[0] = 0.0;
  y[2] = AMPLITUDE*arm_sin_f32(kk);
  y[1] = 0.0;
  return;
}

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  gpio_set(DIAGNOSTIC_PIN, HIGH);
  sample.uint32bit = i2s_rx();
  sweep_count++;
  if (sweep_count >= SWEEP_PERIOD)
  {
    if (freq >= MAX_FREQ) 
      freq = MIN_FREQ;
    else 
      freq += STEP_FREQ;
    coeff_gen(freq);
    sweep_count = 0;
  }
  y[0] = -(y[1]*a1)-y[2];
  y[2] = y[1];                         // update y1(n-2)
  y[1] = y[0];                         // update y1(n-1)
  sample.uint16bit[LEFT] = (int16_t)(y[0]);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  gpio_set(DIAGNOSTIC_PIN, LOW);
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  coeff_gen(freq);
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
