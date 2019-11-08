/*
 * arduino_rollende_luiken.c
 *
 * Created: 31-Oct-19 12:21:19
 * Author: Karen Arkojan & Shafik Hoshan
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "display.h"
#include "schedular.h"
#include "serial.h"

static int ID = 1;          // ID of device

// PD
const int trigPin = 2;        // Trigger    PD2
const int echoPin = 3;        // Echo     PD3
const int RLED = 4;        // Red LED    PD4
const int YLED = 5;     // Yellow LED PD5
const int GLED = 6;      // Green LED  PD6
const int lichtsensor = 0;      // light sensor  PA0

volatile uint16_t gv_counter;   // 16 bit counter value
volatile uint8_t gv_echo;     // a flag
volatile uint16_t afstand;     // distance of roller shutter
volatile double licht_intens;  //value of light sensitivity

// Default values of the maximal and minimal variables
uint8_t afstand_max = 65;     //max distant roller shutter
uint8_t afstand_min = 5;      //min distant roller shutter
uint8_t licht_min = 15;   //min light intensity
uint8_t licht_max = 65; //max light intensity

// setting up mode for arduino
typedef enum{INROLLEN = 0, UITROLLEN = 1, BEZIG = 2, STOP = 3} mode_t;
mode_t mode = BEZIG;

// setting up modes for received requests from python
//  ASCII values: 35==# ; 36==$ ; 37==% ; 38==& ;
typedef enum{NONE = -1, ROLLER = 35, LICHT = 36, COMMANDO = 37} data_mode;
data_mode d_modes = NONE;

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
   //enable the ADC & prescale = 128
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
 * calculation of light sensitivity
 * parameter: adc_value is raw data from the light sensor
 */
double berekenlichtwaarde(double adc_value)
{
  adc_value = adc_value / 1023;     // calculate to volt
  adc_value = adc_value * 100;      // to percentage
  return adc_value;
}

/*
 * send pulse trough Trigger on HC-SR04
 * show distance on led&key screen
 */
void berekenafstand(void)
{
  gv_echo = BEGIN;      // Set gv_echo to BEGIN
  PORTD |= _BV(trigPin);    // Set trigPin to 1 -> send pulse
  _delay_us(12);        // Wait for pulse to complete
  PORTD &= ~(1<<trigPin);   // Clear PORTD (trigPin & LEDs)
  _delay_ms(30);        // Wait to make sure the signal of the pulse has been returned to echo
  
  afstand = ms_to_cm(gv_counter);
  show_afstand(afstand);
}


/*
 * check if light sensitivity is normal,
 * when to high or low act on it
 */
void licht_controle()
{
  // first calculate the average over 2 measurement times
  double temp = licht_intens;
  double raw_value = adc_val(lichtsensor);
  raw_value = berekenlichtwaarde(raw_value);
  
  if (raw_value == -1)
  {
    return;
  }
  if (temp > 0) {
    temp = temp + raw_value;
    raw_value = temp / 2;
  }
  licht_intens = raw_value;
  
  if (licht_intens >= licht_max)
  {
    mode = UITROLLEN;
  }
  else if (licht_intens <= licht_min)
  {
    mode = INROLLEN;
  }
}

void binnenkomend() 
{
  uint8_t r = uart_recieve();
  _delay_us(25);
  uart_transmit_char(r);
  
  switch (r)
  {
    case ROLLER:
      d_modes = ROLLER;
    break;
    case LICHT:
      d_modes = LICHT;
    break;
    case COMMANDO:
      d_modes = COMMANDO;
    break;
    case NONE:
      d_modes = NONE;
    break;
  }
  
  if (d_modes != NONE)
  {
    uint8_t *data;
    switch (d_modes)
    {
      case ROLLER:
        data = insert_data_from_pyhton(ROLLER);
        afstand_min = data[0];
        afstand_max = data[1];
        free(data);
        d_modes = NONE;
      break;
      case LICHT: 
        data = insert_data_from_pyhton(LICHT);
        licht_min = data[0];
        licht_max = data[1];
        free(data);
        d_modes = NONE;
      break;
      case COMMANDO:
        _delay_us(25);
        
        uint8_t received = uart_recieve();
        _delay_us(50);
        
        if (received >= 48 && received <= 57) // ASCII 0-9
        { 
          ID = from_ascii_to_digit(received);
        }
        else if (received == 100)       // ASCII d
        {   
          mode = UITROLLEN;       
        }
        else if (received == 117)       // ASCII u
        {   
          mode = INROLLEN;       
        }
        else if (received == 115)       // ASCII s
        {   
          mode = STOP;      
        }
                
        d_modes = NONE;
      break;
    }
  }
  d_modes = NONE;
}

