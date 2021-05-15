#include <Keypad.h>
#include <Keypad_I2C.h> // Library for I2C keypad
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 4);

// for Sensor
#define red 7
#define green 8
#define blue 12

#define reset A1
#define LDR A0

// for RGB LED
#define R 9
#define G 10
#define B 11

static byte row = 0; //variable to identify the current row in the menu
static byte col = 0; //static means can update inside local scopes too
static bool branch1 = false; //varible to identify whether in main menu or in sensor mode
static bool branch2 = false; //varible to identify whether in main menu or in rgb mode
int r,g,b;

float data[] = {-1,-1,-1,-1,-1,-1,-1};
float data2[] = {-1,-1,-1};
String calib[]={"Calibrating","Red background","Green background","Blue background","Black for Red","Black for Green","Black for Blue"};

byte digit = 0; //variable to get each digit of r,g,b values
byte num = 1; //varible to identify current column in RGB mode
byte rgb[] = {0,0,0,11,11,0,0,0,11,11,0,0,0};
static bool stop_loop = false;

char keys[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
// A-Select, B-Up, C-Down, D-Back, *-Left, #-Right
byte row_pins[4] = {0,1,2,3}; // connect P0-P3 to the row pinouts of the keypad
byte col_pins[4] = {4,5,6,7}; // connect P4-P6 to the column pinouts of the keypad

Keypad_I2C pad(makeKeymap(keys), row_pins, col_pins, 4, 4, 0x20, PCF8574);

void menu() {
  branch1 = false;
  branch2 = false;
  row=0,col=0;
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("*1.Calibration");
  lcd.setCursor(1, 1); lcd.print("2.Sensor Mode");
  lcd.setCursor(-3, 2); lcd.print("3.Input RGB");
  lcd.setCursor(-3, 3); lcd.print("4.Reset");
}

void major() { // Function to switch between branches and menu
  while (true) {
    loop();
    if (stop_loop == true) {
      stop_loop = false;
      break;
    }
  }
}

void setup() {
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(LDR, INPUT);

  Serial.begin(9600);
  pad.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3, 1); lcd.print("RGB Colour");
  lcd.setCursor(1, 2); lcd.print("Sensor");
  delay(1000);
  menu();
}

