//******************************************************************************
// MSP430F2011 Program
//
// Tone Generator Skeleton
//
//  This program sleeps in LPM3 mode for a set amount of time, and after waking
//  up, produces a tone on pin P1.1 in LPM1 mode. (The CPU is turned off). The
//  watchdog dog timer restarts the program after a specified amount of time.
//
//******************************************************************************

#include  <msp430x20x1.h>

unsigned char data;

void main(void)
{
  while(1)
  {
    WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  
    // Turn DCO to slowest clock (method from Errata sheet)
    DCOCTL = 0x00;
    // Set RSEL bits
    BCSCTL1 &= 0x80;    // 0b1000_0000 -> Clear out previous settings (defaults)
    /*BCSCTL1 |= 0x00;    // Place new setting for RSEL : 0b0000_xxxx
    DCOCTL |= 0x00;     // Place new setting for DCO  : 0bxxx0_0000*/
    
    TACTL = 0x00;     // Turn OFF timer A and clear current values (defaults)
    
    // Set P1.ALL to be Output ports
    P1OUT = 0x00;
    P1DIR = 0xFF;
    
    // Setup ACLK source to be from internal VLOCLK (~10 kHz)
    BCSCTL3 |= LFXT1S_2;
    
    //// Sleep for x milliseconds
    // Initialize timer A1
    TACTL &= TACLR;
    CCTL0 = CCIE;                         // CCR0 interrupt enabled          
    CCR0 = 9000;                          // 10 kHz source * x ms = x cycles
    TACTL = TASSEL_1 + MC_1;              // Source: ACLK(src:VLOCLK), Mode: UP
    
    // Enter LPM3 Mode
    _BIS_SR(LPM3_bits + GIE);             // Enter LPM3 w/ interrupts
    // -- Wake up -- //
    
    data = 0xB2;  // Value to transmit
    
    //// Generate x kHz tone on P1.x
    // Setup ACLK(src:VLOCLK) to be divided by 2 for entire system (5 kHz)
    BCSCTL1 |= DIVA0;
    
    // Set TimerA0 to toggle on successful compare
    TACTL = 0x00;                         // Turn off Timer A completely
    P1SEL = 0x03;                         // Set P1.1 to toggle based on T0
    CCTL0 = OUTMOD_4;                     // Toggle on Compare, do not enable interrupts
    CCR1 = 0;
    CCR0 = 0xFFFF;  // Set to highest value for now
    
    // Turn DCO to slowest clock (method from Errata sheet)
    DCOCTL = 0x00;
    // Set RSEL bits
    BCSCTL1 &= 0xF0;    // 0b1111_0000 -> Clear out previous setting
    BCSCTL1 |= 0x06;    // Place new setting for RSEL : 0b0000_xxxx
    DCOCTL |= 0xE0;     // Place new setting for DCO  : 0bxxx0_0000 
    
    // Turn on Timer A here so that there will not be no low-frequency signals at the beginning
    TACTL = TASSEL_2 + MC_1;       // Source: SMCLK, UP Mode, Div by 2
    
    // Setup Watchdog Timer to cause interrupt after xxx ms
    WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL + WDTSSEL + WDTIS1 + WDTIS0;
    IE1 |= WDTIE;
                                                   // Clear count, Source = ACLK, 64 cycles
                                                   // [ACLK (10 kHz) divided by 5] (5 kHz) * 64 cycles = 12.8 ms (bit half period)
                                                   // Cause interrupt instead of system reset
                                                   // Actually enable the WDT vector
    
    // Transmit FSK encoded(Manchester encoded) data
    // 0 bit : 0->1 = LF -> HF
    // 1 bit : 1->0 = HF -> LF
    for(unsigned char i = 8 ; i != 0 ; i-- )
    {
      // Send bit by bit
      if( (data & BIT0) == 0x00 )
      {
        // zero bit: LF to HF transition
        // LF
        CCR0 = 2;
        _BIS_SR(LPM0_bits + GIE);   // Enter LPM0 (CPU OFF)
        // -- Wake up (watchdog) -- //
        
        // HF
        CCR0 = 1;
        _BIS_SR(LPM0_bits + GIE);   // Enter LPM0 (CPU OFF)
        // -- Wake up (watchdog) -- //
      }
      else
      {
        // one bit: HF to LF transition
        // HF
        CCR0 = 1;
        _BIS_SR(LPM0_bits + GIE);   // Enter LPM0 (CPU OFF)
        // -- Wake up (watchdog) -- //
        
        // LF
        CCR0 = 2;
        _BIS_SR(LPM0_bits + GIE);   // Enter LPM0 (CPU OFF)
        // -- Wake up (watchdog) -- //
      }
      
      data = data >> 1; //Shift data right one
    } // End FOR loop
  } // restart at beginning
}

// Timer A0 interrupt service routine
//
// Used to sleep for 900 milliseconds between transmissions
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A0 (void)
{  
  // Turn Timer A OFF
  TACTL = 0x00;
  // Turn CPU on
  _BIC_SR_IRQ(LPM3_bits + GIE);
}

/*// Timer A1 interrupt service routine
#pragma vector=TIMERA1_VECTOR
__interrupt void Timer_A1 (void)
{
  // Turn timer interrupts OFF
  CCTL1 = 0x00;
  // Turn Timer A OFF
  TACTL = 0x00;
  // Turn CPU on
  _BIC_SR_IRQ(LPM3_bits + GIE);
}*/

// Watchdog Timer interrupt service routine
// Used for timing of bit periods
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
  // Reset WDTCounter
  //WDTCTL |= WDTPW + WDTCNTCL;
  // Wake system up
  _BIC_SR_IRQ(LPM0_bits + GIE);
}