void verzend_info()
{ 
  uart_transmit_char('l');
  uart_transmit_char('=');
  uart_transmit_int(licht_intens);
  uart_transmit_char('\n');
}

void uitrollen(void)
{
  while(afstand <= afstand_max && mode != STOP)
  {
    binnenkomend();
    verzend_info();
    PORTD ^= (1 << YLED);
    if(afstand >= afstand_max || afstand <= afstand_min)
    {
      PORTD &= ~(1 << YLED);
    }
    
    show_afstand(afstand);
    afstand += 1;
    _delay_ms(2500);
  }
  mode = BEZIG;
} 


void inrollen(void)
{
  while(afstand >= afstand_min && mode != STOP)
  { 
    binnenkomend();
    verzend_info();
    PORTD ^= (1 << YLED);
    if(afstand >= afstand_max || afstand <= afstand_min)
    {
      PORTD &= ~(1 << YLED);
    }
    
    show_afstand(afstand);
    afstand -= 1;
    _delay_ms(2500);
  }
  mode = BEZIG;
}

int main(void)
{ 
  
  interr();   // Init external interrupts (INT1)
  SCH_Init_T0();    // Init schedular (Timer0)
  setup();      // Init ports
  analog_dig_conv();     // Init analog
  uart_init();    // Init uart (setup serial usb connection)
  
  char naam[] = "licht";
  for(int i=0;strlen(naam)>i;i++){
    uart_transmit_char(naam[i]);
  }
  uart_transmit_char('\n');
  
  int tasks[3];
  
  uint8_t quick = 10;
  uint8_t demonstration = 0;
  
  if (demonstration == 1)   // increase speed for a quick demonstration
  {
    quick = 1;  
  }
  
  tasks[0] = SCH_Add_Task(binnenkomend, 0, 10*quick);     // check every 10ms * 100 = 1000ms if python has updated some data
  tasks[1] = SCH_Add_Task(licht_controle, 0, 300*quick);       // check light intensity every 10ms * 3000 = 30 sec with zero delay
  tasks[2] = SCH_Add_Task(verzend_info, 0, 600*quick);       // 10ms * 6000 = 60 sec
    
  sei();        // Set interrupt flag
  _delay_ms(50);    // Make sure everything is initialized
  
  reset_display();  // Clear display
  
  while(1)
  {
    SCH_Dispatch_Tasks();
    PORTD &= ~(1 << GLED);     // Clear green LED (set to 0)
    PORTD &= ~(1 << RLED);     // Clear red LED (set to 0)
    
    switch(mode)
    {
      case UITROLLEN:
        uitrollen();
        break;
      case INROLLEN:
        inrollen();
        break;
      case STOP:
        show_afstand(afstand);
        PORTD &= ~(1 << YLED);
        break;
      case BEZIG:
        berekenafstand();
        break;
    }
    
    if (afstand >= afstand_max)
    {
      PORTD |= (1<<GLED);    // Set green LED to 1
    }
    else if (afstand <= afstand_min) 
    {
      PORTD |= (1<<RLED);      // Set red LED to 1
    }

    _delay_ms(500);
    
  }
  
  for (int t = 0; t < tasks; t++)
  {
    SCH_Delete_Task(tasks[t]);
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