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
	UCSR0A = 0;
	UCSR0B = _BV(TXEN0) | _BV(RXEN0);
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}

// function to receive data
unsigned char uart_receive (void)
{
	while(!(UCSR0A) & (1<<RXC0));	// wait while data is being received
	return UDR0;					// return 8-bit data
}


// function to send data
void uart_transmit_char (char data) 
{
	// wait while register is free
	while (!( UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

void uart_transmit_string (char data[]){
	for(int i=0;strlen(data)>i;i++){
		uart_transmit_char(data[i]);
	}
}
void uart_transmit_int (int data){
	char getal[5];
	itoa(data,getal,10);
	uart_transmit_string(getal);
}

void line_break (void){
	uart_transmit_char('\n');
}