#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
byte buttons[] = {6,7,8}; //10 for additional pin
const byte nrButtons = 3; // change to 4 if additional button added
int menusize = 3;
String menu[] = {
  "1.Sensor Mode",
  "2.Input RGB",
  "3.Reset"
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



void setup() {
  // put your setup code here, to run once:
   lcd.begin(16,4); 
  lcd.clear();
  
  for (int t=0;t<nrButtons;t++) {
    pinMode(buttons[t],INPUT_PULLUP); 
  }
  lcd.print("hello");
  updateMenu();

}

void loop() {
  // put your main code here, to run repeatedly:
  switch (checkButtonPress()) {
  case 1:
    posCount=posCount+1;
    updateMenu();
    break;
  case 2:
    lcd.clear();
    break;
  default:
    //lcd.clear();
    break;
}

}
