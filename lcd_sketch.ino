#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#include <Wire.h>
#include <avr/wdt.h>

#define ledPin 13 // Onboard LED
#define LINE_SIZE 16
#define PAGE_SIZE 32

byte c_ellipsis[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10101,
  B00000,
};

LCDKeypad lcd;

// Restarts program from beginning but does not reset the peripherals and registers
void _SoftwareReset() {
  wdt_enable(WDTO_15MS);
}

//Set LCD Keypad Shield backlight brightness
void setLCDBrightness(byte brightness) {
  analogWrite(10, brightness);
}

void setup() {
  
 lcd.createChar(1,c_ellipsis);
 lcd.begin(16, 2);
 lcd.clear();
 lcd.print("Arduino started");
 setLCDBrightness(100);
 pinMode(ledPin, OUTPUT);
 digitalWrite(ledPin, LOW);

 Wire.begin(0x08); // 0x08 is the arduino dock 2 i2c port
 Wire.onReceive(i2c_on_receive_handler);
 //Wire.onRequest(i2c_on_request_handler);
}


// called when a transmission is received via the i2c port
void i2c_on_receive_handler(int bytes_received) {
  char cmd = Wire.read();
  char str[33];
  char str1[17];
  char str2[17];
  int i;
      
  switch (cmd) {
    case 0xde:
      char next;
      next = Wire.read();
      if (next == 0xad) {
        _SoftwareReset();
      }
      break;
     case 1:
      setLCDBrightness(Wire.read());
      break;
     default:
      i = 0;
      while (Wire.available() > 0) {
        str[i++] = char(Wire.read());
      }
      for(int j=0; j < min(16,i); j++)
      {
        str1[j] = str[j];
      }
      str1[min(16,i)] = 0;
      for(int j=16; j < min(32,i); j++)
      {
        str2[j-16] = str[j];
      }
      str2[max(0,min(16,i-16))] = 0;
      
      for(i=0; i < 16; i++) {
        lcd.scrollDisplayLeft();
        delay(150);
      }
      lcd.noAutoscroll();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(str1);
      lcd.setCursor(0,1);
      lcd.print(str2);
  }
}

int key;
int b = 100;

void loop() {
  key = analogRead (0);
  if (key > 30) {
    if(key < 150 && b < 240) {
      b += 20;
    }
    else if(key >= 150 && key < 360 && b > 20) {
      b -= 20;
    }
    setLCDBrightness(b);
  }
  delay(100);
}
