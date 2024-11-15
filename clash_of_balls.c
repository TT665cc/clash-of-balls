/* gcc clash_of_balls.c -o clash_of_balls.o -lSDL2 -lm */

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define bool _Bool
#define true 1
#define false 0

const int BALL_SIZE = 300;
const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 854;
const int FRAME_RATE = 100;
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
    double mass;
    int colliding;
} Ball;

void createBall(Ball *ball, int size, double mass, Vect position, Vect speed, SDL_Texture *texture);
void updateBall(Uint64 dt, Ball *ball, Ball *balls, int num_boule_actuelle, int nb_balls);
void drawBall(SDL_Renderer *renderer, Ball *ball);
void cleanup(SDL_Window *window, SDL_Renderer *renderer, int nb_textures, SDL_Texture **textures, Ball *balls, int ballCount);
Vect posCenter(Ball *ball);
double distance(Ball *ball1, Ball *ball2);
void calculer_collision(Ball *b1, Ball *b2, Vect *vA_f, Vect *vB_f);
double norm(Vect v);
double dot(Vect v1, Vect v2);



int initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Erreur d'initialisation de SDL: %s\n", SDL_GetError());
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{

    if (initSDL() < 0)
    {
        return -1;
    }

    int maxBalls = 50;
    int nb_balls = 4;

    char imgs[4][30] = {
        "./img/boule_rouge.bmp",
        "./img/boule_bleu.bmp",
        "./img/boule_verte.bmp",
        "./img/boule_bleu_clair.bmp"
    };

    SDL_Texture **textures = malloc(sizeof(SDL_Texture*) * 4);


    /* Création de la fenêtre et du renderer */
    SDL_Window *window = SDL_CreateWindow("Clash_of_balls", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    /* Charger l'image pour les boules */

    for(int i = 0; i < 4; i++) {
        SDL_Surface *surf = SDL_LoadBMP(imgs[i]);
        if (!surf)
        {
            printf("Erreur de chargement de l'image: %s\n", SDL_GetError());
            cleanup(window, renderer, 4, textures, NULL, 0);
            return -1;
        }
        SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, surf);
        
        SDL_FreeSurface(surf);
        textures[i] = imageTexture;

    }

    /* Allocation et génération des boules */
    
    Ball *balls = malloc(maxBalls * sizeof(Ball));

    createBall(&balls[0], 50, 10, (Vect) {100.0, 100.0}, (Vect) {600.0, 900.0}, textures[0]);
    createBall(&balls[1], 60, 20, (Vect) {300.0, 200.0}, (Vect) {0.0, 0.0}, textures[1]);
    createBall(&balls[2], 90, 800, (Vect) {400.0, 300.0}, (Vect) {-100.0, 40.0}, textures[2]);
    createBall(&balls[3], 60, 20, (Vect) {300.0, 500.0}, (Vect) {0.0, 0.0}, textures[3]);

    /* Boucle principale */
    bool isRunning = true;
    SDL_Event event;
    Uint64 lastTime = SDL_GetTicks();
    Uint64 frameTime = SDL_GetTicks();
    Uint64 elapsedTime = 0;
    int frameCount = 0;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    createBall(&balls[nb_balls++], 60, 20, (Vect) {event.button.x - 30, event.button.y - 30}, (Vect) {100.0, -50.0}, textures[0]);
                    printf("Clic gauche détecté en (%d, %d)\n", event.button.x, event.button.y);
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    printf("Clic droit détecté en (%d, %d)\n", event.button.x, event.button.y);
                }
            }
        }

        /* Mise à jour des boules */
        for (int i = 0; i < nb_balls; i++)
        {
            updateBall(elapsedTime, &balls[i], balls, i, nb_balls);
        }

        /* Rafraîchissement de l'écran */
        SDL_SetRenderDrawColor(renderer, 0, 0, 100, 255);
        SDL_RenderClear(renderer);

        /* Dessiner les boules */
        for (int i = 0; i < nb_balls; i++)
        {
            drawBall(renderer, &balls[i]);
        }

        SDL_RenderPresent(renderer);

        /* Contrôle du framerate */
        Uint64 currentTime = SDL_GetTicks64();
        elapsedTime = currentTime - frameTime;
        /*if (elapsedTime < DT)
        {
            SDL_Delay((int)(DT - elapsedTime));
        }*/
        frameCount++;

        // Every second, display the FPS (frame count since the last second)

        if (currentTime - lastTime >= 5000)
        {
            printf("FPS: %d\n", frameCount / 5);

        
            frameCount = 0;
            lastTime = currentTime;
        }

        frameTime = SDL_GetTicks64();
    }

    /* Libération des ressources */
    cleanup(window, renderer, 4, textures, balls, maxBalls);

    return 0;
}

