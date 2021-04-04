#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 4);
byte buttons[] = {6,7,8,9,10}; //10 for additional pin
const byte nrButtons = 5; // change to 4 if additional button added
int LDR=A0;
int menusize = 4;
int RGB[]={200,200,200};
int CalibRGB[]={-1,-1,-1,-1,-1,-1};
int senRGB[]={0,0,0};
String disp[]={"Red background","Green background","Blue background","Black background","Black background","Black background"};
const int red=0;
const int green=1;
const int blue=2;
//int LDR = A0; 

/*void caliber(){
  for (int i=0; i<3; i++){
    int n=0
    
    }
  
  }*/
String menu[] = {
  "1.Calibration",
  "2.Sensor Mode",
  "3.Input RGB",
  "4.Reset"
};
byte posCount=0;
byte prev=-1;

void updateMenu () {
  if (posCount>=menusize){posCount=posCount%(menusize);}
  if (prev!=posCount){
    lcd.clear();
    for(int i=0; i<menusize;i++){
      if (i==posCount){
        lcd.setCursor(0,i);lcd.print(menu[i]+"*");
        continue;
        }
      lcd.setCursor(0,i);lcd.print(menu[i]);  
      }
      prev==posCount;
    }
}

void RGBupdate(byte t){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("R   G   B");
  for (byte i=0;i<3;i++){
    if (t==i+menusize){
      lcd.setCursor(i*4,1);lcd.print(RGB[i]);lcd.print("*");
      }
  lcd.setCursor(i*4,1);lcd.print(RGB[i]);
  }
  }
void RGBprint(){
  lcd.clear();
  lcd.print("R   G   B");
  for (byte i=0;i<3;i++){
    
  lcd.setCursor(i*4,1);lcd.print(senRGB[i]);
  lcd.setCursor(-4,2);lcd.print("Press OK");
  lcd.setCursor(-4,3);lcd.print("to scan again");
  }
  
  }  
byte checkButtonPress() {
  byte bP = 0;
  byte rBp = 0;
  for (int t = 0; t<nrButtons;t++) {
    if (digitalRead(buttons[t]) == 0) {bP = (t + 1);}
  }
  rBp = bP;
  while (bP != 0) { // wait while the button is still down
    bP = 0;
    for (int t = 0; t<nrButtons;t++) {
      if (digitalRead(buttons[t]) == 0) {bP = (t + 1);}
    }
  }
  return rBp;
}
void calibration(){
  for (int i=0; i<6; i++){
    int tot=0;
    byte sensi=50;
    lcd.clear();
    lcd.setCursor(0,0);lcd.print(disp[i]);
    delay(500);
    digitalWrite((i%3),HIGH);
    
    for(byte n=0; n<50;n++){
      tot+=analogRead(A0);
      delay(10);
      }
      CalibRGB[i]=tot/sensi;
      digitalWrite((i%3),LOW);
    }
    lcd.clear();
     lcd.setCursor(0,0);lcd.print(CalibRGB[0]);lcd.print(CalibRGB[3]);
     delay(2000);
     lcd.clear();
     
  
  }
void sence(){
  for (int i=0; i<3; i++){
    int tot=0;
    byte sensi=50;
    /*lcd.clear();
    lcd.setCursor(0,0);lcd.print("");
    delay(500);*/
    digitalWrite((i),HIGH);
    
    for(byte n=0; n<50;n++){
      tot+=analogRead(A0);
      delay(10);
      }
      senRGB[i]=tot/sensi;
      digitalWrite((i%3),LOW);
    }
  
  }  

int p=-1;
void setup() {
  // put your setup code here, to run once:
   lcd.begin(16,4); 
  lcd.clear();
  
  for (int t=0;t<nrButtons;t++) {
    pinMode(buttons[t],INPUT_PULLUP); 
  }
  pinMode(red,OUTPUT);
  pinMode(green,OUTPUT);
  pinMode(blue,OUTPUT);
  
  lcd.print("hello");
  delay(1000);
  updateMenu();
  

}

void loop() {
  // put your main code here, to run repeatedly:
  switch (checkButtonPress()) {
  case 1://SELECTION
    posCount=posCount+1;
    if (posCount<menusize){
      updateMenu();
      break;}
    else if(menusize<=posCount and posCount<menusize+3 and p!=-1){
      RGBupdate(posCount);
      break;
      }  
    else if (menusize+3>=7 and p!=-1){
      posCount=menusize;
      RGBupdate(posCount);
      break;
      }
    else{
      updateMenu();
      break;}
        
  case 2://OK
    if (posCount==0){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Begin...");
      delay(2000);
      lcd.clear();
      calibration();
      lcd.setCursor(0,0);
      lcd.print("Caliberation Over...");
      delay(1000);
      lcd.clear();
      
      updateMenu();
      }
    else if (posCount==2){
      p=posCount;
      posCount=menusize;
      RGBupdate(posCount);
      delay(2000);
      }
    else if(posCount==1){
      if (CalibRGB[0]!=-1){
          lcd.clear();
          lcd.setCursor(1,3);lcd.print("Sencing..");
          sence();
          RGBprint();
        }
        else{
          lcd.clear();
        lcd.setCursor(0,0);lcd.print("Calibrate");
        lcd.setCursor(2,1);lcd.print("First");
        delay(1000);
        lcd.clear();
        posCount=0;
        updateMenu();
          }
      }  
        
    
    break;
  case 3://RGB UP
  lcd.clear();
    if (menusize<=posCount and posCount<menusize+3){
    RGB[posCount-menusize]+=1;
    if ( RGB[posCount-menusize]>255){
         RGB[posCount-menusize]=0;
        }
      RGBupdate(posCount);      
      }
    break;  
  case 4://RGB DOWN
      if (menusize<=posCount and posCount<menusize+3){
      RGB[posCount-menusize]-=1;
      if ( RGB[posCount-menusize]<0){
         RGB[posCount-menusize]=255;
        }
       RGBupdate(posCount);     
      }
      break;
  case 5://Back
  if (posCount>menusize){
    posCount=p;
    p=-1;
    updateMenu();}
  else{updateMenu();}      
    
    
      
  default:
    //lcd.clear();
    break;
}

}
