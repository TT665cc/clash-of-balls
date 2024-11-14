/* gcc clash_of_balls.c -o clash_of_balls -lSDL2 -lm */

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#define PI 3.14159265358979323846

const int BALL_SIZE = 300;
const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 854;
const int FRAME_RATE = 64;
const float DT = 1000.0f / FRAME_RATE;

typedef struct {
    int x, y; /* Coordonnées pour les calculs de vecteurs */
} Vect;

typedef struct {
    SDL_Rect rect; /* Position et taille de la boule */
    Vect speed; /* Vitesse de la boule */
    SDL_Texture* texture; /* Texture de la boule */
} Ball;

/* Prototypes des fonctions */
int initSDL();
void createBall(Ball* ball, int size, Vect positions, Vect speed, SDL_Texture* texture);
void updateBall(Ball* ball, Ball* balls, int num_boule_actuelle, int nb_balls);
void drawBall(SDL_Renderer* renderer, Ball* ball);
void cleanup(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture1, SDL_Texture* texture2, Ball* balls, int ballCount);

int main(int argc, char* argv[]) {
    srand(time(NULL));

    if (initSDL() < 0) {
        return -1;
    }

    /* Création de la fenêtre et du renderer */
    SDL_Window* window = SDL_CreateWindow("Clash_of_balls", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    /* Charger l'image pour les boules */
    SDL_Surface* imageSurfaceRouge = SDL_LoadBMP("boule_rouge.bmp");
    SDL_Surface* imageSurfaceBleu = SDL_LoadBMP("boule_bleu.bmp");
    if (!imageSurfaceRouge) {
        printf("Erreur de chargement de l'image: %s\n", SDL_GetError());
        cleanup(window, renderer, NULL, NULL, NULL, 0);
        return -1;
    }
    if (!imageSurfaceBleu) {
        printf("Erreur de chargement de l'image: %s\n", SDL_GetError());
        cleanup(window, renderer, NULL, NULL, NULL, 0);
        return -1;
    }
    SDL_Texture* imageTexture_bouleRouge = SDL_CreateTextureFromSurface(renderer, imageSurfaceRouge);
    SDL_Texture* imageTexture_bouleBleu = SDL_CreateTextureFromSurface(renderer, imageSurfaceBleu);
    SDL_FreeSurface(imageSurfaceRouge);
    SDL_FreeSurface(imageSurfaceBleu);

    /* Allocation et génération des boules */
    int maxBalls = 3;
    Ball* balls = (Ball*)malloc(maxBalls * sizeof(Ball));
    Vect pos_depart_rouge = {100, 100};
    Vect pos_depart_bleu = {200, 200};
    Vect v_depart_rouge = {5, -4};
    Vect v_depart_bleu = {6, 6};
    createBall(&balls[0], 50, pos_depart_bleu, v_depart_bleu, imageTexture_bouleBleu);
    createBall(&balls[1], 50, pos_depart_rouge, v_depart_rouge, imageTexture_bouleRouge);
    createBall(&balls[2], 50, (Vect){300, 150}, v_depart_rouge, imageTexture_bouleRouge);

    /* Boucle principale */
    bool isRunning = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }

        /* Mise à jour des boules */
        for (int i = 0; i < maxBalls; i++) {
            updateBall(&balls[i], balls, i, maxBalls);
        }

        /* Rafraîchissement de l'écran */
        SDL_SetRenderDrawColor(renderer, 0, 0, 100, 255);
        SDL_RenderClear(renderer);

        /* Dessiner les boules */
        for (int i = 0; i < maxBalls; i++) {
            drawBall(renderer, &balls[i]);
        }

        SDL_RenderPresent(renderer);

        /* Contrôle du framerate */
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsedTime = currentTime - lastTime;
        if (elapsedTime < DT) {
            SDL_Delay((int)(DT - elapsedTime));
        }
        lastTime = SDL_GetTicks();
    }

    /* Libération des ressources */
    cleanup(window, renderer, imageTexture_bouleRouge, imageTexture_bouleBleu, balls, maxBalls);

    return 0;
}

int initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur d'initialisation de SDL: %s\n", SDL_GetError());
        return -1;
    }
    return 0;
}

void createBall(Ball* ball, int size, Vect positions, Vect speed, SDL_Texture* texture) {
    /* Initialiser la position de la boule avec les coordonnées fournies */
    ball->rect.w = size;
    ball->rect.h = size;
    ball->rect.x = positions.x;
    ball->rect.y = positions.y;
    ball->texture = texture;

    /* Initialiser la vitesse de la boule avec des valeurs aléatoires */
    ball->speed = speed;
}

Vect posCenter(Ball* ball) {
    Vect res;
    res.x = ball->rect.x + ball->rect.w / 2;
    res.y = ball->rect.y + ball->rect.h / 2;
    return res;
}

double distance(Ball* ball1, Ball* ball2) {
    Vect m1 = posCenter(ball1);
    Vect m2 = posCenter(ball2);
    return sqrt((m1.x - m2.x) * (m1.x - m2.x) + (m1.y - m2.y) * (m1.y - m2.y));
}

void calculer_collision(Vect vA_i, Vect vB_i, double mA, double mB, Vect *vA_f, Vect *vB_f) {
    // Calcul de la différence de position et de vitesse
    Vect delta_v = {vA_i.x - vB_i.x, vA_i.y - vB_i.y};
    double dist2 = delta_v.x * delta_v.x + delta_v.y * delta_v.y;

    // Calcul du produit scalaire entre delta_v et delta_r
    double scalar = 2.0 * mB / (mA + mB) * ((delta_v.x * (vA_i.x - vB_i.x) + delta_v.y * (vA_i.y - vB_i.y)) / dist2);

    // Calcul des nouvelles vitesses
    vA_f->x = vA_i.x - scalar * (vA_i.x - vB_i.x);
    vA_f->y = vA_i.y - scalar * (vA_i.y - vB_i.y);
    vB_f->x = vB_i.x + scalar * (vA_i.x - vB_i.x);
    vB_f->y = vB_i.y + scalar * (vA_i.y - vB_i.y);
}

void updateBall(Ball* ball, Ball* balls, int num_boule_actuelle, int nb_balls) {
    int diametre = ball->rect.w;
    ball->rect.x += ball->speed.x;
    ball->rect.y += ball->speed.y;

    if (ball->rect.x <= 0) {
        ball->speed.x = -ball->speed.x;
        ball->rect.x += 5;
    }
    if (ball->rect.x + ball->rect.w >= SCREEN_WIDTH) {
        ball->speed.x = -ball->speed.x;
        ball->rect.x -= 5;
    }
    if (ball->rect.y <= 0) {
        ball->speed.y = -ball->speed.y;
        ball->rect.y += 5;
    }
    if (ball->rect.y + ball->rect.h >= SCREEN_HEIGHT)
    {
        ball->speed.y = -ball->speed.y;
        ball->rect.y -= 5;
    }
    

    for (int i = 0; i < nb_balls; i++) {
        if (i != num_boule_actuelle) {
            Ball* other = &balls[i];
            double dist = distance(ball, other);

            if (dist <= diametre) {
                Vect vA_f;
                Vect vB_f;
                calculer_collision(ball->speed, other->speed, 1, 1, &vA_f, &vB_f);
                ball->speed = vA_f;
                other->speed = vB_f;
            }
        }
    }
}

void drawBall(SDL_Renderer* renderer, Ball* ball) {
    SDL_RenderCopy(renderer, ball->texture, NULL, &ball->rect);
}

void cleanup(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture1, SDL_Texture* texture2, Ball* balls, int ballCount) {
    if (balls) {
        free(balls);
    }
    if (texture1) {
        SDL_DestroyTexture(texture1);
    }
    if (texture2) {
        SDL_DestroyTexture(texture2);
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}
