#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF
#define MONO_CHROME_WHITE 0x01

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

void setup() {
  Serial.begin(9600);

  Serial.println("OLED FeatherWing test");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  Serial.println("IO test");

  // text display tests
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("Connecting to SSID\n'adafruit':");
  display.print("connected!");
  display.setCursor(0,0);
  display.display(); // actually display all of the above
}
// here I set up the basic math to describe the situation that Breathe Easy asked of me.
#define TARGET_BAR_PERCENT 0.75 // we'll draw the target line at this percentage of the height
#define GOOD_BAR_PERCENT 0.1 // 10% below the target percentage
#define BAD_BAR_PERCENT 0.1 // 10% above the target percentage

// This defines two separate "bars" --- because I only have a monocrhome display,
// I am doing two side-by-side
#define MEASURE_BAR_START_HORIZONTAL 0.1
#define MEASURE_BAR_WIDTH_HORIZONTAL 0.4
#define TARGET_BAR_START_HORIZONTAL 0.6
#define TARGET_BAR_WIDTH_HORIZONTAL 0.4

float target_tidal_volume_ml = 450.0; // This is used to set the meaning of the "target" bar
float breath_length_ms = 6000.0; // the length of a breath in ms.
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
// draw the empty bar
void render_empty_measure_bar() {
  uint16_t left = SCREEN_WIDTH * MEASURE_BAR_START_HORIZONTAL;
  uint16_t top = SCREEN_HEIGHT * (1.0 - ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT)) ;
  uint16_t width = SCREEN_WIDTH * (float) MEASURE_BAR_WIDTH_HORIZONTAL;
  uint16_t bheight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT);
  uint16_t theight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT);
  uint16_t gheight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT - GOOD_BAR_PERCENT); 
  
  myDrawRect(left,top,width,bheight,MONO_CHROME_WHITE, false);

  display.drawLine(left, SCREEN_HEIGHT - theight,left+ width-1, SCREEN_HEIGHT - theight, MONO_CHROME_WHITE);
  display.drawLine(left, SCREEN_HEIGHT - gheight,left+ width-1, SCREEN_HEIGHT - gheight, MONO_CHROME_WHITE);
}
void render_empty_target_bar() {
   uint16_t left = SCREEN_WIDTH * TARGET_BAR_START_HORIZONTAL;
  uint16_t top = SCREEN_HEIGHT * (1.0 - ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT)) ;
  uint16_t width = SCREEN_WIDTH *  (float) TARGET_BAR_WIDTH_HORIZONTAL;
  uint16_t height = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT);

  uint16_t bheight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT);
  uint16_t theight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT);
  uint16_t gheight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT - GOOD_BAR_PERCENT); 
  myDrawRect(left,top,width,height,MONO_CHROME_WHITE, false);
  
  display.drawLine(left, SCREEN_HEIGHT - theight,left+ width-1, SCREEN_HEIGHT - theight, MONO_CHROME_WHITE);
  display.drawLine(left, SCREEN_HEIGHT - gheight,left+ width-1, SCREEN_HEIGHT - gheight, MONO_CHROME_WHITE); 
}
void render_empty_bars() {
  render_empty_measure_bar();
  render_empty_target_bar();
}

// rebnder the bar to a certain percentage height, dealing with "good" and "bad" color changes
void render_bar_percentage(float percent) {
}

// convert a tidal volume to a percentage 
float convert_tv_ml_to_percent(int tv) {
  
}

// render the "target line" as a percentage
void render_target_line(float percent) {
  uint16_t left = SCREEN_WIDTH * TARGET_BAR_START_HORIZONTAL;
  uint16_t width = SCREEN_WIDTH *  (float) TARGET_BAR_WIDTH_HORIZONTAL;
  uint16_t top = SCREEN_HEIGHT * (1.0 - ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT)) ;
 uint16_t theight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT);
  uint16_t pheight = (float) theight * ((float) percent);
  
  display.drawLine(left, 
    SCREEN_HEIGHT - pheight,
    left+ width-1, 
    SCREEN_HEIGHT - pheight, 
    MONO_CHROME_WHITE);  
}

// render_tv as a filled_in rect
void render_tv(float tv) {
  uint16_t left = SCREEN_WIDTH * (float) MEASURE_BAR_START_HORIZONTAL;
  uint16_t width = SCREEN_WIDTH *  (float) MEASURE_BAR_WIDTH_HORIZONTAL;
  uint16_t top = SCREEN_HEIGHT * (1.0 - ((float) TARGET_BAR_PERCENT + (float) BAD_BAR_PERCENT)) ;
 uint16_t theight = SCREEN_HEIGHT * ((float) TARGET_BAR_PERCENT);
 // measured height
 uint16_t mheight = (float) theight * (tv / target_tidal_volume_ml);


  myDrawRect(left,SCREEN_HEIGHT - mheight,width, SCREEN_HEIGHT,MONO_CHROME_WHITE, true);
  display.drawLine(left, 
    SCREEN_HEIGHT - mheight,
    left+ width-1, 
    SCREEN_HEIGHT - mheight, 
    MONO_CHROME_WHITE);  
}
float compute_percent_of_inspiration(long ms) {
  if (ms < (long) inspiration_time_ms) {
    Serial.println("xxx");
    return (float) ms / (float) inspiration_time_ms;
  } else {
    return 0.0;
  }
}

float error_for_faking = 1.2;
bool error_reset = false;
void loop() {

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
  
  float percent_full = compute_percent_of_inspiration(m);
  display.fillScreen(BLACK);

  render_empty_bars();

  render_target_line(percent_full);

  float f = (m > inspiration_time_ms) ? 1.0 * error_for_faking : percent_full * error_for_faking;

  long fake_tv = f * target_tidal_volume_ml;

  Serial.println("fake tv");
  Serial.println(fake_tv);
  render_tv(fake_tv);
  display.display();
  yield();
  delay(10);
}
