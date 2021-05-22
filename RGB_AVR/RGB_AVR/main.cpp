/*
 * RGB_AVR.cpp
 *
 * Created: 5/11/2021 6:18:04 PM
 * Author : Asus ROG
 */ 

#define F_CPU 8000000UL
#include <stdio.h>
#include <avr/io.h>
#include <util\delay.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>

#define databus_direction DDRB
#define controlbus_direction DDRC
#define control_bus PORTC
#define databus PORTB
using namespace std;

#define rs 1
//#define rw 1
#define en 2

#define LCDMaxLines 4
#define LCDMaxChars 16
#define LineOne 0x80
#define LineTwo 0xc0

#define BlankSpace ' '

#define RowColDirection DDRD //Data Direction Configuration for keypad
#define ROW PORTD            //Lower four bits of PORTC are used as ROWs
#define COL PIND           //Higher four bits of PORTC are used as COLs

void KEYPAD_WaitForKeyRelease()
{
	unsigned char key;
	do
	{
		ROW=0x0f;         // Pull the ROW lines to low and Column high low.
		key=COL & 0x0f;   // Read the Columns, to check the key press
	}while(key!=0x0f);   // Wait till the Key is released,
	// If no Key is pressed, Column lines will remain high (0x0f)
}
void KEYPAD_WaitForKeyPress()
{
	unsigned char key;
	do
	{
		do
		{
			ROW=0x0f;	   // Pull the ROW lines to low and Column lines high.
			key=COL & 0x0F;   // Read the Columns, to check the key press
		}while(key==0x0f); // Wait till the Key is pressed,
		// if a Key is pressed the corresponding Column line go low
		
		_delay_ms(1);		  // Wait for some time(debounce Time);
		
		ROW=0x0f;		  // After debounce time, perform the above operation
		key=COL & 0x0F;	  // to ensure the Key press.
		
	}while(key==0x0f);
	
}
unsigned char KEYPAD_ScanKey()
{
	
	unsigned char ScanKey = 0xe0,i, key;
	
	for(i=0;i<0x04;i++)           // Scan All the 4-Rows for key press
	{
		ROW=ScanKey + 0x0F;         // Select 1-Row at a time for Scanning the Key
		key=COL & 0x0F;             // Read the Column, for key press
		
		if(key!= 0x0F)             // If the KEY press is detected for the selected
		break;                   // ROW then stop Scanning,
		
		ScanKey=(ScanKey<<1)+ 0x10; // Rotate the ScanKey to SCAN the remaining Rows
	}
	
	key = key + (ScanKey & 0xf0);  // Return the row and COL status to decode the key
	
	return(key);
}
void KEYPAD_Init()
{
	RowColDirection=0xf0;   // Configure Row lines as O/P and Column lines as I/P
}
char KEYPAD_GetKey()
{
	unsigned char key;
	
	KEYPAD_WaitForKeyRelease();    // Wait for the previous key release
	_delay_ms(20);
	
	KEYPAD_WaitForKeyPress();      // Wait for the new key press
	key = KEYPAD_ScanKey();        // Scan for the key pressed.
	char k;
	switch(key)                       // Decode the key
	{
		case 0x7e: k=7; break;
		case 0x7d: k=8; break;
		case 0x7b: k=9; break;
		case 0x77: k='A'; break;
		case 0xbe: k=4; break;
		case 0xbd: k=5; break;
		case 0xbb: k=6; break;
		case 0xb7: k='B'; break; 
		case 0xde: k=1; break;
		case 0xdd: k=2; break;
		case 0xdb: k=3; break;
		case 0xd7: k='C'; break;
		case 0xee: k=0; break;
		case 0xed: k='F'; break;
		case 0xeb: k='E'; break;
		case 0xe7: k='D'; break;
		
		default: k='H';
	}
	return(k);                      // Return the key
}

