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

Vect addVect(Vect v1, Vect v2, double coeff)
{
    return (Vect){v1.x + coeff * v2.x, v1.y + coeff * v2.y};
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

    if (i == 0)
    {
        return -1;
    }
    return res;
}

Mat2 multiplyVect(Vect v, Vect coeff)
{
    return (Mat2){v.x * coeff.x, v.x * coeff.y, v.y * coeff.x, v.y * coeff.y};
}

double angleBetween(Vect v1, Vect v2)
{
    Mat2 m = (Mat2){v1.x, v2.x, v1.y, v2.y};
    return atan2(detMat2(m), dot(v1, v2));
}

Vect rotateVect(Vect v, double angle)
{
    return (Vect) { v.x * cos(angle) - v.y * sin(angle), v.x * sin(angle) + v.y * cos(angle)};
}

Vect diagonalMat2(Mat2 m)
{
    return (Vect){m.a, m.d};
}

Mat2 inverseMat2(Mat2 m)
{
    double det = detMat2(m);
    return (Mat2){m.d / det, -m.b / det, -m.c / det, m.a / det};
}

double detMat2(Mat2 m)
{
    return m.a * m.d - m.b * m.c;
}

Mat2 multiplyMat2(Mat2 m1, Mat2 m2)
{
    return (Mat2){m1.a * m2.a + m1.b * m2.c, m1.a * m2.b + m1.b * m2.d, m1.c * m2.a + m1.d * m2.c, m1.c * m2.b + m1.d * m2.d};
}

Vect multiplyMat2Vect(Mat2 m, Vect v)
{
    return (Vect){m.a * v.x + m.b * v.y, m.c * v.x + m.d * v.y};
}

double traceMat2(Mat2 m)
{
    return m.a + m.d;
}

double distance(Vect v1, Vect v2)
{
    return norm(addVect(v1, v2, -1));
}

void normalize(Vect *v)
{
    double n = norm(*v);
    v->x /= n;
    v->y /= n;
}