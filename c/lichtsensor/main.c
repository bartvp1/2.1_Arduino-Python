/*
 * lichtsensor.c
 *
 * Created: 31-Oct-19 12:21:19
 * Author: Karen Arkojan & Shafik Hoshan
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include "display.h"
#include "schedular.h"
#include "serial.h"


const int trigPin = 2;      // Trigger		PD2
const int echoPin = 3;      // Echo			PD3
const int RLED = 4;			// Red LED		PD4
const int YLED = 5;			// Yellow LED	PD5
const int GLED = 6;			// Green LED	PD6
const int lichtsensor = 0;  // light sensor PA0

volatile uint16_t gv_counter;   // 16 bit counter
volatile uint8_t gv_echo;		// a flag
volatile uint16_t afstand;		// distance to roller shutter
volatile double licht_intens;	// value of light sensitivity

uint8_t afstand_max = 65;	// max distance roller shutter
uint8_t afstand_min = 5;	// min distance roller shutter
uint8_t licht_drempelwaarde = 60;	// hierboven moet het rolgordijn dicht
uint8_t manual_bool = 0;	// make decisions based on sensor data or not
typedef enum{NONE = -1, INROLLEN = 0, UITROLLEN = 1, INGEROLD = 2, UITGEROLD = 3} mode_t;
mode_t mode = NONE;

/*
 * initialize PORTB and PORTD
 */
void setup(void)
{
  // Set Trigger to OUTPUT, Echo to INPUT, Red LED to OUTPUT, Yellow LED to OUTPUT, Green LED to OUTPUT
  DDRD = (1<<trigPin) | (0<<echoPin) | (1<<RLED) | (1<<YLED) | (1<<GLED);
  
  // Set Clock to OUTPUT, Strobe to OUTPUT, Data to OUTPUT
  DDRB = (1<<strobe) | (1<<clock) | (1<<data);
  
  sendCommand(0x89); // activate and set brightness to medium
}

/*
 * initialize external interrupt 1
 */ 
void interr(void)
{
  // any change triggers ext interrupt 1
  EICRA = (1 << ISC10);
  EIMSK = (1 << INT1);
}

/*
 * initialize ADC (analog-digital-converter)
 */
