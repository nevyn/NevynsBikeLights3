#include <FastLED.h>

class SubStrip
{
public:
    CRGB *leds;
    int length;

    SubStrip(CRGB *start, int length)
    {
        this->leds = start;
        this->length = length;
    }

    int numPixels() {
        return length;
    }
};
