/* gcc clash_of_balls.c -o clash_of_balls.o -lSDL2 -lm */

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#define bool _Bool
#define true 1
#define false 0

#define PI 3.14159265358979323846

const int BALL_SIZE = 300;
const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 854;
const int FRAME_RATE = 64;
const float DT = 1000.0f / FRAME_RATE;

typedef struct
{
    double x, y; /* Coordonnées pour les calculs de vecteurs */
} Vect;

typedef struct
{
    SDL_Rect rect;        /* Affichage de la boule */
    Vect speed;           /* Vitesse de la boule */
    Vect position;        /* Position de la boule */
    SDL_Texture *texture; /* Texture de la boule */
} Ball;

/* Prototypes des fonctions */
int initSDL();
void createBall(Ball *ball, int size, Vect position, Vect speed, SDL_Texture *texture);
void updateBall(Ball *ball, Ball *balls, int num_boule_actuelle, int nb_balls);
void drawBall(SDL_Renderer *renderer, Ball *ball);
void cleanup(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *texture1, SDL_Texture *texture2, Ball *balls, int ballCount);
void addVect(Vect v1, Vect v2, double coeff, Vect *dest);
double dot(Vect v1, Vect v2);
double norm(Vect v);
void calculer_collision(Ball *b1, Ball *b2, double mA, double mB, Vect *vA_f, Vect *vB_f);

int main(int argc, char *argv[])
{
    srand(time(NULL));

    if (initSDL() < 0)
    {
        return -1;
    }

    /* Création de la fenêtre et du renderer */
    SDL_Window *window = SDL_CreateWindow("Clash_of_balls", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    /* Charger l'image pour les boules */
    SDL_Surface *imageSurfaceRouge = SDL_LoadBMP("boule_rouge.bmp");
    SDL_Surface *imageSurfaceBleu = SDL_LoadBMP("boule_bleu.bmp");
    if (!imageSurfaceRouge)
    {
        printf("Erreur de chargement de l'image: %s\n", SDL_GetError());
        cleanup(window, renderer, NULL, NULL, NULL, 0);
        return -1;
    }
    if (!imageSurfaceBleu)
    {
        printf("Erreur de chargement de l'image: %s\n", SDL_GetError());
        cleanup(window, renderer, NULL, NULL, NULL, 0);
        return -1;
    }
    SDL_Texture *imageTexture_bouleRouge = SDL_CreateTextureFromSurface(renderer, imageSurfaceRouge);
    SDL_Texture *imageTexture_bouleBleu = SDL_CreateTextureFromSurface(renderer, imageSurfaceBleu);
    SDL_FreeSurface(imageSurfaceRouge);
    SDL_FreeSurface(imageSurfaceBleu);

    /* Allocation et génération des boules */
    int maxBalls = 2;
    Ball *balls = (Ball *)malloc(maxBalls * sizeof(Ball));
    Vect pos_depart_rouge = {100.0, 100.0};
    Vect pos_depart_bleu = {200, 200};
    Vect v_depart_rouge = {0.0, 0.0};
    Vect v_depart_bleu = {6.0, 6.0};
    createBall(&balls[0], 50, pos_depart_rouge, v_depart_bleu, imageTexture_bouleRouge);
    createBall(&balls[1], 50, pos_depart_bleu, v_depart_rouge, imageTexture_bouleBleu);

    /* Boucle principale */
    bool isRunning = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }
        }

        /* Mise à jour des boules */
        for (int i = 0; i < maxBalls; i++)
        {
            updateBall(&balls[i], balls, i, maxBalls);
        }

        /* Rafraîchissement de l'écran */
        SDL_SetRenderDrawColor(renderer, 0, 0, 100, 255);
        SDL_RenderClear(renderer);

        /* Dessiner les boules */
        for (int i = 0; i < maxBalls; i++)
        {
            drawBall(renderer, &balls[i]);
        }

        SDL_RenderPresent(renderer);

        /* Contrôle du framerate */
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsedTime = currentTime - lastTime;
        if (elapsedTime < DT)
        {
            SDL_Delay((int)(DT - elapsedTime));
        }
        lastTime = SDL_GetTicks();
    }

    /* Libération des ressources */
    cleanup(window, renderer, imageTexture_bouleRouge, imageTexture_bouleBleu, balls, maxBalls);

    return 0;
}

int initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Erreur d'initialisation de SDL: %s\n", SDL_GetError());
        return -1;
    }
    return 0;
}

