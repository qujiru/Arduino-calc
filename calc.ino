/*
 * Calculator on Arduino
 * 
 * keymap:
 *  Normal : 1 2 3 4 5 6 7 8 9 fn
 *  Fn mode: 0 X + - * / = DEL AC N
 * LCD: AQM1248A-RN
 */

#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <SPI.h>

#define RS 9
#define CS 10
#define AL 5
#define RATE 10000000L

void lcdCommand(uint8_t cmd) {
  digitalWrite(CS, LOW);
  digitalWrite(RS, LOW);
  SPI.transfer(cmd);
  digitalWrite(CS, HIGH);
}

void lcdData(uint8_t dat) {
  digitalWrite(CS, LOW);
  digitalWrite(RS, HIGH);
  SPI.transfer(dat);
  digitalWrite(CS,HIGH);
}

void lcdDatas(uint8_t *dat, int len) {
  for (int i=0; i<len; i++) {
    lcdData(dat[i]);
  }
}

void lcdChar(char c) {
  switch (c) {
    case '0': lcdData(0x1e);lcdData(0x31);lcdData(0x29);lcdData(0x1e);lcdData(0x0);lcdData(0x0);break;
case '1': lcdData(0x22);lcdData(0x3f);lcdData(0x20);lcdData(0x0);lcdData(0x0);lcdData(0x0);break;
case '2': lcdData(0x32);lcdData(0x29);lcdData(0x25);lcdData(0x22);lcdData(0x0);lcdData(0x0);break;
case '3': lcdData(0x12);lcdData(0x21);lcdData(0x29);lcdData(0x16);lcdData(0x0);lcdData(0x0);break;
case '4': lcdData(0x18);lcdData(0x14);lcdData(0x12);lcdData(0x3f);lcdData(0x10);lcdData(0x0);break;
case '5': lcdData(0x17);lcdData(0x29);lcdData(0x29);lcdData(0x29);lcdData(0x10);lcdData(0x0);break;
case '6': lcdData(0x1c);lcdData(0x2a);lcdData(0x29);lcdData(0x29);lcdData(0x10);lcdData(0x0);break;
case '7': lcdData(0x3);lcdData(0x39);lcdData(0x5);lcdData(0x3);lcdData(0x0);lcdData(0x0);break;
case '8': lcdData(0x16);lcdData(0x29);lcdData(0x29);lcdData(0x16);lcdData(0x0);lcdData(0x0);break;
case '9': lcdData(0x6);lcdData(0x29);lcdData(0x29);lcdData(0x1e);lcdData(0x0);lcdData(0x0);break;
    case ' ': for(int i=0; i<6; i++){lcdData(0x0);}break;
    default:  for(int i=0; i<6; i++){lcdData(0xff);}break;
  }
}

void lcdStr(char *str, int len) {
  for (int i=0; i<len; i++) {
    lcdChar(str[i]);
  }
}

void lcdNum(long long num) {
  if (num == 0) {
    lcdChar('0');
    return;
  }
  char buf[32] = {};
  ltoa(num, buf, 10);
  //sprintf(buf, "%lld", num);
  int dig = log10(num)+1;
  for (int i=0; i<dig; i++) {
    lcdChar(buf[i]);
  }
}

void initLcd() {
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.setDataMode(SPI_MODE3);
  lcdCommand(0xae);
  lcdCommand(0xa0);
  lcdCommand(0xc8);
  lcdCommand(0xa3);
  lcdCommand(0x2c);
  delay(2);
  lcdCommand(0x2e);
  delay(2);
  lcdCommand(0x2f);
  lcdCommand(0x23);
  lcdCommand(0x81);
  lcdCommand(0x1c);
  lcdCommand(0xa4);
  lcdCommand(0x40);
  lcdCommand(0xa6);
  lcdCommand(0xaf);
}

void clearScreen() {
  for (int page=0; page<6;page++) {
    lcdCommand(0xb0+page);  // set page address to $page
    lcdCommand(0x10);       // set most significant colom address 
    lcdCommand(0x00);       // set 
    for (int i=0; i<128; i++){
      lcdData(0);
    }
  }
}

int checkinput(int v) {
  if      (v < 600) return 1;
  else if (v < 700) return 2;
  else if (v < 800) return 3;
  else if (v < 835) return 4;
  else if (v < 864) return 5;
  else if (v < 885) return 6;
  else if (v < 902) return 7;
  else if (v < 915) return 8;
  else if (v < 926) return 9;
  else if (v < 936) return -1;
  else return 0;
}

void setup() {
  Serial.begin(9600);
  pinMode(RS, OUTPUT);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  SPI.begin();
  
  initLcd();

  clearScreen();
}

//====== global var ======
bool f_func = false;
bool f_prev = f_func;

long currentvalue = 0LL;
long buffer = 0;
char opr = 'n';
//====== global var end ======

void loop() {
  clearScreen();
  int cnt = 0;
  lcdCommand(0xb0+0);
  lcdCommand(0x10);    
  lcdCommand(0x00);
  int v = analogRead(5);
  int val = checkinput(v);
  if (val == -1) {
    f_func = !f_func;
    delay(30);
  }
  if (f_func) {   // fn mode
    lcdChar('a');
    lcdChar(' ');
    lcdNum(val);
    switch(val) {
      case 1: currentvalue *= 10; break;  // '0'
      case 2: break;    // '.'  UNDEF
      case 3: buffer = currentvalue; opr = '+'; currentvalue = 0; break;  // '+'
      case 4: buffer = currentvalue; opr = '-'; currentvalue = 0; break;  // '-'
      case 5: buffer = currentvalue; opr = '*'; currentvalue = 0; break;  // '*'
      case 6: buffer = currentvalue; opr = '/'; currentvalue = 0; break;  // '/'
      case 7: switch(opr) {       // '='
        case '+': currentvalue += buffer; break;
        case '-': currentvalue = buffer - currentvalue; break;
        case '*': currentvalue *= buffer; break;
        case '/': currentvalue = buffer - currentvalue; break;
      } buffer = 0; opr = 'n';break;
      case 8: currentvalue /= 10; break;    // 'DEL'
      case 9: currentvalue  =  0; break;    // 'AC'
    }
  } else {    //normal mode
    if (val > 0 && currentvalue < LONG_MAX / 10) {
      lcdNum(val);
      currentvalue *= 10;
      currentvalue += val;
    } else if (currentvalue >= LONG_MAX) {
      lcdChar('n');
      currentvalue = 0;
    }
  }
  lcdCommand(0xb0+2);  // set page address to 2
  lcdCommand(0x10);       // set most significant colom address
  lcdCommand(0x00);       // set
  lcdNum(currentvalue);
  delay(250);
}
