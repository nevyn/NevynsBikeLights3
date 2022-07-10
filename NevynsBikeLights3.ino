#include <FastLED.h>
#include <ezButton.h>
#include <OverAnimate.h>
#include "SubStrip.h"

// global data
#define DATAPIN    5
#define CLOCKPIN   6

typedef enum {
    IndicatorPixelCount = 3,
    FrontPixelCount = 23,
    TotalPixelCount = 26
} PixelCounts;
CRGB leds[TotalPixelCount];
SubStrip full(leds, TotalPixelCount);
SubStrip indicators(leds, IndicatorPixelCount);
SubStrip front(leds+3, FrontPixelCount);

// buttons
ezButton btnLeft(10);
ezButton btnRight(16);

// animations
AnimationSystem ansys;

void BlinkFunc(Animation *self, int direction, float t);
void ShineFunc(Animation *self, int _, float t);
void BlackFunc(Animation *self, int _, float t);

BoundFunctionAnimation blinkLeft(BlinkFunc,   1);
BoundFunctionAnimation blinkRight(BlinkFunc, -1);
BoundFunctionAnimation shine(ShineFunc, 0);
BoundFunctionAnimation black(BlackFunc, 0);
BoundFunctionAnimation *anims[] = {
  &black,
  &shine,
  &blinkLeft,
  &blinkRight
};
static const int kAnimCount = sizeof(anims)/sizeof(BoundFunctionAnimation*);


// main app
void setup()
{ 
    FastLED.addLeds<DOTSTAR, DATAPIN, CLOCKPIN, BGR>(leds, TotalPixelCount);
    FastLED.setBrightness(192);
    Serial.begin(9600);

    for(int i = 0; i < kAnimCount; i++) {
        BoundFunctionAnimation *anim = anims[i];
        anim->beginTime = ansys.now();
        anim->duration = 1.0;
        anim->repeats = true;
        anim->enabled = false;
        ansys.addAnimation(anim);
    }
    black.enabled = true;
}


unsigned long lastMillis;
void loop()
{
    btnLeft.loop();
    btnRight.loop();
  
    unsigned long now = millis();
    if(!lastMillis) {
        lastMillis = now;
    }
    unsigned long diff = now - lastMillis;
    lastMillis = now;
    TimeInterval delta = diff/1000.0;

    update();

    ansys.playElapsedTime(delta);
    FastLED.show();
}

void update()
{
    if(btnLeft.getState() == LOW)
    {
        if(blinkLeft.enabled == false)
        {
            blinkLeft.beginTime = ansys.now();
        }
        blinkLeft.enabled = true;
        blinkRight.enabled = false;
    }
    else if(btnRight.getState() == LOW)
    {
        if(blinkRight.enabled == false)
        {
            blinkRight.beginTime = ansys.now();
        }
        blinkRight.enabled = true;
        blinkLeft.enabled = false;
    }
    else
    {
        blinkRight.enabled = false;
        blinkLeft.enabled = false;
    }
}

// animation funcs

void BlinkFunc(Animation *self, int direction, float f)
{
    SubStrip *led = &front;
    
    int beginAtIndex = led->numPixels()/2;
    int litIndex = beginAtIndex + f*direction*led->numPixels()/2;
    int oneBehind = litIndex - direction;
    int twoBehind = litIndex - direction*2;
    led->leds[litIndex] = CHSV(HUE_YELLOW, 192, 255);
    led->leds[oneBehind] = CHSV(HUE_YELLOW, 192, 128);
    led->leds[twoBehind] = CHSV(HUE_YELLOW, 192, 64);
    

    int c = sin8(f*255)/2;
    indicators.leds[(direction == 1) ? 0 : indicators.length-1] = CRGB(c, c, 0);
}

void ShineFunc(Animation *self, int _, float t)
{
  /*LedStrip *leds[] = {&frontLeds, &rearLeds};
  for(int l = 0; l < 2; l++) {
    LedStrip *led = leds[l];
    int mid = led->numPixels()/2;
    for(int i = 0; i < led->numPixels(); i++) {
      int distance = abs(mid - i);
      int range = led->numPixels()/4;
      int strength = frontLedStrip.gamma8(clamp((range-distance)*(255/range), 0, 255));
      led->setPixelColor(i, Adafruit_NeoPixel::Color(strength, l==0?strength:0, l==0?strength:0));
    }
  }

  dashLeds.setPixelColor(2, Adafruit_NeoPixel::Color(0, 0, 1));*/
}

void BlackFunc(Animation *self, int _, float t)
{
    for(int i = 0; i < full.numPixels(); i++) {
        int c = 0;
        full.leds[i] = CRGB(0,0,0);
    }
}
