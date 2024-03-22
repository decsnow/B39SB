// fm4_sweep_intr.c

#include "fm4_wm8731_init.h"

#include "sine8000_table.h"  //one cycle with 8000 points
#define SAMPLING_FREQ 8000.0
#define N 8000
#define START_FREQ 500.0
#define STOP_FREQ 3800.0
#define START_INCR START_FREQ*N/SAMPLING_FREQ
#define STOP_INCR STOP_FREQ*N/SAMPLING_FREQ
#define SWEEPTIME 4
#define DELTA_INCR (STOP_INCR - START_INCR)/(N*SWEEPTIME)
#define AMPLITUDE 10

float32_t float_index = 0.0;
float32_t float_incr = START_INCR;
int16_t int_incr;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  gpio_toggle(DIAGNOSTIC_PIN);
  sample.uint32bit = i2s_rx();
  float_incr += DELTA_INCR;
  if (float_incr > STOP_INCR) float_incr = START_INCR;
  float_index += float_incr;
  if (float_index > N) float_index -= N;
  int_incr = (int16_t)(float_index);	
  sample.uint16bit[LEFT] = ((int16_t)(AMPLITUDE*sine8000[int_incr])); 
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];  
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
