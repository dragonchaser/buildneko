#include <SPI.h>
#include <Wire.h>
#include <Ethernet.h>
#include <Servo.h>
#include <FastLED.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// begin servo
Servo servo;
uint8_t pos = 90;
// end servo

// begin network
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
EthernetServer server(80);
// end network

// begin led
#define NUM_LEDS 2
#define DATA_PIN 2
CRGB eyes[NUM_LEDS];
// end led

// begin display
#define OLED_MOSI  5 // alt: 9
#define OLED_CLK   7 // alt: 10
#define OLED_DC    1 // alt: 11
#define OLED_CS    12
#define OLED_RESET 3 // alt: 13

#define SERVO_ACTIVE
#define NOOB_MODE

Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
//end display
void setup()   {
  // Initialise Display
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  Ethernet.begin(mac);
  server.begin();
  // Attach Servo
  servo.attach(6);
  // Led
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(eyes, NUM_LEDS);
  eyes[0] = CRGB(0, 0, 16);
  eyes[1] = CRGB(0, 0, 16);
  FastLED.show();
  delay(2000);
  eyes[0] = CRGB(0, 0, 0);
  eyes[1] = CRGB(0, 0, 0);
  FastLED.show();
  // Display
  display.begin(SSD1306_SWITCHCAPVCC);
  refreshDisplay();
    
  doWink(3);doWink(3);doWink(3);
  // init done
}

void loop() {
  // listen for incoming clients
  String request  = "";
  uint8_t len = 0;
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if(len > 4 && len < 24) {
          request += c;
        }
        if(len < 24) {
          len++;
        }
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          bool evalSuccess = evaluateRequest(request);
          // send a standard http response header
          if(evalSuccess) {
            client.println("HTTP/1.1 200 OK");
          } else {
            client.println("HTTP/1.1 418 I AM A TEAPOT!");
          }
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          if(evalSuccess) {
            client.println("200-OK");
          } else {            
            client.println("<pre>");
            client.println("418-I AM A TEAPOT");
            client.println("E-clrscr");
            client.println("R-redraw");
            client.println("Tsizetext-print");
            client.println("Crrggbbrrggbb-eyes");
            client.println("Wtimesdelay-wink");
            client.println("</pre>");
          }
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    //delay(1);
    // close the connection:
    client.stop();
  }
}

bool evaluateRequest(String request) {
  switch(request[0]) {
    case 'E':
      display.clearDisplay();
      display.setCursor(0,0);
      display.display();
      break;
    case 'R':
      refreshDisplay();      
      //retVal = 'r';
      break;
    case 'W':
      if(request[1] != ' ' && request[2] != ' ') {
        uint8_t am = (request[1] - '0');
        if(am > 5) {
          am = 5;
        }        
        uint8_t del = (request[2] - '0');
        if(del > 10) {
          del = 10;
        }
        if(del < 2) {
          del = 2;
        }
        for(uint8_t i = 0; i < am; i++) {
          doWink(del);
        }
      }
      break;
    case 'C':
      if(request.length() >= 13) {
        char r1[2] = {request[1], request[2]};
        char g1[2] = {request[3], request[4]};
        char b1[2] = {request[5], request[6]};
        char r2[2] = {request[7], request[8]};
        char g2[2] = {request[9], request[10]};
        char b2[2] = {request[11], request[12]};
        eyes[0] = CRGB(hex2int(r1, 2) / 8, hex2int(g1, 2) / 8, hex2int(b1, 2) / 8);
        eyes[1] = CRGB(hex2int(r2, 2) / 8, hex2int(g2, 2) / 8, hex2int(b2, 2) / 8);
        FastLED.show();
      } else {
        eyes[0] = CRGB::HotPink;
        eyes[1] = CRGB::HotPink;
      }
      break;
    case 'T':
      if(request[1] != ' ' && request[2] != ' ') {
        uint8_t tSize = (request[1] - '0');
        String tmp = "";
        String cS = request.substring(2);
        for(uint8_t i = 0; i < cS.length(); i++) {
          if(!(cS[i] == ' ')) {
            if(cS[i] == '_') {
              tmp = tmp + ' ';
            } else {
              tmp = tmp + cS[i];
            }
          } else {
            break;
          }
        }
        display.setTextSize(tSize);
        display.println(tmp);
        display.display();
      }
      break;
    default:
      return false;
      break;
  }
  return true;
}

void doWink(uint8_t winkDelay) {
#ifdef SERVO_ACTIVE
  for (pos = 110; pos <= 180; pos++) // goes from 0 degrees to 180 degrees
  { // in steps of 1 degree
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(winkDelay);                       // waits 15ms for the servo to reach the position
  }

  for (pos = 180; pos >= 110; pos--)  // goes from 180 degrees to 0 degrees
  {
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(winkDelay);                       // waits 15ms for the servo to reach the position
  }
#endif
}

void refreshDisplay() {
  // text display tests
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  IPAddress addr = Ethernet.localIP();
  display.print("http://"); display.print(String(addr[0])); display.print("."); display.print(String(addr[1])); display.print("."); display.print(String(addr[2])); display.print("."); display.println(String(addr[3]));
  display.println();
  display.setTextSize(2);
  display.println("BUILD-NEKO");
  display.display();
}


unsigned long hex2int(char *a, unsigned int len)
{
   int i;
   unsigned long val = 0;

   for(i=0;i<len;i++)
      if(a[i] <= 57)
       val += (a[i]-48)*(1<<(4*(len-1-i)));
      else
       val += (a[i]-55)*(1<<(4*(len-1-i)));
   return val;
}

