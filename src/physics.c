#include "../include/physics.h"

bool sphereCollidesWall(SDL_FRect sphere, SDL_FRect wall)
{

    Vect dist = (Vect){fmax(fabs(sphere.x - wall.x) - wall.w / 2, 0.0), fmax(fabs(sphere.y - wall.y) - wall.h / 2, 0.0)};

    return (norm(dist) <= sphere.w / 2);
}

bool __collides(SDL_FRect rect1, SDL_FRect rect2, ObjectType type1, ObjectType type2)
{
    if (type1 == Ball && type2 == Ball)
            return distance((Vect){rect1.x, rect1.y}, (Vect){rect2.x, rect2.y}) < rect1.w / 2 + rect2.w / 2;
    else if (type1 == Ball && type2 == Wall)
            return sphereCollidesWall(rect1, rect2);
    else if (type1 == Wall && type2 == Ball)
            return sphereCollidesWall(rect2, rect1);
    else if (type1 == Wall && type2 == Wall)
            return !(rect1.x + rect1.w < rect2.x || rect1.x > rect2.x + rect2.w || rect1.y + rect1.h < rect2.y || rect1.y > rect2.y + rect2.h);
    return false;

}

double cineticEnergy(Object object)
{
    double speed = norm(object.speed);
    return object.mass * speed * speed;
}

void choc(Object *ball1, Object *ball2, int *num_objects_list, Vect *nb_color_balls)
{
    if(ball1->type != 0 || ball2->type != 0)
    {
        return;
    }
    int nb_aleat = rand() % 100;
    if (nb_aleat % 3 != 0 && ball1->color != ball2->color)
    { /* 1 chance sur 3 égalité */

        if (ball1->invincible && ball2->invincible)
        {
            return;
        }
        else if (ball1-> invincible || cineticEnergy(*ball1) * 100 / (cineticEnergy(*ball1) + cineticEnergy(*ball2)) > nb_aleat)
        {
            /* ball1 gagne */
            deleteObject(*ball2, num_objects_list, nb_color_balls);
            ball1->colliding = -1;
        }
        else
        {
            /* ball2 gagne */
            deleteObject(*ball1, num_objects_list, nb_color_balls);
            ball2->colliding = -1;
        }
    }
}

void updateObjects(Uint64 elapsedTime, Object *objects, int *num_objects_list, Vect *nb_color_balls, Vect limits)
{
    for (int i = 0; i < max_objects; i++)
    {
        if (num_objects_list[i] == 1)
        {
            updateObject(elapsedTime, &objects[i], objects, i, num_objects_list, nb_color_balls, limits);
        }
    }

    for (int i = 0; i < max_objects; i++)
    {
        if (num_objects_list[i] == 1)
        {
            objects[i].position = addVect(objects[i].position, objects[i].speed, elapsedTime / 1000.0);
            objects[i].rect.x = floor(objects[i].position.x - objects[i].rect.w / 2);
            objects[i].rect.y = floor(objects[i].position.y - objects[i].rect.h / 2);
        }
    }
}

void createBall(SDL_Renderer *renderer, Object *objects, int radius, double mass, Vect position, Vect speed, int color, Vect *nb_color_balls, int *num_objects_list, bool invincible)
{
    for (int i = 0; i < max_objects; i++)
    {
        if (num_objects_list[i] == 0)
        {
            num_objects_list[i] = 1;
            objects[i].rect.w = radius * 2;
            objects[i].rect.h = radius * 2;
            objects[i].rect.x = floor(position.x - radius);
            objects[i].rect.y = floor(position.y - radius);
            objects[i].texture = createCircleTexture(renderer, radius, color);
            objects[i].position = position;
            objects[i].mass = mass;
            objects[i].colliding = -1;
            objects[i].num = i;
            objects[i].color = color;
            objects[i].type = Ball;
            objects[i].speed = speed;
            objects[i].invincible = invincible;

            if (color == 0)
            {
                nb_color_balls->x += 1;
            }
            else
            {
                nb_color_balls->y += 1;
            }

            break;
        }
    }
}

void createWall(SDL_Renderer *renderer, Object *objects, int width, int height, Vect position, int color, int *num_objects_list)
{
    for (int i = 0; i < max_objects; i++)
    {
        if (num_objects_list[i] == 0)
        {
            num_objects_list[i] = 1;
            objects[i].rect.w = width;
            objects[i].rect.h = height;
            objects[i].rect.x = (int) floor(position.x - width / 2);
            objects[i].rect.y = (int) floor(position.y - height / 2);
            objects[i].texture = createRectangleTexture(renderer, width, height, color);
            objects[i].position = position;
            objects[i].mass = 0;
            objects[i].colliding = -1;
            objects[i].num = i;
            objects[i].color = 0;
            objects[i].type = Wall;
            objects[i].speed = (Vect){0, 0};
            break;
        }
    }
}

void deleteObject(Object object, int *num_objects_list, Vect *nb_color_balls)
{
    num_objects_list[object.num] = 0;
    if (object.texture != NULL)
    {
        SDL_DestroyTexture(object.texture);
        object.texture = NULL;
    }
    if (object.type != 0)
        return;
    if (object.color == 0)
    {
        nb_color_balls->x -= 1;
    }
    else
    {
        nb_color_balls->y -= 1;
    }
}

