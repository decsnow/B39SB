// fm4_fastconv_dma.c

#include "fm4_wm8731_init.h"
#include "hp55.h"

#define NN (DMA_BUFFER_SIZE)

typedef struct
{
  float real;
  float imag;
} COMPLEX;

#include "fft.h"

COMPLEX twiddle[NN*2];
COMPLEX procbuf[NN*2],coeffs[NN*2];
float32_t overlap[NN];
float32_t a,b;

void process_dma_buffer(void) 
{
  int i;
  uint32_t *txbuf, *rxbuf;
  union WM8731_data sample;

  if(tx_proc_buffer == PING) txbuf = dma_tx_buffer_ping; 
  else txbuf = dma_tx_buffer_pong; 
  if(rx_proc_buffer == PING) rxbuf = dma_rx_buffer_ping; 
  else rxbuf = dma_rx_buffer_pong; 



	  for (i = 0; i < (2*NN) ; i++) 
  {
    procbuf[i].real = 0.0;
    procbuf[i].imag = 0.0;
  }
  for (i = 0; i < NN ; i++) 
  {
    sample.uint32bit = rxbuf[i];
    procbuf[i].real = (float32_t)(sample.uint16bit[LEFT]);
  }  
  fft(procbuf,2*NN,twiddle);
  for (i=0 ; i<(2*NN) ; i++) 
  {                             
    a = procbuf[i].real;
    b = procbuf[i].imag;
    procbuf[i].real = coeffs[i].real*a 
                      - coeffs[i].imag*b;
    procbuf[i].imag = -(coeffs[i].real*b 
                        + coeffs[i].imag*a);
  }
  fft(procbuf,2*NN,twiddle);
  for (i=0 ; i<(2*NN) ; i++)
  {
    procbuf[i].real /= (2*NN);
  }
  for (i = 0; i < NN ; i++) 
  {
    sample.uint16bit[LEFT] = (int16_t)(procbuf[i].real + overlap[i]);
    sample.uint16bit[RIGHT] = 0;
    overlap[i] = procbuf[i+NN].real;
		txbuf[i] = sample.uint32bit;
  }
  tx_buffer_empty = 0;
  rx_buffer_full = 0;
}

int main (void)
{
   int i;

  for (i=0 ; i< (2*NN) ; i++)
  {
    twiddle[i].real = cos(PI*i/(2*NN));
    twiddle[i].imag = -sin(PI*i/(2*NN));
  }	
  for(i=0 ; i<((2*NN)) ; i++) 
  { coeffs[i].real = 0.0; coeffs[i].imag = 0.0;}
  for(i=0 ; i<N ; i++) coeffs[i].real = h[i];
  fft(coeffs,(2*NN),twiddle); 
  fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_DMA, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
  {
    while (!(rx_buffer_full && tx_buffer_empty)){}
    gpio_set(DIAGNOSTIC_PIN, HIGH);
    process_dma_buffer();
    gpio_set(DIAGNOSTIC_PIN, LOW);
  }
}
