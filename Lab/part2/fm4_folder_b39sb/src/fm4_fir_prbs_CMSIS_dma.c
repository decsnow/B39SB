// fm4_fir_prbs_CMSIS_dma.c

#include "fm4_wm8731_init.h"
#include "bp1750.h"

float32_t x[DMA_BUFFER_SIZE], y[DMA_BUFFER_SIZE], state[N+DMA_BUFFER_SIZE-1];

arm_fir_instance_f32 S;

void process_buffer(void) 
{
  int i;
  uint32_t *txbuf, *rxbuf;
  union WM8731_data sample;

  if(tx_proc_buffer == PING) txbuf = dma_tx_buffer_ping; 
  else txbuf = dma_tx_buffer_pong; 
  if(rx_proc_buffer == PING) rxbuf = dma_rx_buffer_ping; 
  else rxbuf = dma_rx_buffer_pong; 
	
  for(i=0; i<DMA_BUFFER_SIZE ; i++)
    x[i] = prbs(8000);
  arm_fir_f32(&S,x,y,DMA_BUFFER_SIZE);
  for(i=0; i<DMA_BUFFER_SIZE ; i++)
  {
    sample.uint16bit[LEFT] = (int16_t)(y[i]);
    sample.uint16bit[RIGHT] = (int16_t)(y[i]);
    txbuf[i] = sample.uint32bit;
  }

  tx_buffer_empty = 0;
  rx_buffer_full = 0;
}

int main (void) 
{
  arm_fir_init_f32(&S, N, h, state, DMA_BUFFER_SIZE);
  fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_DMA, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
  {
    while (!(rx_buffer_full && tx_buffer_empty)){}
    gpio_set(DIAGNOSTIC_PIN, HIGH);
    process_buffer();
    gpio_set(DIAGNOSTIC_PIN, LOW);
  }
}


