/*
 * temperatuursensor
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

typedef enum{NONE = 1, INROLLEN = 2, INGEROLD = 3, UITROLLEN = 4, UITGEROLD = 5, MANUAL = 6} mode_t;
typedef enum{OFF = 0, ON = 1} toggle;
mode_t mode = NONE;
mode_t prev_mode = NONE;

const int trigPin = 2;      // Trigger		PD2
const int echoPin = 3;      // Echo			PD3
const int RLED = 4;			// Red LED		PD4
const int YLED = 5;			// Yellow LED	PD5
const int GLED = 6;			// Green LED	PD6
const int temperatuursensor = 0;  // temperatuur sensor PA0

volatile uint16_t gv_counter;   // 16 bit counter
volatile uint8_t gv_echo;		// a flag
volatile uint16_t afstand;		// distance to roller shutter

uint8_t temperatuurwaarde;
uint16_t afstand_max = 30;	// max distance roller shutter
uint16_t afstand_min = 10;	// min distance roller shutter
uint8_t temperatuur_drempelwaarde = 45;		// boven deze waarde moet het rolgordijn dicht

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


/************************************************************************/
/* initialize external interrupt 1                                      */
/************************************************************************/
void interr(void)
{
  // any change triggers ext interrupt 1
  EICRA = (1 << ISC10);
  EIMSK = (1 << INT1);
}


/************************************************************************/
/* initialize ADC (analog-digital-converter)                            */
/************************************************************************/
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


uint16_t ms_to_cm(uint16_t counter)
{
  // Min 2cm - Max 70cm
  uint16_t microSec = counter / 16;
  return (microSec/58.2);
}


void setLed(int LED, toggle state){
	if(state == ON){
		PORTD |= (1 << LED);	// Turn LED on
	}
	if(state == OFF){
		PORTD &= ~(1 << LED);	// Turn LED off
	}
}


/************************************************************************/
/* calculation of temperature                                           */
/* parameter: adc_value is raw data from the TP36 sensor	            */
/************************************************************************/
double bereken_temperature(double adc_value)
{
	adc_value = adc_value * (5.0/1023);		// calculate value to volt
	adc_value = adc_value - 0.5;			// convert to celcius
	adc_value = adc_value * 100;
	return adc_value;
}

/************************************************************************/
/* de afstand uitrekenen                                                */
/************************************************************************/
void check_afstand(void){
	gv_echo = BEGIN;			// Set gv_echo to BEGIN
	PORTD |= _BV(trigPin);    // Send pulse
	_delay_us(12);			// Wait for pulse to complete
	PORTD &= ~(1<<trigPin);   // Clear PORTD (trigPin & LEDs)
	_delay_ms(30);			// Wait to make sure the signal is received
	afstand =  ms_to_cm(gv_counter);
	show_afstand(afstand);
}


/************************************************************************/
/* de gemiddelde temperatuur berekenen                                  */
/************************************************************************/
void temperatuur_controle()
{
  double previous_temp = temperatuurwaarde;
  temperatuurwaarde = bereken_temperature(adc_val(temperatuursensor));
  if (temperatuurwaarde == -1)
  {
    return;
  }
  if (previous_temp > 0) {
    int sum = previous_temp + temperatuurwaarde;
    temperatuurwaarde = sum / 2;
  }
  previous_temp = temperatuurwaarde;
}

void binnenkomend() 
{
	char str[] = "";
	char received = uart_recieve();
	if (strlen(received)>0){
		strncat(str, received, 1);
	}
	if(strlen(str)>0){
		uart_transmit_char(str);
	//char str[] = "";
	}
	
	
	//~TODO~
	// in of uit rollen
	/*
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
	*/
}

void verzend_info()
{ 
  uart_transmit_char('t');
  uart_transmit_char('=');
  uart_transmit_int(temperatuurwaarde);
  uart_transmit_char('\n');
}

void setLeds(void){
	if(mode != MANUAL){
		prev_mode = mode;
			
		//change mode depending on the distance
		if (afstand > afstand_max) {
			mode = INGEROLD;
		}
		else if (afstand < afstand_min) {
			mode = UITGEROLD;
		}
		else if(afstand > afstand_min && afstand < afstand_max) {	// afstand zit tussen de randwaarden
			if(prev_mode == UITGEROLD){
				mode = INROLLEN;
			}
			else if(prev_mode == INGEROLD){
				mode = UITROLLEN;
			}
		}
		//change mode depending on the temperature

		/*
		if (temperatuurwaarde >= temperatuur_drempelwaarde) {
			mode = UITROLLEN;
		}
		else if(temperatuurwaarde < temperatuur_drempelwaarde) {
			mode = INROLLEN;
		}
		*/
	}
		
	//set leds
	int flash_speed = 0;
	switch(mode){
		case UITGEROLD:			// Turn red on
			setLed(RLED, ON);	
			setLed(GLED, OFF);
			setLed(YLED, OFF);
		break;
			
		case INGEROLD:			// Turn yellow on
			setLed(RLED, OFF);
			setLed(GLED, ON);
			setLed(YLED, OFF);
		break;
			
		case UITROLLEN:			// Turn red&yellow on
			setLed(RLED, ON);
			setLed(GLED, OFF);
			_delay_ms(flash_speed);
			setLed(YLED, ON);
			_delay_ms(flash_speed);
		break;
			
		case INROLLEN:			// Turn green&yellow on
			setLed(RLED, OFF);
			setLed(GLED, ON);
			_delay_ms(flash_speed);
			setLed(YLED, ON);
			_delay_ms(flash_speed);
		break;
			
		/*
		case MANUAL:			// Turn all LEDs off
			setLed(RLED, OFF);
			setLed(GLED, OFF);
		*/
	}
	if (flash_speed > 0){
		setLed(YLED, OFF);
	}
}


int main(void)
{ 
  interr();				// Init external interrupts (INT1)
  SCH_Init_T0();			// Init schedular (Timer0)
  setup();				// Init ports
  analog_dig_conv();		// Init analog
  uart_init();			// Init uart (setup serial connection)
  reset_display();		// Clear display
  uart_transmit_string("temperatuur");
  
  int tasks[5];
   
  //tasks[0] = SCH_Add_Task(init, 0, 0);				// initialize arduino + python
  tasks[0] = SCH_Add_Task(binnenkomend, 0, 1);		// check if python send data
  tasks[1] = SCH_Add_Task(check_afstand, 0, 5);		// check the state of the roller shutter
  tasks[2] = SCH_Add_Task(temperatuur_controle, 0, 400);	// every 4s: check light intensity
  tasks[3] = SCH_Add_Task(verzend_info, 600, 600);	// every 6th sec: send sensor data to python
  tasks[4] = SCH_Add_Task(setLeds, 0, 1);			// set the LEDS accordingly
  
  _delay_ms(50);    // Make sure everything is initialized
  
  SCH_Start();
 
	while(1) {
		SCH_Dispatch_Tasks();		
	}
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