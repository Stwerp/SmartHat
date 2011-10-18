//******************************************************************************
// MSP430F2011 Program
//
// I/Q Data Transmission
//
//  The program sleeps for 900 ms in LPM3 mode, then wakes up and outputs
//  a 250 kHz square wave tone on P1OUT.0 for 100 ms
//
//******************************************************************************

#include  <msp430x20x1.h>

/*unsigned char mask2;      // 1 byte = 8 bits
unsigned short position;  // 2 bytes = 16 bits
unsigned long data;       // 4 bytes = 32 bits
*/

unsigned int State;

void main(void)
{
  // SETUP:
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
  
  // Set P1.ALL to be correct Output/Input ports
  P1OUT = 0x00;
  //      1111_1111
  P1DIR = 0xFF;
  P1SEL = 0x00;
  
  // Setup ACLK source to be from internal VLOCLK (~10 kHz)
  BCSCTL3 |= LFXT1S_2;
  // Setup ACLK to be dvided by 8
  BCSCTL1 |= DIVA1 + DIVA0;
  
  // Initialize
  State = 0;
  
  // MAIN LOOP:
  while (1)
 {
   P1OUT = 0x00;
   // Set State  
   if(State == 0)
   {
     // -- Toggle LED 1 (Red) -- //
     P1OUT ^= 0x08;
   }
   else if(State == 1)
   {
     // -- Toggle LED 2 (Green) -- //
     P1OUT ^= 0x10;
   }
   else if(State == 2)
   {
     // -- Toggle LED 3 (Yellow)-- //
     P1OUT ^= 0x40;  
   }
   else
   {
     // -- Toggle ALL LEDs -- //
     P1OUT ^= 0x58;  
   }
   
   // Set A0/A1 outputs based on State's Value
   P1OUT ^= (State);
   
   // Check States Value, and reset
   if (++State > 3)
   { 
     State = 0;
   }
   
     

   // Sleep  2 sec
   // Setup ACLK source to be from internal VLOCLK (~10 kHz)
  BCSCTL3 |= LFXT1S_2;
  // Setup ACLK to be dvided by 8
  BCSCTL1 |= DIVA1 + DIVA0;
   // Initialize timerA0
   //TACTL &= TACLR;
   CCTL0 = CCIE;                             // CCR0 interrupt enabled
   CCR0 = 10000;                              // 10 khz source * 2000 ms = 20,000  cycles
   //CCR0 = 50;                              // 10 khz source * 2000 ms = 20,000  cycles
   TACTL = TACLR + TASSEL_1 + MC_1;                  // Source: ACLK, Mode: UP

   // Enter LPM3 Mode
   _BIC_SR(OSCOFF);
   _BIS_SR(LPM3_bits + GIE);                 // Enter LPM3 w/ interrupts
   // -- Wake up -- //

   // Turn OFF SMCLK and external Oscillator
   _BIS_SR(OSCOFF + SCG1);
   
   //Turn OFF LED's
   //OUT = (P1OUT & 0xA7);
   
 }  
}

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

