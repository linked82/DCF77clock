/*
 * ST7528.c
 *
 *  Created on: 02/06/2013
 *      Author: Mario
 */

#include  <msp430g2553.h>
#include "ST7528.h"
#include "arial.h"





/*****************************************************/

const unsigned char Init_Vector1[] =
{
	0xA2,	// Icon off
	0xAE,	// LCD off
	0x48,	// Set Duty ratio
	0x64,
	0xA0,	// Set scan direction
	0xC8,	// SHL select
	0x40,	// initial Com0 register
	0x00,	// scan from Com0
	0xAB,	// OSC on
	0x64	// 3x
};

const unsigned char Init_Vector2[] =
{
	0x65	// 4x
};

const unsigned char Init_Vector3[] =
{
	0x66	// 5x
};

const unsigned char Init_Vector4[] =
{
	0x67	// 6x
};

const unsigned char Init_Vector5[] =
{
	0x25,	// Select regulator resistor
	0x81,	// Select electronic volumen register
	55,
	0x55, 	// Select LCD BIAS
	0x92,	// FRC & PWM
	0x2C	// Sequential power 1
};


const unsigned char Init_Vector6[] =
{
	0x2E	// Sequential power 2
};


const unsigned char Init_Vector7[] =
{

	0x2F	// Sequential power 3
};



const unsigned char Init_Vector8[] =
{
	0x92,	// FRC & PWM
	0x38,   // Mode 1
	0x75,
	0x97,	// 3 FRC, 45 PWM
	0x80,	// start 16-level grayscale settings
	0x00,
	0x81,
	0x00,
	0x82,
	0x00,
	0x83,
	0x00,
	0x84,
	0x06,
	0x85,
	0x06,
	0x86,
	0x06,
	0x87,
	0x06,
	0x88,
	0x0B,
	0x89,
	0x0B,
	0x8A,
	0x0B,
	0x8B,
	0x0B,
	0x8C,
	0x10,
	0x8D,
	0x10,
	0x8E,
	0x10,
	0x8F,
	0x10,
	0x90,
	0x15,
	0x91,
	0x15,
	0x92,
	0x15,
	0x93,
	0x15,
	0x94,
	0x1A,
	0x95,
	0x1A,
	0x96,
	0x1A,
	0x97,
	0x1A,
	0x98,
	0x1E,
	0x99,
	0x1E,
	0x9A,
	0x1E,
	0x9B,
	0x1E,
	0x9C,
	0x23,
	0x9D,
	0x23,
	0x9E,
	0x23,
	0x9F,
	0x23,
	0xA0,
	0x27,
	0xA1,
	0x27,
	0xA2,
	0x27,
	0xA3,
	0x27,
	0xA4,
	0x2B,
	0xA5,
	0x2B,
	0xA6,
	0x2B,
	0xA7,
	0x2B,
	0xA8,
	0x2F,
	0xA9,
	0x2F,
	0xAA,
	0x2F,
	0xAB,
	0x2F,
	0xAC,
	0x32,
	0xAD,
	0x32,
	0xAE,
	0x32,
	0xAF,
	0x32,
	0xB0,
	0x35,
	0xB1,
	0x35,
	0xB2,
	0x35,
	0xB3,
	0x35,
	0xB4,
	0x38,
	0xB5,
	0x38,
	0xB6,
	0x38,
	0xB7,
	0x38,
	0xB8,
	0x3A,
	0xB9,
	0x3A,
	0xBA,
	0x3A,
	0xBB,
	0x3A,
	0xBC,
	0x3C,
	0xBD,
	0x3C,
	0xBE,
	0x3C,
	0xBF,
	0x3C,	// End grayscale settings
	0x38,
	0x74,
	0xAF 	// Display on

};




void t_delay (unsigned long i)
{
	// TODO: Do hardware delay

	i = i<<8;

	while (i>0)
	{
		__no_operation();
		i--;
	}
}


unsigned char * block_ptr = 0;
unsigned int block_length = 0;
unsigned int block_repeat = 0;

void block_tansfer_LCD(unsigned char A0, unsigned char * data, unsigned int length)
{
	// Copy global flags. If sending data, the flag block_repeat will enable the transmission of each byte four times.
	block_ptr = data;
	block_length = length;
	if(A0!=0)	block_repeat = 1;

    // Enable TX ready interrupt
	IE2 |= UCB0TXIE;
    // I2C TX, start condition
	UCB0CTL1 |= UCTR + UCTXSTT;
    // Write first byte: command
	UCB0TXBUF = A0;
    // Enter LPM0 w/ interrupts
	__bis_SR_register(CPUOFF + GIE);

	// Disable TX ready interrupt
	IE2 &= ~UCB0TXIE;
	// Loop until I2C STT is sent
	while (UCB0CTL1 & UCTXSTT);
	// I2C stop condition
	UCB0CTL1 |= UCTXSTP;
}


