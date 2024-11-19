/* gcc clash_of_balls.c -o clash_of_balls.o -lSDL2 -lm */

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Déclarations et définitions inchangées

const int ARENA_WIDTH = 480;   // Largeur de l'arène
const int ARENA_HEIGHT = 854;  // Hauteur de l'arène
const int SCREEN_WIDTH = 1400;  // Largeur de la fenêtre
const int SCREEN_HEIGHT = 950; // Hauteur de la fenêtre
const int BALL_SIZE = 300;
const int FRAME_RATE = 100;
const float DT = 1000.0f / FRAME_RATE;

#define bool _Bool
#define true 1
#define false 0

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
    int num;
} Ball;

void createBall(Ball* balls, int size, double mass, Vect position, Vect speed, SDL_Texture *texture, int* num_balls_list, int max_balls);
void updateBall(Uint64 dt, Ball *ball, Ball *balls, int num_boule_actuelle, int maxBalls, int* num_balls_list);
void updateBalls(Uint64 elapsedTime, Ball *balls, int maxBalls, int* num_balls_list);
void drawBall(SDL_Renderer *renderer, Ball *ball);
void drawBalls(SDL_Renderer* renderer, Ball* balls, int maxBalls, int* num_balls_list);
void cleanup(SDL_Window *window, SDL_Renderer *renderer, int nb_textures, SDL_Texture **textures, Ball *balls);
Vect posCenter(Ball *ball);
double distance(Ball *ball1, Ball *ball2);
void calculer_collision(Ball *b1, Ball *b2, Vect *vA_f, Vect *vB_f);
double norm(Vect v);
double dot(Vect v1, Vect v2);
bool canAppear(Vect position, int width, int height, Ball* balls, int max_balls);
void deleteBall(Ball ball, int* num_balls_list);
double cineticEnergy(Ball ball);
void choc(Ball* ball1, Ball* ball2, int* num_balls_list);
void aiPlay(int* num_balls_list, Ball* balls, SDL_Texture *texture, int max_balls);

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
    srand( time( NULL ) );

    if (initSDL() < 0)
    {
        return -1;
    }

    int maxBalls = 100;
    int num_balls_list[maxBalls];
    for (int i = 0; i < maxBalls; i++) {
        num_balls_list[i] = 0;
    }

    char imgs[4][30] = {
        "./img/boule_rouge.bmp",
        "./img/boule_bleu.bmp",
        "./img/boule_noir.bmp",
        "./img/boule_blanche.bmp"
    };

    SDL_Texture **textures = malloc(sizeof(SDL_Texture*) * 4);

    // Création de la fenêtre
    SDL_Window *window = SDL_CreateWindow("Clash_of_balls", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Charger les textures
    for(int i = 0; i < 4; i++) {
        SDL_Surface *surf = SDL_LoadBMP(imgs[i]);
        if (!surf)
        {
            printf("Erreur de chargement de l'image: %s\n", SDL_GetError());
            cleanup(window, renderer, 4, textures, NULL);
            return -1;
        }
        SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        textures[i] = imageTexture;
    }

    Ball *balls = malloc(maxBalls * sizeof(Ball));

    // Ajouter quelques boules initiales
    createBall(balls, 50, 10, (Vect) {100.0, 100.0}, (Vect) {600.0, 900.0}, textures[3], num_balls_list, maxBalls);
    createBall(balls, 60, 20, (Vect) {300.0, 200.0}, (Vect) {0.0, 0.0}, textures[2], num_balls_list, maxBalls);
    createBall(balls, 90, 800, (Vect) {400.0, 300.0}, (Vect) {-100.0, 40.0}, textures[3], num_balls_list, maxBalls);

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
            if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
            {
                isRunning = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseXInArena = event.button.x - (SCREEN_WIDTH - ARENA_WIDTH) / 10;
                int mouseYInArena = event.button.y - (SCREEN_HEIGHT - ARENA_HEIGHT) / 2;

                if (event.button.button == SDL_BUTTON_LEFT &&
                    canAppear((Vect) {mouseXInArena - 30, mouseYInArena - 30}, 60, 60, balls, maxBalls)) {
                    createBall(balls, 60, 20,
                                (Vect) {mouseXInArena - 30, mouseYInArena - 30},
                                (Vect) {100.0, -50.0}, textures[2], num_balls_list, maxBalls);
                    printf("Clic gauche détecté en (%d, %d)\n", event.button.x, event.button.y);
                }
            }
        }

        // Mise à jour des boules
        updateBalls(elapsedTime, balls, maxBalls, num_balls_list);

        // Rendu
        SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255); // Fond noir pour toute la fenêtre
        SDL_RenderClear(renderer);

        // Affichage de la bordure autour de l'arène
        SDL_Rect borderRect = { 
            (SCREEN_WIDTH - ARENA_WIDTH) / 10 - 10, 
            (SCREEN_HEIGHT - ARENA_HEIGHT) / 2 - 10, 
            ARENA_WIDTH + 20, 
            ARENA_HEIGHT + 20 
        };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Blanc
        SDL_RenderDrawRect(renderer, &borderRect);

        // Configurer le viewport pour l'arène
        SDL_Rect arenaViewport = {
            .x = (SCREEN_WIDTH - ARENA_WIDTH) / 10,
            .y = (SCREEN_HEIGHT - ARENA_HEIGHT) / 2,
            .w = ARENA_WIDTH,
            .h = ARENA_HEIGHT
        };
        SDL_RenderSetViewport(renderer, &arenaViewport);

        // Dessiner le fond de l'arène
        SDL_SetRenderDrawColor(renderer, 217, 217, 217, 255);
        SDL_RenderFillRect(renderer, NULL);

        // Dessiner les boules
        drawBalls(renderer, balls, maxBalls, num_balls_list);

        // Réinitialiser le viewport pour dessiner des éléments extérieurs
        SDL_RenderSetViewport(renderer, NULL);

        // Exemple d'affichage de texte ou éléments (ajouter une fonction de dessin si nécessaire)
        // drawText(renderer, "Score: 0", 10, 10);

        SDL_RenderPresent(renderer);

        // Gestion du framerate
        Uint64 currentTime = SDL_GetTicks64();
        elapsedTime = currentTime - frameTime;
        frameCount++;

        if (currentTime - lastTime >= 2000) {
            printf("FPS: %d\n", frameCount / 5);
            frameCount = 0;
            lastTime = currentTime;
            aiPlay(num_balls_list, balls, textures[3], maxBalls);
        }

        frameTime = SDL_GetTicks64();
    }

    cleanup(window, renderer, 4, textures, balls);

    return 0;
}

