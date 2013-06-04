/*
 * ST7528.h
 *
 *  Created on: 02/06/2013
 *      Author: Mario
 */

#ifndef ST7528_H_
#define ST7528_H_


/*
 * Definition for GPIO
 */
#define SET_RST		P2OUT|=BIT5
#define CLR_RST		P2OUT&=~BIT5
#define SET_CSB		P2OUT|=BIT4
#define CLR_CSB		P2OUT&=~BIT4


/*
 * Definition for Address bytes
 */
#define Comsend 0x00
#define Datasend 0x40



// Headers


void t_delay (unsigned long i);

void init_LCD(void);

void write_line(unsigned char *text);

void send_line(unsigned char line);

void clear_line(unsigned char line);

void clear_screen(void);




#endif /* ST7528_H_ */