void I2C_Start()
{
	TWCR = ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN));
	while (!(TWCR & (1<<TWINT)));
}
void I2C_Stop(void)
{
	TWCR = ((1<< TWINT) | (1<<TWEN) | (1<<TWSTO));
	_delay_us(10) ; //wait for a short time
}
void I2C_Write(unsigned char dat)
{
	TWDR = dat ;
	TWCR = ((1<< TWINT) | (1<<TWEN));
	while (!(TWCR & (1 <<TWINT)));
}
void I2C_Init()
{
	TWSR=0x00; //set presca1er bits to zero
	TWBR=0x46; //SCL frequency is 100K for XTAL = 7.3728M
	TWCR=0x04; //enab1e TWI module
}

void LCD_CmdWrite( char cmd)
{
	
	databus=(cmd & 0xf0);        // Send the Higher Nibble of the command to LCD
	control_bus &=~(1<<rs);  // Select the Command Register by pulling RS LOW
	//control_bus &=~(1<<rw);  // Select the Write Operation  by pulling RW LOW
	control_bus |=1<<en;     // Send a High-to-Low Pusle at Enable Pin
	_delay_us(1);
	control_bus &=~(1<<en);
	
	_delay_us(10);				// wait for some time
	
	databus=((cmd<<4) & 0xf0);   // Send the Lower Nibble of the command to LCD
	control_bus &=~(1<<rs);  // Select the Command Register by pulling RS LOW
	//control_bus &=~(1<<rw);  // Select the Write Operation  by pulling RW LOW
	control_bus |=1<<en;     // Send a High-to-Low Pusle at Enable Pin
	_delay_us(1);
	control_bus &=~(1<<en);
	
	_delay_ms(1);
}
void LCD_DataWrite( char dat)
{
	databus=(dat & 0xf0);	  // Send the Higher Nibble of the Data to LCD
	control_bus |=1<<rs;	  // Select the Data Register by pulling RS HIGH
	//control_bus &=~(1<<rw);	  // Select the Write Operation  by pulling RW LOW
	control_bus |=1<<en;	  // Send a High-to-Low Pusle at Enable Pin
	_delay_us(1);
	control_bus &=~(1<<en);
	_delay_us(10);
	
	databus=((dat <<4) & 0xf0); // Send the Lower Nibble of the Data to LCD
	control_bus |=1<<rs;	   // Select the Data Register by pulling RS HIGH
	//control_bus &=~(1<<rw);	   // Select the Write Operation  by pulling RW LOW
	control_bus |=1<<en;	   // Send a High-to-Low Pusle at Enable Pin
	_delay_us(1);
	control_bus &=~(1<<en);
	_delay_ms(1);
	
}
void LCD_Init()
{
	_delay_ms(50);
	databus_direction = 0xf0;  // Configure  databus as output
	controlbus_direction=0x06; // Configure  controlbus as output
	LCD_CmdWrite(0x02);	       //Initilize the LCD in 4bit Mode
	LCD_CmdWrite(0x28);
	LCD_CmdWrite(0x0C);	      // Display ON cursor off
	LCD_CmdWrite(0x01);	      // Clear the LCD
	LCD_CmdWrite(0x80);	      // Move the Cursor to First line First Position
	
}
void LCD_Clear()
{
	LCD_CmdWrite(0x01);	// Clear the LCD and go to First line First Position
	LCD_CmdWrite(LineOne);
}
void LCD_GoToLineOne()
{
	LCD_CmdWrite(LineOne);	// Move the Cursor to First line First Position
}
void LCD_DisplayString(char *string_ptr)
{
	while(*string_ptr)
	LCD_DataWrite(*string_ptr++);
}
void LCD_GoToXY(char row, char col)
{
	if (row == 0 && col<16)
	LCD_CmdWrite((col & 0x0F)|0x80);
	else if (row == 1 && col<16)
	LCD_CmdWrite((col & 0x0F)|0xC0);
	else if (row == 2 && col<16)
	LCD_CmdWrite(((col) & 0x0F)| 0x90);
	else if (row == 3 && col<16)
	LCD_CmdWrite(((col) & 0x0F)|0xD0);
	LCD_CmdWrite(0x0C); //enable cursor cursor off
}

uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with ’7? will always keep the value
	// of ‘ch’ between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
	
	// start single convertion
	// write ’1? to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes ’0? again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}
void adc_init()
{
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}


char menu[4][16] = {"1.Calibration\0", "2.Sensor Mode\0", "3.Input RGB\0", "4.Reset\0"};
char menusize=4;
char posCount=0;
char prev=-1;
char p=20;
int calibRGB[6]={-1,-1,-1,-1,-1,-1}; //{white_R,white_G,white_B,Black_R,Black_G,Black_B} *Boundries for the measurements
int senRGB[3]={0,0,0};
int RGBval[3]={0,0,0};
	
int BLUE=255;
int GREEN=0;
int RED=0;

void RGB_init()
{
	DDRB|=0X0e;
	TCCR2A=(1<<COM2A1)| (1<<WGM20) | (1<<WGM21);
	TIMSK2=(1<<TOIE2);
	OCR2A=BLUE;
	sei();
	TCCR2B=(1<<CS22)|(1<<CS21)|(1<<CS20);
	
	
	TCCR1A=(1<<COM1B1)| (1<<WGM12) | (1<<WGM10)|(1<<COM1A1);
	TIMSK1=(1<<TOIE1);
	OCR1A=RED;
	OCR1B=GREEN;
	sei();
	TCCR1B=(1<<CS10)|(1<<CS12);
	
}
ISR(TIMER2_OVF_vect){
	OCR2A=BLUE;
}
ISR(TIMER1_OVF_vect){
	OCR1B=GREEN;
	OCR1A=RED;
}
void RGBSENCEE_LIGHT(){
	RED=senRGB[0];
	GREEN=senRGB[1];
	BLUE=senRGB[2];
	RGB_init();
}
void RGBUPDATE_LIGHT(){
	RED=RGBval[0];
	GREEN=RGBval[1];
	BLUE=RGBval[2];
	RGB_init();
}
void RGBUPDATE_off(){
	RED=0;
	GREEN=0;
	BLUE=0;
	RGB_init();
}	
void updateMenu(){
	if (posCount>=menusize){posCount=posCount%(menusize);}
	if (prev!=posCount){
		LCD_Clear();	
		for (char i=0;i<4;i++){
			LCD_GoToXY(i,0);
			if (posCount==i){
				LCD_DisplayString(menu[i]);
				LCD_GoToXY(i,14);
				LCD_DisplayString("*");
				continue;
			}
			LCD_DisplayString(menu[i]);
		}
		prev==posCount;
	}
}	

char checkButtonPress(){
	char button;
	if (KEYPAD_GetKey()==65){
		button=1;
	}
	else if (KEYPAD_GetKey()==66){
		button=2;
	}
	else if (KEYPAD_GetKey()==67){
		button=3;
	}
	else if (KEYPAD_GetKey()==68){
		button=4;
	}
	else if (KEYPAD_GetKey()==69){
		button=5;
	}
	else {
		button=6;
	}
	
	return button;
}

