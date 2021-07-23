#include <SFM3X00.h>

#include <Wire.h>
#include <SPI.h>

#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1351.h>
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

/*
Public Invention's BVM-monitor Project is a monitor for Bag Valve Masks to measure flow (tidal volume) and prevent operator misuse. This project includes open-source software and hardware designs to support manufacturing of a bag valve sensor to support operators as they administer first aid. This device aims to measure and correct both the tidal volume delivered and the flow. Copyright (C) 2021 Robert Read, Alois Chipfurutse, Matthew Gutierrez, Alvin K. Ibeabuchi, and Darío Hereñú.

This program is free Firmware/Hardware designs: you can redistribute, use, study it and/or modify it under the terms of the CERN Open Hardware License Version 2 as published here: https://ohwr.org/cern_ohl_s_v2.txt

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the CERN Open Hardware License Version 2 for more details.
You should have received a copy of the CERN Open Hardware License Version 2 along with this program.  If not, see https://ohwr.org/cern_ohl_s_v2.txt.

This program includes free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. See the GNU Affero General Public License for more details. You should have received a copy of the GNU Affero General Public License along with this program.  If not, see https://www.gnu.org/licenses/
*/

int interval = 0;
int alarmcount = 0;
const int speakerPin = 5;
int ledState = LOW;

bool screenUpdate = true;

//int count = 0;
bool squeeze = false;
int restime;
int startreset;
int bigcounting = 1;
// address of sensor
// usually 64 or 0x40 by default
#define FLOW_SENSOR_ADDRESS 0x40

// minimum flow value to recognize as valid
#define MINIMUM_FLOW        0.25

// Sometimes you want to install backwards because
// of the physical connections you are using.
bool SENSOR_INSTALLED_BACKWARD = false;

// create insance of sensor with address
SFM3X00 flowSensor(FLOW_SENSOR_ADDRESS);

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320 // Change this to 96 for 1.27" display.

//#define SCREEN_WIDTH  128
//#define SCREEN_HEIGHT 128 // Change this to 96 for 1.27" display.

// The SSD1351 is connected like this (plus VCC plus GND)
//const uint8_t   display_pin_scl_sck        = 13;
//const uint8_t   display_pin_sda_mosi       = 11;
//const uint8_t   display_pin_cs_ss          = 10;
//const uint8_t   display_pin_res_rst        = 8;
//const uint8_t   display_pin_dc_rs          = 7;
#define TFT_CS        10
#define TFT_RST        8 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         7

// declare the display
//Adafruit_SSD1351 display = Adafruit_SSD1351(SCREEN_WIDTH,SCREEN_HEIGHT,&SPI,display_pin_cs_ss,display_pin_dc_rs,display_pin_res_rst);
//Adafruit_ST7735 display = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// SSD1331 color definitions
const uint16_t  Black        = 0x0000;
const uint16_t  Blue         = 0x001F;
const uint16_t  Red          = 0xF800;
const uint16_t  Green        = 0x07E0;
const uint16_t  Cyan         = 0x07FF;
const uint16_t  Magenta      = 0xF81F;
const uint16_t  Yellow       = 0xFFE0;
const uint16_t  White        = 0xFFFF;
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF
#define MONO_CHROME_WHITE 0x01

// The colors we actually want to use
uint16_t        display_Text_Color         = Black;
uint16_t        display_Background_Color    = Blue;



unsigned long previousMillis = 0;

//Defining the alarm sounds
#define FIRSTTONE 261.626
#define SECONDTONE 440
#define THIRDTONE 349.228
#define FOURTHTONE 0
#define FIFTHTONE 440
#define SIXTHTONE 349.228
#define TONEDURATION_MS 500

/*tone(speakerPin, 261.626, 250);
    //Serial.println(millis());
    tone(speakerPin, 440, 250);
    tone(speakerPin, 349.228, 250);
    tone(speakerPin, 0, 250);
    tone(speakerPin, 440, 250);
    tone(speakerPin, 349.228, 250);*/

// Variables for supporting multiple tones
boolean FirstTone = false;
boolean SecondTone = false;
boolean ThirdTone = false;
boolean FourthTone = false;
boolean FifthTone = false;
boolean SixthTone = false;
long startFirst;
long startSecond;
long startThird;
long startFourth;
long startFifth;
long startSixth;
boolean alarm;