void analog_dig_conv(void)
{
  // turn on channels
  ADMUX = (1<<REFS0);
   //enable ADC and prescale = 128
  ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

/*
 * read value from PADC
 * parameter: ADC_pin is a 0-7 option to choose a pin to read
 */
uint16_t adc_val(uint8_t ADC_pin)
{
  // Clear the previously read channel.
  ADC_pin &= 0b0000111;
  ADMUX = (ADMUX & 0xF8) | ADC_pin;
  ADCSRA |= (1<<ADSC); // start conversion
  while(ADCSRA & (1<<ADSC));
  //loop_until_bit_is_clear(ADCSRA, ADSC);
  if (PORTC | ADC_pin != ADC_pin) {
    return -1;
  }
  return ADC; // 8-bit resolution, left adjusted
}

/*
 * calculation from a time in microseconds to centimeters
 * parameter: counter is time in microseconds (us)
 */
uint16_t ms_to_cm(uint16_t counter)
{
  // Min 2cm - Max 70cm
  uint16_t microSec = counter / 16;
  return (microSec/58.2);
}


/*
 * calculation of light intensity
 * parameter: adc_value is raw data from the light sensor
 */
double berekenlichtwaarde(double adc_value)
{
  adc_value = adc_value / 1023;	// convert to volt
  return adc_value*100;			// to %
}


/*
 * send pulse trough Trigger on HC-SR04
 * show distance on led&key screen
 */
uint16_t berekenafstand(void)
{
  gv_echo = BEGIN;			// Set gv_echo to BEGIN
  PORTD |= _BV(trigPin);    // Send pulse
  _delay_us(12);			// Wait for pulse to complete
  PORTD &= ~(1<<trigPin);   // Clear PORTD (trigPin & LEDs)
  _delay_ms(30);			// Wait to make sure the signal is received
  
  return ms_to_cm(gv_counter);
}


void check_afstand(void){
	afstand = berekenafstand();
	_delay_ms(50);
	if(afstand >= afstand_max && afstand <= afstand_min) // afstand zit tussen de randwaarden
	{
		if(mode == UITGEROLD){
			mode = INROLLEN;
		}
		else if(mode == INGEROLD){
			mode = UITROLLEN;
		}
	}
		
	if (afstand >= afstand_max) {
		mode = INGEROLD;
	}
	else if (afstand <= afstand_min) {
		mode = UITGEROLD;
	}
	
	switch(mode){
		case UITGEROLD:
			PORTD |= (1 << RLED);	// Set red LED to 1
			PORTD &= ~(1 << YLED);	// Set yellow LED to 0
			break;

		case INGEROLD:
			PORTD |= (1 << GLED);	// Set green LED to 1
			PORTD &= ~(1 << YLED);	// Set yellow LED to 0
			break;
		
		case UITROLLEN:
			PORTD |= (1 << RLED);	// Set red LED to 1
			PORTD ^= (1 << YLED);	// Set yellow LED to 1
			break;
		
		case INROLLEN:
			PORTD |= (1 << GLED);	// Set green LED to 1
			PORTD ^= (1 << YLED);	// Set yellow LED to 1
			break;
	}
	show_afstand(afstand);
}



/*
 * check of de lichtintensiteit binnen de waardes is
 */
void licht_controle()
{
  double temp = licht_intens;
  double lichtwaarde = berekenlichtwaarde(adc_val(lichtsensor));
  
  if (lichtwaarde == -1)
  {
    return;
  }
  if (temp > 0) {
    temp = temp + lichtwaarde;
    lichtwaarde = temp / 2;
  }
  licht_intens = lichtwaarde;
  
  
  if (lichtwaarde >= licht_drempelwaarde)
  {
    mode = UITROLLEN;
  }
  else if (lichtwaarde <= licht_drempelwaarde)
  {
    mode = INROLLEN;
  }
}

void binnenkomend() 
{
	char string[20] = "";
	uint8_t i = 0;
	while(uart_recieve()){
		string[i] = uart_recieve();
		i++;
		if(i==19){
			break;
		}
	}
	if(strlen(string)>0){
		uart_transmit_string(string);
	}
	string[20] = "";
}

void verzend_info()
{ 
  uart_transmit_char('l');
  uart_transmit_char('=');
  uart_transmit_int(licht_intens);
  uart_transmit_char('\n');
}


int main(void)
{ 
  
  interr();				// Init external interrupts (INT1)
  SCH_Init_T0();		// Init schedular (Timer0)
  setup();				// Init ports
  analog_dig_conv();    // Init analog
  uart_init();			// Init uart (setup serial connection)
  
  uart_transmit_string("licht");
  
  int tasks[4];
    
  tasks[0] = SCH_Add_Task(binnenkomend, 0, 1);		// every 10ms: check if python send data
  tasks[1] = SCH_Add_Task(check_afstand, 0, 1);		// check the state of the roller shutter
  tasks[2] = SCH_Add_Task(licht_controle, 0, 300);	// every 3s: check light intensity
  tasks[3] = SCH_Add_Task(verzend_info, 600, 600);	// every 6th sec: send sensor data to python
    
  sei();            // Set interrupt flag
  _delay_ms(50);    // Make sure everything is initialized
  reset_display();  // Clear display
  
	while(1) {
		SCH_Dispatch_Tasks();
		//PORTD &= ~(1 << GLED);     // Clear green LED
		//PORTD &= ~(1 << RLED);     // Clear red LED

		//_delay_ms(500);
	}
  
	/*for (int t = 0; t < tasks; t++) {
		SCH_Delete_Task(tasks[t]);
	}*/

	cli();
	return 0;
}


ISR(INT1_vect)
{ 
  if (gv_echo == BEGIN)
  {
    TCNT1 = 0;
    TCCR1B = _BV(CS10);
    gv_echo = END;
  } 
  else
  {
    TCCR1B = 0;
    gv_counter = TCNT1;
  }
}