void loop() {
  char key = pad.getKey();
  if (key!=NO_KEY) {
    Serial.println(key);
  }
  switch (key) {
    case 'A': //Select button
      if ((branch1==false) and (branch2==false)) {
        lcd.clear();
        if (row==0) {                         // Calibration Mode
          for (int i = 0; i < 7; i++) {
            if (i==1) {
              digitalWrite(red, HIGH);
            }
            else if (i==2) {
              digitalWrite(green, HIGH);
            }
            else if (i==3) {
              digitalWrite(blue, HIGH);
            }
            else if (i==4) {
              digitalWrite(red, HIGH);
            }
            else if (i==5) {
              digitalWrite(green, HIGH);
            }
            else if (i==6) {
              digitalWrite(blue, HIGH);
            }

            lcd.print(calib[i]);
            for (byte j = 0; j < 3; j++) {
              lcd.setCursor(4,2);
              lcd.print(String(3-j));
              delay(1000);
            }
            lcd.clear();

            float sum = 0;
            for (int k = 0; k < 50; k++) {
              float val = analogRead(LDR);
              sum = sum + val;
            }
            float avg = sum/50;
            data[i] = avg;
            Serial.println(String(avg) + " " + calib[i]);

            delay(50);

            digitalWrite(red, LOW);
            digitalWrite(green, LOW);
            digitalWrite(blue, LOW);
          }
          menu();
        }
        else if (row==1) {                    // Sensor Mode
          if (data[0]==-1) {
            lcd.setCursor(0, 1); lcd.print("Calibrate first");
            delay(500);
            menu();
          }
          else {
            pos1:
            lcd.clear();
            lcd.setCursor(4, 1); lcd.print("Wait...");
            for (byte i = 0; i < 3; i++) {
              if (i==0) {
                digitalWrite(red, HIGH);
              }
              else if (i==1) {
                digitalWrite(green, HIGH);
              }
              else if (i==2) {
                digitalWrite(blue, HIGH);
              }
              delay(1000);

              float sum = 0;
              for (int k = 0; k < 50; k++) {
                float val = analogRead(LDR);
                sum = sum + val;
              }
              float avg = sum/50;
              data2[i] = avg;

              Serial.println(String(avg));

              delay(50);

              digitalWrite(red, LOW);
              digitalWrite(green, LOW);
              digitalWrite(blue, LOW);
            }
            r = ((data2[0]-data[4])/(data[1]-data[4]))*255;
            g = ((data2[1]-data[5])/(data[2]-data[5]))*255;
            b = ((data2[2]-data[6])/(data[3]-data[6]))*255;

            Serial.println(String(r)+' '+String(g)+' '+String(b));

            analogWrite(R,r);
            analogWrite(G,g);
            analogWrite(B,b);
          }
          lcd.clear();
          lcd.setCursor(0, 0); lcd.print("Again - Press SELECT");
          lcd.setCursor(8, 1); lcd.print("SELECT");
          lcd.setCursor(-4, 2); lcd.print("Menu  - Press");
          lcd.setCursor(4, 3); lcd.print("BACK");
          branch1 = true;
          major();
          menu();
        }
        else if (row==2) {                    // Input RGB Mode
          pos2:
          lcd.clear();
          lcd.print("  R    G    B ");
          lcd.setCursor(0,1);
          lcd.print(" 000  000  000 ");
          lcd.setCursor(-3,2);
          lcd.print("-");
          num = 1;
          branch2 = true;
          major();
          menu();
        }
        else if (row==3) {                    // Reset data
          analogWrite(reset,255);
        }
      }
      else if (branch1==true) {
        analogWrite(R,0);
        analogWrite(G,0);
        analogWrite(B,0);
        Serial.println("Again sensor");
        goto pos1;
      }
      else if (branch2==true) {
        r = (String(rgb[0])+String(rgb[1])+String(rgb[2])).toInt();
        g = (String(rgb[5])+String(rgb[6])+String(rgb[7])).toInt();
        b = (String(rgb[10])+String(rgb[11])+String(rgb[12])).toInt();
        r = (r>255) ? 255:r;
        g = (g>255) ? 255:g;
        b = (b>255) ? 255:b;
        analogWrite(R,r);
        analogWrite(G,g);
        analogWrite(B,b);
        Serial.println(String(r)+' '+String(g)+' '+String(b));
        num = 1;
        memset(rgb,0,sizeof(rgb));
        goto pos2;
      }
    break;

    case 'B': //Up button
      if ((branch1==false) and (branch2==false)) {
        col = (row<2) ? 0:-4;
        lcd.setCursor(col, row); lcd.print(" ");
        row = (row == 0) ? 3:row-1;
        col = (row<2) ? 0:-4;
        lcd.setCursor(col, row); lcd.print("*");
      }
      else {
        Serial.println("Nothing happens");
      }
    break;

    case 'C': //Down button
      if ((branch1==false) and (branch2==false)) {
        col = (row<2) ? 0:-4;
        lcd.setCursor(col, row); lcd.print(" ");
        row = (row == 3) ? 0:row+1;
        col = (row<2) ? 0:-4;
        lcd.setCursor(col, row); lcd.print("*");
      }
      else {
        Serial.println("Nothing happens");
      }
    break;

    case '#': //Right button
      if (branch2==true) {
        lcd.setCursor(-4+num, 2); lcd.print(" ");
        if ((num==3) or (num==8)) {
          num = num + 3;
        }
        else if (num==13) {
          num = 1;
        }
        else {
          num = num + 1;
        }
        lcd.setCursor(-4+num, 2); lcd.print("-");
      }
      else {
        Serial.println("Nothing happens");
      }
    break;

    case '*': //Left button
      if (branch2==true) {
        lcd.setCursor(-4+num, 2); lcd.print(" ");
        if ((num==6) or (num==11)) {
          num = num - 3;
        }
        else if (num==1) {
          num = 13;
        }
        else {
          num = num - 1;
        }
        lcd.setCursor(-4+num, 2); lcd.print("-");
      }
      else {
        Serial.println("Nothing happens");
      }
    break;

    case 'D': //Back button
      if ((branch1==true) or (branch2==true)) {
        analogWrite(R,0);
        analogWrite(G,0);
        analogWrite(B,0);
        stop_loop = true;
      }
      else {
        Serial.println("Nothing happens");
      }
    break;

    case '0':
    case '1':
      if (branch2==true) {
        digit = String(key).toInt();
        lcd.setCursor(num, 1); lcd.print(key);
        rgb[num-1] = digit;
      }
      else {
        Serial.println("Nothing happens");
      }
    break;

    case '2':
      if (branch2==true) {
        digit = (rgb[num]>5) ? 1:2;
        lcd.setCursor(num, 1); lcd.print(digit);
        rgb[num-1] = digit;
      }
      else {
        Serial.println("Nothing happens");
      }
    break;

    case '3':
    case '4':
    case '5':
      if (branch2==true) {
        digit = String(key).toInt();
        if ((num==1) or (num==6) or (num==11)) {
          digit = (rgb[num]>5) ? 1:2;
        }
        lcd.setCursor(num, 1); lcd.print(digit);
        rgb[num-1] = digit;
      }
      else {
        Serial.println("Nothing happens");
      }
    break;

    case '6':
    case '7':
    case '8':
    case '9':
      if (branch2==true) {
        if ((num==1) or (num==6) or (num==11)) {
          digit = (rgb[num]>5) ? 1:2;
        }
        else if ((num==2) or (num==7) or (num==12)) {
          digit = (rgb[num-2]==2) ? 5:String(key).toInt();
        }
        else if ((num==3) or (num==8) or (num==13)) {
          if (rgb[num-3]==2) {
            digit = (rgb[num-2]>=5) ? 5:String(key).toInt();
          }
        }
        lcd.setCursor(num, 1); lcd.print(digit);
        rgb[num-1] = digit;
      }
      else {
        Serial.println("Nothing happens");
      }
    break;
  }
}
