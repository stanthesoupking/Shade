#pragma once

namespace Shade
{
    struct Rect
    {
        float x;
        float y;
        float width;
        float height;

        Rect()
        {
            this->x = 0;
            this->y = 0;
            this->width = 0;
            this->height = 0;
        }

        Rect(float _x, float _y, float _width, float _height)
        {
            this->x = _x;
            this->y = _y;
            this->width = _width;
            this->height = _height;
        }

        Rect(float _width, float _height)
        {
            this->x = 0;
            this->y = 0;
            this->width = _width;
            this->height = _height;
        }
    };
}