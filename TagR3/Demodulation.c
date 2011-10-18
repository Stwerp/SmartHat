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
#include  <stdlib.h>

/*unsigned char mask2;      // 1 byte = 8 bits
unsigned short position;  // 2 bytes = 16 bits
unsigned long data;       // 4 bytes = 32 bits
*/

unsigned char OutVal;

unsigned char Demod;
/*unsigned int seed = 2;
unsigned int Irand;
unsigned int Qrand;
unsigned int Irand2;
unsigned int Qrand2;
unsigned int Irand3;
unsigned int Qrand3;
unsigned int Irand4;
unsigned int Qrand4;
unsigned int RorC;*/


unsigned char count;

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
  //srand(seed);
  //8Mhz Clock
  // Change DCO clock(method from Errata sheet)
  DCOCTL = 0x00;
  // Set RSEL bits
  BCSCTL1 &= 0xF0;    // 0b1111_0000 -> Clear out previous setting
  // 16MHz clock
  BCSCTL1 = CALBC1_16MHZ;                    // Set range
  DCOCTL = CALDCO_16MHZ;                     // Set DCO step + modulation */
  BCSCTL2 = 0x00;
  //BCSCTL2 |= DIVM0; //Divide MCLK by 2  

  while (1)
 {
   // // // // // // // // // // //
   // // //   Demod Code   // // //
   // // // // // // // // // // //
   P1OUT = 0x00;
/*  // Pins
  P1.0 -> A0(Mod) / Input from LT5505 (demod)
   1.1 -> A1 (Mod)
   1.2 -> RCV/MOD' (Demod '1', Modulate '0'
   1.3 -> LED1 (Red)
   1.4 -> LED2 (Grn)
   1.5 -> Beeper
   1.6 -> LED3 (Yel)
   1.7 -> LT5505En (Power for 5505)
*/
   
   
   //Test 1:
   // turn on everything, constantly sample and set LED1 to be state
   
   P1DIR = 0xFE; //Set P1.0 to be input
   P1OUT |= 0x84; //Turn on RCV mode and Enable LTT5505
   
   while(1)
   {
     if ( ( P1IN & 0x01 ) == ( 0x01 ) )
       P1OUT |= 0x08; //Turn ON Led1
     else
       P1OUT &= 0xF7; //Turn OFF LED1
     //Loop
   }
   
   

   // -- Toggle LED 3 (Yellow)-- //
   //P1OUT ^= 0x40;   
   P1OUT = 0x00;   
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

