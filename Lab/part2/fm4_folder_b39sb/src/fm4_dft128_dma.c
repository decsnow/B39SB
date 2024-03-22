// fm4_dft128_dma.c
//
// #define DMA_BUFFER_SIZE 128 in file fm4_wm8731_init.h

#include "fm4_wm8731_init.h"
#include "math.h"

#define N (DMA_BUFFER_SIZE)
#define TRIGGER 12000
#define MAGNITUDE_SCALING_FACTOR 32

typedef struct
{
  float32_t real;
  float32_t imag;
} COMPLEX;

COMPLEX twiddle[N];
COMPLEX cbuf[N];
int16_t inbuffer[N];
int16_t outbuffer[N];
float32_t magnitude[N];

void dftw(COMPLEX *x, COMPLEX *w)
{
  COMPLEX result[N];
  int k,n;

  for (k=0 ; k<N ; k++)
  {
    result[k].real=0.0;
    result[k].imag = 0.0;

    for (n=0 ; n<N ; n++)
    {
      result[k].real += x[n].real*w[(n*k)%N].real
                      - x[n].imag*w[(n*k)%N].imag;
      result[k].imag += x[n].imag*w[(n*k)%N].real 
                      + x[n].real*w[(n*k)%N].imag;
    }
  }
  for (k=0 ; k<N ; k++)
  {
    x[k] = result[k];
  }
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
	
  
    for(i=0; i<DMA_BUFFER_SIZE ; i++)
    {
	    sample.uint32bit =  rxbuf[i];	
      cbuf[i].real = (float32_t)(sample.uint16bit[LEFT]);
      cbuf[i].imag = 0.0f;
      inbuffer[i] = (int16_t)cbuf[i].real;
    }
   dftw(cbuf,twiddle);
  arm_cmplx_mag_f32((float32_t *)(cbuf),magnitude,DMA_BUFFER_SIZE);
  for(i=0; i<DMA_BUFFER_SIZE ; i++)
  {
 //   sample.uint16bit[LEFT] = (int16_t)(0);
    sample.uint16bit[LEFT] = (int16_t)(magnitude[i]/MAGNITUDE_SCALING_FACTOR);
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

  for (n=0 ; n<N ; n++)
  {
    twiddle[n].real = (float32_t)(cos(2*PI*n/N));
    twiddle[n].imag = (float32_t)(-sin(2*PI*n/N));
  }	
  fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_DMA, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
  {
    while (!(rx_buffer_full && tx_buffer_empty)){}
    gpio_set(DIAGNOSTIC_PIN, HIGH);
    process_dma_buffer();
    gpio_set(DIAGNOSTIC_PIN, LOW);
  }
}