void deleteAllObjects(Object *objects, int *num_objects_list, Vect *nb_color_balls)
{
    for (int i = 0; i < max_objects; i++)
    {
        if (num_objects_list[i] == 1)
        {
            deleteObject(objects[i], num_objects_list, nb_color_balls);
        }
    }
}

void ballCollision(Object *b1, Object *b2)
{
    // Calcul de la différence de position et de vitesse
    Vect delta_p = addVect(b1->position, b2->position, -1);

    normalize(&delta_p);

    double p = 2 * (dot(b1->speed, delta_p) - dot(b2->speed, delta_p)) / (b1->mass + b2->mass);

    b1->speed = addVect(b1->speed, delta_p, -p * b2->mass);
    b2->speed = addVect(b2->speed, delta_p, p * b1->mass);
}

void wallCollision(Object *ball, Object *wall)
{

    // On suppose que la collision est déjà détectée

    double distanceX = abs(ball->position.x - wall->position.x) - wall->rect.w / 2;
    double distanceY = abs(ball->position.y - wall->position.y) - wall->rect.h / 2;

    if (distanceX <= 0 && distanceY > 0)
    {
        ball->speed.y *= -1;
        return;
    }

    if (distanceX > 0 && distanceY <= 0)
    {
        ball->speed.x *= -1;
        return;
    }
    
    // Gestion des coins
    // 1. Trouver le coin le plus proche (parmis les 4 coins)
    Vect nearestCorner = (Vect) {wall->position.x + wall->rect.w / 2, wall->position.y + wall->rect.h / 2};

    for (int i = 0; i < 4; i++)
    {
        Vect corner = (Vect){wall->position.x + (i % 2 == 0 ? -1 : 1) * (wall->rect.w / 2), wall->position.y + (i / 2 == 0 ? -1 : 1) * (wall->rect.h / 2)};
        if (distance(ball->position, corner) <= distance(ball->position, nearestCorner))
        {
            nearestCorner = corner;
        }
    }

    // Calcul de l'angle entre le vecteur vitesse et le vecteur de collision

    Vect collisionVector = addVect(ball->position, nearestCorner, -1);
    double angle = angleBetween(ball->speed, collisionVector);

    // Calcul de la nouvelle vitesse

    double speed = norm(ball->speed);

    Vect newSpeed = rotateVect(ball->speed, 2 * angle);

    newSpeed.x *= -1; // Inversion de la vitesse
    newSpeed.y *= -1;

    ball->speed = newSpeed;
}

bool canAppear(Vect position, int width, int height, Object *objects, int *num_objects_list, ObjectType type, Vect limits)
{
    if ((position.x - width / 2 <= 0) || (position.x + width / 2 >= limits.x))
    {
        return false;
    }
    if ((position.y - height / 2 <= 0) || (position.y + height / 2 >= limits.y))
    {
        return false;
    }

    SDL_FRect rect1 = { position.x, position.y, width, height };

    for (int i = 0; i < max_objects; i++)
    {
        if (num_objects_list[i] == 0)
        {
            continue;
        }
        Object *other = &objects[i];

        SDL_FRect rect2 = { other->position.x, other->position.y, other->rect.w, other->rect.h };

        if (__collides(rect1, rect2, type, other->type))
            return false;
    }
    return true;
}

double collides(Object *o1, Object *o2)
{
    SDL_FRect rect1 = {o1->position.x, o1->position.y, o1->rect.w, o1->rect.h};
    SDL_FRect rect2 = {o2->position.x, o2->position.y, o2->rect.w, o2->rect.h};

    return __collides(rect1, rect2, o1->type, o2->type);
}

void handleCollision(Object *o1, Object *o2)
{
    if (o1->type == 1)
    {
        wallCollision(o2, o1);
    }
    else if (o2->type == 1)
    {
        wallCollision(o1, o2);
    } else {
        ballCollision(o1, o2);
    }
}

void updateObject(Uint64 dt, Object *object, Object *objects, int num_boule_actuelle, int *num_objects_list, Vect *nb_color_balls, Vect limits)
{

    if ((object->rect.x <= 0 && object->speed.x < 0) || (object->rect.x + object->rect.w >= limits.x && object->speed.x > 0))
    {
        object->speed.x *= -1;
    }
    if ((object->rect.y <= 0 && object->speed.y < 0) || (object->rect.y + object->rect.h >= limits.y && object->speed.y > 0))
    {
        object->speed.y *= -1;
    }

    for (int i = num_boule_actuelle + 1; i < max_objects; i++)
    {
        if (num_objects_list[i] == 1)
        {
            Object *other = &objects[i];

            if(object->colliding == -1 && other->colliding == -1 && collides(object, other))
            {
                handleCollision(object, other);
                object->colliding = i;
                other->colliding = num_boule_actuelle;
                choc(object, other, num_objects_list, nb_color_balls);
            }

            if ((object->colliding == i && other->colliding == num_boule_actuelle) && !collides(object, other))
            {
                object->colliding = -1;
                other->colliding = -1;
            }
        }
    }
}