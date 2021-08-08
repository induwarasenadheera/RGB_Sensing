/*
 * Finalprto.c
 *
 * Created: 8/7/2021 6:34:40 PM
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

#define rs 1
#define en 2

#define LCDMaxLines 4
#define LCDMaxChars 20
#define LineOne 0x80 
#define LineTwo 0xc0
#define BlankSpace ' '

#define KEY_PRT 	PORTD
#define KEY_DDR		DDRD
#define KEY_PIN		PIND

char keypad[4][4] = {{' F',0,'E','D'},
{1,2,3,'C'},	
{4,5,6,'B'},	
{7,8,9,'A'}	};


unsigned char col1, row1;
char keyfind(void){
	
	while(1){
		DDRD = 0xF0;     // set the port B as input(column) and output(Row)
		PORTD =0xFF;	// set all the connected pins to high
		do
		{
			PORTD &= 0x0F;      // mask PORT for column read only
			asm("NOP");
			_delay_ms(40);
			col1 = (PIND & 0x0F);    // read status of column
		}while(col1 == 0x0F);       // if any button is pressed then do the following
		_delay_ms(100);
		// now check for rows
		PORTD = 0xEF;            /* check for pressed key in 1st row */
		asm("NOP");
		col1 = (PIND & 0x0F);
		if(col1 != 0x0F)
		{
			row1 = 0;
			break;
		}
		PORTD = 0xDF;		/* check for pressed key in 2nd row */
		asm("NOP");
		col1 = (PIND & 0x0F);
		if(col1 != 0x0F)
		{
			row1 = 1;
			break;
		}
		
		PORTD = 0xBF;		/* check for pressed key in 3rd row */
		asm("NOP");
		col1 = (PIND & 0x0F);
		if(col1 != 0x0F)
		{
			row1 = 2;
			break;
		}
		PORTD = 0x7F;		/* check for pressed key in 4th row */
		asm("NOP");
		col1 = (PIND & 0x0F);
		if(col1 != 0x0F)
		{
			row1 = 3;
			break;
		}
	}
	if(col1 == 0x0E){
		return(keypad[row1][0]);
	}
	else if(col1 == 0x0D){
		return(keypad[row1][1]);
	}
	else if(col1 == 0x0B){
		return(keypad[row1][2]);
	}
	else if(col1 == 0x07){
		return(keypad[row1][3]);
	}
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
	if (row == 0 && col<LCDMaxChars)
	LCD_CmdWrite((col & 0x0F)|0x80);
	
	else if (row == 1 && col<LCDMaxChars)
	LCD_CmdWrite((col & 0x0F)|0xC0);
	
	else if (row == 2 && col<LCDMaxChars)
	LCD_CmdWrite((((col) & 0x0F)+ 0x94)&0xFF);
	
	else if (row == 3 && col<LCDMaxChars)
	LCD_CmdWrite((((col) & 0x0F)+0xD4)&0xFF);
	
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


char menu[4][16] = {"1.Calibration\0", "2.Sensor Mode\0", "3.Input RGB\0", "4.Process RGB\0"};
char menusize=4;
char posCount=0;						//Calculate the Current Position
char prev=-1;
char p=20;

int calibRGB[6]={-1,-1,-1,-1,-1,-1};	//{white_R,white_G,white_B,Black_R,Black_G,Black_B} *Boundries for the measurements
int senRGB[3]={-1,-1,-1};				//{SenceMODE RED Val,SenceMODE GREEN Val,SenceMODE BLUE Val}
int RGBval[3]={0,0,0};					//{Input RGB MODE RED Val,Input RGB MODE GREEN Val,Input RGB MODE BLUE Val}
int adcRGB[3]={100,10,255};				//RGB VALUE GIVEN TO THE SENSOR
int regSen[3]={0,0,0};					//RGB VALUES TAKEN FROM  REGRESSION

