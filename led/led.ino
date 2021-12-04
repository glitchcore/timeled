#include <Adafruit_NeoPixel.h>
// #include <rp2040_pio.h>
#include "glsl.h"

const int NUM_LEDS = 200;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, 3, NEO_RGB + NEO_KHZ800);

size_t stars_count = 0;
float stars_x[40];

void create_stars() {
  stars_count = 0;
  float star_x = (float)random(20, 75) / 1000.;
  while(star_x < 1.) {
    stars_x[stars_count++] = star_x;
    star_x += (float)random(20, 75) / 1000.;
  }
}

void setup() {
  strip.begin();
  strip.setBrightness(255);
  strip.clear();

  Serial.begin(115200);
  Serial.setTimeout(100);

  pinMode(13, OUTPUT);

  randomSeed(analogRead(0));

  create_stars();
}

float fcos(float x) {
  float tp = 1./(2.*M_PI);
  x *= tp;
  x -= (.25) + floor(x + (.25));
  x *= (16.) * (abs(x) - (.5));
  return x;
}

void mix(vec3* out_color, vec3* color_0, vec3* color_1, float value) {
  if(value > 1.) value = 1.;
  if(value < 0.) value = 0.;
  // int32_t i_value = (int32_t)value * 250;
  
  float c_value = 1. - value;
  
  out_color->r = (uint8_t)((uint16_t)color_0->r * c_value + (uint16_t)color_1->r * value) >> 0;
  out_color->g = (uint8_t)((uint16_t)color_0->g * c_value + (uint16_t)color_1->g * value) >> 0;
  out_color->b = (uint8_t)((uint16_t)color_0->b * c_value + (uint16_t)color_1->b * value) >> 0;
}

vec3 main_color = {0, 0, 0};

vec3 sun = {0xff, 0xd9, 0x25};
vec3 sky = {0x23, 0x5b, 0x82};
vec3 cloud = {0xff, 0xff, 0xff};
// vec3 cloud = {0xff, 0x00, 0x00};
vec3 grass_0 = {0x10, 0x30, 0x00};
vec3 grass_1 = {0x00, 0x0a, 0x00};

#define CLOUD_COUNT 3

float cloud_position[CLOUD_COUNT] = {0.5, 0.8, 0.3};
float cloud_size[CLOUD_COUNT] = {0.03, 0.02, 0.025};
float cloud_speed[CLOUD_COUNT] = {0.01, 0.005, 0.007};

float frac(float v) {
  return v - floor(v);
}

float sky_x(float x) {
  if(x < 0.75) {
    return 0.34 + x * 0.667;
  } else {
    return 0.84 + (x - 0.75) * 0.64;
  }
}

bool is_ground(float x) {
  return (x > 0.68 && x < 0.84) || (x < 0.34);
}

void render_day(float t, float x, vec3* out_color) {
  if(is_ground(x)) {
    vec3 grass;
    mix(&grass, &grass_0, &grass_1, fcos(x * 200 + t * 0.3) * 3);
    memcpy(out_color, &grass, sizeof(vec3));
  } else {
    memcpy(out_color, &sky, sizeof(vec3));
    for(size_t i = 0; i < CLOUD_COUNT; i++) {
      float cloud_x = sky_x(frac(cloud_position[i] + frac(t * cloud_speed[i])));
      mix(out_color, out_color, &cloud,
        (cloud_size[i] - abs(x - cloud_x)) * 200
      );
    }

    float sun_x = sky_x(frac(t * 0.01));
    float sun_size = 0.03;
    
    mix(out_color, out_color, &sun,
      (sun_size - abs(x - sun_x)) * 100
    );
  }
}

vec3 night_sky = {0x01,0x00,0x01};
vec3 star = {0x10, 0x10, 0x06};

void render_night(float t, float x, vec3* out_color) {
  memcpy(out_color, &night_sky, sizeof(vec3));
  for(size_t i = 0; i < stars_count; i++) {
    if(abs(x - stars_x[i]) < 0.0025) {
      memcpy(out_color, &star, sizeof(vec3));
    }
  }
}

void render_waves(float t, float x, vec3* out_color) {
  /*out_color->b = 90 * (1. + fcos((t*6/6 + x * 6)));
  out_color->r = 255 - out_color->b;*/
}

void render(float t, float x, vec3* out_color) {
  render_day(t, x, out_color);
  // render_night(t, x, out_color);
}

void user_input() {
  String s = Serial.readStringUntil('\n');
  if(s == NULL) return;

  vec3* target = NULL;
  switch(s[0]) {
    case '#':
      target = &main_color;
    break;
    case 'g':
      target = &grass_0;
    break;
    case 's':
      target = &sky;
    break;
    case 'n':
      target = &night_sky;
    break;
    case 't':
      target = &star;
    break;
  }

  if(target != NULL) {
    Serial.println(s);
    sscanf(&s.c_str()[1], "%2X%2X%2X", &(target->r), &(target->g), &(target->b));
  } else {
    Serial.println("enter <>hex");
  }
}

void loop() {
  // user_input();
  
  strip.clear();
  float t = (float)millis() / 1000;
  // Serial.println(t);
  for(uint16_t n = 0; n < NUM_LEDS; n++) {
    vec3 out_color = {0, 0, 0};
    render(t, ((float)n / NUM_LEDS), &out_color);
    strip.setPixelColor(n, out_color.r, out_color.g, out_color.b);
    digitalWrite(13, n < NUM_LEDS/2 ? HIGH : LOW);
  }
  strip.show();
}