void init_LCD(void)
{
	// Send each block with required delays
	block_tansfer_LCD(Comsend, (unsigned char *) Init_Vector1, 10);
	t_delay(2);
	block_tansfer_LCD(Comsend, (unsigned char *) Init_Vector2, 1);
	t_delay(2);
	block_tansfer_LCD(Comsend, (unsigned char *) Init_Vector3, 1);
	t_delay(2);
	block_tansfer_LCD(Comsend, (unsigned char *) Init_Vector4, 1);
	t_delay(2);
	block_tansfer_LCD(Comsend, (unsigned char *) Init_Vector5, 6);
	t_delay(100);
	block_tansfer_LCD(Comsend, (unsigned char *) Init_Vector6, 1);
	t_delay(100);
	block_tansfer_LCD(Comsend, (unsigned char *) Init_Vector7, 1);
	t_delay(100);
	block_tansfer_LCD(Comsend, (unsigned char *) Init_Vector8, 135);
	t_delay(2);

}

volatile unsigned char dbuffer[2][160];


void write_line(unsigned char *text)
{
	// Caharacter index
	unsigned char n=0;
	// Byte index
	unsigned int i=0;
	// Character bitmap index
	unsigned char k=0;


	while((text[n]!='\0')&&(i<155))
	{
		for(k=0;k<((arial_8ptDescriptors[text[n]-0x20][0])<<1);k++)
		{
			dbuffer[0][i]=	arial_8ptBitmaps[arial_8ptDescriptors[text[n]-0x20][1]+k];
			dbuffer[1][i]=	arial_8ptBitmaps[arial_8ptDescriptors[text[n]-0x20][1]+k+1];
			i++;
			k++;
		}
		// Insert one line between characters
		dbuffer[0][i]=0;
		dbuffer[1][i]=0;
		i++;
		n++;
	}
	// Insert trailing blank spaces
	while(i<160)
	{
		dbuffer[0][i]= 0;
		dbuffer[1][i]= 0;
		i++;
	}
}

void send_line(unsigned char line)
{
	unsigned char page_column[] = {0xB0,0x10,0x01};
	page_column[0] = 0xB0+(line&0x0F);

	// Set address to upper line
	block_tansfer_LCD(Comsend, (unsigned char *) page_column, 3);
	t_delay(2);

	// Send upper bitmap
	block_tansfer_LCD(Datasend, (unsigned char *) dbuffer[1], 160);

	// Set address to lower line
	page_column[0] = 0xB0+(line&0x0F)+1;
	block_tansfer_LCD(Comsend, (unsigned char *) page_column, 3);
	t_delay(2);

	// Send lower bitmap
	block_tansfer_LCD(Datasend, (unsigned char *) dbuffer[0], 160);

}

void clear_line(unsigned char line)
{
	unsigned char i;
	unsigned char page_column[] = {0xB0,0x10,0x01};

	//Clear bitmap
	for(i=0;i<160;i++)
	{
		dbuffer[0][i]=0;
		dbuffer[1][i]=0;
	}

	// Set address to upper line
	page_column[0] = 0xB0+(line&0x0F);
	block_tansfer_LCD(Comsend, (unsigned char *) page_column, 3);
	t_delay(2);

	// Send upper bitmap
	block_tansfer_LCD(Datasend, (unsigned char *) dbuffer[1], 160);

	// Set address to lower line
	page_column[0] = 0xB0+(line&0x0F)+1;
	block_tansfer_LCD(Comsend, (unsigned char *) page_column, 3);
	t_delay(2);

	// Send lower bitmap
	block_tansfer_LCD(Datasend, (unsigned char *) dbuffer[0], 160);
}


void clear_screen(void)
{
	unsigned char i=0;

	while(i<14)
	{
		clear_line(i);
		i=i+2;
	}
}


#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  static unsigned int ByteCtr;

  // For data, send four times each byte for entire black
  if(block_repeat)
  {
	  UCB0TXBUF = block_ptr[ByteCtr>>2];
	  ByteCtr++;

	  if(ByteCtr>(block_length<<2))
	  {
		  __bic_SR_register_on_exit(CPUOFF + GIE);        // Exit LPM0
		  ByteCtr=0;
		  block_repeat = 0;
	  }
  }
  // For commands, normal behavior
  else
  {
	  UCB0TXBUF = block_ptr[ByteCtr++];          // Transmit data byte

	  if(ByteCtr>block_length)
	  {
		  __bic_SR_register_on_exit(CPUOFF + GIE);        // Exit LPM0
		  ByteCtr=0;
	  }
  }
}


