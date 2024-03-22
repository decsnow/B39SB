// fm4_fft128_CMSIS_dma.c
//
// #define DMA_BUFFER_SIZE 128 in file fm4_wm8731_init.h
// with user-switched Hamming window

#include "fm4_wm8731_init.h"
#include "math.h"
#include "bartlett128.h"
#include "blackman128.h"
#include "hamm128.h"
#include "kaiser128.h"

#define N (DMA_BUFFER_SIZE)
#define TRIGGER 12000
#define MAGNITUDE_SCALING_FACTOR 32

float32_t cbuf[2*N];
float32_t *cbufptr;
float32_t outbuffer[N];
int16_t inbuffer[N];
int16_t window_flag = 0;

static void switch_delay( __IO uint32_t nCount)
{ for (; nCount > 0; nCount--);
}

void process_dma_buffer(void) 
{
  int i;
  uint32_t *txbuf, *rxbuf;
  union WM8731_data sample;

  if(tx_proc_buffer == PING) txbuf = dma_tx_buffer_ping; 
  else txbuf = dma_tx_buffer_pong; 
  if(rx_proc_buffer == PING) rxbuf = dma_rx_buffer_ping; 
  else rxbuf = dma_rx_buffer_pong; 
	
  switch(window_flag)
  {  
		case 0:
			for(i=0; i<DMA_BUFFER_SIZE ; i++)
    {
	    sample.uint32bit =  rxbuf[i];	
      cbuf[i*2] = (float32_t)(sample.uint16bit[LEFT]);
      cbuf[(i*2)+1] = 0.0;
      inbuffer[i] = (int16_t)cbuf[i*2];
    }
    break;
    case 1:
      for(i=0; i<DMA_BUFFER_SIZE ; i++)
    {
	    sample.uint32bit =  rxbuf[i];	
      cbuf[i*2] = (float32_t)(sample.uint16bit[LEFT])*hamming[i];
      cbuf[(i*2)+1] = 0.0;
      inbuffer[i] = (int16_t)cbuf[i*2];
    }
    break;
    case 2:
      for(i=0; i<DMA_BUFFER_SIZE ; i++)
    {
	    sample.uint32bit =  rxbuf[i];	
      cbuf[i*2] = (float32_t)(sample.uint16bit[LEFT])*blackman[i];
      cbuf[(i*2)+1] = 0.0;
      inbuffer[i] = (int16_t)cbuf[i*2];
    }
    break;
    case 3:
      for(i=0; i<DMA_BUFFER_SIZE ; i++)
    {
	    sample.uint32bit =  rxbuf[i];	
      cbuf[i*2] = (float32_t)(sample.uint16bit[LEFT])*kaiser[i];
      cbuf[(i*2)+1] = 0.0;
      inbuffer[i] = (int16_t)cbuf[i*2];
    }
    break;
    case 4:
      for(i=0; i<DMA_BUFFER_SIZE ; i++)
    {
	    sample.uint32bit =  rxbuf[i];	
      cbuf[i*2] = (float32_t)(sample.uint16bit[LEFT])*bartlett[i];
      cbuf[(i*2)+1] = 0.0;
      inbuffer[i] = (int16_t)cbuf[i*2];
    }
    break;
	
	
	
	}
  cbufptr = cbuf;
  arm_cfft_f32(&arm_cfft_sR_f32_len128, (float32_t *)(cbuf), 0, 1);
  arm_cmplx_mag_f32((float32_t *)(cbuf),outbuffer,N);
     for(i=0; i<DMA_BUFFER_SIZE ; i++)
 {
		sample.uint16bit[LEFT] = (int16_t)(outbuffer[i]/MAGNITUDE_SCALING_FACTOR);
//  sample.uint16bit[RIGHT] = 0;
		sample.uint16bit[RIGHT] = inbuffer[i];
    if (i==0) {sample.uint16bit[LEFT] = TRIGGER;}
    txbuf[i] = sample.uint32bit;	
  }
  tx_buffer_empty = 0;
  rx_buffer_full = 0;
}

int main (void)
{
  fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_DMA, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
  {
    while (!(rx_buffer_full && tx_buffer_empty))
		{
      switch_delay(10000);
      if (!gpio_get(USER_SWITCH))  // if user switch is closed/pressed
		  {
			  switch_delay(10000);
			  window_flag = (window_flag+1)%5;
        while (!gpio_get(USER_SWITCH)){} // wait until user switch not pressed
	    }
		}
    gpio_set(DIAGNOSTIC_PIN, HIGH);
    process_dma_buffer();
    gpio_set(DIAGNOSTIC_PIN, LOW);
  }
}
