#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#include <Wire.h>
#include <avr/wdt.h>
#include <string.h>

#define ledPin 13 // Onboard LED
#define LINE_SIZE 16
#define PAGE_SIZE 32
#define NUM_PAGES 30
#define PAGE_FLIP_TIME 2500

char pages[NUM_PAGES][PAGE_SIZE+1];
int curr_page = -1;
int max_page = -1;
int scrolled_page = 0;
int scroll_return_deadline;
int page_flip_deadline;
bool need_redraw = false;

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

byte c_start[8] = {
  B11100,
  B10010,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111,
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

void print_page(int n) {
  char str[2][17];

  memset(&str[0],0,17);
  memset(&str[0],0,17);

  for(int j=0; j<32; j++)
  {
    str[j/16][j%16] = pages[n][j];
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(str[0]);
  lcd.setCursor(0,1);
  lcd.print(str[1]);
}

void setup() {

 lcd.createChar(1,c_ellipsis);
 lcd.createChar(2,c_start);
 lcd.begin(16, 2);
 lcd.clear();
 lcd.noAutoscroll();
 lcd.print("Arduino started\0x01");
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
      curr_page++;
      if(curr_page == NUM_PAGES) curr_page = 0;
      memset(&pages[curr_page], 0, PAGE_SIZE+1);
      int i = 0;
      while (Wire.available() > 0) {
        pages[curr_page][i++] = char(Wire.read());
      }
      need_redraw = true;
      if(max_page<NUM_PAGES-1)
      {
        max_page = curr_page;
      }
  }
}

int key;
int b = 100;
#define DELAY_MS 60

void loop() {
  while(true) {
    key = analogRead (0);
    if (key < 760) {
      if(key < 30) {
          scrolled_page++;
          if(scrolled_page>max_page) scrolled_page = 0;
          need_redraw = true;
      } else if(key < 150 && b < 240) {
        b += 20;
      }
      else if(key < 360 && b > 20) {
        b -= 20;
      } else if (key < 535) {
          scrolled_page--;
          if (scrolled_page < 0) scrolled_page = max_page;
          need_redraw = true;
      }
      setLCDBrightness(b);
    }
    if(need_redraw && (max_page >= 0)) {
      print_page(scrolled_page);
      need_redraw = false;
      page_flip_deadline = PAGE_FLIP_TIME;
    }
    if(page_flip_deadline>0) {
      page_flip_deadline-=DELAY_MS;
    } else {
      scrolled_page++;
      if(scrolled_page>max_page) scrolled_page = 0;
      need_redraw = true;
    }
    delay(DELAY_MS);
  }
}