bool checklist(char cc){
	
	for (char i=0;i<10;i++){
		if (cc=i){
			return 1;
		}
	
	}	
	return 0;
		
}
void RGBupdate(char t){  //not completed have to done more
	LCD_Clear();
	LCD_DisplayString("R   G   B");
	for (char i=0;i<3;i++){ //not completed interface part only
		LCD_GoToXY(1,i*4);
		char num_char[7];
		itoa(RGBval[i], num_char, 10);
		LCD_DisplayString(num_char);
		if (t==i+menusize){
			LCD_GoToXY(2,i*4);LCD_DisplayString("___");
		}
	}
}
void RGBupdate_mech(char t){
	RGBupdate(t);
	RGBval[t-4]=0;
	char turns=3;
	int total=0;
	while (turns>0){
		char c=KEYPAD_GetKey();
		if (c!='A'& c!='B' & c!='C' & c!='D' & c!='E'& c!='F' & c!='H'){
			if (turns==3 && c!=0){
			total=c*pow(10,turns-1)+total+1;}
			else if( turns==2 || turns==1 || (turns==3 && c==0)){total=c*pow(10,turns-1)+total;}
			
			
			if (total<256){
			RGBval[t-4]=total;
			RGBUPDATE_LIGHT();
			turns=turns-1;
			}
			else{
				total=RGBval[t-4];
				LCD_Clear();
				LCD_DisplayString("Include Values");
				LCD_GoToXY(1,0);
				LCD_DisplayString("Less than 255");
				_delay_ms(500);
				
			}
			RGBupdate(t);
		
		}
		else if (c=='A'){
			turns=0;}
		else if (c=='E'){
			LCD_GoToXY(3,10);
			LCD_DisplayString("**");
			_delay_ms(250);
			posCount=2;
			updateMenu();
			RGBUPDATE_off();
		}	
		
		
	}
	
}


void sensce(){//not complete
	unsigned char run=1;
	LCD_Clear();
	LCD_DisplayString("Hold the Color");
	_delay_ms(1000);
	LCD_GoToXY(2,0);
	LCD_DisplayString("Sensing start now");
	_delay_ms(500);
	LCD_Clear();
	LCD_DisplayString("Hold the Color");
	for (int i=0;i<3;i++){
		I2C_Start();            // Start I2C communication
		I2C_Write(0x40);   // Connect to DS1307 by sending its ID on I2c Bus
		I2C_Write(0x0f);     // Select the Ds1307 ControlRegister to configure Ds1307
		I2C_Write(run);        // Write 0x00 to Control register to disable SQW-Out
		I2C_Stop();
	
		_delay_ms(500);
		int tot=0;
		for (char j=0; j<50;j++){
			tot+=adc_read(0);
			_delay_ms(25);
		}
		senRGB[i]=255*tot/((calibRGB[i+3]-calibRGB[i])*50);
		
		I2C_Start();            // Start I2C communication
		I2C_Write(0x40);   // Connect to DS1307 by sending its ID on I2c Bus
		I2C_Write(0x0f);     // Select the Ds1307 ControlRegister to configure Ds1307
		I2C_Write(0x00);        // Write 0x00 to Control register to disable SQW-Out
		I2C_Stop();
		
		RGBSENCEE_LIGHT();
		run=run<<1;
	}
	_delay_ms(2000);
	RGBUPDATE_off();
}

