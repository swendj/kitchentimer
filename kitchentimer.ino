/*****************************************************************************
  This is the "kitchentimer" sketch.
  Module        : main module, kitchentimer.ino
  Author        : Swen Hopfe (dj)
  Design        : 2018-12-20
  Last modified : 2019-01-09
  It works with the Adafruit LED 7-Segment backpacks
  with I2C interface.  It was tested with ESP32-ST,
  wiring for a wroom eboxmaker himalaya board.
 ****************************************************************************/

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include <Adafruit_GFX.h>
#include <WiFi.h>

// WiFi and web app stuff
const char* ssid      = "your_ssid";
const char* password  = "your_password";
WiFiServer server(80);
boolean confl = false;
const char* local_url = "192.168.178.xx";

// initialize segment display
Adafruit_7segment display = Adafruit_7segment();

// input button
const int input_pin = 4;

// output for piezo
const int signal_pin = 2;

// to switch the colon
boolean colon = false;

// one minute shortest time to go
int minutes_togo = 1;

// global display status, initial value
String kstate = "-1:--";

// action switch
// 0 - idle state
// 1 - push button pressed in status "Start" to start countdown
// 2 - push button pressed in status "Running" to stop countdown
int sfl = 0;

//----------------------------------------------------------------------------
// sound

// beeps num times with delay of 100ms
void beep(int num) {
  for(int i = 0; i < num; i++ ) {
    digitalWrite(signal_pin, true); delay(100);
    digitalWrite(signal_pin, false); delay(100);
  }
}

// switch piezo on
void beep_on() {
  digitalWrite(signal_pin, true);
  }

// switch piezo off
void beep_off() {
  digitalWrite(signal_pin, false);
  }

//----------------------------------------------------------------------------
// web server

int web(String kt_state, String refr, String bsign) {

  int retval = 0;

  // kt_state : string for the virtual display
  // refr     : refresh rate of the current site
  // bsign    : how the push button should look like

  WiFiClient client = server.available();

  // if there is a client
  if (client) {
    Serial.println("New client.");
    String linebuf = "";

    while (client.connected()) {
      if (client.available()) {
        char rd = client.read();

        // end of request is marked with an empty line and the 'newline' character
        if (rd == '\n') {
          if (linebuf.length() == 0) {
            // now we have to send our response to the client, the webpage

            // the web page header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.println("<!DOCTYPE html><html>");
            client.println("<head><META name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<META HTTP-EQUIV=\"refresh\" CONTENT=\"" + refr + "; url=http://" + local_url + "\">");

            // some styling
            client.println("<style>html {font-family: \"Helvetica\",\"Arial\",sans-serif; display: inline-block;");
            client.println("margin: 0px auto; text-align: center;}");
            client.println("h1 , h2 { font-family: \"Helvetica\",\"Arial\",sans-serif;}");
            if(bsign == "Start") client.println(".button1 { font-family: monospace; background-color: #51AE4B; border: none; color: white; padding: 15px 15px;");
            if(bsign == "Stop") client.println(".button1 { font-family: monospace; background-color: #999900; border: none; color: white; padding: 15px 15px;");
            if(bsign == "Stopped") client.println(".button1 { font-family: monospace; background-color: #0000FF; border: none; color: white; padding: 15px 15px;");
            if(bsign == "Up!") client.println(".button1 { font-family: monospace; background-color: #FF0000; border: none; color: white; padding: 15px 15px;");
            client.println("text-decoration: none; margin: 10px; font-size: 20px;}");
            client.println(".button2 { font-family: monospace; background-color: #999; border: none; color: white; padding: 15px 15px;");
            client.println("text-decoration: none; margin: 10px; font-size: 14px;}");
            client.println("</style></head>");

            // web page body
            client.println("<body><h2>kitchen timer</h2>");
            client.println("<p><h1>" + kt_state + "</h1></p>");

            if(bsign == "Stop") {
              client.println("<p><a href=\"?b1/stop\"><button class=\"button1\">" + bsign + "</button></a></p>");
            } else if (bsign == "Start") {
              client.println("<p><a href=\"?b1/start\"><button class=\"button1\">" + bsign + "</button></a></p>");
            } else {
              client.println("<p><button class=\"button1\">" + bsign + "</button></p>");
            }

            // show increase / decrease buttons only when not running
            if(bsign == "Start") {
              client.println("<p><a href=\"?b2/inc\"><button class=\"button2\">+ 1</button></a>");
              client.println("<a href=\"?b2/i10\"><button class=\"button2\">+10</button></a></p>");
              client.println("<p><a href=\"?b2/dec\"><button class=\"button2\">- 1</button></a>");
              client.println("<a href=\"?b2/d10\"><button class=\"button2\">-10</button></a></p>");
            }

            // let's close the tags and end up with a blank line
            client.println("</body></html>");
            client.println();
            break;

          } else {
            // at the end of line ('\n') clear buffer
            linebuf = "";
          }

        } else if (rd != '\r') {
          // all to add except '\r'
          linebuf += rd;
          }

        //if (rd == '\r') Serial.println(linebuf);

        // check as what the button was pushed
        if (linebuf.endsWith("GET /?b1/start HTTP/1.1")) {
          Serial.println("[start] pressed.");
          retval = 1;
          }
        if (linebuf.endsWith("GET /?b1/stop HTTP/1.1")) {
          Serial.println("[stop] pressed.");
          retval = 2;
          }
        if (linebuf.endsWith("GET /?b2/inc HTTP/1.1")) {
          Serial.println("[up] pressed.");
          retval = 4;
          }
        if (linebuf.endsWith("GET /?b2/dec HTTP/1.1")) {
          Serial.println("[down] pressed.");
          retval = 5;
          }
        if (linebuf.endsWith("GET /?b2/i10 HTTP/1.1")) {
          Serial.println("[up] pressed.");
          retval = 6;
          }
        if (linebuf.endsWith("GET /?b2/d10 HTTP/1.1")) {
          Serial.println("[down] pressed.");
          retval = 7;
          }

      }
    }
    // close the connection:
    client.stop();
    Serial.println("disconnected.");
  }
  return(retval);
}

