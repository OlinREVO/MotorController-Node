#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000
#include <util/delay.h>

// #define CS02 2
// #define CS01 1
// #define CS00 0

// #define COM0A1 7
// #define COM0A0 6

// Command to flash scripts is `sudo make flash`

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
    DDRD |= ( 1 << PD3 ); // set OC0 as output.
    DDRE |= ( 1 << PE1 ); // set pin 10 (PE1) as digital output?

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
    // Setting PE1 (button) as an input?
    DDRE = ~(_BV(PE2));
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
    OCR0A = 0; // change this later, setting the duty cycle
    // pin 11 button input (PE2)
    // pin 10 LED output (PE1)
    // read button input
    uint8_t on = 0;
    uint8_t button = 0;
    uint8_t prev_button = 0;
    uint16_t pot_reading = 0;

    while(1) {
        button = PINE & _BV(PE2);
        if (button && (button != prev_button)) {
            // flip LED state
            on ^= 0x01;
            PORTE ^= (1 << PE1);

        }
        if (on) {
            // Uses ADC1 pin (pin 12, which is PD4)
            pot_reading = read_ADC(0x01);
        } else {
            pot_reading = 0x0;
        }
        if (pot_reading > 0b1110111111) {
            pot_reading = 0b1110111111;
        }
        // Later prevent 0 to 10 and 90 to 100% PWM
        // Change 10 bit reading to 8 bit output
        OCR0A = (pot_reading >> 2);
        // Stabilize things
        _delay_ms(1);
        prev_button = button;
    }
    return 0;
}