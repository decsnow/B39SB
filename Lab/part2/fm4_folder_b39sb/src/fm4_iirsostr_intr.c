// fm4_iirsostr_intr.c

#include "fm4_wm8731_init.h"
#include "elliptic.h"

float32_t w[NUM_SECTIONS][2] = {0};

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;
  int16_t section;    // second order section number
  float32_t input;    // input to each section
  float32_t yn;       // intermediate and output values

  gpio_set(DIAGNOSTIC_PIN, HIGH);
  sample.uint32bit = i2s_rx();
  input =(float32_t)(sample.uint16bit[LEFT]);
//  input =(float32_t)(prbs(8000));
  for (section=0 ; section< NUM_SECTIONS ; section++)
  {
    yn = b[section][0]*input + w[section][0];
    w[section][0] = b[section][1]*input + w[section][1] - a[section][1]*yn;
    w[section][1] = b[section][2]*input - a[section][2]*yn;
    input = yn;              // output of current section will be input to next
  }
  sample.uint16bit[LEFT] = (int16_t)(yn);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  gpio_set(DIAGNOSTIC_PIN, LOW);
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
