/*
 * serial.h
 *
 * Created: 31-Oct-19 14:31:13
 *  Author: Karen Arkojan & Shafik Hoshan
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>

#define F_CPU 16000000
#define BAUD 9600
#include <util/setbaud.h>

#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

void uart_init(void);
unsigned char uart_recieve(void);
void uart_transmit_char(char data);
void uart_transmit_int(int data);
void uart_transmit_string (char data[]);
void serialSend(char* sendString);
uint8_t* insert_data_from_pyhton(uint8_t from_sensor);

#endif /* SERIAL_H_ */