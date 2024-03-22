// fm4_sysid_CMSIS_dma.c
//
// #define DMA_BUFFER_SIZE 32 in file fm4_wm8731_init.h

#include "fm4_wm8731_init.h"

#define BLOCK_SIZE 1
#define NUM_TAPS 256

float32_t beta = 1E-12;
float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];
float32_t firCoeffs32[NUM_TAPS] = { 0.0f };
arm_lms_instance_f32 S;
float32_t yn, adapt_in, adapt_out, adapt_err,input,wn;

void process_dma_buffer(void) 
{
  int i;
  uint32_t *txbuf, *rxbuf;
  union WM8731_data sample;

  if(tx_proc_buffer == PING) txbuf = dma_tx_buffer_ping; 
  else txbuf = dma_tx_buffer_pong; 
  if(rx_proc_buffer == PING) rxbuf = dma_rx_buffer_ping; 
  else rxbuf = dma_rx_buffer_pong; 
	
  for (i=0 ; i<(DMA_BUFFER_SIZE) ; i++)
  {
    sample.uint32bit = rxbuf[i];
		input = (float32_t)(sample.uint16bit[LEFT]);
    adapt_in = (float32_t)(prbs(8000));
    arm_lms_f32(&S, &adapt_in, &input, &adapt_out, &adapt_err, BLOCK_SIZE);	
    sample.uint16bit[LEFT] = (int16_t)(adapt_in);
    sample.uint16bit[RIGHT] = (int16_t)(adapt_err);
    txbuf[i] = sample.uint32bit;
	} 

  tx_buffer_empty = 0;
  rx_buffer_full = 0;
}

int main (void)
{
  arm_lms_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], beta, BLOCK_SIZE);
  fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_DMA, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
  {
    while (!(rx_buffer_full && tx_buffer_empty)){}
    gpio_set(DIAGNOSTIC_PIN, HIGH);
    process_dma_buffer();
    gpio_set(DIAGNOSTIC_PIN, LOW);
  }
}
