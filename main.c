/*
 * main.c
 */

#include  <msp430g2553.h>
#include <time.h>

volatile unsigned long long wheel=0;
volatile unsigned long long DCFdata=0;
volatile unsigned char pointer=0;
struct tm DCFtime;


int checksumDCF (unsigned char start, unsigned char end)
{
	int retval=0;
	unsigned char i;
	unsigned long long j= ((DCFdata>>start)&0x01);
	start++;
	for(i=start;i<=end;i++)
	{
		j^=((DCFdata>>i)&0x01);
	}
	if(j==1)
		retval = -1;
	return (retval);
}



void main(void)
{


	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	BCSCTL1 = CALBC1_8MHZ;                    // Set DCO to 8MHz
	DCOCTL =  CALDCO_8MHZ;


	P1DIR |= 0x01;                            // P1.0 output
	P2REN |= BIT1+BIT0;
	P2OUT |= BIT1+BIT0;
	P2DIR &= ~BIT1+BIT0;
	P2SEL &= ~BIT1+BIT0;
	P2SEL2 &= ~BIT1+BIT0;
	P2IES = 0;

	TAR = 0;
	CCTL0 = CCIE;                             // CCR0 interrupt enabled
	CCR0 = 32767;
	TACTL = TASSEL_1 + MC_1;                  // ACLK, upmode


	P2IFG = 0;
	P2IE = BIT0;
	P2SEL |= BIT1;
	P2SEL2 |= BIT1;

	TA1CCR0 = 60000;							// Up to two seconds



	while(1)
	{
		_BIS_SR(LPM0_bits + GIE);                 // Enter LPM0 w/ interrupt

		__no_operation();

		checksumDCF(0,22);
		checksumDCF(23,29);
		checksumDCF(30,37);

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
		DCFtime.tm_wday -= 1;			 /* days since Sunday          - [0,6]   */

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
	if(pointer>=58)
	{
		DCFdata = wheel;
		__bic_SR_register_on_exit(CPUOFF);
	}
	pointer = 0;
	wheel = 0;
	TA1R = 0;
	TA1CCTL0 = 0;
	TA1CCTL1 = 0;
	TA1CTL = TASSEL_1 + MC_0;
}


// Timer A0 interrupt service routine
#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer1_A1 (void)
{
	unsigned int read;

	switch( TA1IV )
	{
	case  2:

		read = TA1CCR1;

//		if((read&CCI)==CCI)
//		{
//			TA1R = 0;
//		}
//		else
//		{
			if(read>2000)
			{
				// 0
				if(read<4000)
				{
					pointer++;
					wheel = wheel<<1;
				}
				else
				{
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
	P2IFG = 0;

	TA1R = 0;
	TA1CCTL0 = CCIE;
	TA1CCTL1 = CM_2 + CCIS_0 + CCIE + CAP;     	// CCR0 interrupt enabled
	TA1CTL = TASSEL_1 + MC_1;                  	// ACLK, upmode



}


