/*
 * serial.c
 *
 * Created: 31-Oct-19 14:29:58
 *  Author: Karen Arkojan & Shafik Hoshan
 */

#include "serial.h"
#include <stdlib.h>
#include <string.h>

void uart_init(void) 
{
	    UBRR0H = UBRRH_VALUE;
	    UBRR0L = UBRRL_VALUE;

	    #if USE_2X
	    UCSR0A |= _BV(U2X0);
	    #else
	    UCSR0A &= ~(_BV(U2X0));
	    #endif

	    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
	
}

char uart_getchar(void) {
	loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
	return UDR0;
}


void uart_putchar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
	UDR0 = c;
	//loop_until_bit_is_set(UCSR0A, TXC0);/* Wait until transmission ready. */
}

void uart_transmit_string (char data[]){
	for(int i=0;strlen(data)>i;i++){
		uart_putchar(data[i]);
	}
}
void uart_transmit_int (int data){
	char getal[5];
	itoa(data,getal,10);
	uart_transmit_string(getal);
}

void line_break (void){
	uart_putchar('\n');
}