void RGB_init()
{
	DDRB|=0X0e;
	TCCR2A=(1<<COM2A1)| (1<<WGM20) | (1<<WGM21);
	TIMSK2=(1<<TOIE2);
	OCR2A=adcRGB[2];//BLUE
	sei();
	TCCR2B=(1<<CS22)|(1<<CS21)|(1<<CS20);
	
	
	TCCR1A=(1<<COM1B1)| (1<<WGM12) | (1<<WGM10)|(1<<COM1A1);
	TIMSK1=(1<<TOIE1);
	OCR1A=adcRGB[0];//RED
	OCR1B=adcRGB[1];//GREEN
	sei();
	TCCR1B=(1<<CS10)|(1<<CS12);
	
}
ISR(TIMER2_OVF_vect){
	OCR2A=adcRGB[2];//BLUE
}
ISR(TIMER1_OVF_vect){
	OCR1B=adcRGB[1];//GREEN
	OCR1A=adcRGB[0];//RED
}

void RGBSENCEE_LIGHT(){
	//ASSIGN VALUES FROM SENCE MODE TO POWER THE RGB
	adcRGB[0]=senRGB[0];
	adcRGB[1]=senRGB[1];
	adcRGB[2]=senRGB[2];
	RGB_init();
}
void RGBUPDATE_LIGHT(){
	//ASSIGN VALUES FROM VALUE GET FROM RGB INPUT MODE TO POWER THE RGB
	adcRGB[0]=RGBval[0];
	adcRGB[1]=RGBval[1];
	adcRGB[2]=RGBval[2];
	RGB_init();
}

void RGBUPDATE_off(){
	//TURN OFF RGB
	adcRGB[0]=0;
	adcRGB[1]=0;
	adcRGB[2]=0;
	RGB_init();}
	
