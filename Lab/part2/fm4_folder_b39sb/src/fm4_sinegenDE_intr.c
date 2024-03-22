// fm4_sinegenDE_intr.c

#include "fm4_wm8731_init.h"

float32_t y[3]; // filter states - previous output values
float32_t a1;   // filter coefficient

const float32_t AMPLITUDE = 8000.0;
const float32_t FREQ = 2000.0;
const float32_t SAMPLING_FREQ = 8000.0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;
  int16_t section;    // second order section number

  gpio_set(DIAGNOSTIC_PIN, HIGH);
  sample.uint32bit = i2s_rx();
  y[0] = -(y[1]*a1)-y[2];              // new y(n)
  y[2] = y[1];                         // update y(n-2)
  y[1] = y[0];                         // update y(n-1)
  sample.uint16bit[LEFT] = (int16_t)(y[0]);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  gpio_set(DIAGNOSTIC_PIN, LOW);
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  y[1] = 0.0;
  y[2] = AMPLITUDE*sin(2.0*PI*FREQ/SAMPLING_FREQ);
  a1 = -2.0*cos(2.0*PI*FREQ/SAMPLING_FREQ); 
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
