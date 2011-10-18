//******************************************************************************
// MSP430F2011 Program
//
// Semi-Active 2.5 kHz Generator
//
//  * IGNORE The program sleeps for 900 ms in LPM3 mode, then wakes up and outputs
//  a 2.5 kHz square wave tone on P1OUT.0 for 100 ms
//
//******************************************************************************

#include  <msp430x20x1.h>

/*unsigned char mask2;      // 1 byte = 8 bits
unsigned short position;  // 2 bytes = 16 bits
unsigned long data;       // 4 bytes = 32 bits
*/

unsigned char RFSignal;   // 1-bit state of RF Signal

void main(void)
{
  // Turn DCO to slowest clock (method from Errata sheet)
  DCOCTL = 0x00;
  // Set RSEL bits
  BCSCTL1 &= 0xF0;    // 0b1111_0000 -> Clear out previous setting
  /*BCSCTL1 |= 0x00;    // Place new setting for RSEL : 0b0000_xxxx
  DCOCTL |= 0x00;     // Place new setting for DCO  : 0bxxx0_0000*/
  BCSCTL2 |= DIVM0; //Divide MCLK by 2

  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  
  // Turn OFF SMCLK and external Oscillator
  _BIS_SR(OSCOFF + SCG1);
  
  // Set P1.ALL to be Output ports
  P1OUT = 0x00;
  //      1111_0110
  P1DIR = 0xF6; //0x1111_0110
  P1SEL = 0x00;
  
  // Setup ACLK source to be from internal VLOCLK (~10 kHz)
  BCSCTL3 |= LFXT1S_2;
  // Setup ACLK to be dvided by 8
  BCSCTL1 |= DIVA1 + DIVA0;

  // Setup Comparator
  CACTL1 = CAREF0 + CAREF1 + CARSEL; // Setup + Reference
  CACTL2 = CAF + P2CA0 + P2CA2 + P2CA1; // Filter ON, use CA3 and CA0
  
  while (1)
  {
    // Get Current Signal Value
    // // Turn on Comparator
    CACTL1 |= CAON;
    // // Test Signal Value
    RFSignal = (CACTL2 & CAOUT); // Capture Value
    // // Turn off Comparator
    CACTL1 &= 0xF7; // CAON = 0b0000_1000;
    
    while (RFSignal == 0x01)
    {
      // Beep Until Signal is not present
      // Sleep  LONG ms
      // Initialize timerA0
      //TACTL &= TACLR;
      CCTL0 = CCIE;                            // CCR0 interrupt enabled
      CCR0 = 700;                              // 10 khz source * 900 ms = 9,000  cycles
      TACTL = TACLR + TASSEL_1 + MC_1;         // Source: ACLK, Mode: UP
  
      // Enter LPM3 Mode
      _BIC_SR(OSCOFF);
      _BIS_SR(LPM3_bits + GIE);                // Enter LPM3 w/ interrupts
      // -- Wake up -- //
  
      // Turn OFF SMCLK and external Oscillator
      _BIS_SR(OSCOFF + SCG1);
  
      // Generate 5.0 kHz tone -- Tone 1
      for ( unsigned short i = 175 ; i != 0 ; i-- )
      {
        // Toggle Pin ON
        P1OUT = 0x20;
        // Delay for 1/2 period of 2.5 kHz (10 cycles)
        //__delay_cycles(1);
        
        // Toggle Pin OFF
        P1OUT = 0x00;
        // Delay
        __delay_cycles(1);
      } // End Tone Generation
      
      // RE-Test for RF Signal
      // // Turn on Comparator
      CACTL1 |= CAON;
      // // Test Signal Value
      RFSignal = (CACTL2 & CAOUT); // Capture Value
      // // Turn off Comparator
      CACTL1 &= 0xF7; // CAON = 0b0000_1000;
    }
    
    // Sleep until Rising Edge on Comparator
    // // Enable Comparator Interrupts
    CACTL1 |= CAIE; //0x02
    // // Turn on Comparator
    CACTL1 |= CAON;
    // // Enter LPM4 + GIE
    _BIS_SR(LPM4_bits + GIE);
    // -- Wake up -- //
    // // Turn off Comparator
    CACTL1 &= 0xF7; // CAON = 0b0000_1000;
  }
} // Close Program


// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A0 (void)
{
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
  // Disable Comparator Interrupts
  CACTL1 &= 0xFD; //turn off 0x02
  // Turn CPU on
  _BIC_SR_IRQ(LPM4_bits + GIE);
}


/*******************************************/
/*
Main: (PUC/POR)
Reset clock
Housekeeping
Enter Beep Loop

Turn on Comparator
Test Value
Turn off comparator
Main Loop{ -- loop forever
{
  while( Signal Present )
  {
    Beep until Falling Edge on Comparator
    ----
    LPM3 Sleep (only A0 interrupt enabled)
    Output Tone for ON duration
    Turn On Comparator
    Test Value -- Is Signal Present?
    Turn Off Comparator
  } // Re-test value and loop
  // Assume signal not present or have exited loop
  :Sleep until rising edge on Comprator
  ---
  Enable Comparator interrupts
  Turn on
  Enter LPM4 + GIE
}


Timer A0 interrupt:
set A0 flag
Test sleep state flag
return from sleep

Comparator interrupt:
set comparator flag
test sleep state flag
return from sleep
*/

