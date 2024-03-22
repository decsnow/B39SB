// fm4_sineDTMF_intr.c

#include "fm4_wm8731_init.h"

#define TABLESIZE 512        // size of look up table
#define SAMPLING_FREQ 16000
#define STEP_770 (float)(770 * TABLESIZE)/SAMPLING_FREQ
#define STEP_1336 (float)(1336 * TABLESIZE)/SAMPLING_FREQ
#define STEP_697 (float)(697 * TABLESIZE)/SAMPLING_FREQ
#define STEP_852 (float)(852 * TABLESIZE)/SAMPLING_FREQ
#define STEP_941 (float)(941 * TABLESIZE)/SAMPLING_FREQ
#define STEP_1209 (float)(1209 * TABLESIZE)/SAMPLING_FREQ
#define STEP_1477 (float)(1477 * TABLESIZE)/SAMPLING_FREQ
#define STEP_1633 (float)(1633 * TABLESIZE)/SAMPLING_FREQ

int16_t sine_table[TABLESIZE];
float32_t loopindexlow = 0.0;
float32_t loopindexhigh = 0.0;

void PRGCRC_I2S_IRQHandler(void) 
{
  union WM8731_data sample;

  gpio_toggle(DIAGNOSTIC_PIN);
  sample.uint32bit = i2s_rx();
  sample.uint16bit[LEFT] = (sine_table[(int16_t)loopindexlow]
                           + sine_table[(int16_t)loopindexhigh]);
  loopindexlow += STEP_770;
  if (loopindexlow > (float)TABLESIZE)
    loopindexlow -= (float)TABLESIZE;
  loopindexhigh += STEP_1477;
  if (loopindexhigh > (float)TABLESIZE)
    loopindexhigh -= (float)TABLESIZE;
	sample.uint16bit[RIGHT] = sample.uint16bit[LEFT];
  i2s_tx(sample.uint32bit);
  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
}

int main(void)
{
  fm4_wm8731_init (FS_8000_HZ, WM8731_MIC_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
