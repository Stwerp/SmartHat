//******************************************************************************
// MSP430F2011 Program
// PAM GENERATOR FOR C-ONLY CASE!!!
// 2.5 kHz Generator
//
//  The program sleeps for 900 ms in LPM3 mode, then wakes up and outputs
//  a 2.5 kHz square wave tone on P1OUT.0 for 100 ms
//
//******************************************************************************

#include  <msp430x20x1.h>

/*unsigned char mask2;      // 1 byte = 8 bits
unsigned short position;  // 2 bytes = 16 bits
unsigned long data;       // 4 bytes = 32 bits
*/

unsigned char Data;
const unsigned char MASK = 0x0F;

// Look Up Table, populated with randomly generated values corresponding to
//  the 4 values that generate a PAM signal with the C-Only 16 QAM tag
static const unsigned char LUT[256] = 
{3, 7, 3, 2, 2, 7, 2, 6, 2, 7, 3, 7, 7, 7, 2, 2, 7, 3, 7, 7, 2, 2, 7, 6, 7,
2, 2, 7, 2, 2, 6, 3, 3, 3, 2, 2, 2, 6, 3, 2, 6, 6, 6, 2, 3, 2, 6, 2, 6, 6,
2, 2, 3, 6, 6, 6, 3, 3, 2, 6, 7, 6, 6, 7, 3, 3, 6, 7, 3, 7, 7, 2, 3, 7, 3,
3, 6, 7, 7, 6, 2, 3, 6, 7, 2, 7, 2, 7, 2, 7, 7, 7, 7, 2, 6, 6, 2, 6, 3, 7,
7, 2, 2, 6, 2, 3, 6, 6, 2, 3, 2, 2, 3, 7, 3, 2, 7, 7, 2, 2, 7, 2, 7, 7, 6,
3, 6, 3, 7, 7, 6, 7, 3, 6, 3, 6, 7, 3, 7, 2, 3, 2, 7, 7, 6, 6, 3, 2, 3, 3,
7, 2, 3, 2, 3, 6, 3, 2, 2, 2, 7, 6, 3, 2, 6, 2, 6, 6, 7, 6, 2, 3, 6, 2, 3,
3, 7, 2, 2, 6, 2, 2, 3, 6, 7, 7, 3, 7, 3, 6, 7, 3, 7, 7, 6, 7, 6, 7, 6, 2,
3, 7, 6, 7, 2, 2, 2, 3, 7, 3, 3, 3, 7, 6, 7, 3, 2, 6, 3, 7, 7, 6, 3, 2, 6,
3, 3, 2, 2, 6, 2, 2, 7, 6, 7, 2, 3, 6, 3, 3, 7, 6, 6, 7, 3, 7, 2, 3, 3, 2,
6, 2, 3, 2, 2, 3 };

void main(void)
{

  // Disable Entire System
//  _BIS_SR(LPM4_bits);
  
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
  CCR0 = 900;                              // 10 khz source * 900 ms = 9,000  cycles
  TACTL = TACLR + TASSEL_1 + MC_1;                  // Source: ACLK, Mode: UP
  // Enter LPM3 Mode
  _BIC_SR(OSCOFF);
  _BIS_SR(LPM3_bits + GIE);                 // Enter LPM3 w/ interrupts
  // -- Wake up -- //
  
  // Turn OFF SMCLK and external Oscillator
  _BIS_SR(OSCOFF + SCG1);
  
 while (1)
 {
   // Generate 5.0 kHz tone -- Tone 1
   for ( unsigned short i = 175 ; i != 0 ; i-- )
   {
     // Toggle Pin ON
     P1OUT ^= 0x20;
     // Delay for 1/2 period of 2.5 kHz (10 cycles)
     //__delay_cycles(1);
     
     // Toggle Pin OFF
     P1OUT ^= 0x00;
     // Delay
     __delay_cycles(1);
   }
  
   BCSCTL1 = CALBC1_16MHZ;                    // Set range
   DCOCTL = CALDCO_16MHZ;                     // Set DCO step + modulation */
   //16-QAM Testing
   Data = 0;
   while(1)
   {
     //P1OUT = Data++;
     P1OUT = (MASK & LUT[Data++]);
     //__delay_cycles(5);
   }
   
   
   /*
   // -- Toggle A0 / (LT5505 Input pin) -- //
   //P1OUT ^= 0x01;
   
   // -- Toggle A1 --//
   //P1OUT ^= 0x02;
   
   // -- Toggle RCV/MOD' -- //
   P1OUT ^= 0x04;
   
   // -- Toggle LT5501 Power pin -- //
   //P1OUT ^= 0x80;


   // -- Toggle LED 1 (Red) -- //
   P1OUT ^= 0x08;

   // -- Toggle LED 2 (Green) -- //
   P1OUT ^= 0x10;
   
   // -- Toggle LED 3 (Yellow)-- //
   P1OUT ^= 0x40;
   */
   
   // Sleep  LONG ms
   // Initialize timerA0
   //TACTL &= TACLR;
   CCTL0 = CCIE;                             // CCR0 interrupt enabled
   CCR0 = 700;                              // 10 khz source * 900 ms = 9,000  cycles
   TACTL = TACLR + TASSEL_1 + MC_1;                  // Source: ACLK, Mode: UP

   // Enter LPM3 Mode
   _BIC_SR(OSCOFF);
   _BIS_SR(LPM3_bits + GIE);                 // Enter LPM3 w/ interrupts
   // -- Wake up -- //

   // Turn OFF SMCLK and external Oscillator
   _BIS_SR(OSCOFF + SCG1);
   
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

