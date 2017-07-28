/*
 * Gas detector.c
 *
 * Created: 30.5.2017. 1:15:46
 * Author : Viktorija Alilovic
 */ 
 
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

/* Defines the threshold value for MQ-2 Gas Sensor */
#define		MQ2_SENSOR_REF		1500

/* Variable declarations */
int mq2_gas_sensor_output;
uint8_t data;
uint16_t ADC_value;
char ADC_string[20];
char alarm_message[20] = "Gas detected!";

void USART_init (uint16_t baud)
{
	uint16_t baudPrescaler;
	baudPrescaler = (F_CPU/(16UL*baud))-1;
	UBRRH = (uint8_t)(baudPrescaler>>8);
	UBRRL = (uint8_t)baudPrescaler;
	UCSRA &= ~(1<<U2X);
	UCSRB = (1<<RXEN)|(1<<TXEN);
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
}
void USART_send (uint8_t data)
{
	while (!(UCSRA &(1<<UDRE)));
	UDR=data;
}

uint8_t USART_receive (void)
{
	while (!(UCSRA&(1<<RXC)));
	return UDR;
}

void USART_message (char *data)
{
	while ( *data != '\0')
	USART_send(*data++);
}

void ADC_init ()
{
	ADMUX = 0x00;
	ADMUX &= ~(1<<ADLAR);
	ADMUX |=(1<<REFS0)|(1<<REFS1);
	ADCSRA = 0x00;
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);
}

void ADC_start (void)
{
	ADCSRA |= (1<<ADSC);
}

uint16_t ADC_read (uint8_t channel)
{
	ADMUX &= 0b01100000;
	ADMUX |=channel;
	
	ADC_start();
	
	uint16_t value = 0;
	while (!(ADCSRA&(1<<ADIF)));
	value = ADCL;
	value += (ADCH<<8);
	return value;
}

int main(void)
{
	/* ADC initialization */
	ADC_init ();
	/* USART initialization */
	USART_init (9600);

	/* PBO pin of PortB is declared output (LED is connected) */
	DDRB = 0x01;
	/* PCO pin of PortC is declared output (Buzzer is connected) */
	DDRC = 0x01;	
	/* PBO pin of PortB is declared low */
	PORTB = 0b00000001;
	
    while (1) 
    {
		/* Read analog value from pin A0 */
		ADC_value = ADC_read(0);
		mq2_gas_sensor_output = ADC_value*10;
		/* Send value to string */
		sprintf(ADC_string, "Value: %d ppm", mq2_gas_sensor_output);
		/* Print value on serial monitor */
		USART_message(ADC_string);
		/* Print new line */
		USART_send (10);
		USART_send (13);
		
		/* Checking MQ-2 Gas Sensor output with threshold to turn On or Off Buzzer */
		if(mq2_gas_sensor_output > MQ2_SENSOR_REF)
		{
			/* Buzzer is On */
			PORTC = 0x01;
			/* Set LED state high */
			PORTB &=~ (1<<PB0);
			USART_message(alarm_message);
			USART_send (10);
			USART_send (13);
		}
		else
		{
			/* Buzzer is Off */
			PORTC = 0x00;
			/* Set LED state low */
			PORTB |= (1<<PB0);
		}
    }
	
	return 0;
}
