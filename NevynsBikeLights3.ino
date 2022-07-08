#include <FastLED.h>
#include <ezButton.h>
#define DATAPIN    5
#define CLOCKPIN   6

typedef enum {
  IndicatorPixelCount = 3,
  FrontPixelCount = 23,
  TotalPixelCount = 26
} PixelCounts;
CRGB leds[TotalPixelCount];

ezButton btnLeft(10);
ezButton btnRight(16);


void setup()
{ 
  FastLED.addLeds<DOTSTAR, DATAPIN, CLOCKPIN>(leds, TotalPixelCount); 
  FastLED.setBrightness(128);
  Serial.begin(9600);
}


void loop()
{
  btnLeft.loop();
  btnRight.loop();
  
  static uint8_t hue = 0, active = false;
  if(btnLeft.getState() == LOW)
  {
    hue--;
    active = true;
  }
  else if(btnRight.getState() == LOW)
  {
    hue++;
    active = true;
  }
  else
  {
    active = false;
  }
  
  if(active)
  {
    for(int i = 0; i < TotalPixelCount; i++) {
      leds[i] = CHSV(hue + (255/TotalPixelCount)*i, 255, 255);
    }
  } else {
    fill_solid(leds, TotalPixelCount, CRGB(0,0,0));
  }
  FastLED.show();
  
  delay(10);
}
