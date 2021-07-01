#include <SFM3X00.h>

#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1351.h>
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

int interval = 0;
const int speakerPin = 5;
int ledState = LOW;

int count = 0;
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

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 // Change this to 96 for 1.27" display.

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
Adafruit_ST7735 display = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// SSD1331 color definitions
const uint16_t  Black        = 0x0000;
const uint16_t  Blue         = 0x001F;
const uint16_t  Red          = 0xF800;
const uint16_t  Green        = 0x07E0;
const uint16_t  Cyan         = 0x07FF;
const uint16_t  Magenta      = 0xF81F;
const uint16_t  Yellow       = 0xFFE0;
const uint16_t  White        = 0xFFFF;

// The colors we actually want to use
uint16_t        display_Text_Color         = Black;
uint16_t        display_Backround_Color    = Blue;



unsigned long previousMillis = 0;

////////////////////////////////
void setup() {
   display.initR(INITR_GREENTAB);

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
  
  delay(100);
  
  zero_integration(millis());
}

// For calibrating, we will simply compute positive and negative volumes via integration,
// Setting to zero on putton press of the display
float G_volume = 0.0; 
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
      display.setTextSize(3);
      display.setTextColor(Blue, Black);
      display.setCursor(10,50);
      display.print("0.0000");

      display.setTextSize(2);
      display.setTextColor(Black, Black);
      display.setCursor(25,100);
      display.print("DANGER");
      count = 0;
     }
     
  }
  else
  {
    squeeze = false;
  }

  float flow_milliliters_per_minute =  (flow * 1000.0);

  G_volume = add_to_running_integration(G_volume, ms,flow_milliliters_per_minute);
   Serial.println(G_volume);
 /* Serial.print("(press return to zero) Volume (ml): ");
  Serial.println(G_volume);
  /*Serial.print("min flow: ");
  Serial.println(min_recorded_flow);
  Serial.print("max flow: ");
  Serial.println(max_recorded_flow);
  Serial.print("raw ");
  Serial.println(raw_flow_slm);*/


  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    interval = interval +1;
  }
 
  Serial.println(interval);
      if (interval % 6 == 0){
      //tone (12, 700, 1000);
      tone(speakerPin, 700, 1000); 
      //Serial.println("Speaker");
      } 
      else {
      noTone(speakerPin);
     // Serial.println("NoSpeaker");
     
      }
  
  count = count+1;
  if(count == 70)
  {
    
    if(G_volume >= 0 && G_volume < 400)
    {

    //display.fillScreen(Blue);
    display.setTextSize(5);
    display.setTextColor(Blue, Black);
    display.setCursor(10,50);
    display.print(G_volume);
    count = 0;
    }
    
    else if(G_volume >= 450 && G_volume < 650)
    {
    //display.fillScreen(Green);
    display.setTextSize(5);
    display.setTextColor(Green, Black);
    display.setCursor(10,50);
    display.print(G_volume);
    count = 0;
    }

    else if(G_volume >= 650)
    {
    //display.fillScreen(Red);
    display.setTextSize(5);
    display.setTextColor(Red, Black);
    display.setCursor(10,50);
    display.print(G_volume);
    
    display.setTextSize(2);
    display.setTextColor(Red, Black);
    display.setCursor(25,100);
    display.print("DANGER");
    count = 0;
    //}
  }
  }
}

  

  
