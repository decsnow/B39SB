// fm4_average_prbs_intr.c

#include "fm4_wm8731_init.h"

#define N 5

float32_t h[N] = {0.08333, 0.25, 0.3333333, 0.25, 0.08333};
float32_t x[N];
int16_t k = 0;

void PRGCRC_I2S_IRQHandler(void) 
{
  float32_t yn = 0.0;
  int i;
  union WM8731_data sample;

  gpio_set(DIAGNOSTIC_PIN, HIGH);
  sample.uint32bit = i2s_rx();
  x[k++] = (float32_t)(prbs(8000));
  if (k >= N) k = 0;
  for (i=0 ; i<N ; i++)
  {
    yn += h[i]*x[k++];
    if (k >= N) k = 0;
  }
  sample.uint16bit[LEFT] = (int16_t)(yn);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  gpio_set(DIAGNOSTIC_PIN, LOW);
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  int i;

  for (i=0 ; i<N ; i++) h[i] = 1.0/N;
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
