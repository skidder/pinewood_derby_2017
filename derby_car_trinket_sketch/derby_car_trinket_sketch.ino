#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 6
#define NUM_PIXELS 24

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRBW + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

Adafruit_LIS3DH lis = Adafruit_LIS3DH();

float x, y, z;

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  x = -1;
  y = -1;
  z = -1;

  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'

  // accelerometer setup
  Serial.begin(9600);
  Serial.println("LIS3DH test!");

  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("LIS3DH found!");

  lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!

  Serial.print("Range = "); Serial.print(2 << lis.getRange());
  Serial.println("G");

  sensors_event_t event;
  lis.getEvent(&event);
  x = event.acceleration.x;
  y = event.acceleration.y;
  z = event.acceleration.z;
}

void loop() {
  /* Or....get a new sensor event, normalized */
  sensors_event_t event;
  lis.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("\t\tX: "); Serial.print(event.acceleration.x);
  Serial.print(" \tY: "); Serial.print(event.acceleration.y);
  Serial.print(" \tZ: "); Serial.print(event.acceleration.z);
  Serial.println(" m/s^2 ");

  Serial.println();

  if (fabs(x - event.acceleration.x) > 2 || fabs(y - event.acceleration.y) > 2 || fabs(z - event.acceleration.z) > 2) {
    // Send a theater pixel chase in...
    //    theaterChase(strip.Color(127, 127, 127), 50); // White
    //    theaterChase(strip.Color(127, 0, 0), 50); // Red
    //    theaterChase(strip.Color(0, 0, 127), 50); // Blue
    larsonScanner(strip.Color(127, 0, 0), 50);
  }
  x = event.acceleration.x;
  y = event.acceleration.y;
  z = event.acceleration.z;
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, c);  //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
  strip.show(); // Initialize all pixels to 'off'
}

// Larson Scanner
// https://learn.adafruit.com/larson-scanner-shades/software
void larsonScanner(uint32_t c, uint8_t wait) {
  int pos = 0, dir = 1; // Position, direction of "eye"
  int iteration;

  for (iteration = 0; iteration < 300; iteration++) {
    int j;
    // Draw 5 pixels centered on pos.  setPixelColor() will clip any
    // pixels off the ends of the strip, we don't need to watch for that.
    strip.setPixelColor(pos - 2, 0x100000); // Dark red
    strip.setPixelColor(pos - 1, 0x800000); // Medium red
    strip.setPixelColor(pos    , 0xFF3000); // Center pixel is brightest
    strip.setPixelColor(pos + 1, 0x800000); // Medium red
    strip.setPixelColor(pos + 2, 0x100000); // Dark red

    strip.show();
    delay(30);

    // Rather than being sneaky and erasing just the tail pixel,
    // it's easier to erase it all and draw a new one next time.
    for (j = -2; j <= 2; j++) strip.setPixelColor(pos + j, 0);

    // Bounce off ends of strip
    pos += dir;
    if (pos < 0) {
      pos = 1;
      dir = -dir;
    } else if (pos >= NUM_PIXELS) {
      pos = NUM_PIXELS - 2;
      dir = -dir;
    }
  }
  strip.show();
}

