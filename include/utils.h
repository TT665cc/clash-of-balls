#include <SDL2/SDL.h>
#include <math.h>

#define bool int
#define true 1
#define false 0

typedef struct Vect
{
    double x, y; /* Coordonnées pour les calculs de vecteurs */
} Vect;


/**
 * @brief Check if a point is in a box
 * 
 * @param point Point
 * @param box Box
 * @return true if the point is in the box, false otherwise
 */
bool isInBox(Vect point, SDL_Rect box);


/**
 * @brief Calculate the norm of a vector
 * 
 * @param v Vector
 */
double norm(Vect v);

/**
 * @brief Calculate the dot product of two vectors
 * 
 * @param v1 First vector
 * @param v2 Second vector
 * @return Dot product of the two vectors
 */
double dot(Vect v1, Vect v2);


/**
 * @brief Add two vectors
 * 
 * @param v1 First vector
 * @param v2 Second vector
 * @param coeff Coefficient to multiply the second vector
 * @param dest Destination vector
 * @return void
 */
void addVect(Vect v1, Vect v2, double coeff, Vect *dest);

/**
 * @brief Convert the beginning of a string to an integer
 * 
 * @param str String to convert
 * @return int Converted integer
 */
int partialAtoi(const char *str);