void createBall(Ball *ball, int size, Vect position, Vect speed, SDL_Texture *texture)
{
    /* Initialiser la position de la boule avec les coordonnées fournies */
    ball->rect.w = size;
    ball->rect.h = size;
    ball->rect.x = floor(position.x);
    ball->rect.y = floor(position.y);
    ball->texture = texture;
    ball->position = position;

    /* Initialiser la vitesse de la boule avec des valeurs aléatoires */
    ball->speed = speed;
}

Vect posCenter(Ball *ball)
{
    Vect res;
    res.x = ball->rect.x + ball->rect.w / 2;
    res.y = ball->rect.y + ball->rect.h / 2;
    return res;
}

double distance(Ball *ball1, Ball *ball2)
{
    Vect m1 = posCenter(ball1);
    Vect m2 = posCenter(ball2);
    Vect delta = {m1.x - m2.x, m1.y - m2.y};
    return norm(delta);
}

void calculer_collision(Ball *b1, Ball *b2, double mA, double mB, Vect *vA_f, Vect *vB_f)
{
    // Calcul de la différence de position et de vitesse
    Vect delta_v = {b1->speed.x - b2->speed.x, b1->speed.y - b2->speed.y};
    Vect delta_p = {b1->position.x - b2->position.x, b1->position.y - b2->position.y};

    double alpha1 = atan2(delta_p.y, delta_p.x);
    double beta1 = atan2(b1->speed.y, b1->speed.x);
    double gamma1 = beta1 - alpha1;

    double alpha2 = atan2(-delta_p.y, -delta_p.x);
    double beta2 = atan2(b2->speed.y, b2->speed.x);
    double gamma2 = beta2 - alpha2;

    double v1 = norm(b1->speed);
    double v2 = norm(b2->speed);

    double v12 = v1 * cos(gamma1);
    double v21 = v2 * cos(gamma2);
    double v11 = v1 * sin(gamma1);
    double v22 = v2 * sin(gamma2);

    double v1_f = (v12 * (mA - mB) - 2 * mB * v21) / (mA + mB);
    double v2_f = (v21 * (mA - mB) + 2 * mA * v12) / (mA + mB);

    vA_f->x = v1_f * cos(alpha1) - v11 * sin(alpha1);
    vA_f->y = v1_f * sin(alpha1) + v11 * cos(alpha1);

    vB_f->x = v2_f * cos(alpha2) - v22 * sin(alpha2);
    vB_f->y = v2_f * sin(alpha2) + v22 * cos(alpha2);
}

void updateBall(Ball *ball, Ball *balls, int num_boule_actuelle, int nb_balls)
{
    int diametre = ball->rect.w;
    ball->position.x += ball->speed.x;
    ball->position.y += ball->speed.y;

    ball->rect.x = floor(ball->position.x);
    ball->rect.y = floor(ball->position.y);

    if ((ball->rect.x <= 0 && ball->speed.x < 0) || (ball->rect.x + ball->rect.w >= SCREEN_WIDTH && ball->speed.x > 0))
    {
        ball->speed.x *= -1;
        ball->position.x += ball->speed.x;
    }
    if ((ball->rect.y <= 0 && ball->speed.y < 0) || (ball->rect.y + ball->rect.h >= SCREEN_HEIGHT && ball->speed.y > 0))
    {
        ball->speed.y *= -1;
        ball->position.y += ball->speed.y;
    }
    if (ball->rect.y + ball->rect.h >= SCREEN_HEIGHT)
    {
        ball->speed.y = -ball->speed.y;
        ball->rect.y -= 5;
    }
    

    for (int i = num_boule_actuelle + 1; i < nb_balls; i++)
    {
        Ball *other = &balls[i];
        double dist = distance(ball, other);

        if (dist <= diametre)
        {
            Vect vA_f;
            Vect vB_f;
            calculer_collision(ball, other, 1, 1, &vA_f, &vB_f);
            ball->speed = vA_f;
            other->speed = vB_f;
        }
    }
}

void drawBall(SDL_Renderer *renderer, Ball *ball)
{
    SDL_RenderCopy(renderer, ball->texture, NULL, &ball->rect);
}

void cleanup(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *texture1, SDL_Texture *texture2, Ball *balls, int ballCount)
{
    if (balls)
    {
        free(balls);
    }
    if (texture1)
    {
        SDL_DestroyTexture(texture1);
    }
    if (texture2)
    {
        SDL_DestroyTexture(texture2);
    }
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }
    if (window)
    {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

void addVect(Vect v1, Vect v2, double coeff, Vect *dest)
{
    dest->x = v1.x + coeff * v2.x;
    dest->y = v1.y + coeff * v2.y;
}

double dot(Vect v1, Vect v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

double norm(Vect v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}
