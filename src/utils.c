#include <SDL2/SDL.h>
#include <math.h>
#include "../include/utils.h"

bool isInBox(Vect point, SDL_Rect box)
{
    if (point.x >= box.x && point.x <= box.x + box.w && point.y >= box.y && point.y <= box.y + box.h)
    {
        return true;
    }
    return false;
}

double norm(Vect v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}

double dot(Vect v1, Vect v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

void addVect(Vect v1, Vect v2, double coeff, Vect *dest)
{
    dest->x = v1.x + coeff * v2.x;
    dest->y = v1.y + coeff * v2.y;
}

int partialAtoi(const char *str)
{
    int res = 0;
    int i = 0;
    while (str[i] >= '0' && str[i] <= '9')
    {
        res = res * 10 + str[i] - '0';
        i++;
    }
    return res;
}