#include <SDL2/SDL.h>
#include <math.h>

#define bool int
#define true 1
#define false 0

typedef struct Vect
{
    double x, y; /* Coordonnées pour les calculs de vecteurs */
} Vect;

typedef struct Mat2
{
    double a, b, c, d; /* Coordonnées pour les calculs de matrices 2x2 */
} Mat2;


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
 * @return Vect Resulting vector
 */
Vect addVect(Vect v1, Vect v2, double coeff);

/**
 * @brief Convert the beginning of a string to an integer
 * 
 * @param str String to convert
 * @return int Converted integer
 */
int partialAtoi(const char *str);

/**
 * @brief Multiply two vectors
 * 
 * @param v1 First vector
 * @param v2 Second vector
 * 
 * @return Mat2 Resulting matrix
 */
Mat2 multiplyVect(Vect v1, Vect v2);

/**
 * @brief Calculate the diagonal of a matrix
 * 
 * @param m Matrix
 * 
 * @return Vect Diagonal of the matrix
 */
Vect diagonalMat2(Mat2 m);

/**
 * @brief Calculate the determinant of a matrix
 * 
 * @param m Matrix
 * 
 * @return double Determinant of the matrix
 */
double detMat2(Mat2 m);

/**
 * @brief Calculate the inverse of a matrix
 * 
 * @param m Matrix
 * 
 * @return Mat2 Inverse of the matrix
 */
Mat2 inverseMat2(Mat2 m);

/**
 * @brief Multiply a matrix by a vector
 * 
 * @param m Matrix
 * @param v Vector
 * 
 * @return Vect Resulting vector
 */
Vect multiplyMat2Vect(Mat2 m, Vect v);

/**
 * @brief Multiply two matrices
 * 
 * @param m1 First matrix
 * @param m2 Second matrix
 * 
 * @return Mat2 Resulting matrix
 */
Mat2 multiplyMat2(Mat2 m1, Mat2 m2);

/**
 * @brief Calculate the trace of a matrix
 * 
 * @param m Matrix
 * 
 * @return double Trace of the matrix
 */
double traceMat2(Mat2 m);

/**
 * @brief Rotate a vector by an angle
 * 
 * @param v Vector to rotate
 * @param angle Angle to rotate the vector by
 * 
 * @return Vect Rotated vector
 */

Vect rotateVect(Vect v, double angle);

/**
 * @brief Calculate the angle between two vectors
 * 
 * @param v1 First vector
 * @param v2 Second vector
 * 
 * @return double Angle between the two vectors
 */
double angleBetween(Vect v1, Vect v2);

/**
 * @brief Calculate the distance between two points
 * 
 * @param v1 First point
 * @param v2 Second point
 * 
 * @return double Distance between the two points
 */
double distance(Vect v1, Vect v2);

/**
 * @brief Normalize a vector
 * 
 * @param v Vector to normalize
 */
void normalize(Vect *v);