void updateMenu(){
	//FUCTION TO NAVIGATE THROUGH THE MENU AND PRINT THE MENU
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
	//CHECKING THE BUTTON That CONTROL THE DEVIES AND RETURN THE BUTTON NUMBER TO OPERATE THE CODE IN MAIN SWITH FUNCTION
	char button;
	if (keyfind()==65){
		button=1;
	}
	else if (keyfind()==66){
		button=2;
	}
	else if (keyfind()==67){
		button=3;
	}
	else if (keyfind()==68){
		button=4;
	}
	else if (keyfind()==69){
		button=5;
	}
	else {
		button=6;
	}
	
	return button;
}
void RGBupdate(char t){  
	//INTERFACE PART AND UPDATE PART OF INPUT RGB MODE
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
	//THE COMPLETE THING HAPPEN INSIDE INPUT RGB MODE
	//UDATING VALUE OF RGB AND POWERUP RGB
	RGBupdate(t);
	RGBval[t-4]=0;
	char turns=3;
	int total=0;
	while (turns>0){
		char c=keyfind();
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
void RGBcalib(){
	//TAKING INTIAL VLUES FOR WHITE AND BLACK FOR CALIBRATE THE DEVICE BY LIGHT UP RED, GREEN,BLUE LIGHTS AND GET THE LIGHT INTENSITY
	controlbus_direction|=0x38;
	char calibmenu[2][16]={"Hold White","Hold Black"};
	for (char j=0;j<2;j++){
		unsigned char run=8;
		LCD_Clear();
		LCD_DisplayString(calibmenu[j]);
		for (int i=0;i<3;i++){
			control_bus|=run;
			_delay_ms(1000);
			int tot=0;
			for (char b=0;b<50;b++){// TAKING 50 VALUES FROM THE SENSOR AND TAKING THE MEAN FROM IT 
				tot+=adc_read(0);
				_delay_ms(25);
			}
			calibRGB[j*3+i]=tot/50;
			char num_char[7];
			itoa(tot/50, num_char, 10);
			LCD_GoToXY(3,0);
			LCD_DisplayString(num_char);
			_delay_ms(1000);
			control_bus&=~(run);
			run=run<<1;
		}
			
	}
}


void sensce(){
	//SENSE A GIVEN COLOUR BY MEASURING RED GREEN BLUE REFLECTING LIGHT INTENSITY AND CALCULATE THE RGB VLAUES AND DISPLAY
	unsigned char run=8;
	LCD_Clear();
	LCD_DisplayString("Hold the Color");
	_delay_ms(1000);
	LCD_GoToXY(2,0);
	LCD_DisplayString("Sensing start now");
	_delay_ms(500);
	LCD_Clear();
	LCD_DisplayString("Hold the Color");
	controlbus_direction|=0x38;
	for (int i=0;i<3;i++){
		control_bus|=run;
		
		_delay_ms(500);
		int tot=0;
		for (char j=0; j<50;j++){//TAKING 50 VALUES FOR A BETTER OUTPUT 
			tot+=adc_read(0);
			_delay_ms(25);
		}
		senRGB[i]=255*tot/((calibRGB[i+3]-calibRGB[i])*50);
		regSen[i]=tot/50;
		char num_char[7];
		itoa(senRGB[i], num_char, 10);
		LCD_GoToXY(3,0);
		LCD_DisplayString(num_char);
		_delay_ms(1000);
		control_bus&=~(run);
	
		
		RGBSENCEE_LIGHT();
		run=run<<1;
	}
	_delay_ms(2000);
	RGBUPDATE_off();
}
void RGB_process(void){
	//THIS FUNCTION USE TO CALCULATE THE RGB VALUES FROM THE PROCESS DATA 
	//THIS VALUES THAT TAKEN FROM REGRESSION PROCESS IS UNIQUE TO THE SENSOR
	
	int R=-5290.766426+652.355371*pow(regSen[0],0.5)-26.12035*regSen[0]+0.353154*pow(regSen[0],1.5);
	int G=-2271.896090+335.979404*pow(regSen[1],0.5)-15.299415*regSen[1]+0.235826*pow(regSen[1],1.5);
	int B=-4185.025706+563.334615*pow(regSen[2],0.5)-24.261617*regSen[2]+0.351613*pow(regSen[2],1.5);
	R=(senRGB[0]+R)/2;
	G=(senRGB[1]+G)/2;
	B=(senRGB[2]+B)/2;
	adcRGB[0]=R;
	adcRGB[1]=G;
	adcRGB[2]=B;
	//CORRECT VALUES IF THEY EXCEEP THE RGB RANGE
	for (int i=0;i<3;i++){
		if (adcRGB[i]<0){
			adcRGB[i]=0;
		}
		else if(adcRGB[i]>255){
			adcRGB[i]=255;
		}
	}
	RGB_init();
	_delay_ms(5000);
	RGBUPDATE_off();
	
	
}

int main(void)
{
	//INITIALIZE LCD 
	LCD_Init();
	LCD_GoToXY(1,5);LCD_DisplayString("RGB Colour");
	LCD_GoToXY(2,7);LCD_DisplayString("Sensor");
	_delay_ms(1000);
	
	//INITIALIZE ANALOG TO DIGITAL COMMUNICATION
	adc_init();
	
	LCD_Clear();
	updateMenu();
	
    /* Replace with your application code */
    while (1) 
   {
		switch (checkButtonPress()){
			case 1://SELECTION BUTTON
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
			case 2://OK BUTTON
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
				else if(posCount==3){
					LCD_Clear();
					if(calibRGB[0]!=-1){
						if(senRGB[0]!=-1){
							RGB_process();
						}
						else{
							LCD_DisplayString("sense First");
							_delay_ms(500);
							posCount=1;
						}
					}else{
						LCD_DisplayString("Calibrate and Sense First");
						_delay_ms(500);
						posCount=0;
					}
					updateMenu();
			
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
			case 5://BACK BUTTON
				RGBUPDATE_off();
				if (posCount>=menusize){
					
					RGBUPDATE_off();
					
					posCount=p;
					p=20;
					updateMenu();
				}
				
			
		}
		
    }
}

