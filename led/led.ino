#include <Adafruit_NeoPixel.h>
// #include <rp2040_pio.h>
#include "glsl.h"

const int NUM_LEDS = 200;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, 3, NEO_RGB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.setBrightness(255);
  strip.clear();

  Serial.begin(115200);

  pinMode(13, OUTPUT);
}

float fcos(float x) {
  x /= 32768;
  float tp = 1./(2.*M_PI);
  x *= tp;
  x -= (.25) + floor(x + (.25));
  x *= (16.) * (abs(x) - (.5));
  #if EXTRA_PRECISION
  x += T(.225) * x * (std::abs(x) - T(1.));
  #endif
  return x;
}

void render(uint32_t t_x, uint16_t i_x, vec3* out_color) {
  float x = (float)i_x / 32768;
  float t = (float)t_x / 32;
  
  // out_color->g = 50 * (1. + fcos((float)t/4000 + x * 0));
  out_color->b = 90 * (1. + fcos(32768 * ((float)t/2000 + x * 6)));
  out_color->r = 255 - out_color->b;
}

void loop() {
  // strip.clear();
  uint32_t t = (uint32_t)millis() * 32.768 * 2.5;
  // Serial.println(t);
  for(uint16_t n = NUM_LEDS; n > 0; n--) {
    vec3 out_color = {0, 0, 0};
    render(t, ((uint32_t)n * 32768 / NUM_LEDS), &out_color);
    strip.setPixelColor(n, out_color.r, out_color.g, out_color.b);
    digitalWrite(13, n < NUM_LEDS/2 ? HIGH : LOW);
  }
  strip.show();
}
