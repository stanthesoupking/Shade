#pragma once

namespace Shade
{
    struct Colour
    {
        float r;
        float g;
        float b;
        float a;

        Colour(float _r, float _g, float _b, float _a = 1.0f)
        {
            r = _r;
            g = _g;
            b = _b;
            a = _a;
        }
    };
}