void createBall(Ball *ball, int size, double mass, Vect position, Vect speed, SDL_Texture *texture)
{
    /* Initialiser la position de la boule avec les coordonnées fournies */
    ball->rect.w = size;
    ball->rect.h = size;
    ball->rect.x = floor(position.x);
    ball->rect.y = floor(position.y);
    ball->texture = texture;
    ball->position = position;
    ball->mass = mass;
    ball->colliding = -1;

    /* Initialiser la vitesse de la boule avec des valeurs aléatoires */
    ball->speed = speed;
}

Vect posCenter(Ball *ball)
{
    Vect res;
    res.x = ball->position.x + ball->rect.w / 2;
    res.y = ball->position.y + ball->rect.h / 2;
    return res;
}

double distance(Ball *ball1, Ball *ball2)
{
    Vect m1 = posCenter(ball1);
    Vect m2 = posCenter(ball2);
    Vect delta = {m1.x - m2.x, m1.y - m2.y};
    return norm(delta);
}

void calculer_collision(Ball *b1, Ball *b2, Vect *vA_f, Vect *vB_f)
{
    // Calcul de la différence de position et de vitesse
    Vect delta_v = {b1->speed.x - b2->speed.x, b1->speed.y - b2->speed.y};
    Vect p1 = posCenter(b1);
    Vect p2 = posCenter(b2);
    Vect delta_p = {p1.x - p2.x, p1.y - p2.y};

    double d = norm(delta_p);

    delta_p.x /= d;
    delta_p.y /= d;

    double p = 2*(dot(b1->speed, delta_p) - dot(b2->speed, delta_p)) / (b1->mass + b2->mass); 

    vA_f->x = b1->speed.x - p * b2->mass * delta_p.x;
    vA_f->y = b1->speed.y - p * b2->mass * delta_p.y;

    vB_f->x = b2->speed.x + p * b1->mass * delta_p.x;
    vB_f->y = b2->speed.y + p * b1->mass * delta_p.y;
    
}

void updateBall(Uint64 dt, Ball *ball, Ball *balls, int num_boule_actuelle, int nb_balls)
{
    int diametre = ball->rect.w;

    ball->rect.x = floor(ball->position.x);
    ball->rect.y = floor(ball->position.y);

    if ((ball->rect.x <= 0 && ball->speed.x < 0) || (ball->rect.x + ball->rect.w >= SCREEN_WIDTH && ball->speed.x > 0))
    {
        ball->speed.x *= -1;
    }
    if ((ball->rect.y <= 0 && ball->speed.y < 0) || (ball->rect.y + ball->rect.h >= SCREEN_HEIGHT && ball->speed.y > 0))
    {
        ball->speed.y *= -1;
    }
    

    for (int i = num_boule_actuelle + 1; i < nb_balls; i++)
    {
        Ball *other = &balls[i];

        double collideDistance = ((double) ball->rect.w + (double) other->rect.w) / 2;

        double dist = distance(ball, other);

        if (dist <= collideDistance && (ball->colliding == -1 && other->colliding == -1))
        {
            Vect vA_f;
            Vect vB_f;
            calculer_collision(ball, other, &vA_f, &vB_f);
            ball->speed = vA_f;
            other->speed = vB_f;

            ball->colliding = i;
            other->colliding = num_boule_actuelle;
            printf("Collision entre %d et %d\n", num_boule_actuelle, i);
        }
        
        if (dist > collideDistance && (ball->colliding == i && other->colliding == num_boule_actuelle)){
            ball->colliding = -1;
            other->colliding = -1;
        }
    }

    ball->position.x += ball->speed.x * dt / 1000;
    ball->position.y += ball->speed.y * dt / 1000;
}

void drawBall(SDL_Renderer *renderer, Ball *ball)
{
    SDL_RenderCopy(renderer, ball->texture, NULL, &ball->rect);
}

void cleanup(SDL_Window *window, SDL_Renderer *renderer, int nb_textures, SDL_Texture **textures, Ball *balls, int ballCount)
{
    int i;
    if (textures) {
        for (i = 0; i < nb_textures; i++) {
            if (textures[i]) {
                SDL_DestroyTexture(textures[i]);  // Correct: utiliser SDL_DestroyTexture
            }
        }
        free(textures);
    }
    if (balls) {
        free(balls);
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
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
