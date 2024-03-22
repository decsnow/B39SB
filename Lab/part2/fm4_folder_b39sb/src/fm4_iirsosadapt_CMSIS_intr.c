// fm4_iirsosadapt_CMSIS_intr.c

#include "fm4_wm8731_init.h"
#include "elliptic.h"

volatile int16_t audio_chR=0;    //16 bits audio data channel right
volatile int16_t audio_chL=0;    //16 bits audio data channel left

float32_t w[NUM_SECTIONS][2] = {0};
#define BLOCK_SIZE 1
#define NUM_TAPS 256

float32_t beta = 1E-12;

float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];

float32_t firCoeffs32[NUM_TAPS] = { 0.0f };

arm_lms_instance_f32 S;

float32_t yn, adapt_in, adaptfir_out, error,input,wn;


void PRGCRC_I2S_IRQHandler(void) 
{
  int16_t section;   // index for section number
  float32_t input;   // input to each section
  float32_t wn,yn;   // intermediate and output values
  union WM8731_data sample;

  sample.uint32bit = i2s_rx();
  gpio_set(DIAGNOSTIC_PIN, HIGH);

    input = (float32_t)(prbs(8000)); 
    adapt_in = input;
    for (section=0 ; section< NUM_SECTIONS ; section++)
    {
      wn = input - a[section][1]*w[section][0]
           - a[section][2]*w[section][1];
      yn = b[section][0]*wn + b[section][1]*w[section][0]
           + b[section][2]*w[section][1];
      w[section][1] = w[section][0];
      w[section][0] = wn;
      input = yn; // output of current section is input to next
    }
// IIR output now in yn
// could choose from a range of signals to output
  sample.uint16bit[LEFT] = (int16_t)(error);
  sample.uint16bit[RIGHT] = (int16_t)(adaptfir_out);
	
    arm_lms_f32(&S, &adapt_in, &yn, &adaptfir_out, &error, BLOCK_SIZE);
		gpio_set(DIAGNOSTIC_PIN, LOW);
  
  i2s_tx(sample.uint32bit);

  NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn); // probably not needed
}

int main(void)
{
  arm_lms_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], beta, BLOCK_SIZE);
  fm4_wm8731_init(FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_INTR, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1){}
}