////////////////////////////////
// here I set up the basic math to describe the situation that Breathe Easy asked of me.
#define TARGET_BAR_PERCENT 0.556 // we'll draw the target line at this percentage of the height
#define GOOD_BAR_PERCENT 0.0895 // 10% below the target percentage
#define BAD_BAR_PERCENT 0.0895 // 10% above the target percentage

// This defines two separate "bars" --- because I only have a monocrhome display,
// I am doing two side-by-side
#define MEASURE_BAR_START_HORIZONTAL 0.1
#define MEASURE_BAR_WIDTH_HORIZONTAL 0.4
#define TARGET_BAR_START_HORIZONTAL 0.6
#define TARGET_BAR_WIDTH_HORIZONTAL 0.4

float target_tidal_volume_ml = 556; // This is used to set the meaning of the "target" bar
float breath_length_ms = 8000.0; // the length of a breath in ms.
float inspiration_time_ms = 2000.0; // the target length of an "inspiration"


// This functional gives us a little encapsulation and let's me cover up a bug in
// the Featherwing that doesn't draw the horizontal lines of a rectangle
void myDrawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color, bool fillNotDraw) {
  if (fillNotDraw) {
   display.fillRect(x0, y0, w, h, color);
  }
  display.drawRect(x0, y0, w, h, color);
  display.drawLine(x0, y0, x0+w-1,y0, color);
  display.drawLine(x0, y0+h-1, x0+w-1,y0+h-1, color);
}

void myEraseInteriorRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color) {
  display.fillRect(x0+1, y0+1, w-1, h-1, color);
}
// draw the empty bar
void render_empty_measure_bar() {
  uint16_t left = SCREEN_WIDTH * MEASURE_BAR_START_HORIZONTAL;
  float total_height = 0.95;
 //uint16_t top = SCREEN_HEIGHT * (1.0 - ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT)) ;
  uint16_t top = SCREEN_HEIGHT * (1.0 - ((float) total_height));
  uint16_t width = SCREEN_WIDTH * (float) MEASURE_BAR_WIDTH_HORIZONTAL;
  uint16_t bheight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT);
  uint16_t theight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT);
  uint16_t gheight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT - GOOD_BAR_PERCENT);
  uint16_t height = SCREEN_HEIGHT * total_height;

  myDrawRect(left,top,width,height,WHITE, false);

  display.drawLine(left, SCREEN_HEIGHT - theight,left+ width-1, SCREEN_HEIGHT - theight, GREEN);
  display.drawLine(left, SCREEN_HEIGHT - gheight,left+ width-1, SCREEN_HEIGHT - gheight, BLUE);
  display.drawLine(left, SCREEN_HEIGHT - bheight,left+ width-1, SCREEN_HEIGHT - bheight, RED);
}
void render_empty_target_bar() {
   uint16_t left = SCREEN_WIDTH * TARGET_BAR_START_HORIZONTAL;
   float total_height = 0.95;
  //uint16_t top = SCREEN_HEIGHT * (1.0 - ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT)) ;
  uint16_t top = SCREEN_HEIGHT * (1.0 - ((float) total_height));
  uint16_t width = SCREEN_WIDTH *  (float) TARGET_BAR_WIDTH_HORIZONTAL;
 // uint16_t height = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT);
 uint16_t height = SCREEN_HEIGHT * total_height;

  uint16_t bheight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT);
  uint16_t theight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT);
  uint16_t gheight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT - (float) GOOD_BAR_PERCENT);
  myDrawRect(left,top,width,height,WHITE, false);

  display.drawLine(left, SCREEN_HEIGHT - theight,left+ width-1, SCREEN_HEIGHT - theight, GREEN);
  display.drawLine(left, SCREEN_HEIGHT - gheight,left+ width-1, SCREEN_HEIGHT - gheight, BLUE);
  display.drawLine(left, SCREEN_HEIGHT - bheight,left+ width-1, SCREEN_HEIGHT - bheight, RED);
}
void render_empty_bars() {
  render_empty_measure_bar();
  render_empty_target_bar();
}

// render the bar to a certain percentage height, dealing with "good" and "bad" color changes
void render_bar_percentage(float percent) {
}

// convert a tidal volume to a percentage
float convert_tv_ml_to_percent(int tv) {

}

