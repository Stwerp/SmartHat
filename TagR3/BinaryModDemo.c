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
#include <stdlib.h>

/*unsigned char mask2;      // 1 byte = 8 bits
unsigned short position;  // 2 bytes = 16 bits
unsigned long data;       // 4 bytes = 32 bits
*/

unsigned char OutVal;

unsigned int seed = 2;
unsigned int Irand;
unsigned int Irand2;
unsigned int Irand3;
unsigned int Irand4;

int returnval;

void ShortBeep()
{
  //Emit a tone over the beeper
  for (unsigned int i = 500; i!=0; i--)
  {
    //Toggle Pin
    P1OUT ^= 0x20;
    //Delay time
    __delay_cycles(1000);
  }
  P1OUT = 0;
}


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
  
  // Set P1.ALL to be correct Output/Input ports
  P1OUT = 0x00;
  //      1111_1111
  P1DIR = 0xFF;
  P1SEL = 0x00;
  
  // Setup ACLK source to be from internal VLOCLK (~10 kHz)
  BCSCTL3 |= LFXT1S_2;
  // Setup ACLK to be dvided by 8
  BCSCTL1 |= DIVA1 + DIVA0;
  
  // Sleep 20 ms
  // Initialize timerA0
  //TACTL &= TACLR;
  CCTL0 = CCIE;                             // CCR0 interrupt enabled
  CCR0 = 1500;                              // 10 khz source * 900 ms = 9,000  cycles
  TACTL = TACLR + TASSEL_1 + MC_1;                  // Source: ACLK, Mode: UP
  // Enter LPM3 Mode
  _BIC_SR(OSCOFF);
  _BIS_SR(LPM3_bits + GIE);                 // Enter LPM3 w/ interrupts
  // -- Wake up -- //

  //Setup random number generator
  srand(seed);
  // Turn DCO to slowest clock (method from Errata sheet)
  DCOCTL = 0x00;
  // Set RSEL bits
  BCSCTL1 &= 0xF0;    // 0b1111_0000 -> Clear out previous setting
  BCSCTL1 = CALBC1_8MHZ;                    // Set range
  DCOCTL = CALDCO_8MHZ;                     // Set DCO step + modulation
  BCSCTL2 = 0x00;
  //BCSCTL2 |= DIVM0; //Divide MCLK by 2  

  while (1)
 {
   // // // // // // // // // // //
   // // // Begin Mod Code // // //
   // // // // // // // // // // //
   
   //Short Beep
   ShortBeep();
   
   // Generate random numbers... 
   Irand  = (unsigned int) rand();
   Irand2 = (unsigned int) rand();
   Irand3 = (unsigned int) rand();
   Irand4 = (unsigned int) rand();
   
   //Setup random number generator
   srand( (unsigned char)Irand );
   
   //while(1)
   for (unsigned long i=500000; i!=0; i--)
   {
     //Delay Time (to reach 100kbps)
     __delay_cycles(16);
     
     //Output Value
     P1OUT = (unsigned char) (Irand & 0x02);
     
     //Toggle LED on and off == Data clock.
     P1OUT ^= 0x08;
     
     //Irand = _rotr(Irand,1);
     asm("CLRC"); //Clear Carry Bit;
     asm("RLC.W &Irand");
     asm("RLC.W &Irand2");
     asm("RLC.W &Irand3");
     asm("RLC.W &Irand4");
     asm("ADC.W &Irand");
     
     //Toggle LED on and off == Data clock.
     P1OUT ^= 0x08;
   }
   //Loops forever
   
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



