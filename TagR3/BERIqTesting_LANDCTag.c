//******************************************************************************
// MSP430F2011 Program
//
// I/Q Data Transmission -- L&C
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

unsigned int seed = 2;
unsigned int Irand;
unsigned int Qrand;
unsigned int Irand2;
unsigned int Qrand2;
unsigned int Irand3;
unsigned int Qrand3;
unsigned int Irand4;
unsigned int Qrand4;
unsigned int RorC;


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
  srand(seed);
  //8Mhz Clock
  // Turn DCO to slowest clock (method from Errata sheet)
  DCOCTL = 0x00;
  // Set RSEL bits
  BCSCTL1 &= 0xF0;    // 0b1111_0000 -> Clear out previous setting
  BCSCTL1 = CALBC1_16MHZ;                    // Set range
  DCOCTL = CALDCO_16MHZ;                     // Set DCO step + modulation */
  BCSCTL2 = 0x00;
  //BCSCTL2 |= DIVM0; //Divide MCLK by 2  

  while (1)
 {
   // // // // // // // // // // //
   // // // Begin I/Q Code // // //
   // // // // // // // // // // //

   //count = 0;
   
   // Generate random numbers... which are actually hardcoded
   // Start Bits:
   //  I: 0011 0011 (LSByte of Irand)
   //  Q: 1111 0000 (LSByte of Qrand)

   //Assign 'random' values (It's ok! They were generated by a random dice throw. Guaranteed to be random!)
   Irand  = 0x1933;
   Irand2 = 0x9DFE;
   Irand3 = 0xCC5F;
   Irand4 = 0x594B;
   
   Qrand  = 0xCAF0;
   Qrand2 = 0xBBBE;
   Qrand3 = 0x85A8;
   Qrand4 = 0xF994;
   
   //Rotate I to the left once (Since it transmits on 1.1)
   asm("CLRC"); //Clear Carry Bit;
   asm("RLC.W &Irand");
   asm("RLC.W &Irand2");
   asm("RLC.W &Irand3");
   asm("RLC.W &Irand4");
   asm("ADC.W &Irand");


   //Rotate Q to the left twice (Since it transmits on 1.2)
   asm("CLRC"); //Clear Carry Bit;
   asm("RLC.W &Qrand");
   asm("RLC.W &Qrand2");
   asm("RLC.W &Qrand3");
   asm("RLC.W &Qrand4");
   asm("ADC.W &Qrand");
   
   asm("CLRC"); //Clear Carry Bit;
   asm("RLC.W &Qrand");
   asm("RLC.W &Qrand2");
   asm("RLC.W &Qrand3");
   asm("RLC.W &Qrand4");
   asm("ADC.W &Qrand");   
   
   
   // I AND Q modulation
   // -- Toggle LED 3 (Yellow)-- //
   //P1OUT ^= 0x40;   
   //for ( unsigned short cycles = 300 ; cycles != 0 ; cycles-- )
   while(1)
   {
     P1OUT = (unsigned char) ((Irand & 0x02) | (Qrand & 0x04));
     
     //Toggle LED on and off == Data clock.
     P1OUT ^= 0x08;
     
     //Irand = _rotr(Irand,1);
     asm("CLRC"); //Clear Carry Bit;
     asm("RLC.W &Irand");
     asm("RLC.W &Irand2");
     asm("RLC.W &Irand3");
     asm("RLC.W &Irand4");
     asm("ADC.W &Irand");
     
     //RorC = 0x00; // Clear out Carry Bit
     //Qrand = rotl(Qrand,1);
     asm("CLRC"); //Clear Carry Bit;
     asm("RLC.W &Qrand");
     asm("RLC.W &Qrand2");
     asm("RLC.W &Qrand3");
     asm("RLC.W &Qrand4");
     asm("ADC.W &Qrand");
     //asm("ADC.W &Irand");
     
     //Toggle LED on and off == Data clock.
     P1OUT ^= 0x08;
     
     //Output Value found above
     //P1OUT = OutVal;

     //Toggle LED off.
     //P1OUT ^= 0x08;
   }
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