void RGBcalib(){
	char calibmenu[2][16]={"Hold White","Hold Black"};
	for (char j=0;j<2;j++){
		unsigned char run=1;
		LCD_Clear();
		LCD_DisplayString(calibmenu[j]);
		for (int i=0;i<3;i++){
			I2C_Start();            // Start I2C communication
			I2C_Write(0x40);   // Connect to DS1307 by sending its ID on I2c Bus
			I2C_Write(0x0f);     // Select the Ds1307 ControlRegister to configure Ds1307
			I2C_Write(run);        // Write 0x00 to Control register to disable SQW-Out
			I2C_Stop();
			
			_delay_ms(500);
			int tot=0;
			for (char b=0;b<50;b++){
				tot+=adc_read(0);
				_delay_ms(25);
			}
			calibRGB[j*3+i]=tot/50;
			/*char num_char[7];
			itoa(calibRGB[j*3+i], num_char, 10);
			
			LCD_DisplayString(num_char);
			_delay_ms(1000);*/
			
			
			I2C_Start();            // Start I2C communication
			I2C_Write(0x40);   // Connect to DS1307 by sending its ID on I2c Bus
			I2C_Write(0x0f);     // Select the Ds1307 ControlRegister to configure Ds1307
			I2C_Write(0x00);        // Write 0x00 to Control register to disable SQW-Out
			I2C_Stop();
			run=run<<1;
		}
			
	}
}
int main(void)
{
	LCD_Init();
	KEYPAD_Init();
	adc_init();
	/*char aa=0;
	int a=adc_read(aa);
	char num_char[7];
	itoa(a, num_char, 10);
	
	LCD_DisplayString(num_char);
	_delay_ms(1000);*/

	//I2C_Init();
	//LCD_DisplayString(menu[1]);
	
	I2C_Init();             // Initilize the I2c module.
	I2C_Start();            // Start I2C communication
	I2C_Write(0x40);   // Connect to DS1307 by sending its ID on I2c Bus
	I2C_Write(0x0f);     // Select the Ds1307 ControlRegister to configure Ds1307
	I2C_Write(0x00);        // Write 0x00 to Control register to disable SQW-Out
	
	I2C_Stop();
	LCD_GoToXY(1,3);LCD_DisplayString("RGB Color");
	LCD_GoToXY(2,5);LCD_DisplayString("Sensor");
	_delay_ms(1000);
	LCD_Clear();
	updateMenu();
	
    /* Replace with your application code */
    while (1) 
    {
		switch (checkButtonPress()){
			case 1://Selection
				posCount=posCount+1;
				if (posCount<menusize){
					updateMenu();
					break;
				}
				else if(menusize<=posCount && posCount<menusize+3 && p!=20){
					//if (p==-1){LCD_DisplayString("wrong");_delay_ms(1000);}
					RGBupdate(posCount);
					break;
				}
				else if (posCount>=menusize+3 && p!=20){
					posCount=menusize;
					RGBupdate(posCount);
					break;
				}
				else{
					updateMenu();
					break;
				}
			case 2://0k
				if (posCount==0){//Caliberation Mode
					LCD_Clear();
					RGBcalib();
					_delay_ms(1000);
					updateMenu();//not complete
				}
				else if (posCount==1){// Sensor Mode
					LCD_Clear();
					if (calibRGB[0]!=-1){
						sensce();
						_delay_ms(250);
					}
					else{
						LCD_DisplayString("Calibrate First");
						_delay_ms(500);
						posCount=0; //Might give an error "HERE"
					}
					updateMenu();
				}
				else if (posCount==2){//RGB mode selection
					p=posCount;
					posCount=menusize;
					RGBupdate(posCount);
				}
				else if (posCount>=4){//Shifting inside RGB Mode
					LCD_GoToXY(3,10);
					LCD_DisplayString("**");
					_delay_ms(250);
					RGBupdate_mech(posCount);
					
					
				}
			case 3:
			   continue;
			case 4:
			   continue;   	
			case 5://back
				RGBUPDATE_off();
				if (posCount>=menusize){
					
					RGBUPDATE_off();
					char num_char[7];
					itoa(RED, num_char, 10);
					
					LCD_DisplayString(num_char);
					_delay_ms(1000);
					posCount=p;
					p=20;
					updateMenu();
				}
				
			
		}
		/*char aa=0;
		int a=adc_read(aa);
		char num_char[7];
		itoa(a, num_char, 10);
		
		LCD_DisplayString(num_char);
		_delay_ms(250);
		LCD_Clear();*/
		/*char a[1]={KEYPAD_GetKey()};
		LCD_DisplayString(a);*/
		//_delay_ms(2000);
		            // Initilize the I2c module.
		/*I2C_Start();            // Start I2C communication
		I2C_Write(0x40);   // Connect to DS1307 by sending its ID on I2c Bus
		I2C_Write(0x0f);     // Select the Ds1307 ControlRegister to configure Ds1307
		I2C_Write(0x0f);        // Write 0x00 to Control register to disable SQW-Out
		I2C_Stop();
		
		_delay_ms(1000);
		
		I2C_Init();             // Initilize the I2c module.
		I2C_Start();            // Start I2C communication
		I2C_Write(0x40);   // Connect to DS1307 by sending its ID on I2c Bus
		I2C_Write(0x0f);     // Select the Ds1307 ControlRegister to configure Ds1307
		I2C_Write(0x00);        // Write 0x00 to Control register to disable SQW-Out
		I2C_Stop();
		_delay_ms(1000);*/
    }
}