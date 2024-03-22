// fm4_loop_dma.c

#include "fm4_wm8731_init.h"

void process_dma_buffer(void) 
{
  int i;
  uint32_t *txbuf, *rxbuf;

  if(tx_proc_buffer == PING) txbuf = dma_tx_buffer_ping; 
  else txbuf = dma_tx_buffer_pong; 
  if(rx_proc_buffer == PING) rxbuf = dma_rx_buffer_ping; 
  else rxbuf = dma_rx_buffer_pong; 
	
  for(i=0; i<DMA_BUFFER_SIZE ; i++)
  {
    *txbuf++ = *rxbuf++;
  }

  tx_buffer_empty = 0;
  rx_buffer_full = 0;
}

int main (void) 
{
  fm4_wm8731_init (FS_48000_HZ, WM8731_MIC_IN, IO_METHOD_DMA, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
  {
    while (!(rx_buffer_full && tx_buffer_empty)){}
    gpio_set(DIAGNOSTIC_PIN, HIGH);
    process_dma_buffer();
    gpio_set(DIAGNOSTIC_PIN, LOW);
  }
}


