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

unsigned int seed = 2;
unsigned int Irand;
unsigned int Qrand;
unsigned int RorC;


unsigned char count;


//Taken from Wikipedia article "Circular Shift"
/* assumes number of bits in an unsigned int is 32 */
 
unsigned int _rotl(unsigned int value, int shift) {
    if ((shift &= 31) == 0)
      return value;
    return (value << shift) | (value >> (16 - shift));
}
 
unsigned int _rotr(unsigned int value, int shift) {
    if ((shift &= 31) == 0)
      return value;
    return (value >> shift) | (value << (16 - shift));
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
  //      1111_0110
  P1DIR = 0xF6;
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
  
  while (1)
 {
  // -- Reset Clock to Lo Power settings
  // Turn DCO to slowest clock (method from Errata sheet)
  DCOCTL = 0x00;
  // Set RSEL bits
  BCSCTL1 &= 0xF0;    // 0b1111_0000 -> Clear out previous setting
  /*BCSCTL1 |= 0x00;    // Place new setting for RSEL : 0b0000_xxxx
  DCOCTL |= 0x00;     // Place new setting for DCO  : 0bxxx0_0000*/
  BCSCTL2 = 0x00;
  BCSCTL2 |= DIVM0; //Divide MCLK by 2
  
  /* 
  // Generate 5.0 kHz tone -- Tone 1
   for ( unsigned short i = 50 ; i != 0 ; i-- )
   {
     // Toggle Pin ON
     P1OUT = 0x20;
     // Delay for 1/2 period of 2.5 kHz (10 cycles)
     //__delay_cycles(1);
     
     // Toggle Pin OFF
     P1OUT = 0x00;
     // Delay
     //__delay_cycles(2);
   }*/
  
   // Sleep  LONG ms
   // Setup ACLK source to be from internal VLOCLK (~10 kHz)
  BCSCTL3 |= LFXT1S_2;
  // Setup ACLK to be dvided by 8
  BCSCTL1 |= DIVA1 + DIVA0;
   // Initialize timerA0
   //TACTL &= TACLR;
   CCTL0 = CCIE;                             // CCR0 interrupt enabled
   CCR0 = 900;                              // 10 khz source * 900 ms = 9,000  cycles
   TACTL = TACLR + TASSEL_1 + MC_1;                  // Source: ACLK, Mode: UP

   // Enter LPM3 Mode
   _BIC_SR(OSCOFF);
   _BIS_SR(LPM3_bits + GIE);                 // Enter LPM3 w/ interrupts
   // -- Wake up -- //

   // Turn OFF SMCLK and external Oscillator
   _BIS_SR(OSCOFF + SCG1);
   
   // // // // // // // // // // //
   // // // Begin I/Q Code // // //
   // // // // // // // // // // //

   //8Mhz Clock
   // Turn DCO to slowest clock (method from Errata sheet)
   DCOCTL = 0x00;
   // Set RSEL bits
   BCSCTL1 &= 0xF0;    // 0b1111_0000 -> Clear out previous setting
   BCSCTL1 = CALBC1_16MHZ;                    // Set range
   DCOCTL = CALDCO_16MHZ;                     // Set DCO step + modulation */
   BCSCTL2 = 0x00;
   //BCSCTL2 |= DIVM0; //Divide MCLK by 2
  
   count = 0;
   
   // Generate random numbers.
   Irand = (unsigned int)rand();
   Qrand = (unsigned int)rand();
   
   // Ramp Up
   for ( unsigned short cycles = 250 ; cycles != 0 ; cycles-- )
   {
     P1OUT ^= 0x06;
     __delay_cycles(32);
   }
   P1OUT = 0x00;
   //Delay
   __delay_cycles(200);
   
   //Q ONLY modulation
   for ( unsigned short cycles = 300 ; cycles != 0 ; cycles-- )
   {
     P1OUT ^= 0x04;
     __delay_cycles(32);
   }
   P1OUT = 0x00;
   //Delay
   __delay_cycles(200);
   
   //I ONLY modulation
   for ( unsigned short cycles = 300 ; cycles != 0 ; cycles-- )
   {
     P1OUT ^= 0x02;
     __delay_cycles(32);
   }
   P1OUT = 0x00;
   //Delay
   __delay_cycles(200);
   
   
   // I AND Q modulation
   for ( unsigned short cycles = 300 ; cycles != 0 ; cycles-- )
   {
     OutVal = (unsigned char) ((Irand & 0x01) << 1) | ((Qrand & 0x01) << 2);
     //Irand = _rotr(Irand,1);
     asm("RRC.W &Irand");
     RorC = 0x00;
     asm("RRC.W &RorC");
     Irand = Irand | RorC;
     
     asm("");
     //Qrand = _rotl(Qrand,1);
     asm("RLC.W &Qrand");
     RorC = 0x00;
     asm("ADC.W &RorC");
     Qrand = Qrand | RorC;

     P1OUT = OutVal;

     // Toggle P1.1's state (I)
     //0x0000_0010
     //I
     //P1OUT ^= 0x02;
     //Q
     //P1OUT ^= 0x04;
     //__delay_cycles(25);
     
     // Toggle P1.2's state (Q)
     //++count;
     //if(count == 5)
     //{
       //0x0000_0100
       //I
       //P1OUT ^= 0x02;
       //Q
       //P1OUT ^= 0x04;
       //count = 0;
     //}
     //else
     //{
     //  __delay_cycles(10);
     //}
     
     //__delay_cycles(2);
   }
   P1OUT = 0x00;
   seed = Irand;
   
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

