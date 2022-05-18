/*
 * Lab07 - Computer Keyboard Piano
 * Author : Jace Johnson
 * Created: 4/5/2020 11:18:26 AM
 *	  Rev 1 4/5/2020
 *
 * Description:	ATMEG2560 Computer keyboard piano. Plays a key from the 4th
 *				octave on a piano when a char is received through USART and
 *				stops when no chars are received. Can only play one key at a
 *				time. Tested using SSH through a computer terminal to send
 *				chars.
 * 
 * Table for tone played for the received char:
 *
 *		char	tone
 *		 a		 C
 *		 s		 D
 *		 d		 E
 *		 f		 F
 *		 j		 G
 *		 k		 A
 *		 l		 Bb
 *		 ;		 B
 *
 * Hardware:	ATMega 2560 operating at 16 MHz
 *				Computer to communicate with
 *				Passive piezo buzzer
 *				jumper wires
 * Configuration for USART0 can be done using a USB Male A to USB Male B to
 * connect with a computer or using TX0 (pin 1) and RX0 (pin 0) pins on the
 * ATMega 2560. Configuration for buzzer shown below
 *
 * ATMega 2560
 *  PORT   pin			 Buzzer
 * -----------         ----------
 * | B7	   13|---------|Pos		|
 * |	  GND|---------|Neg		|
 * -----------         ----------
 */

#define F_CPU 16000000

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>


void playKey(char key);	//play input key
void initUSART0Buzzer();
char getChar();
void initBuzzer();
void startBuzzer(int freq);
void stopBuzzer();


#define USART_BAUDRATE 57600	//set baud rate
#define BAUD_PRESCALE F_CPU / (USART_BAUDRATE * 16UL) - 1


int count = 0;


/*
 * Function:  main
 *  Calls functions to initialize USART0 and the LCD. also sets up input and 
 *	output pins for PIR sensor and LED. Also repeatedly calls functions to 
 *	print Messages to LCD screen and computer terminal
 *
 *  returns:  1   program ends unexpectedly
 */
int main(void)
{
	
	char str[40] = "";
	char key = '0';	//initialize key char
	
	initBuzzer();	//initialize timer0 for the buzzer
	//initUSART0();
	initUSART0Buzzer();
	
	DDRB |= 0b00100000;			//OC0A (PORTB 7, Pin 13) set as output
	key = getChar();	//get next key
	//fprintf(&USART0_OUT, "here4");
	playKey('k');
	//fprintf(&USART0_OUT, "here5");
	while(1){
		PORTB ^= 0b00100000;
		//fprintf(&USART0_OUT, "here6");
	}
	/*
	TCNT0 = 0;
	OCR0A = 62;
	TCCR0B |= (1 << CS02);
	while(1){
		TCCR0B |= (1 << CS02);
		_delay_ms(1);
		TCCR0B &= !((1 << CS02)|(1 << CS01)|(1 << CS00));
		}
	*/
	//startBuzzer(1);
	//while(1){}
    
	while(1){
		key = getChar();	//get next key
		//fprintf(&USART0_OUT, "here4");
		playKey(key);		//play tone
		//fprintf(&USART0_OUT, "here5");
		_delay_ms(1000);	//short delay
		//fprintf(&USART0_OUT, "here6");
		stopBuzzer();		//stop playing tone
		//fprintf(&USART0_OUT, "here7");
	}
	
    return 1;	//loop exits unexpectedly
}

/*
 * Function:  playKey
 *  Takes char input and starts the buzzer at a certain frequency based on the
 *	inputted char. 
 *		char	tone	Freq
 *		 a		 C		261.6
 *		 s		 D		293.7
 *		 d		 E		329.6
 *		 f		 F		349.2
 *		 j		 G		392.0
 *		 k		 A		440.0
 *		 l		 Bb		466.2
 *		 ;		 B		493.9
 *	   other	none	n/a
 *
 *  returns:  none
 */
void playKey(char key){
	switch(key){				//switch statement for key tone to play
		case 'a':
			startBuzzer(2616);	//C key
			break;
		case 's':
			startBuzzer(2937);	//D key
			break;
		case 'd':
			startBuzzer(3296);	//E key
			break;
		case 'f':
			startBuzzer(3492);	//F key
			break;
		case 'j':
			startBuzzer(3920);	//G key
			break;
		case 'k':
			startBuzzer(4400);	//A key
			break;
		case 'l':
			startBuzzer(4662);	//Bb key
			break;
		case ';':
			startBuzzer(4939);	//B key
			break;
		default:				//default case (exit immediately)
			break;
	}
	return;
}

/*
 * Function:	initUSART0
 *	Sets up USART0 to use a baudrate equal to the global constant 
 *	USART_BAUDRATE, and use 8 bit character frames and async mode.
 *
 *  returns:	none
 */
void initUSART0Buzzer(){
	//Enable RX with 8 bit character frames in async mode
	UCSR0B |= (1 << RXEN0);
	UCSR0C |= (1 << UCSZ01)|(1 << UCSZ00);	
	
	//set baud rate (upper 4 bits should be zero)
	UBRR0L = BAUD_PRESCALE;
	UBRR0H = (BAUD_PRESCALE >> 8);
	return;
}

/*
 * Function:	getChar
 *	Waits for a new byte to be received from USART0 and returns that data
 *
 *  returns:	char	byte received using USART0
 */
char getChar(){
	//fprintf(&USART0_OUT, "count: %d\n", count);
	count++;
	char temp = 'l';
	
	while((UCSR0A & (1 << RXC0)) == 0){}	//wait for receive complete flag set
	temp = UDR0;						//store received byte
	//fprintf(&USART0_OUT, "received: %c", temp);
	return temp;						//return received byte
}

/*
 * Function:  initBuzzer
 *  Sets up timer0 to output a square wave to the OC0A port (B7) in 8-bit CTC
 *	mode. Stops timer immediately.
 *
 *  returns:	none
 */
void initBuzzer(){
	DDRB |= 0b10000000;			//OC0A (PORTB 7, Pin 13) set as output
	
	TCCR0B |= (1 << WGM02);		//8-bit CTC mode
	TCCR0A |= (1 << COM0A0);	//toggle OC0A on compare match
	TCCR0B &= !((1 << CS02)|(1 << CS01)|(1 << CS00));		//stop timer 0
	
   	sei();    	//Enable global interrupts by setting global interrupt enable
                //bit in SREG
   	TCNT0 = 0;	//initialize timer counter at 0
	   
   	TIMSK0 |= (1 << OCIE0A);	//enable timer0 compare A
   	return;
}

/*
 * Function:  startBuzzer
 *  Starts timer 0 with 256 prescaler. Uses input frequency to calculate and
 *	set timer counter compare value.
 *
 *	freq	double	frequency of tone that the buzzer will play
 *
 *  returns:	none
 */
void startBuzzer(int freq){
	int temp = 0;
	TCNT0 = 0;				//start timer counter at 0
	
	//Set output compare value using frequency equation for CTC mode from
	//ATMEGA2560 datasheet page 121:  Fclk = Foc / (2 * N * (1 + OCR))
	OCR0A = ((F_CPU / (freq * 2 * 256) / 2) - 1);
	
	//fprintf(&USART0_OUT, "%d", OCR0A);
	TCCR0B |= (1 << CS02);	//start timer 0 with 256 prescaler
	return;
}

/*
 * Function:  stopBuzzer
 *  Stops timer 0
 *
 *  returns:	none
 */
void stopBuzzer(){
	TCCR0B &= !((1 << CS02)|(1 << CS01)|(1 << CS00));		//stop timer 0
	return;
}