//----------------------------------------------------------------------------

void setup() {

  // set pin for input purpose
  pinMode(input_pin, INPUT);

  // set signal pin to output
  pinMode(signal_pin, OUTPUT);
  digitalWrite(signal_pin, false);

  // start serial
  Serial.begin(115200);
  Serial.println("Started...");

  // connect to display with adress 0x70
  display.begin(0x70);

  // WiFi connect
  WiFi.begin(ssid, password);
  int wc = 0;
  // wait for connection 6*0.5=3sec
  while ((WiFi.status() != WL_CONNECTED) && (wc < 7)) {
      if(wc < 5) { if(wc==2)wc++; display.writeDigitNum(wc, 1, false);}
      display.writeDisplay();
      delay(500);
      wc++;
  }
  // message to serial and display
  // and start the server if connected
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    display.print(0xc0, HEX);
    display.writeDisplay();
    confl = true;
    // beep 3 times
    beep(3);
  }
  else {
    Serial.println("WiFi not connected.");
    display.print(0xee, HEX);
    display.writeDisplay();
    // beep 2 times
    beep(2);
  }
  delay(2000);

  if(confl)server.begin();

  // Regular start screen
  display.println(10000);
  // this was to cause an error to display start screen "-- --"
  display.writeDigitNum(1, minutes_togo, false);
  // we've got "-1 --" now
  display.writeDisplay();
}

//----------------------------------------------------------------------------

