// fm4_wm8731_init.c
//
// support for Cypress FM4 version of example programs
//

#include "s6e2cc.h"
#include "fm4_wm8731_init.h"

static void DonaldDelay( __IO uint32_t nCount)
{ 
  for (; nCount > 0; nCount--);
}

uint32_t dma_tx_buffer_pong[DMA_BUFFER_SIZE];
uint32_t dma_tx_buffer_ping[DMA_BUFFER_SIZE]; 
uint32_t dma_rx_buffer_ping[DMA_BUFFER_SIZE]; 
uint32_t dma_rx_buffer_pong[DMA_BUFFER_SIZE];
uint32_t rx_proc_buffer = PING;
uint32_t tx_proc_buffer = PING;
volatile int tx_buffer_empty = 0;
volatile int rx_buffer_full = 0;

typedef union 
{
  uint16_t value;
  struct 
  {
    unsigned char bit0 : 1;
    unsigned char bit1 : 1;
    unsigned char bit2 : 1;
    unsigned char bit3 : 1;
    unsigned char bit4 : 1;
    unsigned char bit5 : 1;
    unsigned char bit6 : 1;
    unsigned char bit7 : 1;
    unsigned char bit8 : 1;
    unsigned char bit9 : 1;
    unsigned char bit10 : 1;
    unsigned char bit11 : 1;
    unsigned char bit12 : 1;
    unsigned char bit13 : 1;
    unsigned char bit14 : 1;
    unsigned char bit15 : 1;
  } bits;
} shift_register;

shift_register sreg = {0x0001};

int16_t prbs(int16_t noise_level)
{
  char fb;
  
  fb =((sreg.bits.bit15)+(sreg.bits.bit14)+(sreg.bits.bit3)+(sreg.bits.bit1))%2;
  sreg.value = sreg.value << 1;
  sreg.bits.bit0 = fb;			      
  if(fb == 0)	return(-noise_level); else return(noise_level);
}

uint32_t prand_seed = 1;       // used in function prand()

uint32_t rand31_next()
{
  uint32_t hi, lo;

  lo = 16807 * (prand_seed & 0xFFFF);
  hi = 16807 * (prand_seed >> 16);

  lo += (hi & 0x7FFF) << 16;
  lo += hi >> 15;

  if (lo > 0x7FFFFFFF) lo -= 0x7FFFFFFF;

  return(prand_seed = (uint32_t)lo);
}

