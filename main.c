/*
 * main.c
 */

#include <stdio.h>
#include <msp430g2553.h>
#include <time.h>
#include "ST7528.h"

	// Variables for storing the received bit bang data, up to 64 bits
volatile unsigned long long wheel=0;
volatile unsigned long long DCFdata=0;
	// Pointer to last received bit
volatile unsigned char pointer=0;
	// Represented DCF time in non-condensated form as specified in time.h
struct tm DCFtime;


/*
 * Function to check that the received checksums match with the sum of specified bits
 * Returns 0 if OK, and -1 if fails.
 *
 */
int checksumDCF (unsigned char start, unsigned char end)
{
	// Return value
	int retval=0;
	// Index
	unsigned char i;
	// Start of the bit walk through
	unsigned long long j= ((DCFdata>>start)&0x01);
	start++;

	// Loop XOR-ing selected bits, including the checksum, that matches the final bit
	for(i=start;i<=end;i++)
	{
		j^=((DCFdata>>i)&0x01);
	}

	// The sum must be odd, but when XOR-ed with the incoming checksum, it must result as a zero
	if(j==1)
		retval = -1;
	return (retval);
}



void main(void)
{
	// Output buffer for text representation on screen
	char text[30];

	// Stop WDT
	WDTCTL = WDTPW + WDTHOLD;

	// Set DCO to 8MHz
	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL =  CALDCO_8MHZ;

	// Launchpad LED pin as output
	P1DIR |= 0x01;

	// DCF77 pins as inputs with on interrupt and CCR1 input
	P2REN |= BIT1+BIT0;
	P2OUT |= BIT1+BIT0;
	P2DIR &= ~BIT1+BIT0;
	P2SEL &= ~BIT1+BIT0;
	P2SEL2 &= ~BIT1+BIT0;
	P2IES = 0;

	P2IFG = 0;
	P2IE = BIT0;
	P2SEL |= BIT1;
	P2SEL2 |= BIT1;

	// Display pins for the ST7528 display
	P1SEL |= BIT6 + BIT7;                 	// Assign I2C pins to USCI_B0
	P1SEL2|= BIT6 + BIT7;                   // Assign I2C pins to USCI_B0

	P2SEL &= ~(BIT4 + BIT5);				// RST and CSB outputs
	P1SEL &= ~(BIT4 + BIT5);
	P2DIR |= (BIT4 + BIT5);
	P2OUT &= ~(BIT5);
	P2OUT |= BIT4;

	UCB0CTL1 |= UCSWRST;                     // Enable SW reset
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;    // I2C Master, synchronous mode
	UCB0CTL1 = UCSSEL_2 + UCSWRST;           // Use SMCLK, keep SW reset
	UCB0BR0 = 20;                            // fSCL = SMCLK/20 = 400kHz
	UCB0BR1 = 0;
	UCB0I2CSA = 0x3F;                        // Set slave address
	UCB0CTL1 &= ~UCSWRST;                    // Clear SW reset, resume operation

	// Timer1 and CCR0
	TAR = 0;
	CCTL0 = CCIE;                             // CCR0 interrupt enabled
	CCR0 = 32767;
	TACTL = TASSEL_1 + MC_1;                  // ACLK, upmode

	TA1CCR0 = 60000;							// Up to two seconds


	// Start

	// Reset display
	CLR_RST;
	CLR_CSB;
	t_delay(10);
	SET_RST;
	t_delay(10);
	SET_CSB;
	t_delay(10);

	// Init LCD. Interrupts are enabled inside
	init_LCD();

	//Clear entire screen
	clear_screen();

	// Transform text to bitmap
	write_line((unsigned char *) "DCF77 CLOCK");
	// Send bitmap buffer to desired line
	send_line(0);



	while(1)
	{
		// Wait for sinchronisation
		_BIS_SR(LPM0_bits + GIE);
		// For debug purposes
		__no_operation();

		// Incoming DCF77 data. If no errors, then convert it to text and display on screen
		if((!checksumDCF(0,22)) && (!checksumDCF(23,29)) && (!checksumDCF(30,37)))
		{
			DCFtime.tm_year =  ((DCFdata>>1) & 0x1)*80;
			DCFtime.tm_year += ((DCFdata>>2) & 0x1)*40;
			DCFtime.tm_year += ((DCFdata>>3) & 0x1)*20;
			DCFtime.tm_year += ((DCFdata>>4) & 0x1)*10;
			DCFtime.tm_year += ((DCFdata>>5) & 0x1)*8;
			DCFtime.tm_year += ((DCFdata>>6) & 0x1)*4;
			DCFtime.tm_year += ((DCFdata>>7) & 0x1)*2;
			DCFtime.tm_year += ((DCFdata>>8) & 0x1);
			DCFtime.tm_year += 100;			 /* years since 1900                     */

			DCFtime.tm_mon = ((DCFdata>>9)  & 0x1)*10;
			DCFtime.tm_mon += ((DCFdata>>10) & 0x1)*8;
			DCFtime.tm_mon += ((DCFdata>>11) & 0x1)*4;
			DCFtime.tm_mon += ((DCFdata>>12) & 0x1)*2;
			DCFtime.tm_mon += ((DCFdata>>13) & 0x1);
			DCFtime.tm_mon -= 1;		 	/* months since January       - [0,11]  */

			DCFtime.tm_wday = ((DCFdata>>14) & 0x1)*4;
			DCFtime.tm_wday += ((DCFdata>>15) & 0x1)*2;
			DCFtime.tm_wday += ((DCFdata>>16) & 0x1);
			if(DCFtime.tm_wday==7)	DCFtime.tm_wday=0;
											/* days since Sunday          - [0,6]   */

			DCFtime.tm_mday = ((DCFdata>>17)  & 0x1)*20;
			DCFtime.tm_mday += ((DCFdata>>18)  & 0x1)*10;
			DCFtime.tm_mday += ((DCFdata>>19) & 0x1)*8;
			DCFtime.tm_mday += ((DCFdata>>20) & 0x1)*4;
			DCFtime.tm_mday += ((DCFdata>>21) & 0x1)*2;
			DCFtime.tm_mday += ((DCFdata>>22) & 0x1);
											/* day of the month           - [1,31]  */

			DCFtime.tm_hour = ((DCFdata>>24)  & 0x1)*20;
			DCFtime.tm_hour += ((DCFdata>>25)  & 0x1)*10;
			DCFtime.tm_hour += ((DCFdata>>26) & 0x1)*8;
			DCFtime.tm_hour += ((DCFdata>>27) & 0x1)*4;
			DCFtime.tm_hour += ((DCFdata>>28) & 0x1)*2;
			DCFtime.tm_hour += ((DCFdata>>29) & 0x1);
			 	 	 	 	 	 	 	 	 /* hours after the midnight   - [0,23]  */

			DCFtime.tm_min = ((DCFdata>>31)  & 0x1)*40;
			DCFtime.tm_min += ((DCFdata>>32)  & 0x1)*20;
			DCFtime.tm_min += ((DCFdata>>33)  & 0x1)*10;
			DCFtime.tm_min += ((DCFdata>>34) & 0x1)*8;
			DCFtime.tm_min += ((DCFdata>>35) & 0x1)*4;
			DCFtime.tm_min += ((DCFdata>>36) & 0x1)*2;
			DCFtime.tm_min += ((DCFdata>>37) & 0x1);
											/* minutes after the hour     - [0,59]  */

			DCFtime.tm_sec = 0;

			clear_line(2);
			strftime( text, 30, "%A %d, %B, %Y", &DCFtime );
			write_line((unsigned char *) text);
			// Send bitmap buffer to desired line
			send_line(2);

			clear_line(4);
			strftime( text, 30, "%H:%M:%S", &DCFtime );
			write_line((unsigned char *) text);
			// Send bitmap buffer to desired line
			send_line(4);
		}
	}
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
{
	P1OUT ^= 0x01;                            // Toggle P1.0
}





// Timer A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void)
{
	// When there is no bit mark, the pointer must have reached 58 positions in the wheel,
	// thus completing the end of the data received
	if(pointer>=58)
	{
		DCFdata = wheel;
		__bic_SR_register_on_exit(CPUOFF);
	}

	// Stop counting and reset variables
	pointer = 0;
	wheel = 0;
	TA1R = 0;
	TA1CCTL0 = 0;
	TA1CCTL1 = 0;
	TA1CTL = TASSEL_1 + MC_0;
}


// Timer A1 interrupt service routine
#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer1_A1 (void)
{
	unsigned int read;

	switch( TA1IV )
	{
	case  2:	// CCR1

		read = TA1CCR1;

//		if((read&CCI)==CCI)
//		{
//			TA1R = 0;
//		}
//		else
//		{
		// Distinguish 0's and 1's with time window
			if(read>2000)
			{
				// This is a '0', theoretically 3267 cycles
				if(read<4000)
				{
					pointer++;
					wheel = wheel<<1;
				}
				else
				{	//This is a '1', theoretically 6553 cycles
					if(read<7000)
					{
						pointer++;
						wheel = wheel<<1;
						wheel |= 0x01;
					}
				}

			}
//		}
	//__bic_SR_register_on_exit(CPUOFF + GIE);
	break;
//	case 0x0A:
//	break;

	}
}




// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
	// On positive edge of interrupt, start measuring the width of the pulse
	// The TA1CCR1 must capture the negative edge

	// Reset interrupts
	P2IFG = 0;

	// Start Timer1 A1
	TA1R = 0;
	TA1CCTL0 = CCIE;
	TA1CCTL1 = CM_2 + CCIS_0 + CCIE + CAP;     	// CCR0 interrupt enabled
	TA1CTL = TASSEL_1 + MC_1;                  	// ACLK, upmode



}


