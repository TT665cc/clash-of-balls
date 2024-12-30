#ifndef PHYSICS_H

#define PHYSICS_H

#include "../include/utils.h"
#include "../include/texture.h"
#define max_objects 100

typedef enum Type
{
    Ball,
    Wall
} ObjectType;

typedef struct Object
{
    SDL_Rect rect;        /* Affichage de la boule */
    Vect speed;           /* Vitesse de la boule */
    Vect position;        /* Position de la boule */
    SDL_Texture *texture; /* Texture de la boule */
    double mass;
    int colliding;
    int life;
    int num;
    int color; /* 0 pour blanc, 1 pour noir */
    ObjectType type;
    bool invincible;
} Object;

/*typedef struct Mouse
{
    SDL_Rect shape;
    ObjectType type;
};*/

/**
 * @brief Check if two objects collide
 * 
 * @param rect1 First object
 * @param rect2 Second object
 * @param type Type of the objects
 * 
 * @return true if the two objects collide, false otherwise
 */
bool __collides(SDL_FRect rect1, SDL_FRect rect2, ObjectType type1, ObjectType type2);

/**
 * @brief Check if a sphere collides with a wall
 * 
 * @param sphere Sphere
 * @param wall Wall
 * 
 * @return true if the sphere collides with the wall, false otherwise
 */
bool sphereCollidesWall(SDL_FRect sphere, SDL_FRect wall);

/**
 * @brief Create a ball
 * 
 * @param renderer Renderer
 * @param objects List of objects
 * @param radius Radius of the ball
 * @param mass Mass of the ball
 * @param position Position of the ball
 * @param speed Speed of the ball
 * @param color Color of the ball
 * @param nb_color_balls Number of balls of each color
 * @param num_objects_list List of the number of objects
 * @param invincible True if the ball is invincible, false otherwise
 */
void createBall(SDL_Renderer *renderer, Object *objects, int radius, double mass, Vect position, Vect speed, int color, Vect *nb_color_balls, int *num_objects_list, bool invincible);

/**
 * @brief Create a wall
 * 
 * @param renderer Renderer
 * @param objects List of objects
 * @param width Width of the wall
 * @param height Height of the wall
 * @param position Position of the wall
 * @param color Color of the wall
 * @param num_objects_list List of the number of objects
 */
void createWall(SDL_Renderer *renderer, Object *objects, int width, int height, Vect position, int color, int *num_objects_list);

/**
 * @brief Delete an object
 * 
 * @param object Object to delete
 * @param num_objects_list List of the number of objects
 * @param nb_color_balls Number of balls of each color
 */
void deleteObject(Object object, int *num_objects_list, Vect *nb_color_balls);

/**
 * @brief Delete all objects
 * 
 * @param objects List of objects
 * @param num_objects_list List of the number of objects
 * @param nb_color_balls Number of balls of each color
 */
void deleteAllObjects(Object *objects, int *num_objects_list, Vect *nb_color_balls);

/**
 * @brief Handle the collision between two balls
 * 
 * @param b1 First ball
 * @param b2 Second ball
 */
void ballCollision(Object *b1, Object *b2);

/**
 * @brief Handle the collision between a ball and a wall
 * 
 * @param ball Ball
 * @param wall Wall
 */
void wallCollision(Object *ball, Object *wall);

/**
 * @brief Check if a ball can appear at a given position
 * 
 * @param position Position of the ball
 * @param width Width of the ball
 * @param height Height of the ball
 * @param objects List of objects
 * @param num_objects_list List of the number of objects
 * @param type Type of the ball
 * @param limits Limits of the arena
 * 
 * @return true if the ball can appear at the given position, false otherwise
 */
bool canAppear(Vect position, int width, int height, Object *objects, int *num_objects_list, ObjectType type, Vect limits);

/**
 * @brief Update the position of an object
 * 
 * @param dt Time elapsed since the last update
 * @param object Object to update
 * @param objects List of objects
 * @param num_boule_actuelle Number of objects
 * @param num_objects_list List of the number of objects
 * @param nb_color_balls Number of balls of each color
 * @param limits Limits of the arena
 */
void updateObject(Uint64 dt, Object *object, Object *objects, int num_boule_actuelle, int *num_objects_list, Vect *nb_color_balls, Vect limits);

/**
 * @brief Check if two objects collide
 * 
 * @param o1 First object
 * @param o2 Second object
 * 
 * @return true if the two objects collide, false otherwise
 */
double collides(Object *o1, Object *o2);

/**
 * @brief Handle the collision between two objects
 * 
 * @param o1 First object
 * @param o2 Second object
 */
void handleCollision(Object *o1, Object *o2);

/**
 * @brief Update the position of all objects
 * 
 * @param elapsedTime Time elapsed since the last update
 * @param objects List of objects
 * @param num_objects_list List of the number of objects
 * @param nb_color_balls Number of balls of each color
 * @param limits Limits of the arena
 */
void updateObjects(Uint64 elapsedTime, Object *objects, int *num_objects_list, Vect *nb_color_balls, Vect limits);



#endif