// fm4_fft128_dma.c
//
// #define DMA_BUFFER_SIZE 128 in file fm4_wm8731_init.h
// with user-switched Hamming window

#include "fm4_wm8731_init.h"
#include "math.h"
#include "hamm128.h"

#define N (DMA_BUFFER_SIZE)
#define TRIGGER 12000
#define MAGNITUDE_SCALING_FACTOR 32

typedef struct
{
  float32_t real;
  float32_t imag;
} COMPLEX;

#include "fft.h"

COMPLEX twiddle[N];
COMPLEX cbuf[N];
int16_t inbuffer[N];
int16_t outbuffer[N];
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
	
  if (window_flag == 1)
  {  
    for(i=0; i<DMA_BUFFER_SIZE ; i++)
    {
	    sample.uint32bit =  rxbuf[i];	
      cbuf[i].real = (float)(sample.uint16bit[LEFT])*hamming[i];
      cbuf[i].imag = 0.0f;
      inbuffer[i] = (int16_t)cbuf[i].real;
    }
  }
  else
  {  
    for(i=0; i<DMA_BUFFER_SIZE ; i++)
    {
	    sample.uint32bit =  rxbuf[i];	
      cbuf[i].real = (float)(sample.uint16bit[LEFT]);
      cbuf[i].imag = 0.0f;
      inbuffer[i] = (int16_t)cbuf[i].real;
    }
  }
	fft(cbuf,DMA_BUFFER_SIZE,twiddle);
  for(i=0; i<DMA_BUFFER_SIZE ; i++)
  {
		sample.uint16bit[LEFT] = (int16_t)((sqrt(cbuf[i].real * cbuf[i].real + (cbuf[i].imag * cbuf[i].imag )))/MAGNITUDE_SCALING_FACTOR);
//  sample.uint16bit[RIGHT] = 0;
    outbuffer[i] = 	sample.uint16bit[LEFT];
		sample.uint16bit[RIGHT] = inbuffer[i];
    if (i==0) {sample.uint16bit[LEFT] = TRIGGER;}
    txbuf[i] = sample.uint32bit;	
  }
  tx_buffer_empty = 0;
  rx_buffer_full = 0;
}

int main (void)
{
  int n;

  for (n=0 ; n< N ; n++)
  {
    twiddle[n].real = (float32_t) (cos(PI*n/N));
    twiddle[n].imag = (float32_t) (-sin(PI*n/N));
  }	
  fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_DMA, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
  {
    while (!(rx_buffer_full && tx_buffer_empty))
		{
      switch_delay(10000);
      if (!gpio_get(USER_SWITCH))  // if user switch is closed/pressed
		  {
			  switch_delay(10000);
			  window_flag = (window_flag+1)%2;
        while (!gpio_get(USER_SWITCH)){} // wait until user switch not pressed
	    }
		}
    gpio_set(DIAGNOSTIC_PIN, HIGH);
    process_dma_buffer();
    gpio_set(DIAGNOSTIC_PIN, LOW);
  }
}
