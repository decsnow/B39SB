// fm4_flanger_dimpulse_intr.c 

#include "fm4_wm8731_init.h"

// sampling rate 48 kHz
#define TS 0.000020833333
 
#define PERIOD 10
#define MEAN_DELAY 0.001f
#define MODULATION_MAG 0.0008f
#define BUFLENGTH 4096
#define ALPHA 0.5f
#define IMPULSE_PERIOD 2048
#define IMPULSE_AMPLITUDE 10000.0f
 
static int16_t i = 0;
float32_t buffer[BUFLENGTH];
float32_t t = 0.0;
float32_t xn,yn,delay_in_seconds;
int16_t j;
int16_t delay_in_samples;
static int16_t impulse_count = 0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  sample.uint32bit = i2s_rx();
  gpio_set(DIAGNOSTIC_PIN, HIGH);
  if (++impulse_count >= IMPULSE_PERIOD)
  {
    impulse_count = 0;
    xn = IMPULSE_AMPLITUDE;
  }
  else
	  xn = 0.0f;
	buffer[i] = xn;
  i = (i+1)%BUFLENGTH;
  t = t + TS;
  delay_in_seconds = MEAN_DELAY + MODULATION_MAG * arm_sin_f32((6.28/PERIOD)*t); 
  delay_in_samples = (uint16_t)(delay_in_seconds * 48000.0);
  j = (i + BUFLENGTH - delay_in_samples) % BUFLENGTH;
  yn = xn + buffer[j]*ALPHA; // change to "-" for HPF, no bass response
  gpio_set(DIAGNOSTIC_PIN, LOW);
  sample.uint16bit[LEFT] = (int16_t)(yn);
  sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];  
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init(FS_48000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}




