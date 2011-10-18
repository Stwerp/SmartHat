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

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

  // Turn DCO to slowest clock (method from Errata sheet)
  DCOCTL = 0x00;
  // Set RSEL bits
  BCSCTL1 &= 0xF0;    // 0b1111_0000 -> Clear out previous settings (defaults)
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
  TACTL |= TACLR;
  CCTL0 = CCIE;                         // CCR0 interrupt enabled          
  CCR0 = 200;                          // 10 kHz source * x ms = x cycles
  TACTL = TASSEL_1 + MC_1;              // Source: ACLK, Mode: UP
  
  // Enter LPM3 Mode
  _BIS_SR(LPM3_bits + GIE);             // Enter LPM3 w/ interrupts
  // -- Wake up -- //
  
  while(1)
  {
    //// Generate x kHz tone on P1.x 
    // Set Timer A to toggle on successful compare
    P1SEL = 0x02;                         // Set P1.0-1 to toggle based on T1
    TACTL = 0x00;                         // Turn off Timer A
    CCTL0 = OUTMOD_4;                     // Toggle on Compare, do not enable interrupts
    CCR0 = 8;                          // 1 cycles
    
    // Turn DCO to slowest clock (method from Errata sheet)
    DCOCTL = 0x00;
    // Set RSEL bits
    BCSCTL1 &= 0xF0;    // 0b1111_0000 -> Clear out previous setting
    /*BCSCTL1 |= 0x06;    // Place new setting for RSEL : 0b0000_xxxx
    DCOCTL |= 0xE0;     // Place new setting for DCO  : 0bxxx0_0000 */
    
    // Turn on Timer A here so that there will not be no low-frequency signals at the beginning
    TACTL = TASSEL_2 + MC_1;       // Source: SMCLK, UP Mode, Div by 2
    
    // Setup ACLK to be divided by 8 for entire system (2.5 kHz)
    BCSCTL1 &= 0xCF;  //  Clear out previou setting
    BCSCTL1 |= DIVA1 + DIVA0;
    // Setup Watchdog Timer to reset system after 327.68 ms
    WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL + WDTSSEL + WDTIS1 + WDTIS0;  // Clear count, Source = ACLK, 512 cycles
                                                   // [ACLK (10 kHz) divided by 4] (2.5 kHz) * 512 cycles = 204.8 ms
    IE1 |= WDTIE;
    
    // Enter LPM0 (CPU OFF)
    _BIS_SR(LPM0_bits + GIE);
    // -- Wake Up -- //
    //IE1 &= 0xFE; // Turn OFF WDTIE;
    
    // Turn OFF Output Toggling
    CCTL0 = 0x00;
    
    //Sleep
    
    //// Sleep for x milliseconds
    // Setup ACLK to be divided by 1 for entire system (2.5 kHz)
    BCSCTL1 &= 0xCF;  //  Clear out previou setting
    // Initialize timer A0
    TACTL |= TACLR;
    CCTL0 = CCIE;                         // CCR0 interrupt enabled          
    CCR0 = 1900;                          // 10 kHz source * x ms = x cycles
    TACTL = TASSEL_1 + MC_1;              // Source: ACLK, Mode: UP
    
    // Enter LPM3 Mode
    _BIS_SR(LPM3_bits + GIE);             // Enter LPM3 w/ interrupts
    // -- Wake up -- //
  }
    
}

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A0 (void)
{  
  // Turn timer interrupts OFF
  CCTL0 = 0x00;
  // Turn Timer A OFF
  TACTL = 0x00;
  // Turn CPU on
  _BIC_SR_IRQ(LPM3_bits + GIE);
}

// Watchdog Timer interrupt service routine
// Used for timing of bit periods
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
  // Reset WDTCounter
  WDTCTL = WDTPW + WDTHOLD;
  // Wake system up
  _BIC_SR_IRQ(LPM0_bits + GIE);
}
