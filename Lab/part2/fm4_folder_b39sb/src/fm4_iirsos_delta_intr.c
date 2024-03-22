// fm4_iirsos_delta_intr.c

#include "fm4_wm8731_init.h"
#include "elliptic.h"

#define BUFFERSIZE 256
#define AMPLITUDE 60000.0f

float32_t w[NUM_SECTIONS][2] = {0};
float32_t dimpulse[BUFFERSIZE];
float32_t response[BUFFERSIZE];
int16_t indexptr = 0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;
  int16_t section;    // second order section number
  float32_t input;    // input to each section
  float32_t wn, yn;   // intermediate and output values

  gpio_set(DIAGNOSTIC_PIN, HIGH);
  sample.uint32bit = i2s_rx();
  input = dimpulse[indexptr];       // pseudo random input
  for (section=0 ; section< NUM_SECTIONS ; section++)
  {
    wn = input - a[section][1]*w[section][0]
         - a[section][2]*w[section][1];
    yn = b[section][0]*wn + b[section][1]*w[section][0]
         + b[section][2]*w[section][1];
    w[section][1] = w[section][0];
    w[section][0] = wn;
    input = yn; 
  }
  response[indexptr++] = yn;
  if (indexptr >= BUFFERSIZE) indexptr = 0;
  sample.uint16bit[LEFT] = (int16_t)(yn*AMPLITUDE);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  gpio_set(DIAGNOSTIC_PIN, LOW);
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  int i;
  
  for (i=0 ; i< BUFFERSIZE ; i++) dimpulse[i] = 0.0;
  dimpulse[0] = 1.0;
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