void createBall(Ball* balls, int size, double mass, Vect position, Vect speed, SDL_Texture *texture, int* num_balls_list, int max_balls)
{
    bool pas_deja_creee = true;
    for (int i = 0; i < max_balls; i++) {
        if (num_balls_list[i] == 0 && pas_deja_creee) {
            num_balls_list[i] = 1;
            balls[i].rect.w = size;
            balls[i].rect.h = size;
            balls[i].rect.x = floor(position.x);
            balls[i].rect.y = floor(position.y);
            balls[i].texture = texture;
            balls[i].position = position;
            balls[i].mass = mass;
            balls[i].colliding = -1;
            balls[i].num = i;

            /* Initialiser la vitesse de la boule avec des valeurs aléatoires */
            balls[i].speed = speed;

            pas_deja_creee = false;
        }
    }
}

void aiPlay(int* num_balls_list, Ball* balls, SDL_Texture *texture, int max_balls) {
    int max_speed = 500;
    Vect position = (Vect){100, rand() % ARENA_HEIGHT};
    Vect vitesse = (Vect){rand() % max_speed, -(rand() % max_speed)};
    if (canAppear(position, 40, 40, balls, max_balls)) {
        createBall(balls, 40, 50, position, vitesse, texture, num_balls_list, max_balls);
    }
}