// This is for erasing...
int current_line_height = -1;

// render the "target line" as a percentage
void render_target_line(float percent) {
  uint16_t left = SCREEN_WIDTH * TARGET_BAR_START_HORIZONTAL;
  uint16_t width = SCREEN_WIDTH *  (float) TARGET_BAR_WIDTH_HORIZONTAL;
  uint16_t top = SCREEN_HEIGHT * (1.0 - ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT)) ;
 uint16_t theight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT);
  uint16_t pheight = (float) theight * ((float) percent);
  uint16_t gheight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT - GOOD_BAR_PERCENT);

// We will first erase the last line if their is one..
  if (current_line_height > 0) {
   Serial.println("ROB");
   display.drawLine(left,
    SCREEN_HEIGHT - current_line_height,
    left+ width-1,
    SCREEN_HEIGHT - current_line_height,
    BLACK);
  }
  Serial.println("current");
  Serial.println(current_line_height);
  Serial.println(pheight);
  // now recored our current_line_height
  current_line_height = pheight;

  uint16_t color = (current_line_height < gheight) ? BLUE : GREEN;
  display.drawLine(left,
    SCREEN_HEIGHT - pheight,
    left+ width-1,
    SCREEN_HEIGHT - pheight,
    color);
}

// render_tv as a filled_in rect
void render_tv(float tv) {
  uint16_t left = SCREEN_WIDTH * (float) MEASURE_BAR_START_HORIZONTAL;
  uint16_t width = SCREEN_WIDTH *  (float) MEASURE_BAR_WIDTH_HORIZONTAL;
  uint16_t top = SCREEN_HEIGHT * (1.0 - ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT)) ;
 uint16_t theight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT);
 // measured height
 uint16_t mheight = (float) theight * (tv / target_tidal_volume_ml);
 uint16_t gheight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT - GOOD_BAR_PERCENT);
// uint16_t pheight = (float) theight * ((float) percent);
uint16_t bheight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT);


//Squeeze Rectangle
  //myDrawRect(left,SCREEN_HEIGHT - mheight,width, SCREEN_HEIGHT,WHITE, true);
  //display.drawLine(left,SCREEN_HEIGHT - mheight, left+ width-1, SCREEN_HEIGHT - mheight, WHITE);


//Colored Version
if (mheight < gheight)
{
  myDrawRect(left,SCREEN_HEIGHT - mheight,width, SCREEN_HEIGHT,BLUE, false);
  display.drawLine(left,SCREEN_HEIGHT - mheight, left+ width-1, SCREEN_HEIGHT - mheight, BLUE);
}
 else if (mheight > bheight)
 {
  myDrawRect(left,SCREEN_HEIGHT - mheight,width, SCREEN_HEIGHT,RED, false);
  display.drawLine(left,SCREEN_HEIGHT - mheight, left+ width-1, SCREEN_HEIGHT - mheight, RED);
 }
 else if(mheight >=gheight && mheight <= bheight)
 {
  myDrawRect(left,SCREEN_HEIGHT - mheight,width, SCREEN_HEIGHT,GREEN, false);
  display.drawLine(left,SCREEN_HEIGHT - mheight, left+ width-1, SCREEN_HEIGHT - mheight, GREEN);
 }
}

float compute_percent_of_inspiration(long ms) {
  if ((ms >= 0) && (ms < (long) inspiration_time_ms)) {
    Serial.println("xxx");
    return (float) ms / (float) inspiration_time_ms;
  } else {
    return 0.0;
  }
}

float error_for_faking = 1.2;
bool error_reset = false;

void erase_screen() {
  display.fillScreen(ST77XX_BLACK);
  render_empty_bars();
}

void setup() {
display.init(240,320);
  // void setup() {
   //display.initR(INITR_GREENTAB);

  // initialise the SSD1331
 // display.begin();
 // display.setFont();
 // display.fillScreen(Black);
  //the background color of the screen
  //display.setTextWrap(false);

  Serial.begin(115200);
  Wire.begin();
  Serial.println("Good");
  flowSensor.begin();
  Serial.println("Bad");
  Serial.println();
  Serial.print("sensor serial number: ");
  Serial.println(flowSensor.serialNumber, HEX);
  Serial.print("sensor article number: ");
  Serial.println(flowSensor.articleNumber, HEX);
  Serial.println();
  Serial.print("read scale factor: ");
  Serial.println(flowSensor.flowScale);
  Serial.print("read flow offset: ");
  Serial.println(flowSensor.flowOffset);
  display.fillScreen(ST77XX_BLACK);

  delay(100);

  zero_integration(millis());

  erase_screen();
}

