/*
Name: Ahmed Ali
Group: 90
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Define pin connections for various buttons and LEDs
#define BUTTON_TOGGLE PB7
#define ALARM_PIN PD0
#define Reset_Button PD2
#define BUTTON_PAUSE PD3
#define BUTTON_RESUMED PB2
#define BUTTON_DECREMENT_HOUR PB0
#define BUTTON_INCREMENT_HOUR PB1
#define BUTTON_DECREMENT_MINUTE PB3
#define BUTTON_INCREMENT_MINUTE PB4
#define BUTTON_DECREMENT_seconds PB5
#define BUTTON_INCREMENT_SECONDS PB6
#define RED_LED PD4
#define BLUE_LED PD5
#define DELAY_TIME 30
#define MAX_TIME 65535

// Function prototypes
void seven_segment(void);
void display(void);
void timer1_setting(void);
void RESET_INT0(void);
void second_increment(void);
void second_decrement(void);
void minute_increment(void);
void minute_decrement(void);
void hour_increment(void);
void hour_decrement(void);
void toggle(void);

// Variables to store time values (seconds, minutes, hours)
unsigned char seconds1=0, seconds10=0, minute1=0, minute10=0, hour1=0, hour10=0;
unsigned char flag=1;   // Mode flag (1 for increment, 0 for decrement)
unsigned char alarm=0;  // Alarm state

// Initialize the seven-segment display
void seven_segment(void) {
    DDRC |= 0x0F;  // Set lower 4 bits of PORTC as output
    PORTC &= 0xF0; // Clear the lower 4 bits of PORTC
    DDRA |= 0x3F;  // Set the first 6 bits of PORTA as output for 7-segment control
    PORTA &= 0xC0; // Clear the first 6 bits of PORTA
}

// Display the current time on the seven-segment display
void display(void) {
    PORTA = 0x20;  // Enable the first 7-segment display for seconds
    PORTC = (PORTC & 0xF0) | (seconds1 & 0x0F);
    _delay_ms(5);

    PORTA = 0x10;  // Enable the second 7-segment display for tens of seconds
    PORTC = (PORTC & 0xF0) | (seconds10 & 0x0F);
    _delay_ms(5);

    PORTA = 0x08;  // Enable the third 7-segment display for minutes
    PORTC = (PORTC & 0xF0) | (minute1 & 0x0F);
    _delay_ms(5);

    PORTA = 0x04;  // Enable the fourth 7-segment display for tens of minutes
    PORTC = (PORTC & 0xF0) | (minute10 & 0x0F);
    _delay_ms(5);

    PORTA = 0x02;  // Enable the fifth 7-segment display for hours
    PORTC = (PORTC & 0xF0) | (hour1 & 0x0F);
    _delay_ms(5);

    PORTA = 0x01;  // Enable the sixth 7-segment display for tens of hours
    PORTC = (PORTC & 0xF0) | (hour10 & 0x0F);
    _delay_ms(5);
}

// Setup Timer1 for counting seconds using CTC mode
void timer1_setting(void) {
    TCCR1A = (1<<FOC1A);  // Force Output Compare for Channel A
    TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10);  // CTC mode with prescaler of 1024
    TCNT1 = 0;           // Initialize counter value to 0
    OCR1A = 15624;       // Set Output Compare Register for 1-second intervals
    TIMSK |= (1<<OCIE1A); // Enable Timer1 Compare A Match Interrupt
}

// Setup external interrupt on INT0 (Reset button)
void RESET_INT0(void) {
    DDRD &= ~(1<<Reset_Button); // Set Reset_Button pin as input
    PORTD |= (1<<Reset_Button); // Enable pull-up resistor for Reset_Button
    MCUCR |= (1<<ISC01);        // Trigger interrupt on falling edge of INT0
    MCUCR &= ~(1<<ISC00);
    GICR |= (1<<INTF0);         // Enable External Interrupt Request 0
}

// ISR for Reset button (INT0) to reset time
ISR(INT0_vect) {
    hour1 = hour10 = minute1 = minute10 = seconds1 = seconds10 = 0;
    alarm = 0;
}

// Setup external interrupt on INT1 (Pause button)
void PAUSE_INT1(void) {
    DDRD &= ~(1<<BUTTON_PAUSE); // Set Pause button as input
    MCUCR |= (1<<ISC10) | (1<<ISC11); // Trigger interrupt on rising edge of INT1
    GICR |= (1<<INTF1);         // Enable External Interrupt Request 1
}

// ISR for Pause button (INT1) to pause the timer
ISR(INT1_vect) {
    TCCR1B = 0;  // Stop the timer
}

// Setup external interrupt on INT2 (Resume button)
void RESUMED_INT2(void) {
    DDRB &= ~(1<<BUTTON_RESUMED); // Set Resume button as input
    PORTB |= (1<<BUTTON_RESUMED); // Enable pull-up resistor for Resume button
    MCUCSR |= (1<<ISC2);          // Trigger interrupt on falling edge of INT2
    GICR |= (1<<INTF2);           // Enable External Interrupt Request 2
}

// ISR for Resume button (INT2) to resume the timer
ISR(INT2_vect) {
    TCCR1B = (1<<CS10) | (1<<CS12) | (1<<WGM12); // Resume Timer1 with CTC mode
}

// Increment seconds when the increment button is pressed
void second_increment(void) {
    PORTB |= (1<<BUTTON_INCREMENT_SECONDS);
    seconds1++;
    if (seconds1 == 10) {
        seconds10++;
        seconds1 = 0;
        if (seconds10 == 6) {
            seconds10 = 0;
        }
    }
}

// Decrement seconds when the decrement button is pressed
void second_decrement(void) {
    PORTB |= (1<<BUTTON_DECREMENT_seconds);
    if (seconds1 == 0 && seconds10 >= 1) {
        seconds10--;
        seconds1 = 9;
    } else if (seconds1 == 0 && seconds10 == 0) {
        seconds1 = 9;
        seconds10 = 5;
    } else {
        seconds1--;
    }
    if (hour10 == 2 && hour1 == 4) {
        hour10 = 2;
        hour1 = 3;
    }
}

// Increment minutes when the increment button is pressed
void minute_increment(void) {
    PORTB |= (1<<BUTTON_INCREMENT_MINUTE);
    minute1++;
    if (minute1 == 10) {
        minute10++;
        minute1 = 0;
        if (minute10 == 6) {
            minute10 = 0;
        }
    }
    if (hour1 == 2 && hour10 == 4) {
        hour1 = 0;
        hour10 = 0;
    }
}

// Decrement minutes when the decrement button is pressed
void minute_decrement(void) {
    PORTB |= (1<<BUTTON_DECREMENT_MINUTE);
    if (minute1 == 0 && minute10 >= 1) {
        minute10--;
        minute1 = 9;
    } else if (minute10 == 0 && minute1 == 0) {
        minute1 = 9;
        minute10 = 5;
    } else {
        minute1--;
    }
    if (hour10 == 2 && hour1 == 4) {
        hour10 = 2;
        hour1 = 3;
    }
}

// Increment hours when the increment button is pressed
void hour_increment(void) {
    PORTB |= (1<<BUTTON_INCREMENT_HOUR);
    hour1++;
    if (hour10 >= 2 && hour1 >= 4) {
        hour10 = 0;
        hour1 = 0;
    } else if (hour10 <= 1 && hour1 == 10) {
        hour10++;
        hour1 = 0;
    }
}

// Decrement hours when the decrement button is pressed
void hour_decrement(void) {
    PORTB |= (1<<BUTTON_DECREMENT_HOUR);
    if (hour1 == 0 && hour10 == 0) {
        if (minute10 == 0 && minute1 == 0 && seconds10 == 0 && seconds1 == 0) {
            hour10 = 2;
            hour1 = 4;
        } else {
            hour1 = 3;
            hour10 = 2;
        }
    } else if (hour1 == 0 && hour10 > 0) {
        hour1 = 9;
        hour10--;
    } else {
        hour1--;
    }
}

// Toggle between increment and decrement modes
void toggle(void) {
    PORTB |= (1<<BUTTON_TOGGLE);
    if (!(PINB & (1<<BUTTON_TOGGLE))) {
        _delay_ms(30);
        if (!(PINB & (1<<BUTTON_TOGGLE))) {
            flag ^= 1; // Toggle between increment (flag=1) and decrement (flag=0)
            while (!(PINB & (1<<BUTTON_TOGGLE))) {
                display();
            }
        }
    }
}

// Timer1 Compare Match A Interrupt Service Routine (ISR) for time increment/decrement
ISR(TIMER1_COMPA_vect) {
    if (flag == 1) {  // Increment mode
        if (hour10 == 2 && hour1 == 3 && minute10 == 5 && minute1 == 9 && seconds10 == 5 && seconds1 == 9) {
            alarm = 1;
            hour10 = hour1 = minute10 = minute1 = seconds10 = seconds1 = 0;
        } else if (hour1 == 9 && minute10 == 5 && minute1 == 9 && seconds10 == 5 && seconds1 == 9) {
            hour10++;
            hour1 = minute10 = minute1 = seconds10 = seconds1 = 0;
        } else if (minute10 == 5 && minute1 == 9 && seconds10 == 5 && seconds1 == 9) {
            hour1++;
            minute10 = minute1 = seconds10 = seconds1 = 0;
        } else if (minute1 == 9 && seconds10 == 5 && seconds1 == 9) {
            minute10++;
            minute1 = seconds10 = seconds1 = 0;
        } else if (seconds10 == 5 && seconds1 == 9) {
            minute1++;
            seconds10 = seconds1 = 0;
        } else if (seconds1 == 9) {
            seconds10++;
            seconds1 = 0;
        } else {
            seconds1++;
        }
    } else {  // Decrement mode
        if (seconds1 != 0) {
            seconds1--;
            alarm = 0;
        } else if (seconds10 != 0) {
            seconds10--;
            seconds1 = 9;
        } else if (minute1 != 0) {
            minute1--;
            seconds10 = 5;
            seconds1 = 9;
        } else if (minute10 != 0) {
            minute10--;
            minute1 = 9;
            seconds10 = 5;
            seconds1 = 9;
        } else if (hour1 != 0) {
            minute10 = 5;
            minute1 = 9;
            seconds10 = 5;
            seconds1 = 9;
        } else if (hour10 != 0) {
            hour10--;
            hour1 = 9;
            minute10 = 5;
            minute1 = 9;
            seconds10 = 5;
            seconds1 = 9;
        } else {
            alarm = 1;
        }
    }

    // Activate alarm if countdown reaches zero
    if (alarm == 1) {
        PORTC = PORTC & 0xF0;
        PORTD |= (1<<ALARM_PIN);  // Turn on alarm
    } else {
        PORTC = PORTC & 0xF0;
        PORTD &= ~(1<<ALARM_PIN);  // Turn off alarm
    }
}

int main(void) {
    SREG |= (1<<7);  // Enable global interrupts

    // Set up I/O pins for buttons and LEDs
    DDRB &= ~(1<<BUTTON_INCREMENT_SECONDS);
    DDRB &= ~(1<<BUTTON_DECREMENT_seconds);
    DDRB &= ~(1<<BUTTON_DECREMENT_MINUTE);
    DDRB &= ~(1<<BUTTON_RESUMED);
    DDRD &= ~(1<<ALARM_PIN);
    DDRB &= ~(1<<BUTTON_TOGGLE);
    DDRD &= ~(1<<BLUE_LED);
    DDRD &= ~(1<<RED_LED);
    PORTD |= (1<<RED_LED);  // Turn on the red LED (counting up mode)

    // Start Timer1 in CTC mode if flag is 0
    if (flag == 0) {
        TCCR1B |= (1<<CS10) | (1<<CS11) | (1<<WGM12);
        flag = 1;
    }

    // Initialize peripherals
    seven_segment();
    timer1_setting();
    RESET_INT0();
    PAUSE_INT1();
    RESUMED_INT2();

    // Main loop
    while (1) {
        display();  // Update the 7-segment display
        toggle();   // Check for mode toggle

        // Check if increment mode is active and update LED status
        if (flag == 1) {
            PORTD |= (1<<RED_LED);
            PORTD &= ~(1<<BLUE_LED);
        } else {
            PORTD |= (1<<BLUE_LED);
            PORTD &= ~(1<<RED_LED);
        }

        // Handle second increment button
        if (!(PINB & (1<<BUTTON_INCREMENT_SECONDS))) {
            _delay_ms(30);
            if (!(PINB & (1<<BUTTON_INCREMENT_SECONDS))) {
                second_increment();
                display();
            }
            while (!(PINB & (1<<BUTTON_INCREMENT_SECONDS))) {
                display();
            }
        }

        // Handle second decrement button
        if (!(PINB & (1<<BUTTON_DECREMENT_seconds))) {
            _delay_ms(30);
            if (!(PINB & (1<<BUTTON_DECREMENT_seconds))) {
                second_decrement();
                display();
            }
            while (!(PINB & (1<<BUTTON_DECREMENT_seconds))) {
                display();
            }
        }

        // Handle minute increment button
        if (!(PINB & (1<<BUTTON_INCREMENT_MINUTE))) {
            _delay_ms(30);
            if (!(PINB & (1<<BUTTON_INCREMENT_MINUTE))) {
                minute_increment();
                display();
            }
            while (!(PINB & (1<<BUTTON_INCREMENT_MINUTE))) {
                display();
            }
        }

        // Handle minute decrement button
        if (!(PINB & (1<<BUTTON_DECREMENT_MINUTE))) {
            _delay_ms(30);
            if (!(PINB & (1<<BUTTON_DECREMENT_MINUTE))) {
                minute_decrement();
                display();
            }
            while (!(PINB & (1<<BUTTON_DECREMENT_MINUTE))) {
                display();
            }
        }

        // Handle hour increment button
        if (!(PINB & (1<<BUTTON_INCREMENT_HOUR))) {
            _delay_ms(30);
            if (!(PINB & (1<<BUTTON_INCREMENT_HOUR))) {
                hour_increment();
                display();
            }
            while (!(PINB & (1<<BUTTON_INCREMENT_HOUR))) {
                display();
            }
        }

        // Handle hour decrement button
        if (!(PINB & (1<<BUTTON_DECREMENT_HOUR))) {
            _delay_ms(30);
            if (!(PINB & (1<<BUTTON_DECREMENT_HOUR))) {
                hour_decrement();
                display();
            }
            while (!(PINB & (1<<BUTTON_DECREMENT_HOUR))) {
                display();
            }
        }
    }
}