void deleteBall(Ball ball, int* num_balls_list) {
    num_balls_list[ball.num] = 0;
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

bool canAppear(Vect position, int width, int height, Ball* balls, int max_balls) {
    if ((position.x <= 0) || (position.x + width >= ARENA_WIDTH))
        {
            return false;
        }
    if ((position.y <= 0) || (position.y + height >= ARENA_HEIGHT))
        {
            return false;
        }

    for (int i = 0; i < max_balls; i++)
    {
        Ball *other = &balls[i];

        double collideDistance = ((double) width + (double) other->rect.w) / 2;
        Vect m_other = posCenter(other);
        Vect delta = {position.x-m_other.x, position.y-m_other.y};
        double dist = norm(delta);

        if (dist <= collideDistance)
        {
            return false;
        }
    }
}

void updateBall(Uint64 dt, Ball *ball, Ball *balls, int num_boule_actuelle, int maxBalls, int* num_balls_list)
{
    int diametre = ball->rect.w;

    ball->rect.x = floor(ball->position.x);
    ball->rect.y = floor(ball->position.y);

    if ((ball->rect.x <= 0 && ball->speed.x < 0) || (ball->rect.x + ball->rect.w >= ARENA_WIDTH && ball->speed.x > 0))
    {
        ball->speed.x *= -1;
    }
    if ((ball->rect.y <= 0 && ball->speed.y < 0) || (ball->rect.y + ball->rect.h >= ARENA_HEIGHT && ball->speed.y > 0))
    {
        ball->speed.y *= -1;
    }
    

    for (int i = num_boule_actuelle + 1; i < maxBalls; i++)
    {
        if (num_balls_list[i] == 1)
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
                choc(ball, other, num_balls_list);
                printf("Collision entre %d et %d\n", num_boule_actuelle, i);
            }
            
            if (dist > collideDistance && (ball->colliding == i && other->colliding == num_boule_actuelle)){
                ball->colliding = -1;
                other->colliding = -1;
            }
        }
        
    }

    ball->position.x += ball->speed.x * dt / 1000;
    ball->position.y += ball->speed.y * dt / 1000;
}

void updateBalls(Uint64 elapsedTime, Ball *balls, int maxBalls, int* num_balls_list) {
    for (int i = 0; i < maxBalls; i++)
        {
            if (num_balls_list[i] == 1) {
                updateBall(elapsedTime, &balls[i], balls, i, maxBalls, num_balls_list);
            }
        }
}

void choc(Ball* ball1, Ball* ball2, int* num_balls_list) {
    int nb_aleat = rand() % 100;
    if (nb_aleat % 3 != 0 && ball1->texture != ball2->texture) { /* 1 chance sur 3 égalité */
        if ( cineticEnergy(*ball1)*100 / (cineticEnergy(*ball1)+cineticEnergy(*ball2)) > nb_aleat ) {
            /* ball1 gagne */
            deleteBall(*ball2, num_balls_list);
            printf("Ball %d gagne !", ball1->num);
            ball1->colliding = -1;
        }
        else {
            /* ball2 gagne */
            deleteBall(*ball1, num_balls_list);
            printf("Ball %d gagne !", ball1->num);
            ball2->colliding = -1;
        }
    }
}

void drawBall(SDL_Renderer *renderer, Ball *ball)
{
    SDL_RenderCopy(renderer, ball->texture, NULL, &ball->rect);
}

void drawBalls(SDL_Renderer* renderer, Ball* balls, int maxBalls, int* num_balls_list) {
    for (int i = 0; i < maxBalls; i++)
        {
            if (num_balls_list[i] == 1) {
                drawBall(renderer, &balls[i]);
            }
        }
}

void cleanup(SDL_Window *window, SDL_Renderer *renderer, int nb_textures, SDL_Texture **textures, Ball *balls)
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

double cineticEnergy(Ball ball) {
    double speed = norm(ball.speed);
    return ball.mass*speed*speed;
}