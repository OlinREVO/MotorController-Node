#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000
#include <util/delay.h>

// #define CS02 2
// #define CS01 1
// #define CS00 0

// #define COM0A1 7
// #define COM0A0 6

void phase_correct_PWM_OCR0A() {
	// Fuse bits are stupid and automatically slow down the clock by a
	// factor of 8, unless you change them
	// DO NOT change the high fuse bits
	// Change the low fuse bits to 0xE3 (use PLL to get 16MHz clock)
	// sudo avrdude -p atmega16m1 -c avrispmkII -P usb -U lfuse:w:0xe3:m
	// PLL stands for phase lock loop

	// Multiple modes of operation for timer, ideal is phase correct 
	// PWM mode, which allows for symmetric operation (fast PWM is another
	// option that goes twice as fast)

	// OCOA is physical pin on microcontroller that gets PWM output

	// TCCR0 is the timing control register - it sets options that the timer acts upon when it's running
    // Currently configuring the timer to run at full speed
    TCCR0B |= (1 << CS00); // 0b11111101, in order to set CS01 to 0

    // DDRB is the hardware register - a pointer to an 8 bit block of memory that will always be used for that purpose
    // Each bit of that 8 bit memory block controls whether the pin is an input or output
    // PinB is like DDRB except it actually stores the values of 8 digital pins
    DDRD |= ( 1 << PD3 ); //set OC0 as output.

    // TCCR0A is a timer/counter control register, using pins 7 and 6, 
    // which are COM0A1 and COM0A0, we can set the mode of the phase correct
    // PWM mode (look at pg 91 of datasheet)
   	TCCR0A |= (1 << COM0A1) | _BV(WGM00);
   	TCCR0A &= ~(1 << COM0A0);
}

void setup_ADC1() {
	//Enable ADC, with slow clock prescalar
	ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
	//Enable internal reference voltage
	ADCSRB |= _BV(AREFEN);
	//Set internal reference voltage as AVcc
	ADMUX |= _BV(REFS0);
	// Setting PD4 (ADC1) as an input
	DDRD = ~(_BV(PD4));
}

uint16_t read_ADC(uint8_t channel) {
	ADMUX &= ~(0x1F); // Reset channel bits
    ADMUX |= channel; // Read channel 1
    ADCSRA |= _BV(ADSC);
    // Waits until reading is ready, bit ADSC goes low when ready
    while(bit_is_set(ADCSRA, ADSC));
    return ADC;
}

int main() {
	phase_correct_PWM_OCR0A();
	setup_ADC1();
    // OCR0A and OCR0B are 8 bit registers used to set duty cycle
    OCR0A = 200; // change this later, setting the duty cycle

    while(1) {
    	// Uses ADC1 pin (pin 12, which is PD4)
    	uint16_t pot_reading = read_ADC(0x01);
    	// Change 10 bit reading to 8 bit output
    	OCR0A = (pot_reading >> 2);
    	// Stabilize things
    	_delay_ms(1);
    }
    return 0;
}