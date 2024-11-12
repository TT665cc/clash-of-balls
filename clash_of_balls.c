#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#define PI 3.14159265358979323846

const int BALL_SIZE = 50;
const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 854;
const int FRAME_RATE = 64;
const float DT = 1000.0f / FRAME_RATE;

typedef struct {
    SDL_Rect rect; /* Position et taille de la boule */
    int speedX, speedY; /* Vitesse de la boule */
    SDL_Texture* texture; /* Texture de la boule */
} Ball;

typedef struct {
    int x, y; /* Coordonnées pour les calculs de vecteurs */
} Vect;

/* Prototypes des fonctions */
int initSDL();
void createBall(Ball* ball, int size, Vect positions, SDL_Texture* texture);
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
    int maxBalls = 50;
    Ball* balls = (Ball*)malloc(maxBalls * sizeof(Ball));
    Vect pos_depart_rouge = {100, 100};
    Vect pos_depart_bleu = {200, 200};
    createBall(&balls[0], 50, pos_depart_rouge, imageTexture_bouleRouge);
    createBall(&balls[1], 50, pos_depart_bleu, imageTexture_bouleBleu);

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

void createBall(Ball* ball, int size, Vect positions, SDL_Texture* texture) {
    /* Initialiser la position de la boule avec les coordonnées fournies */
    ball->rect.w = size;
    ball->rect.h = size;
    ball->rect.x = positions.x;
    ball->rect.y = positions.y;
    ball->texture = texture;

    /* Initialiser la vitesse de la boule avec des valeurs aléatoires */
    ball->speedX = (rand() % 2 == 0 ? 1 : -1) * (rand() % 7 + 1);
    ball->speedY = (rand() % 2 == 0 ? 1 : -1) * (rand() % 7 + 1);
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

void updateBall(Ball* ball, Ball* balls, int num_boule_actuelle, int nb_balls) {
    int diametre = ball->rect.w;
    ball->rect.x += ball->speedX;
    ball->rect.y += ball->speedY;

    if (ball->rect.x <= 0 || ball->rect.x + ball->rect.w >= SCREEN_WIDTH) {
        ball->speedX = -ball->speedX;
    }
    if (ball->rect.y <= 0 || ball->rect.y + ball->rect.h >= SCREEN_HEIGHT) {
        ball->speedY = -ball->speedY;
    }

    for (int i = 0; i < nb_balls; i++) {
        if (i != num_boule_actuelle) {
            Ball* other = &balls[i];
            double dist = distance(ball, other);

            if (dist <= diametre) {
                int tempSpeedX = ball->speedX;
                int tempSpeedY = ball->speedY;

                ball->speedX = other->speedX;
                ball->speedY = other->speedY;
                other->speedX = tempSpeedX;
                other->speedY = tempSpeedY;
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
