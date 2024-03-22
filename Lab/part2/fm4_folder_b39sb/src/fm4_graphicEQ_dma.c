// fm4_graphicEQ_dma.c

#include "fm4_wm8731_init.h"
#include "GraphicEQcoeff.h"

#define NN (DMA_BUFFER_SIZE)

typedef struct
{
  float real;
  float imag;
} COMPLEX;

#include "fft.h"

COMPLEX procbuf[2*NN],coeffs[2*NN],twiddle[2*NN];
COMPLEX treble[2*NN],bass[2*NN],mid[2*NN];
float32_t overlap[NN];
float32_t a,b;
float32_t bass_gain = 0.05;
float32_t mid_gain = 0.05;
float32_t treble_gain = 1.0;

int16_t NUMCOEFFS = sizeof(lpcoeff)/sizeof(float32_t);

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
//    procbuf[i].real = (float32_t)(sample.uint16bit[LEFT]);
    procbuf[i].real = (float32_t)(prbs(8000));
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
	for (i=0 ; i<NN ; i++)
    {
			coeffs[i].real = bass[i].real*bass_gain
              + mid[i].real*mid_gain 
              + treble[i].real*treble_gain;
     coeffs[i].imag = bass[i].imag*bass_gain
              + mid[i].imag*mid_gain 
              + treble[i].imag*treble_gain;
		}
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
  for(i=0 ; i<(NN*2) ; i++) 
  { 
    coeffs[i].real = 0.0; 
		coeffs[i].imag = 0.0;
    bass[i].real = 0.0;
    mid[i].real = 0.0 ;
    treble[i].real = 0.0;
    bass[i].imag = 0.0;
    mid[i].imag = 0.0 ;
    treble[i].imag = 0.0;
  }
  for(i=0 ; i<NN ; i++) 
  { 
    overlap[i] = 0.0;
  }
  for(i=0 ; i<NUMCOEFFS ; i++)
  {
    bass[i].real = lpcoeff[i]; 
    mid[i].real = bpcoeff[i]; 
    treble[i].real = hpcoeff[i];
  }
  fft(bass,(2*NN),twiddle); 
  fft(mid,(2*NN),twiddle); 
  fft(treble,(2*NN),twiddle); 

 fm4_wm8731_init (FS_8000_HZ, WM8731_LINE_IN, IO_METHOD_DMA, WM8731_HP_OUT_GAIN_0_DB, WM8731_LINE_IN_GAIN_0_DB); 
  while(1)
  {
    while (!(rx_buffer_full && tx_buffer_empty)){}
    gpio_set(DIAGNOSTIC_PIN, HIGH);
    process_dma_buffer();
    gpio_set(DIAGNOSTIC_PIN, LOW);
  }
}
