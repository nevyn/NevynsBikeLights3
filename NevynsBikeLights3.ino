#include <FastLED.h>      // http://fastled.io/
#include <ezButton.h>     // https://arduinogetstarted.com/tutorials/arduino-button-library
#include <OverAnimate.h>  // https://github.com/nevyn/OverAnimate
#define ENCODER_DO_NOT_USE_INTERRUPTS 1
#include "Rotary.h"      // https://github.com/buxtronix/arduino/tree/master/libraries/Rotary

#include "SubStrip.h"


// LEDs/output

#define DATAPIN    5
#define CLOCKPIN   6

typedef enum {
    IndicatorPixelCount = 3,
    FrontPixelCount = 23,
    LeftSidePixelCount = 18,
    RearPixelCount = 51,
    RightSidePixelCount = 18,
    TotalPixelCount = IndicatorPixelCount + FrontPixelCount + LeftSidePixelCount + RearPixelCount + RightSidePixelCount
} PixelCounts;
CRGB leds[TotalPixelCount];
SubStrip full(leds, TotalPixelCount);
SubStrip indicators(leds, IndicatorPixelCount);
SubStrip front(leds+IndicatorPixelCount, FrontPixelCount);
SubStrip leftSide(front.leds+FrontPixelCount, LeftSidePixelCount);
SubStrip rear(leftSide.leds+LeftSidePixelCount, RearPixelCount);
SubStrip rightSide(rear.leds+RearPixelCount, RightSidePixelCount);



// buttons/input
ezButton btnLeft(10);
ezButton btnRight(16);
ezButton btnKnob(14);
Rotary  knob(3, 2); // inputs 2 and 3 have interrupts 1 and 0 respectively on the Pro Micro

typedef enum {
    KnobBg,
    KnobBrightness,

    KnobModeCount
} KnobMode;
KnobMode knobMode = KnobBg;
uint8_t requestedBrightness = 64;

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

static const int defaultBgIndex = 0;
int currentBgIndex = -1;
int requestedBgIndex = defaultBgIndex;
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
    Serial.begin(9600);

    attachInterrupt(digitalPinToInterrupt(knob.pin1), rotate, CHANGE);
    attachInterrupt(digitalPinToInterrupt(knob.pin2), rotate, CHANGE);

    for(int i = 0; i < animsCount; i++) {
        BoundFunctionAnimation *anim = anims[i];
        anim->beginTime = ansys.now();
        anim->duration = 1.0;
        anim->repeats = true;
        anim->enabled = false;
        ansys.addAnimation(anim);
    }
    clear.enabled = true;
    setCurrentBgAnim(defaultBgIndex);
}


unsigned long lastMillis;
void loop()
{
    btnLeft.loop();
    btnRight.loop();
    btnKnob.loop();
  
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
    int newBgIndex = requestedBgIndex % bgAnimsCount;
    setCurrentBgAnim(newBgIndex);

    if(requestedBrightness != FastLED.getBrightness())
    {
        Serial.print("Changing to brightness ");
        Serial.println(requestedBrightness);
        FastLED.setBrightness(requestedBrightness);
    }
    if(btnKnob.isPressed())
    {
        knobMode = (knobMode + 1) % KnobModeCount;
        Serial.print("Changing to knob mode ");
        Serial.println(knobMode);
    }

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

void rotate()
{
    unsigned char result = knob.process();
    int change = (result == DIR_CW) ? 1 :
        (result == DIR_CCW) ? -1 :
        0;
    if(change == 0) return;

    if(knobMode == KnobBg)
    {
        requestedBgIndex = clamp(requestedBgIndex + change, 0, bgAnimsCount-1);
    }
    else if(knobMode == KnobBrightness)
    {
        requestedBrightness = clamp(requestedBrightness + change*10, 0, 255);
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

    static const float wanderFraction = 0.7;
    static const float sideBlinkFraction = 1.0 - wanderFraction;
    if(f > wanderFraction)
    {
        float subf = clamp((f-wanderFraction)*(1.0/sideBlinkFraction), 0.0, 1.0);
        CRGB fadingYellow = CHSV(HUE_YELLOW, 192, (1-subf)*255);
        for(int i = 0, j = direction>0?led->numPixels()-1:0; i < 3; i++, j -= direction)
        {
            led->leds[j] = fadingYellow;
        }
        for(int i = 0, j = direction>0?rear.numPixels()-1:0; i < 17; i++, j -= direction)
        {
            rear.leds[j] = fadingYellow;
        }
        SubStrip *side = direction==1 ? &rightSide : &leftSide;
        for(int i = 0; i < side->numPixels(); i++)
        {
            side->leds[i] = fadingYellow;
        }
    }
    

    int c = sin8(f*255)/2;
    indicators.leds[(direction == -1) ? 0 : indicators.length-1] = CRGB(c, c, 0);
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
    SubStrip *leds[] = {&front, &rear};
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

    indicators.leds[1] = CRGB(128, 128, 128);
}

void SparklyShineFunc(Animation *self, int type, float t)
{
    SubStrip *leds[] = {&front, &rear};
    uint8_t hue1 = t*255;
    uint8_t hue2 = (1-t)*255;
    for(int s = 0; s < sizeof(leds)/sizeof(*leds); s++)
    {
        SubStrip *led = leds[s];
        if(s==1) type ^= 1; // swap direction of right leds
        
        int center = led->numPixels()/2;
        for(int i = 0, c = led->numPixels(); i < c; i++)
        {
            uint8_t hue;
            uint8_t value;
            // type 0 and 2 travel forward, 1 and 3 backward
            if(type%2 == 0)
                hue = i > center ? hue1 : hue2;
            else
                hue = i < center ? hue1 : hue2;
            int distanceToCenter = abs(i - center);
            // type 0 and 1 fade to black towards the end
            if(type < 2)
                value = (1-(distanceToCenter/(float)center))*255;
            else
                value = 255;
            led->leds[i] = CHSV(hue + (255/c)*i, 255, value);
        }
    }
    

    SubStrip *sideLeds[] = {&leftSide, &rightSide};
    for(int s = 0; s < sizeof(sideLeds)/sizeof(*sideLeds); s++)
    {
        SubStrip *led = sideLeds[s];
        if(s==1) type ^= 1; // swap direction of right leds
        for(int i = 0, c = led->numPixels(); i < c; i++)
        {
            uint8_t hue = (type%2) ? hue1 : hue2;
            led->leds[i] = CHSV(hue + (255/c)*i, 255, 255);
        }
    }

    if(type < 2)
        ShineFunc(self, type, t);
    
    indicators.leds[1] = leds[0]->leds[leds[0]->numPixels()/2-2];
}