void loop() {

  // go in if push button pressed
  if ((digitalRead(input_pin)==HIGH) || (sfl==1)) {

    sfl = 0;
    // display to go time for countdown
    display.println(minutes_togo);
    display.writeDisplay();

    delay(2300);
    //--------------------------------------------------
    // if button pressed for additional 3sec
    // increase togo time for 1min each cycle
    // and show it
    // time limit is 90min
    while ((digitalRead(input_pin)==HIGH)&&(minutes_togo < 91)){
      minutes_togo++;
      display.println(minutes_togo);
      display.writeDisplay();
      // shorting waiting time later on
      int delay_time = 1800;
      if(minutes_togo > 2) delay_time = 800;
      if(minutes_togo > 4) delay_time = 400;
      delay(delay_time);
    }
    //--------------------------------------------------
    // starting the countdown, we count seconds (minutes_togo * 60) down
    int cmin = minutes_togo;
    int csec = 0;
    boolean break_flag = false;

    for (int cntdwn = (minutes_togo * 60); cntdwn > 0; cntdwn--) {
      display.print(0000);
      display.drawColon(true);
      // splitting minutes and seconds...
      display.writeDigitNum(0, (cmin / 10) % 10, false);
      display.writeDigitNum(1, (cmin / 1) % 10, false);
      display.writeDigitNum(3, (csec / 10) % 10, false);
      display.writeDigitNum(4, (csec / 1) % 10, false);
      display.writeDisplay();
      // 950ms (below 1s) because there is something to do
      delay(950);
      // it's a "60sec/1min" system
      if (csec == 0) {csec = 60; cmin--;}
      csec--;
      //-----------------------------
      // ending up if button pressed during countdown
      // in this case the togo timer we will reset to 1min
      if ((digitalRead(input_pin)==HIGH) || (sfl==2)) {
        // proclaim it with "EEEE"
        Serial.println("Countdown stopped.");
        display.print(0xEEEE, HEX);
        display.writeDisplay();
        beep_on();
        delay(1000);
        beep_off();
        display.println(10000);
        display.writeDisplay();
        minutes_togo = 1;
        break_flag = true;
        delay(1000);
        break;
      }
      // serve the web here
      String cmin_str = String(cmin);
      String csec_str = String(csec);
      if(cmin_str.length() < 2) cmin_str = "0" + cmin_str;
      if(csec_str.length() < 2) csec_str = "0" + csec_str;
      kstate = cmin_str + ":" + csec_str;
      sfl = web(kstate, "4", "Stop");
    }
    //--------------------------------------------------
    // when time's up
    // perform the completion signal for 20 times
    // not to do, when broke before
    if (break_flag == false) {
      Serial.println("Time's up.");
      for (int cs = 0; cs < 40; cs++) {
        // let's change the display
        // from "8888"
        // ..to "----"
        beep_on();
        display.println(8888);
        display.writeDisplay();
        delay(200);
        beep_off();
        display.println(10000);
        display.writeDisplay();
        delay(200);
        // serve the web here
        kstate = "Time is up!";
        sfl = web(kstate, "4", "Up!");
        delay(500);
      }
    delay(1000);
    }
    else {
      for (int cs = 0; cs < 10; cs++) {
      delay(500);
      kstate = "Stopped.";
      sfl = web(kstate, "4", "Stopped");
      delay(500);
      }
    delay(1000);
    }

    // get the last togo timer "tt --" to display
    if(minutes_togo > 9) display.writeDigitNum(0, (minutes_togo / 10) % 10, false);
    display.writeDigitNum(1, (minutes_togo / 1) % 10, false);
    display.writeDisplay();
    // for the virtual display too
    if(minutes_togo <= 9) kstate = "-" + String(minutes_togo) + ":--";
    if(minutes_togo > 9) kstate = String(minutes_togo) + ":--";

    //--------------------------------------------------

  }

  delay(1000);
  // in idle state switch colon
  display.drawColon(colon);
  display.writeDisplay();
  // generally, this does the display switch
  // from "-- --"
  // ..to "--:--"
  if (colon) colon = false; else colon = true;

  // serve the web here
  sfl = web(kstate, "8", "Start");
  if((sfl > 3)&&(sfl < 8)) {
    if(sfl==4) {
      minutes_togo++; sfl=0; if(minutes_togo > 90)minutes_togo = 90;
    }
    if(sfl==5) {
      minutes_togo--; sfl=0; if(minutes_togo < 1)minutes_togo = 1;
    }
    if(sfl==6) {
      minutes_togo += 10; sfl=0; if(minutes_togo > 90)minutes_togo = 90;
    }
    if(sfl==7) {
      minutes_togo -= 10; sfl=0; if(minutes_togo < 1)minutes_togo = 1;
    }
    if(minutes_togo > 9) display.writeDigitNum(0, (minutes_togo / 10) % 10, false);
    display.writeDigitNum(1, (minutes_togo / 1) % 10, false);
    display.writeDisplay();
    if(minutes_togo <= 9) kstate = "-" + String(minutes_togo) + ":--";
    if(minutes_togo > 9) kstate = String(minutes_togo) + ":--";
  }
}
//----------------------------------------------------------------------------