// For calibrating, we will simply compute positive and negative volumes via integration,
// Setting to zero on putton press of the display
float G_volume = 0.0;
float G_volume_previous = 0.0;
float last_ms = 0.0;
float last_flow = 0.0;

void zero_integration(unsigned long ms) {
  G_volume = 0;
  last_ms = ms;
  last_flow = 0;
}

// Note: v units are milliliters
float add_to_running_integration(float v,unsigned long ms,float flow_millilters_per_minute) {
  float f = flow_millilters_per_minute;
    // Use a basic quadrilateral integration
  // we'll treat a very small flow as zero...
  float ml_per_ms = f / (60.0 * 1000.0);

  v += (ms - last_ms) * (ml_per_ms + last_flow)/2.0;

  last_ms = ms;
  last_flow = ml_per_ms;

  return v;
}

float max_recorded_flow = 0.0;
float min_recorded_flow = 0.0;

/////////////////////////////////

void loop() {
  // display.fillScreen(BLACK);
  // read the millisecond clock...
  long m = millis();
  // if m > breath_length_ms, we are doing to change our random error number...


  // Perform "modulo" operation to get ms within the current breath
  m = m % (long) breath_length_ms;

  // When we are done with the current inspiration we will set another random error...
  if (!error_reset) {
    if (m > inspiration_time_ms) {
      // We'll fake being either 20% too high or too low for now...
      error_for_faking =   1.0 + 0.2 * (float) (random(100) - 50) / 50.0 ;
      error_reset = true;
    } else {
      error_reset = false;
    }
  }

  //float percent_full = compute_percent_of_inspiration(m);
  float percent_full = compute_percent_of_inspiration(m - 1000);

  render_empty_bars();

  render_target_line(percent_full);

  float f = (m > inspiration_time_ms) ? 1.0 * error_for_faking : percent_full * error_for_faking;

  long fake_tv = f * target_tidal_volume_ml;

  short int incomingByte;
  while (Serial.available() > 0) {
    incomingByte = Serial.read();
    Serial.print(incomingByte);
    if (incomingByte == '\n') {
        Serial.println("Zeroing! 0000000000000000000000000000");
          zero_integration(millis());
    }
  }

 // we don't know how long the Serial operation will take,
 // so we read the millisecond clock again...
  unsigned long ms = millis();

  float raw_flow_slm = flowSensor.readFlow();  // standard liters per minute
  bool extreme_range = flowSensor.checkRange(raw_flow_slm);
  if (extreme_range) {
    Serial.println("RANGE OF SENSOR EXCEEDED");

  }
  min_recorded_flow = min(min_recorded_flow,raw_flow_slm);
  max_recorded_flow = max(max_recorded_flow,raw_flow_slm);

  float flow = (SENSOR_INSTALLED_BACKWARD) ? -raw_flow_slm : raw_flow_slm;

  // if the flow is less than 0.01 then round to 0

  if(abs(flow) < MINIMUM_FLOW)
  {
     flow = 0;
     if(squeeze == false)
     {
      startreset = millis();
      squeeze = true;
     }
     restime = millis() - startreset;
     //Serial.println(restime);


     if(restime > 3000)
     {
      G_volume = 0;
      squeeze = false;
      restime = 0;
      //display.fillRect(0, 0, 128, 128, Red);
     /* display.setTextSize(3);
      display.setTextColor(Blue, Black);
      display.setCursor(10,50);
      display.print("0.0000");
      display.setTextSize(2);
      display.setTextColor(Black, Black);
      display.setCursor(25,100);
      display.print("DANGER");
      count = 0;*/
     }

  }
  else
  {
    squeeze = false;
  }

  float flow_milliliters_per_minute =  (flow * 1000.0);

  G_volume = add_to_running_integration(G_volume, ms,flow_milliliters_per_minute);
     /* if( G_volume > 650)
      {
        G_volume = 650;
      }*/

     if (G_volume< 0)
      {
      G_volume = 0;
      }

  Serial.println(G_volume);

  //  display.fillScreen(BLACK);
  //  render_empty_bars();
  //  render_target_line(percent_full);
    if (G_volume_previous != G_volume) {
        G_volume_previous = G_volume;
        render_tv(G_volume);
        screenUpdate = true;
    }

  //display.display();
  yield();
  delay(10);

 /* Serial.print("(press return to zero) Volume (ml): ");
  Serial.println(G_volume);
  /*Serial.print("min flow: ");
  Serial.println(min_recorded_flow);
  Serial.print("max flow: ");
  Serial.println(max_recorded_flow);
  Serial.print("raw ");
  Serial.println(raw_flow_slm);*/


  //unsigned long currentMillis = millis();
 /* if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    interval = interval +1;
  }*/

  interval =m/1000;
  // interval = ((int) floor( m / 1000.0)) % 6;

  Serial.println(interval);
     if (((interval-2) % 8 == 0) ||
         ((interval-1) % 8 == 0))
      {
      //if (interval % 8 == 0){
      //changed from tone (12, 700, 1000);
      tone(speakerPin, 329.628, 2000);
      //tone(speakerPin, 20, 2000);
      //Serial.println("Speaker");
      }
      else {
      noTone(speakerPin);
     // Serial.println("NoSpeaker");

      }

  //count = count+1;
  if(interval % 8 == 0)
  {
      if (screenUpdate==true)
      {
        erase_screen();
        screenUpdate = false;
      }


    if(G_volume >= 0 && G_volume < 466.5)
    {

    /*//display.fillScreen(Blue);
    display.setTextSize(5);
    display.setTextColor(Blue, Black);
    display.setCursor(10,50);
    display.print(G_volume);
    count = 0;*/
    alarmcount = alarmcount +1 ;
    }

    else if(G_volume >= 466.5 && G_volume < 645.5)
    {
    /*//display.fillScreen(Green);
    display.setTextSize(5);
    display.setTextColor(Green, Black);
    display.setCursor(10,50);
    display.print(G_volume);*/
    //count = 0;
    alarmcount = 0;
    }

    else if(G_volume >= 645.5)
    {
    //display.fillScreen(Red);
   /* display.setTextSize(5);
    display.setTextColor(Red, Black);
    display.setCursor(10,50);
    display.print(G_volume);
    display.setTextSize(2);
    display.setTextColor(Red, Black);
    display.setCursor(25,100);
    display.print("DANGER");
    count = 0;*/
    alarmcount = alarmcount +1 ;
    }

    // Now erase the screen...

  }
 /* Serial.println("alarmcount");
  Serial.println(alarmcount);
  if (alarmcount >= 3)
  { //startFirst = millis();

    Serial.println("BVM");
    alarm = true;
  if (alarm) {
    FirstTone = true;
    startFirst = millis();
    tone(speakerPin,FIRSTTONE,TONEDURATION_MS);
    alarm = false;
  }


    Serial.println("BVM");

    //}

  if ( FirstTone && (millis() > (startFirst + TONEDURATION_MS)) /*&& (startFirst != 0) ) {
    SecondTone = true;
    tone(speakerPin,SECONDTONE,TONEDURATION_MS);
    startSecond = millis();
    FirstTone = false;
  }
  if (SecondTone && millis() > (startSecond + TONEDURATION_MS)) {
    tone(speakerPin,THIRDTONE,TONEDURATION_MS);
    startThird = millis();
    SecondTone = false;
  }
  if (ThirdTone && millis() > (startThird + TONEDURATION_MS)) {
    tone(speakerPin,FOURTHTONE,TONEDURATION_MS);
    startFourth = millis();
    ThirdTone = false;
  }
  if (FourthTone && millis() > (startFourth + TONEDURATION_MS)) {
    tone(speakerPin,FIFTHTONE,TONEDURATION_MS);
    startFifth = millis();
    FourthTone = false;
  }
  if (FifthTone && millis() > (startFifth + TONEDURATION_MS)) {
    tone(speakerPin,SIXTHTONE,TONEDURATION_MS);
    startSixth = millis();
    FifthTone = false;
  }
  if (SixthTone && millis() > (startSixth + TONEDURATION_MS)) {
    noTone(speakerPin);
    SixthTone = false;
    alarm = false;
  }*/
  }
