// fm4_sinegenDTMF_intr.c

#include "fm4_wm8731_init.h"

#define FREQLO 770
#define FREQHI 1336
#define SAMPLING_FREQ 8000
#define AMPLITUDE 6000
#define BUFFER_SIZE 256

float ylo[3];
float yhi[3];
float a1lo, a1hi;
float out_buffer[BUFFER_SIZE];
int bufptr = 0;

void PRGCRC_I2S_IRQHandler(void) 
{
  float32_t output;
  union WM8731_data sample;

  sample.uint32bit = i2s_rx();
  gpio_set(DIAGNOSTIC_PIN, HIGH);
  ylo[0] = -(ylo[1]*a1lo)-ylo[2];
  ylo[2] = ylo[1];                     //update y1(n-2)
  ylo[1] = ylo[0];                     //update y1(n-1)
  yhi[0] = -(yhi[1]*a1hi)-yhi[2];
  yhi[2] = yhi[1];                     //update y1(n-2)
  yhi[1] = yhi[0];                     //update y1(n-1)
  output = (yhi[0]+ylo[0]);
  out_buffer[bufptr++] = output;
  if (bufptr >= BUFFER_SIZE)
    bufptr = 0;
  gpio_set(DIAGNOSTIC_PIN, LOW);
  sample.uint16bit[LEFT] = (int16_t)(output);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  ylo[1] = 0.0;
  ylo[2] = AMPLITUDE*sin(2.0*PI*FREQLO/SAMPLING_FREQ);
  a1lo = -2.0*cos(2.0*PI*FREQLO/SAMPLING_FREQ);
  yhi[1] = 0.0;
  yhi[2] = AMPLITUDE*sin(2.0*PI*FREQHI/SAMPLING_FREQ);
  a1hi = -2.0*cos(2.0*PI*FREQHI/SAMPLING_FREQ);
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
