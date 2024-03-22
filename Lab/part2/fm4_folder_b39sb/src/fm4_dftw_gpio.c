// fm4_dftw_gpio.c 

#define ARM_MATH_CM4

#include <s6e2cc.h>
#include "arm_math.h"
#include <stdint.h>
#include "arm_const_structs.h"

#include <math.h> 

//#define PI 3.14159265358979
#define N 100
#define TESTFREQ 800.0
#define SAMPLING_FREQ 8000.0

// offsets in bit-banded memory for GPIO pins used by example programs
#define DIAGNOSTIC_PIN 0x0080

#define PFR_BASE  (0x42DE0000)
#define PCR_BASE  (0x42DE2000)
#define DDR_BASE  (0x42DE4000)
#define PDIR_BASE (0x42DE6000)
#define PDOR_BASE (0x42DE8000)

#define HIGH 1
#define LOW  0

typedef struct
{
  float real;
  float imag;
} COMPLEX;

COMPLEX samples[N];
COMPLEX twiddle[N];

void dftw(COMPLEX *x, COMPLEX *w)
{
}

// find addresses of GPIO register bits in bit-banded memory space
// based on base addresses and on offset values that identify port and pin
// e.g. port C pin 9 has offset value (0x0C x 0x80) + (0x09 x 0x04) = 0x624
// and hence the address in bit-banded memory corresponding to the PCR for 
// port C pin 9 is 0x42DE2624
//
// PFR is port function setting register - 0 for GPIO, 1 for peripheral function
#define GET_PFR(pin_ofs)  ((volatile unsigned char*) (PFR_BASE + pin_ofs)) 
// PCR is port pull-up setting register - 0 for pull-up, 1 for no pull-up
#define GET_PCR(pin_ofs)  ((volatile unsigned char*) (PCR_BASE + pin_ofs))
// PDDR is port direction setting register - 0 for GPIO in, 1 for GPIO out
#define GET_DDR(pin_ofs)  ((volatile unsigned char*) (DDR_BASE + pin_ofs))
// PDIR is port input data register - read input pin status here
#define GET_PDIR(pin_ofs)  ((volatile unsigned char*) (PDIR_BASE + pin_ofs)) 
// PDOR is port output data register - write GPIO output data here
#define GET_PDOR(pin_ofs)  ((volatile unsigned char*) (PDOR_BASE + pin_ofs))

void gpio_set(int pin_ofs, int value) 
{
  *GET_PDOR(pin_ofs) = value;
}


int main()
{    
  int n;
  bFM4_CLK_GATING_CKEN0_GIOCK |= 0x01;           // supply clock GPIO module
	
  // switch off analog input function on those GPIO pins used by example programs
	
  bFM4_GPIO_ADE_AN00 = 0x00; // P10   DIAGNOSTIC_PIN

  *GET_PFR(DIAGNOSTIC_PIN) &= ~0u; // set pin function as GPIO
  *GET_DDR(DIAGNOSTIC_PIN) = 1u  ; // set pin direction as output
  *GET_PCR(DIAGNOSTIC_PIN) &= ~0u; // set pin to have pull-up

  for(n=0 ; n<N ; n++)
  {
    twiddle[n].real = cos(2*PI*n/N);
    twiddle[n].imag = -sin(2*PI*n/N);
  }

  for(n=0 ; n<N ; n++)
  {
    samples[n].real = cos(2*PI*TESTFREQ*n/SAMPLING_FREQ);
    samples[n].imag = 0.0;
  }
  gpio_set(DIAGNOSTIC_PIN, HIGH);
  dftw(samples,twiddle);          //call DFT function
  gpio_set(DIAGNOSTIC_PIN, LOW);
  while(1){}
}	
