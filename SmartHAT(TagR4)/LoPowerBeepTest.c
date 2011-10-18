//******************************************************************************
// MSP430F2011 Program
//
// Code for 915 MHz Transmit Board
// Test program for LEDs, Beeper, Correct LAdjust levels
//
//
//******************************************************************************

#include  <msp430x20x1.h>
#include "./SmartHatDefinitions.h"

/*unsigned char mask2;      // 1 byte = 8 bits
unsigned short position;  // 2 bytes = 16 bits
unsigned long data;       // 4 bytes = 32 bits
*/

unsigned char counter;
unsigned char BEEPCYCLES = 75;

void main(void)
{
  // // // // // // // //
  // // CLOCK SETUP // //
  // // // // // // // //
  
  // Turn DCO to slowest clock (method from Errata sheet)
  DCOCTL = 0x00;
  // Set RSEL bits
  BCSCTL1 &= 0xF0;    // 0b1111_0000 -> Clear out previous setting
  BCSCTL2 |= SELM_3 + DIVM_3;  //MCLK is sourced by LFTX1CLK and div by 8

  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  
  // Turn OFF SMCLK and DC Generator
  _BIS_SR(SCG1 + SCG0);
  
  // Set P1.ALL to be correct Output/Input ports
  P1OUT = 0x00; //(Output Buffer)
  //      1111_1110 --1 for out, 0 for in
  P1DIR = (0xFF);
  P1SEL = 0x00; //1 for special function on pin

  //Begin Code
  P1SEL |= BIT5; //Turn On Output for pin 1.5 (TA0)
  CCTL0 = OUTMOD_4; //Set to Toggle Output (pin 1.5)
  TACTL = TASSEL_1 + MC_1; //ACLK, UP Mode
  
  CCR0 = 4; //9 cycles ~= 3.6kHz
  TACTL |= TACLR;
  
  while(1)
  {
    //for(int i = 1; i < 2 ; ++i){
    CCR0 = 3;  
    __delay_cycles(1000);
    CCTL0 = 0;
    __delay_cycles(500);
    CCR0 = 3;
    CCTL0 = OUTMOD_4;
    __delay_cycles(2000);
    //}
    //CCR0 = 3;
    //__delay_cycles(2300);
    //CCR0 = 3;
    //__delay_cycles(1500);
    //CCR0 = 8;
    //__delay_cycles(800);
    //CCR0 = 9;
    //__delay_cycles(800);
    //__delay_cycles(2500);
    CCTL0 = 0;
    __delay_cycles(8000);
    CCTL0 = OUTMOD_4;
    //if (--CCR0 == 8)
    //  CCR0 = 10;
  }
  
}

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A0 (void)
{
 
}



