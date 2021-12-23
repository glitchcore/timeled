#include <Adafruit_NeoPixel.h>
#include <MIDI.h>
#include "glsl.h"

const int led = 13;
const int NUM_LEDS = 200;

MIDI_CREATE_DEFAULT_INSTANCE();

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, 3, NEO_RGB + NEO_KHZ800);

void handle_note_on(byte channel, byte note, byte velocity) {
  if(note < 60 || note > 62) return;
  uint8_t color = note - 60;
  
  uint32_t v = strip.getPixelColor(0);
  v &= ~((uint32_t)0xFF << color * 8);
  v |= ((uint32_t)(velocity - 1) << color * 8);
  strip.fill(v, 0, 0);
  
  
  digitalWrite(led, HIGH);
}

void handle_note_off(byte channel, byte note, byte velocity) {
  digitalWrite(led, LOW);
}

void setup() {
  strip.begin();
  strip.setBrightness(255);
  strip.clear();

  pinMode(led, OUTPUT);

  randomSeed(analogRead(0));

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(handle_note_on);
  MIDI.setHandleNoteOff(handle_note_off);
  MIDI.turnThruOff();
  Serial.begin(115200);
}


void loop() {
  MIDI.read();
  strip.show();
  delay(1);
}
