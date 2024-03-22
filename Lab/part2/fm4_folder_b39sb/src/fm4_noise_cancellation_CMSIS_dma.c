// fm4_noise_cancellation_CMSIS_dma.c

#include "fm4_wm8731_init.h"
#include "bilinear.h"

#define BLOCK_SIZE 1
#define NUM_TAPS 256

float32_t beta = 2e-11;

float32_t firStateF32[BLOCK_SIZE + NUM_TAPS -1];

float32_t firCoeffs32[NUM_TAPS] = {0.0f};

arm_lms_instance_f32 S;

float32_t input, signoise, wn, yn, yout, error;

float w[NUM_SECTIONS][2] = {0.0f, 0.0f}; // IIR coefficients

void process_dma_buffer(void) 
{
  int i;
  uint32_t *txbuf, *rxbuf;
  float32_t refnoise, signal;
	int16_t section;
  union WM8731_data sample;

  if(tx_proc_buffer == PING) txbuf = dma_tx_buffer_ping; 
  else txbuf = dma_tx_buffer_pong; 
  if(rx_proc_buffer == PING) rxbuf = dma_rx_buffer_ping; 
  else rxbuf = dma_rx_buffer_pong; 
	
  for(i=0; i<DMA_BUFFER_SIZE ; i++)
  {		
    sample.uint32bit =  rxbuf[i];	

		refnoise = (float32_t)(sample.uint16bit[LEFT]);
    signal = (float32_t)(sample.uint16bit[RIGHT]);
		
    input = refnoise;
    for (section=0 ; section<NUM_SECTIONS ; section++)
    {
      wn = input - a[section][1]*w[section][0]
           - a[section][2]*w[section][1];
      yn = b[section][0]*wn + b[section][1]*w[section][0]
           + b[section][2]*w[section][1];
      w[section][1] = w[section][0];
      w[section][0] = wn;
      input = yn;
    }
    signoise = yn + signal;
    arm_lms_f32(&S, &refnoise, &signoise, &yout, &error, 1);
		sample.uint16bit[LEFT] = (int16_t)(error);
    sample.uint16bit[RIGHT] = (int16_t)(error);
		*txbuf++ = sample.uint32bit;
 	} 

  tx_buffer_empty = 0;
  rx_buffer_full = 0;
}

int main (void)
{
	arm_lms_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], beta, 1);
  fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_DMA, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
  {
    while (!(rx_buffer_full && tx_buffer_empty)){}
    gpio_set(DIAGNOSTIC_PIN, HIGH);
    process_dma_buffer();
    gpio_set(DIAGNOSTIC_PIN, LOW);
  }
}