int16_t prand()
{
  return ((int16_t)(rand31_next()>>18)-4096);
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
// PZR is port pseudo open drain setting register
#define GET_PZR(pin_ofs)  ((volatile unsigned char*) (PZR_BASE + pin_ofs))


void gpio_toggle(int pin_ofs)
{
  if ((*GET_PDOR(pin_ofs) & 0x01) == 1)
    *GET_PDOR(pin_ofs) = 0u;
  else
    *GET_PDOR(pin_ofs) = 1u;
}

void gpio_set(int pin_ofs, int value) 
{
  *GET_PDOR(pin_ofs) = value;
}

int gpio_get(int pin_ofs)
{
  return (*GET_PDIR(pin_ofs)) & 0x1;
}

void i2c_init() 
{   
  bFM4_GPIO_EPFR07_SCK2B1 = 1u;                  // GPIO pin P3B used for i2c clock
  bFM4_GPIO_PFR3_PB = 1u;
  bFM4_GPIO_EPFR07_SOT2B1 = 1u;                  // GPIO pin P3A used for i2c data
  bFM4_GPIO_PFR3_PA = 1u;
  FM4_GPIO->PZR3_f.PA = 1u;                      // i2c data pin pseudo open drain
  FM4_MFS2->BGR  = 100000000ul / 400000ul - 1ul; // i2c mode 400k Bit/s @ 100 MHz
  FM4_MFS2->SMR |= 0x80u;       	               // operate MFS2 in mode 4 (i2c)
  bFM4_MFS2_I2C_SMR_RIE = 0u;	                   // disable receive interrupts
  bFM4_MFS2_I2C_SMR_TIE = 0u;	                   // disable transmit interrupts
  FM4_MFS2->SCR |= 0x80u;                        // reset MFS2 (UPCL = 1)
}

void i2c_write(uint8_t device_address, uint8_t register_address, uint8_t cmd) 
{
  FM4_MFS2->ISBA = 0x00u;	                       // disable slave address detection
  FM4_MFS2->ISMK = 0x00;                         // clear slave mask
  bFM4_MFS2_I2C_ISMK_EN = 0x01; 
  FM4_MFS2->TDR = (device_address << 1)|0x00;    // load codec device address into transmit data register
  FM4_MFS2->IBCR = 0x80u;                        // select master mode
  while(!(bFM4_MFS2_I2C_IBCR_INT)) {}            // wait for interrupt flag, i.e. transmission complete
  FM4_MFS2->TDR = (register_address);            // load codec register address into transmit data register  
  bFM4_MFS2_I2C_IBCR_ACKE = 1u;                  // enable acknowledgment
  bFM4_MFS2_I2C_IBCR_WSEL = 1u;                  // set wait selection bit
  bFM4_MFS2_I2C_IBCR_INT = 0u;                   // clear interrupt flag
  while(!(FM4_MFS2->IBCR & 0x01u)) {}            // wait for interrupt flag, i.e. transmission complete	
  FM4_MFS2->TDR = (cmd);                         // load codec register command into transmit data register
  bFM4_MFS2_I2C_IBCR_ACKE = 1u;                  // enable acknowledgment
  bFM4_MFS2_I2C_IBCR_WSEL = 1u;                  // set wait selection bit
  bFM4_MFS2_I2C_IBCR_INT = 0u;                   // clear interrupt flag
  while(!(FM4_MFS2->IBCR & 0x01u)) {}            // wait for interrupt flag, i.e. transmission complete
  FM4_MFS2->IBCR = 0x20u;                        // enable acknowledge
  DonaldDelay(10000);                            // delay 
}

void Codec_WriteRegister( uint8_t RegisterAddr, uint8_t RegisterValue)
{    
  i2c_write(WM8731_I2C_ADDRESS, ((RegisterAddr << 1) & 0xFF), ((RegisterValue) & 0xFF));
}   

stc_dstc_des012345_t stcDES[2];     // storage for DSTC descriptors DES0 to DES3
stc_dstc_des0_t stcDES0Backup0;
stc_dstc_des0_t stcDES0Backup1;


/*****************************************************************************
 ** \brief  Parity calculation for DES0 register.
 **         Calculates 4 - bit parity of DES0 using the below formula
 **         PCHK[3:0] = (DES0[27:24] ^ DES0[23:20] ^ DES0[19:16] ^ DES0[15:12] ^
 **                      DES0[11:8] ^ DES0[7:4])
 **
 ** \param  [in] u32Des0   32 - bit DES0
 **
 ** \retval 4 - bit parity of DES0
 ****************************************************************************/
uint8_t DSTCPchkCalc(uint32_t u32Des0)
{
  uint8_t u8PCHK;

  u8PCHK =  (uint8_t)((((u32Des0) & 0x0F000000ul) >> 24ul) ^
                      (((u32Des0) & 0x00F00000ul) >> 20ul) ^
                      (((u32Des0) & 0x000F0000ul) >> 16ul) ^
                      (((u32Des0) & 0x0000F000ul) >> 12ul) ^
                      (((u32Des0) & 0x00000F00ul) >>  8ul) ^
                      (((u32Des0) & 0x000000F0ul) >>  4ul));
  return u8PCHK;
}
  

void dstc_init(void)
{   
  if(FM4_CLK_GATING->CKEN2_f.PCRCCK != 1u)
  {
    FM4_CLK_GATING->CKEN2_f.PCRCCK = 1u;
  }
  if(FM4_DSTC->CMD == 1u)    // If DSTC is in standby state, then take it to the normal state
  {
    FM4_DSTC->CMD = 4u;      // Standby Release command
    while(FM4_DSTC->CMD != 0u){}
  }
// channel 0 reads from memory (dma_tx_buffer_ping or dma_tx_buffer_pong) and transfers to the I2S peripheral
	
	/*** DES 0-3 registers ***/
  stcDES[0u].DES0.DV    = 0x03;             // Don't execute the DES close process after transfer ends
  stcDES[0u].DES0.ST    = 0u;               // Just a default, DSTC updates this on completion of transfer
  stcDES[0u].DES0.MODE  = 1u;               // Mode 1 -> single transfer for 1 trigger
  stcDES[0u].DES0.ORL   = 1u;               // Outer reload for DES1
  stcDES[0u].DES0.TW    = 0x2;              // 32-bit transfer width
  stcDES[0u].DES0.SAC   = 1u;               // Source address is incremented by TW * 1 at every transfer with reload
  stcDES[0u].DES0.DAC   = 5u;               // Destination address remains unchanged during the transfer
  stcDES[0u].DES0.CHRS  = 0x10u;            // Interrupt flag is set when IRM = 1 and ORM = 1. No Chain start
  stcDES[0u].DES0.DMSET = 1u;               // Set DQMSK = 1 when DES close process is executed
  stcDES[0u].DES0.CHLK  = 0u;               // No Chain start transfer
  stcDES[0u].DES0.ACK   = 1u;               // Output DMA transfer acknowledge to PRGCRC
  stcDES[0u].DES0.RESERVED = 0u;            // Required 
  stcDES[0u].DES0.PCHK  = DSTCPchkCalc(stcDES[0u].u32DES0);  //Parity

  // DES1
  stcDES[0u].DES1_mode1.IIN = (DMA_BUFFER_SIZE);      // Inner loop; max 256
  stcDES[0u].DES1_mode1.IRM = (DMA_BUFFER_SIZE);      // Same as IIN
  stcDES[0u].DES1_mode1.ORM = 1;              			  // Single cout

  // DES2
  stcDES[0u].DES2 = (uint32_t)&dma_tx_buffer_ping ;   // Source address (incremented by TW * 1 for every transfer. Configured in DES0.SAC)

  // DES3
  stcDES[0u].DES3 = (uint32_t)&FM4_I2S0->TXFDAT;      // Destination address - I2S Transmission data register (Same for every transfer. Configured in DES0.DAC)
	// DES4
	stcDES[0u].DES4_mode1 = stcDES[0u].DES1_mode1;      // Used to reload DES1

  // DESTP register
  FM4_DSTC->DESTP = (uint32_t)&stcDES[0u];            // DESTP should contain word - aligned address
	
	 // HWDESP
	FM4_DSTC->HWDESP =  219ul;												  // DESP = 0 for HW channel 219 
// Hardware channel numbers for interrupts/DSTC detailed in FM4_MN709_00001-4v0-E page 374  
	FM4_DSTC->DREQENB6  |= (1ul << 27ul);   					  // Enable HW channel 219 (I2S transmission)
  FM4_DSTC->HWINTCLR6 |= (1ul << 27ul);								// Clear HWINT6 register bit corresponding to HW channel 219(I2S Transmission DSTC)
	

	/****** channel 1 = Reads from the I2S peripheral and transfer to (dma_rx_buffer_ping or dma_rx_buffer_pong)   ******/
	
	  /*** DES 0-3 registers ***/
  stcDES[1u].DES0.DV    = 0x03;             // Don't Execute the DES close process after transfer ends
  stcDES[1u].DES0.ST    = 0u;               // Just a default, DSTC updates this on completion of transfer
  stcDES[1u].DES0.MODE  = 1u;               // Mode 1 -> single transfer for 1 trigger
  stcDES[1u].DES0.ORL   = 1u;               // Outer reload for DES1
  stcDES[1u].DES0.TW    = 0x2;              // 32-bit transfer width
  stcDES[1u].DES0.SAC   = 5u;               // Source address remains unchanged during the transfer
  stcDES[1u].DES0.DAC   = 1u;               // Destination address is incremented by TW * 1 at every transfer with reload
  stcDES[1u].DES0.CHRS  = 0x10u;            // Interrupt flag is set when IRM = 1 and ORM = 1. No Chain start
  stcDES[1u].DES0.DMSET = 1u;               // Set DQMSK = 1 when DES close process is executed
  stcDES[1u].DES0.CHLK  = 0u;               // No Chain start transfer
  stcDES[1u].DES0.ACK   = 1u;               // Output DMA transfer acknowledge to PRGCRC
  stcDES[1u].DES0.RESERVED = 0u;            
  stcDES[1u].DES0.PCHK  = DSTCPchkCalc(stcDES[1u].u32DES0);

  // DES1
  stcDES[1u].DES1_mode1.IIN = (DMA_BUFFER_SIZE);      // Inner loop; max 256
  stcDES[1u].DES1_mode1.IRM = (DMA_BUFFER_SIZE);      // Same as IIN
  stcDES[1u].DES1_mode1.ORM = 1;              			  // Single cout

  // DES2
  stcDES[1u].DES2 = (uint32_t)&FM4_I2S0->RXFDAT ;     // Source address 

  // DES3
  stcDES[1u].DES3 = (uint32_t)&dma_rx_buffer_ping;    // Destination address - I2S Transmission data register (Same for every transfer. Configured in DES0.DAC)
	// DES4
	stcDES[1u].DES4_mode1 = stcDES[1u].DES1_mode1;      // Used to reload DES1

 // HWDESP
	FM4_DSTC->HWDESP =  (0x1Cul << 16ul) | 218ul; 	    // DESP = 0x1C(28bits) for HW channel 218 I2S_RX
	
  FM4_DSTC->DREQENB6  |= (1ul << 26ul);   						// Enable HW channel 218 
  FM4_DSTC->HWINTCLR6 |= (1ul << 26ul);

	// DSTC CFG Register
  // [14:12]    SWPR   = 111      =>    SW transfer has lowest priority
  // [11]       ESTE   = 0        =>    Do not enter error stop state in case of error
  // [10]       RBDIS  = 1        =>    Disable read skip buffer function
  // [9]        ERINTE = 0        =>    disable Error interrupt
  // [8]        SWINTE = 0        =>    Disable software transfer complete interrupt
  FM4_DSTC->CFG = 0x74u;                // 01110100   

  FM4_DSTC->CMD = 0x10u;                // Command to clear the SWINT interrupt
  FM4_DSTC->CMD = 0x20u;                // Command to clear ERINT interrupt. MONERS.EST = 0, MONERS.DER = 0, MONERS.ESTOP = 0
  FM4_DSTC->CMD = 0x80u;                // Command to clear all DQMSK[n] registers
}
	
void fm4_wm8731_init(char fs, char select_input, char io_method, char hp_out_gain, char line_in_gain)
{	
  i2c_init();                                                 // initialise i2c peripheral 
  DonaldDelay(1000);                                          // before writing to codec registers
  Codec_WriteRegister ( WM8731_RESET, 0x00);  		            // reset codec
  Codec_WriteRegister ( WM8731_LINE_IN_LEFT, line_in_gain); 	// set left line in gain
  Codec_WriteRegister ( WM8731_LINE_IN_RIGHT, line_in_gain); 	// set right line in gain
  Codec_WriteRegister ( WM8731_HP_OUT_LEFT, hp_out_gain);			// set left headphone out gain  
  Codec_WriteRegister ( WM8731_HP_OUT_RIGHT, hp_out_gain);		// set right headphone out gain
  Codec_WriteRegister ( WM8731_ANALOG_PATH, select_input); 		// select line in or microphone input 
  Codec_WriteRegister ( WM8731_DIGITAL_PATH, 0x00); 					// can select de-emphasis, HPF and mute here
  Codec_WriteRegister ( WM8731_POWER_DOWN, 0x00); 			  	  // disable power down on all parts of codec
  Codec_WriteRegister ( WM8731_INTERFACE, 0x53); 					    // select digital audio interface (i2s) format
  Codec_WriteRegister ( WM8731_SAMPLING_RATE, fs);            // sample rate control 
  Codec_WriteRegister ( WM8731_CONTROL, 0x01); 					      // activate codec
	
  i2s_init(fs, io_method);
  FM4_I2S0->OPRREG_f.RXENB = 1; 
  FM4_I2S0->CNTREG_f.RXDIS = 0;  
  FM4_I2S0->OPRREG_f.TXENB = 1; 
  FM4_I2S0->CNTREG_f.TXDIS = 0;
  switch(io_method)
  {
	  case IO_METHOD_INTR:
	  {
      FM4_I2S0->INTCNT_f.RFTH = 0x0F & (0x00);
	    FM4_I2S0->INTCNT_f.TFTH = 0x0F & (0x00);
      NVIC_ClearPendingIRQ(PRGCRC_I2S_IRQn);
      NVIC_EnableIRQ(PRGCRC_I2S_IRQn);
      NVIC_SetPriority(PRGCRC_I2S_IRQn, 2); 
      FM4_I2S0->OPRREG_f.START  = 1u ;            // start the i2s module
      break;
	  }
	  case IO_METHOD_DMA:
  	{
		  NVIC_DisableIRQ(DSTC_IRQn);                 // disable DSTC interrupts
	    dstc_init();                                // initialise DSTC channels 0 and 1
	    NVIC_ClearPendingIRQ(DSTC_COMP_IRQn);
      NVIC_EnableIRQ(DSTC_COMP_IRQn);
      NVIC_SetPriority(DSTC_COMP_IRQn, 3);  
      FM4_I2S0->INTCNT_f.RFTH = 0x0F & (0x03);
	    FM4_I2S0->INTCNT_f.TFTH = 0x0F & (0x03);
	    bFM4_I2S0_DMAACT_RL1E0 = 0x01;
	    bFM4_I2S0_DMAACT_RDMACT = 0x01;
      bFM4_I2S0_DMAACT_TL1E0 = 0x01;
	    bFM4_I2S0_DMAACT_TDMACT = 0x01;
 	    FM4_I2S0->OPRREG_f.START  = 1u ;            // start the i2s module
      break;
	  }	
  }

  bFM4_CLK_GATING_CKEN0_GIOCK |= 0x01;           // supply clock GPIO module
	
  // switch off analog input function on those GPIO pins used by example programs
	
  bFM4_GPIO_ADE_AN00 = 0x00; // P10   DIAGNOSTIC_PIN
  bFM4_GPIO_ADE_AN08 = 0x00; // P18   LED_B
  bFM4_GPIO_ADE_AN10 = 0x00; // P1A   LED_R
  bFM4_GPIO_ADE_AN18 = 0x00; // PB2   LED_G
  bFM4_GPIO_ADE_AN20 = 0x00; // P20   USER_SWITCH

  *GET_PFR(DIAGNOSTIC_PIN) &= ~0u; // set pin function as GPIO
  *GET_DDR(DIAGNOSTIC_PIN) = 1u  ; // set pin direction as output
  *GET_PCR(DIAGNOSTIC_PIN) &= ~0u; // set pin to have pull-up

  *GET_PFR(LED_R) &= ~0u;          // set pin function as GPIO
  *GET_DDR(LED_R) = 1u  ;          // set pin direction as output
  *GET_PCR(LED_R) &= ~0u;          // set pin to have pull-up


  *GET_PFR(LED_G) &= ~0u;          // set pin function as GPIO
  *GET_DDR(LED_G) = 1u  ;          // set pin direction as output
  *GET_PCR(LED_G) &= ~0u;          // set pin to have pull-up

  *GET_PFR(LED_B) &= ~0u;          // set pin function as GPIO
  *GET_DDR(LED_B) = 1u  ;          // set pin direction as output
  *GET_PCR(LED_B) &= ~0u;          // set pin to have pull-up

  *GET_PFR(USER_SWITCH) |= 0u;     // set pin function as GPIO
  *GET_DDR(USER_SWITCH) |= 0u;     // set pin direction as input
  *GET_PCR(USER_SWITCH) |= 0u;     // set pin to have pull-up

  gpio_set(LED_R, HIGH);           // turn RGB LED off
  gpio_set(LED_G, HIGH);
  gpio_set(LED_B, HIGH);

}

void i2s_init(char fs, char io_method) 
{   
	bFM4_CLK_GATING_CKEN2_I2SCK0 = 1u;   // supply clock to i2s peripheral.

  FM4_GPIO->EPFR24 |= (1ul << 0ul);    // configure I2S0-MCLK pin as input
  bFM4_GPIO_PFR5_PD = 1u;

  FM4_GPIO->EPFR24 |= (1ul << 10ul);   // configure I2S0-DO pin as output
  bFM4_GPIO_PFR5_PE = 1u;
	
	FM4_GPIO->EPFR24 |= (1ul << 8ul);    // configure I2S0-DI pin as input   
  bFM4_GPIO_PFR3_P0 = 1u;

  FM4_GPIO->EPFR24 |= (1ul << 4ul);    // configure I2S0-CK pin as input/output (input used)
  bFM4_GPIO_PFR3_P1 = 1u;

  FM4_GPIO->EPFR24 |= (1ul << 6ul);    // configure I2S0-WS pin as input/output (input used)
  bFM4_GPIO_PFR5_PF = 1u;

  bFM4_I2SPRE_ICCR_ICEN =0u;					 // disable MCLK output - clock will be generated by WM8731
  FM4_I2S0->OPRREG_f.START = 0u;       // stop i2s interface while configuring 
  FM4_I2S0->CNTREG_f.CKRT  = 0u;       // bypass clock divider
  switch (fs)                          // configure overhead bits according to sampling rate
	{
    case FS_8000_HZ:
      FM4_I2S0->CNTREG_f.OVHD  = 352;
      break;		
		case FS_32000_HZ:
     	FM4_I2S0->CNTREG_f.OVHD  = 64;
      break;		
		case FS_16000_HZ:
     	FM4_I2S0->CNTREG_f.OVHD  = 64;
      break;		
		case FS_24000_HZ:
     	FM4_I2S0->CNTREG_f.OVHD  = 32;
      break;		
		case FS_48000_HZ:
     	FM4_I2S0->CNTREG_f.OVHD  = 32;
      break;		
		case FS_96000_HZ:
     	FM4_I2S0->CNTREG_f.OVHD  = 0;
      break;
    default:	
     	FM4_I2S0->CNTREG_f.OVHD  = 352;
      break;		
	}			
  FM4_I2S0->CNTREG_f.MSKB  = 0x00 ;   // no mask -bit 
  FM4_I2S0->CNTREG_f.MSMD  = 0x00 ;   // i2s slave mode
  FM4_I2S0->CNTREG_f.SBFN  = 0x00 ;   // 1 subframe, 2 channels in subframe 
  FM4_I2S0->CNTREG_f.RHLL  = 0x01 ;   // 32-bit FIFO word comprises two 16-bit samples     
  FM4_I2S0->CNTREG_f.MLSB  = 0x00;    // shift starts from MSB of word 
  FM4_I2S0->CNTREG_f.SMPL  = 0x00;    // sample in middle of received data.
  FM4_I2S0->CNTREG_f.CPOL  = 0x01;    // drive data at the falling edge and sample data at rising edge of I2SCK 
  FM4_I2S0->CNTREG_f.FSPH  = 0x00;    // I2SWS is enabled at the same time as the frame data and first bit 
  FM4_I2S0->CNTREG_f.FSLN  = 0x01;    // frame sync pulse width is one channel (16-bit sample)
  FM4_I2S0->CNTREG_f.FSPL  = 0x00;    // I2SWS is "1", and the frame sync signal is enabled. This is "0" when idle.
  FM4_I2S0->MCR0REG_f.S0CHN  =  1u ;  // (number_of_channels - 1) for subframe 0 (two channels)
  FM4_I2S0->MCR0REG_f.S0CHL  =  15u;  // (bit_length - 1) of the channels that make up subframe 0 
  FM4_I2S0->MCR0REG_f.S0WDL  =  15u;  // subframe 0 word length - 1
  FM4_I2S0->MCR1REG  = 0x00000003u;   // enable channels 0 and 1 in subframe 0

  // Transmit interrupt settings
  FM4_I2S0->INTCNT_f.TXUD0M = 0x01;
  FM4_I2S0->INTCNT_f.TXUD1M = 0x01;
  FM4_I2S0->INTCNT_f.TBERM  = 0x01;
  FM4_I2S0->INTCNT_f.TXOVM  = 0x01;
  switch(io_method)
  {
	  case IO_METHOD_INTR:
      FM4_I2S0->INTCNT_f.TXFDM  = 0x01;
      FM4_I2S0->INTCNT_f.TXFIM  = 0x00;
      FM4_I2S0->INTCNT_f.RXFDM  = 0x01;
	    break;
	  case IO_METHOD_DMA:
	    FM4_I2S0->INTCNT_f.TXFDM  = 0x00; 
      FM4_I2S0->INTCNT_f.TXFIM  = 0x01;
      FM4_I2S0->INTCNT_f.RXFDM  = 0x00;
	    break;	
  }
	FM4_I2S0->INTCNT_f.FERRM  = 0x01;
  FM4_I2S0->INTCNT_f.RBERM  = 0x01; 
  FM4_I2S0->INTCNT_f.RXUDM  = 0x01; 
  FM4_I2S0->INTCNT_f.RXOVM  = 0x01;
  FM4_I2S0->INTCNT_f.EOPM   = 0x01;
  FM4_I2S0->INTCNT_f.RXFIM  = 0x01; 
	FM4_I2S0->INTCNT_f.RPTMR  = 0x0;  
}

uint32_t i2s_rx(void)
{
  return 	FM4_I2S0->RXFDAT; // read data from the receive FIFO register  
}

void i2s_tx(uint32_t c)
{
	FM4_I2S0->TXFDAT = c;     // write data to the transmit FIFO register 
}

void DSTC_COMP_IRQHandler(void)  // handles tx and rx DSTC transfer completion interrupts
{
  if (FM4_DSTC->HWINT6 >> 27 && 0x1) // interrupt is from tx DSTC transfer
  {
    if(tx_proc_buffer == (PONG))
    {
      stcDES[0].DES2 = (uint32_t)&(dma_tx_buffer_pong);    // set source address
      tx_proc_buffer = PING; 
		}
    else
    {
      stcDES[0].DES2 = (uint32_t)&(dma_tx_buffer_ping);    // set source address
      tx_proc_buffer = PONG; 
    }
    tx_buffer_empty = 1;                         // signal that tx buffer is empty					
 		  FM4_DSTC->DQMSKCLR6	|= (1ul << 27ul);
			FM4_DSTC->HWINTCLR6 |= (1ul << 27ul);
  }
  if (FM4_DSTC->HWINT6 >> 26 && 0x1) // interrupt is from tx DSTC transfer
  {
    if(rx_proc_buffer == PONG)
    {
      stcDES[1].DES3 = (uint32_t)&(dma_rx_buffer_pong);   // set estination address
      rx_proc_buffer = PING;
    }
    else
    {
      stcDES[1].DES3 = (uint32_t)&(dma_rx_buffer_ping);   // set destination address
      rx_proc_buffer = PONG;
    }
    rx_buffer_full = 1;              // signal that rx buffer is full
		 FM4_DSTC->DQMSKCLR6 |= (1ul << 26ul);
	   FM4_DSTC->HWINTCLR6 |= (1ul << 26ul);		 
  }
}

