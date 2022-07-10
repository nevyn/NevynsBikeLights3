#include <FastLED.h>      // http://fastled.io/
#include <ezButton.h>     // https://arduinogetstarted.com/tutorials/arduino-button-library
#include <OverAnimate.h>  // https://github.com/nevyn/OverAnimate
#define ENCODER_DO_NOT_USE_INTERRUPTS 1
#include <Encoder.h>      // https://www.pjrc.com/teensy/td_libs_Encoder.html

#include "SubStrip.h"


// LEDs/output

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


// buttons/input
ezButton btnLeft(10);
ezButton btnRight(16);
ezButton btnKnob(14);
Encoder  knob(3, 2); // inputs 2 and 3 have interrupts 1 and 0 respectively on the Pro Micro

// animations
AnimationSystem ansys;

void BlinkFunc(Animation *self, int direction, float t);
void ShineFunc(Animation *self, int _, float t);
void SparklyShineFunc(Animation *self, int _, float t);
void BlackFunc(Animation *self, int _, float t);

BoundFunctionAnimation blinkLeft(BlinkFunc,   1);
BoundFunctionAnimation blinkRight(BlinkFunc, -1);

BoundFunctionAnimation clear(BlackFunc, 0);
BoundFunctionAnimation black(BlackFunc, 0);
BoundFunctionAnimation shine(ShineFunc, 0);
BoundFunctionAnimation sparklyShineIn(SparklyShineFunc, 0);
BoundFunctionAnimation sparklyShineOut(SparklyShineFunc, 1);
BoundFunctionAnimation sparklyIn(SparklyShineFunc, 2);
BoundFunctionAnimation sparklyOut(SparklyShineFunc, 3);
BoundFunctionAnimation *anims[] = {
  &clear,
  &black,
  &shine,
  &sparklyShineIn,
  &sparklyShineOut,
  &sparklyIn,
  &sparklyOut,
  &blinkLeft,
  &blinkRight
};
static const int animsCount = sizeof(anims)/sizeof(BoundFunctionAnimation*);

int currentBgIndex = -1;
BoundFunctionAnimation *bgAnims[] = {
  &black,
  &shine,
  &sparklyShineIn,
  &sparklyShineOut,
  &sparklyIn,
  &sparklyOut,
};
static const int bgAnimsCount = sizeof(bgAnims)/sizeof(BoundFunctionAnimation*);


// main app
void setup()
{ 
    FastLED.addLeds<DOTSTAR, DATAPIN, CLOCKPIN, BGR>(leds, TotalPixelCount);
    FastLED.setBrightness(192);
    Serial.begin(9600);

    for(int i = 0; i < animsCount; i++) {
        BoundFunctionAnimation *anim = anims[i];
        anim->beginTime = ansys.now();
        anim->duration = 1.0;
        anim->repeats = true;
        anim->enabled = false;
        ansys.addAnimation(anim);
    }
    clear.enabled = true;
    setCurrentBgAnim(0);
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
    int newBgIndex = abs(knob.read() % bgAnimsCount);
    setCurrentBgAnim(newBgIndex);

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

void setCurrentBgAnim(int newIndex)
{
    if(newIndex == currentBgIndex) return;
    Serial.print("New background: ");
    Serial.println(newIndex);

    int oldIndex = currentBgIndex;
    currentBgIndex = newIndex;

    bgAnims[oldIndex]->enabled = false;
    bgAnims[currentBgIndex]->enabled = true;
    bgAnims[currentBgIndex]->beginTime = ansys.now();
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

    if(f > 0.7)
    {
        float subf = (f-0.6)*2.5;
        for(int i = 0, j = direction>0?led->numPixels()-1:0; i < 3; i++, j -= direction)
        {
            led->leds[j] = CHSV(HUE_YELLOW, 192, (1-subf)*255);
        }
    }
    

    int c = sin8(f*255)/2;
    indicators.leds[(direction == 1) ? 0 : indicators.length-1] = CRGB(c, c, 0);
}

void BlackFunc(Animation *self, int _, float t)
{
    for(int i = 0; i < full.numPixels(); i++) {
        int c = 0;
        full.leds[i] = CRGB(0,0,0);
    }
}

void ShineFunc(Animation *self, int _, float t)
{
    SubStrip *leds[] = {&front, /*&rear*/};
    for(int l = 0, c = sizeof(leds)/sizeof(*leds); l < c; l++) {
        SubStrip *led = leds[l];
        int mid = led->numPixels()/2;
        for(int i = 0; i < led->numPixels(); i++) {
            int distance = abs(mid - i);
            int range = led->numPixels()/4;
            uint8_t strength = gamma8(clamp((range-distance)*(255/range), 0, 255));
            led->leds[i] = led->leds[i] + CRGB(strength, l==0?strength:0, l==0?strength:0);
        }
    }

    //indicators.leds[1] = CRGB(128, 128, 128);
}

void SparklyShineFunc(Animation *self, int type, float t)
{
    SubStrip *led = &front;

    uint8_t hue1 = t*255;
    uint8_t hue2 = (1-t)*255;
    int center = led->numPixels()/2;
    for(int i = 0, c = led->numPixels(); i < c; i++)
    {
        uint8_t hue;
        uint8_t value;
        if(type%2 == 0)
            hue = i > center ? hue1 : hue2;
        if(type%2 == 1)
            hue = i < center ? hue1 : hue2;
        int distanceToCenter = abs(i - center);
        if(type < 1)
            value = (1-(distanceToCenter/(float)center))*255;
        else
            value = 255;
        led->leds[i] = CHSV(hue + (255/c)*i, 255, value);
    }

    if(type < 1)
        ShineFunc(self, type, t);
    
    //indicators.leds[1] = led->leds[center];
}
