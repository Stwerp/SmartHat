//******************************************************************************
// MSP430F2011 Program
//
// Passive Data Demodulator
//
//  Detects ASK/OOK data. Time between RF breaks indicate digital data
//
//******************************************************************************

#include  <msp430x20x1.h>

// PreProcessor Definitions

// Slowest Clock
// Turn DCO to slowest clock (method from Errata sheet) (line 1)
// 0b1111_0000 -> Clear out previous setting (Set RSEL bits) (line 2)
// Place new setting for RSEL : 0b0000_xxxx (line 3)
#define SLOW_CLOCK  \
  DCOCTL = 0x00;   \
  BCSCTL1 &= 0xF0;  \
  BCSCTL1 |= 0x00;     
  //DCOCTL |= 0x00;      // Place new setting for DCO  : 0bxxx0_0000

// Receive Clock (3.5 MHz)
// Turn DCO to slowest clock (method from Errata sheet) (line 1)
// 0b1111_0000 -> Clear out previous setting (Set RSEL bits) (line 2)
// Place new setting for RSEL : 0b0000_xxxx (line 3)
#define RECEIVE_CLOCK \
  DCOCTL = 0x00; \
  BCSCTL1 &= 0xF0; \
  BCSCTL1 |= RSEL3 + RSEL1 + RSEL0;
  //DCOCTL |= 0x00;      // Place new setting for DCO  : 0bxxx0_0000
  
//States for Timer Readings
#define staIDLE    0
#define staSETUP   1
#define staREADY   2
#define staOTHER   3
  

/*unsigned char mask2;      // 1 byte = 8 bits
unsigned short position;  // 2 bytes = 16 bits
unsigned long data;       // 4 bytes = 32 bits
*/
unsigned char ReadState;
unsigned short DemodThreshold = 5;
unsigned short numBits;
unsigned char DemodByte;
  
  
//Function Definitions
inline void setup_a_clk();
  
    
void main (void)
{
  
  SLOW_CLOCK;
  BCSCTL2 |= DIVM0; //Divide MCLK by 2

  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  
  // Turn OFF SMCLK and external Oscillator
  _BIS_SR(OSCOFF + SCG1);
  
  // Set P1.ALL to be Output ports
  P1OUT = 0x00;
  P1SEL = 0x00;

  //Setup ACLK to 10kHz source (VLO)
  setup_a_clk();
  
  //PLAN:
  // Sleep for CAP to charge
  // Turn on Comparator
  // Connect to Timer subsystem
  // Sleep
  //
  //Timer: Wake on edge, store number
  // Do bit detection
  // Reset
  // Output on Other pin
  
  //Sleep to charge storage cap
  //
  // Initialize timerA0
  //TACTL &= TACLR;
  CCTL0 = CCIE;                            // CCR0 interrupt enabled
  CCR0 = 1000;                              // 10 khz source * 100 ms = 1,000  cycles
  TACTL = TACLR + TASSEL_1 + MC_1;         // Source: ACLK, Mode: UP
  // Enter LPM3 Mode
  _BIC_SR(OSCOFF);
  _BIS_SR(LPM3_bits + GIE);                // Enter LPM3 w/ interrupts
  // -- Wake up -- //
  // Turn OFF external Oscillator
  _BIS_SR(OSCOFF);
 
  numBits = 0;

  // Setup Comparator
  //  CACTL1 = //NO settings Needed. Ready.
  //CACTL1 = CAIE + CAON // Turn on and enable interrupts
  CACTL2 = CAF + P2CA0 + P2CA2 + P2CA1; // Filter ON, use CA3 and CA0
  CAPD = CAPD3 + CAPD0; // Turn off port connections for pins being used (saves power)

  // Main Loop
  while (1)
  {
    //Turn On Comparator and Enable Comparator Interrupts
    CACTL1 = CAIE + CAON;
    //Set State
    ReadState = staSETUP ;
    // GoToSleep
    //Enter LPM3 Mode
    _BIC_SR(OSCOFF); //Osc ON
    _BIS_SR(LPM3_bits + GIE);                // Enter LPM3 w/ interrupts
    //(Only ComparatorA will wake up processor)
    // -- Wake up! -- //
   
    // Turn OFF external Oscillator
    _BIS_SR(OSCOFF);
    
    if (DemodByte == 0x00)
      P1OUT |= 0x02;
    if (DemodByte == 0xFF)
      P1OUT |= 0x04;
    
    //TACTL &= TACLR;
    CCTL0 = CCIE;                            // CCR0 interrupt enabled
    CCR0 = 1000;                              // 10 khz source * 100 ms = 1,000  cycles
    TACTL = TACLR + TASSEL_1 + MC_1;         // Source: ACLK, Mode: UP
    // Enter LPM3 Mode
    _BIC_SR(OSCOFF);
    _BIS_SR(LPM3_bits + GIE);                // Enter LPM3 w/ interrupts
    // -- Wake up -- //
    
    // Turn OFF external Oscillator
    _BIS_SR(OSCOFF);
    
    //Turn Off output pins
    P1OUT &= ~(0x06);
    
    //Repeat
    ReadState =  staIDLE ;
  }
} // Close Program


// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A0 (void)
{
  //Return from Sleep
  //
  // Turn timer interrupts OFF
  CCTL0 = 0x00;
  // Turn OFF Timer
  TACTL = 0x00;
  // Turn CPU on
  _BIC_SR_IRQ(LPM3_bits + GIE);
}

// Comparator A+ interrupt service routine
#pragma vector=COMPARATORA_VECTOR
__interrupt void COMPARATORA (void)
{
  switch( ReadState )
  {
  case  staIDLE :
    //Idle State
    //break;
  case staSETUP:
    //Setup State
    
    //TODO: Want to start capture mode here, and set that as demod value
    
    // Clear TIMER A, set in UP mode
    //CCTL0 = CCIE;                            // CCR0 interrupt enabled
    //CCR0 = 1000;                              // 10 khz source * 100 ms = 1,000  cycles
    
    TACTL = TACLR + TASSEL_1 + MC_1;         // Source: ACLK, Mode: UP
    CCR0 = DemodThreshold;    
    ReadState = staREADY;
    break;
  case staREADY:
    //Shift Over DemodByte
    DemodByte = DemodByte * 2;
    
    //Decode Data
    if ( (TACTL & TAIFG) == 0x01 )
      //Add High Bit to LSB -- Timer Reached (long stop)
      DemodByte |= 0x01;
    else
      //Add Low Bit to LSB -- Timer not Reached (short stop)
      DemodByte &= ~(0x01); //NOT 0x01
    
    //Clear Timer
    TACTL |= TACLR;
    
    //Check for full byte and wake up if full
    numBits++;
    if (numBits == 8)
    {
      // Wake From Sleep
      //Disable comparator Interrupts
      CACTL1 &= 0xFD; //(turn off bit 0x02)
      //Turn CPU back on
      _BIC_SR_IRQ(LPM3_bits + GIE);
    }
    
    break;
  case staOTHER:
    //Who knows
    break;
  }
  //End Switch
}




//Setup A_CLK
inline void setup_a_clk()
{
  // Setup ACLK source to be from internal VLOCLK (~10 kHz)

  BCSCTL3 |= LFXT1S_2;
  // Setup ACLK to be dvided by 8
  BCSCTL1 |= DIVA1 + DIVA0;  
}

