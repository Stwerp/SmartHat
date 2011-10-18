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
  TACTL = TASSEL_1 + MC_1;              // Source: ACLK, Mode: UP
  
  // Enter LPM3 Mode
  _BIS_SR(LPM3_bits + GIE);             // Enter LPM3 w/ interrupts
  // -- Wake up -- //
  
  //// Generate x kHz tone on P1.x
  // Setup ACLK to be divided by 4 for entire system (2.5 kHz)
  BCSCTL1 |= DIVA1;
  // Setup Watchdog Timer to reset system after 327.68 ms
  WDTCTL = WDTPW + WDTCNTCL + WDTSSEL + WDTIS1;  // Clear count, Source = ACLK, 512 cycles
                                                 // [ACLK (10 kHz) divided by 4] (2.5 kHz) * 512 cycles = 204.8 ms
  // Set Timer A to toggle on successful compare
  P1SEL = 0x03;                         // Set P1.0-1 to toggle based on T1
  TACTL = 0x00;                         // Turn off Timer A completely
  CCTL0 = OUTMOD_4;                     // Toggle on Compare
  CCR1 = 0;
  CCR0 = 1;                             // 1 cycles
  
  // Turn DCO to slowest clock (method from Errata sheet)
  DCOCTL = 0x00;
  // Set RSEL bits
  BCSCTL1 &= 0xF0;    // 0b1111_0000 -> Clear out previous setting
  BCSCTL1 |= 0x06;    // Place new setting for RSEL : 0b0000_xxxx
  DCOCTL |= 0xE0;     // Place new setting for DCO  : 0bxxx0_0000 
  
  // Turn on Timer A here so that there will not be no low-frequency signals at the beginning
  TACTL = TASSEL_2 + MC_1;       // Source: SMCLK, UP Mode, Div by 2
  
  // Enter LPM0 (CPU OFF)
  _BIS_SR(LPM0_bits);
}

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A0 (void)
{  
  // Turn timer interrupts OFF
  CCTL1 = 0x00;
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
