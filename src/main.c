/* gcc clash_of_balls.c -o clash_of_balls.o -lSDL2 -lSDL2_ttf -lm*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../include/utils.h"

// Déclarations et définitions inchangées

const int ARENA_WIDTH = 480;   // Largeur de l'arène
const int ARENA_HEIGHT = 854;  // Hauteur de l'arène
const int SCREEN_WIDTH = 1400; // Largeur de la fenêtre
const int SCREEN_HEIGHT = 950; // Hauteur de la fenêtre
const int BALL_SIZE = 300;
const int FRAME_RATE = 100;
const float DT = 1000.0f / FRAME_RATE;

#define max_balls 100

typedef struct
{
    SDL_Rect rect;        /* Affichage de la boule */
    Vect speed;           /* Vitesse de la boule */
    Vect position;        /* Position de la boule */
    SDL_Texture *texture; /* Texture de la boule */
    double mass;
    int colliding;
    int num;
    int color; /* 0 pour blanc, 1 pour noir*/
} Ball;

typedef struct
{
    SDL_Texture *texture;
    int width;
    int height;
} Text;

typedef struct
{
    SDL_Texture *texture;
    SDL_Rect rect;
    bool is_selected;
    bool exist;
} Card;

int loadImagesFromDirectory(const char *directory, SDL_Renderer *renderer, SDL_Texture **textures, int max_textures);
void createBall(Ball *balls, int size, double mass, Vect position, Vect speed, SDL_Texture *texture, int color, Vect *nb_color_balls, int *num_balls_list);
void updateBall(Uint64 dt, Ball *ball, Ball *balls, int num_boule_actuelle, int *num_balls_list, Vect *nb_color_balls);
void updateBalls(Uint64 elapsedTime, Ball *balls, int *num_balls_list, Vect *nb_color_balls);
void drawBall(SDL_Renderer *renderer, Ball *ball);
void drawBalls(SDL_Renderer *renderer, Ball *balls, int *num_balls_list);
void cleanup(SDL_Window *window, SDL_Renderer *renderer, int nb_textures, SDL_Texture **textures, Ball *balls);
Vect posCenter(Ball *ball);
void calculer_collision(Ball *b1, Ball *b2, Vect *vA_f, Vect *vB_f);
bool canAppear(Vect position, int width, int height, Ball *balls, int *num_balls_list);
void deleteBall(Ball ball, int *num_balls_list, Vect *nb_color_balls);
double cineticEnergy(Ball ball);
void choc(Ball *ball1, Ball *ball2, int *num_balls_list, Vect *nb_color_balls);
void drawTransparentRectangle(SDL_Renderer *renderer, int x, int y, int width, int height, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha);
void aiPlay(int *num_balls_list, Ball *balls, SDL_Texture *texture, Vect *nb_color_balls);
Text createText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color);
void drawText(SDL_Renderer *renderer, Text *text, int x, int y);
void destroyText(Text *text);

int initSDL(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Erreur d'initialisation de SDL: %s\n", SDL_GetError());
        return -1;
    }
    if (TTF_Init() == -1)
    {
        printf("Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        return -1;
    }

    return 0;
}

int main(void)
{
    srand(time(NULL));

    srand(time(NULL));

    if (initSDL() < 0)
    {
        return -1;
    }

    int num_balls_list[max_balls];
    Vect nb_color_balls = (Vect){0, 0}; /* x: blanches, y: noire*/
    for (int i = 0; i < max_balls; i++)
    {
        num_balls_list[i] = 0;
    }

    // Configurer le viewport pour l'arène
    SDL_Rect arenaViewport = {
        .x = (SCREEN_WIDTH - ARENA_WIDTH) / 10,
        .y = (SCREEN_HEIGHT - ARENA_HEIGHT) / 2,
        .w = ARENA_WIDTH,
        .h = ARENA_HEIGHT};

    /* Créer un rectangle pour la bordure autour de l'arène (10 pixels de large) */
    SDL_Rect borderRect = {
        (SCREEN_WIDTH - ARENA_WIDTH) / 10 - 10,
        (SCREEN_HEIGHT - ARENA_HEIGHT) / 2 - 10,
        ARENA_WIDTH + 20,
        ARENA_HEIGHT + 20};

    int max_textures = 11;

    SDL_Texture **textures = malloc(sizeof(SDL_Texture *) * max_textures);

    // Création de la fenêtre
    SDL_Window *window = SDL_CreateWindow("Clash_of_balls", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!window || !renderer)
    {
        printf("Erreur lors de la création de la fenêtre: %s\n", SDL_GetError());
        cleanup(window, renderer, 0, textures, NULL);
        return -1;
    }

    // Charger les textures
    int nb_textures = loadImagesFromDirectory("./img", renderer, textures, max_textures);

    SDL_Rect t_ball = {0, 0, 60, 60}; // Boule transparente (lorsqu'elle est sélectionnée)
    bool draw_t_ball = false;

    Ball balls[max_balls];
    int nb_cards = 4;
    Card cards[nb_cards];

    for (int i = 0; i < nb_cards; i++)
    {
        cards[i].texture = textures[9];
        cards[i].rect.x = 900 - 375 * ((i % 3) % 2) + 125 * (i % 3); // tkt c'est pas compliqué (première au centre, deuxième à gauche, troisième à droite)
        cards[i].rect.y = 150 + 400 * (i / 3);                       // on change de ligne tous les 3
        cards[i].rect.w = 150;
        cards[i].rect.h = 250;
        cards[i].exist = true;
        cards[i].is_selected = false;
    }

    // Ajouter quelques boules initiales
    createBall(balls, 50, 10, (Vect){100.0, 100.0}, (Vect){600.0, 900.0}, textures[0], 0, &nb_color_balls, num_balls_list);
    createBall(balls, 60, 20, (Vect){300.0, 200.0}, (Vect){0.0, 0.0}, textures[3], 1, &nb_color_balls, num_balls_list);
    createBall(balls, 90, 800, (Vect){400.0, 300.0}, (Vect){-100.0, 40.0}, textures[0], 0, &nb_color_balls, num_balls_list);

    bool isRunning = true;
    SDL_Event event;
    Uint64 lastTime = SDL_GetTicks();
    Uint64 lastTime_cartes = SDL_GetTicks64();
    Uint64 frameTime = SDL_GetTicks();
    Uint64 elapsedTime = 0;
    int frameCount = 0;

    TTF_Font *font = TTF_OpenFont("Jersey_Sharp.ttf", 32);
    if (!font)
    {
        printf("Erreur lors du chargement de la police: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Color blackColor = {0, 0, 0, 0};
    Text titleText = createText(renderer, font, "Clash of Balls", blackColor);
    Text whiteWon = createText(renderer, font, "White wins !", blackColor);
    Text blackWon = createText(renderer, font, "Black wins !", blackColor);

    Vect mousePos = {0, 0};

    while (isRunning)
    {
        // Configurer le viewport pour l'arène
        SDL_Rect arenaViewport = {
            .x = (SCREEN_WIDTH - ARENA_WIDTH) / 10,
            .y = (SCREEN_HEIGHT - ARENA_HEIGHT) / 2,
            .w = ARENA_WIDTH,
            .h = ARENA_HEIGHT};

        while (SDL_PollEvent(&event))
        {
            int mouseX = event.button.x;
            int mouseY = event.button.y;
            if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
            {
                isRunning = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                for (int i = 0; i < nb_cards; i++)
                {
                    if (cards[i].exist && isInBox((Vect){mouseX, mouseY}, cards[i].rect))
                    {
                        cards[i].is_selected = true;
                        cards[i].texture = textures[8];

                        for (int j = 0; j < nb_cards; j++)
                        {
                            if (j != i)
                            {
                                cards[j].is_selected = false;
                                if (cards[j].exist)
                                {
                                    cards[j].texture = textures[9];
                                }
                            }
                        }
                    }
                }

                for (int i = 0; i < nb_cards; i++)
                {
                    if (cards[i].is_selected)
                    {
                        int mouseXInArena = event.button.x - (SCREEN_WIDTH - ARENA_WIDTH) / 10;
                        int mouseYInArena = event.button.y - (SCREEN_HEIGHT - ARENA_HEIGHT) / 2;
                        draw_t_ball = true;

                        if (event.button.button == SDL_BUTTON_LEFT &&
                            canAppear((Vect){mouseXInArena - 30, mouseYInArena - 30}, 60, 60, balls, num_balls_list))
                        {
                            createBall(balls, 60, 20,
                                       (Vect){mouseXInArena - 30, mouseYInArena - 30},
                                       (Vect){100.0, -50.0}, textures[3], 1, &nb_color_balls, num_balls_list);
                            cards[i].is_selected = false;
                            cards[i].exist = false;
                            cards[i].texture = textures[6];
                            draw_t_ball = false;
                        }
                    }
                }
                printf("Clic gauche détecté en (%d, %d)\n", event.button.x, event.button.y);
            }
            else if (event.type == SDL_MOUSEMOTION)
            {

                mousePos.x = event.motion.x;
                mousePos.y = event.motion.y;

                int mouseXInArena = mousePos.x - (SCREEN_WIDTH - ARENA_WIDTH) / 10;
                int mouseYInArena = mousePos.y - (SCREEN_HEIGHT - ARENA_HEIGHT) / 2;

                t_ball.x = mouseXInArena - 30;
                t_ball.y = mouseYInArena - 30;
            }
        }

        SDL_RenderPresent(renderer);

        // Gestion du framerate
        Uint64 currentTime = SDL_GetTicks64();
        elapsedTime = currentTime - frameTime;
        frameTime = SDL_GetTicks64();
        frameCount++;

        if (currentTime - lastTime_cartes >= 5000 && (!cards[3].exist))
        {
            lastTime_cartes = currentTime;
            cards[3].texture = textures[9];
            cards[3].exist = true;
        }

        if (currentTime - lastTime >= 2000)
        {
            printf("FPS: %d\n", frameCount / 2);
            frameCount = 0;
            lastTime = currentTime;

            // Probabilité d'apparition d'une boule blanche (par rapport au nombre de boules blanches et noires)

            double proba = sqrt((double)(nb_color_balls.y) / (double)(nb_color_balls.x + nb_color_balls.y));

            if (rand() % 100 < proba * 100)
            {
                aiPlay(num_balls_list, balls, textures[0], &nb_color_balls);
            }
            printf("\nboules blanches: %d; boules noires: %d\n", (int)(nb_color_balls.x), (int)(nb_color_balls.y));
        }

        

        // Mise à jour des boules
        updateBalls(elapsedTime, balls, num_balls_list, &nb_color_balls);

        // Rendu
        SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255); // Fond noir pour toute la fenêtre
        SDL_RenderClear(renderer);

        // Affichage de la bordure autour de l'arène

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Blanc
        SDL_RenderDrawRect(renderer, &borderRect);

        SDL_RenderSetViewport(renderer, &arenaViewport);

        // Dessiner le fond de l'arène
        SDL_SetRenderDrawColor(renderer, 217, 217, 217, 255);
        SDL_RenderFillRect(renderer, NULL);

        // Dessiner les boules
        drawBalls(renderer, balls, num_balls_list);

        // Dessiner la boule transparente

        if (draw_t_ball)
        {
            SDL_RenderCopy(renderer, textures[10], NULL, &t_ball);
        }

        // Réinitialiser le viewport pour dessiner des éléments extérieurs
        SDL_RenderSetViewport(renderer, NULL);

        for (int i = 0; i < nb_cards; i++)
        {
            SDL_RenderCopy(renderer, cards[i].texture, NULL, &cards[i].rect);
        }

        drawText(renderer, &titleText, 850, 50);

        if ((int)(nb_color_balls.x) == 0)
        {
            drawTransparentRectangle(renderer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 217, 217, 217, 192);
            drawText(renderer, &blackWon, (SCREEN_WIDTH / 2) - (blackWon.width / 2), (SCREEN_HEIGHT / 2) - (blackWon.height / 2));
        }
        else if ((int)(nb_color_balls.y == 0))
        {
            drawTransparentRectangle(renderer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 217, 217, 217, 192);
            drawText(renderer, &whiteWon, (SCREEN_WIDTH / 2) - (whiteWon.width / 2), (SCREEN_HEIGHT / 2) - (whiteWon.height / 2));
        }

        SDL_RenderPresent(renderer);
    }

    destroyText(&titleText);
    TTF_CloseFont(font);

    cleanup(window, renderer, nb_textures, textures, balls);

    printf("Quitting !\n");

    return 0;
}

Text createText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color)
{
    Text newText;
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, color);
    if (!textSurface)
    {
        fprintf(stderr, "Erreur lors du rendu du texte: %s\n", TTF_GetError());
        newText.texture = NULL;
        return newText;
    }
    newText.texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!newText.texture)
    {
        fprintf(stderr, "Erreur lors de la création de la texture pour le texte: %s\n", SDL_GetError());
    }
    newText.width = textSurface->w;
    newText.height = textSurface->h;
    SDL_FreeSurface(textSurface);
    return newText;
}

void drawText(SDL_Renderer *renderer, Text *text, int x, int y)
{
    if (text->texture)
    {
        SDL_Rect destRect = {x, y, text->width, text->height};
        SDL_RenderCopy(renderer, text->texture, NULL, &destRect);
    }
}
void destroyText(Text *text)
{
    if (text->texture)
    {
        SDL_DestroyTexture(text->texture);
        text->texture = NULL;
    }
}

// Fonction pour dessiner un rectangle transparent
void drawTransparentRectangle(SDL_Renderer *renderer, int x, int y, int width, int height,
                              Uint8 r, Uint8 g, Uint8 b, Uint8 alpha)
{
    // Activer le blending pour permettre la transparence
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Définir la couleur (avec alpha pour la transparence)
    SDL_SetRenderDrawColor(renderer, r, g, b, alpha);

    // Définir les dimensions du rectangle
    SDL_Rect rect = {x, y, width, height};

    // Dessiner le rectangle plein
    SDL_RenderFillRect(renderer, &rect);
}

void createBall(Ball *balls, int size, double mass, Vect position, Vect speed, SDL_Texture *texture, int color, Vect *nb_color_balls, int *num_balls_list)
{
    bool pas_deja_creee = true;
    for (int i = 0; i < max_balls; i++)
    {
        if (num_balls_list[i] == 0 && pas_deja_creee)
        {
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
            balls[i].color = color;

            if (color == 0)
            {
                nb_color_balls->x += 1;
            }
            else
            {
                nb_color_balls->y += 1;
            }

            /* Initialiser la vitesse de la boule avec des valeurs aléatoires */
            balls[i].speed = speed;

            pas_deja_creee = false;
        }
    }
}

void aiPlay(int *num_balls_list, Ball *balls, SDL_Texture *texture, Vect *nb_color_balls)
{
    int max_speed = 500;
    Vect position = (Vect){100, rand() % ARENA_HEIGHT};
    Vect vitesse = (Vect){rand() % max_speed, -(rand() % max_speed)};
    if (canAppear(position, 40, 40, balls, num_balls_list))
    {
        createBall(balls, 60, 60, position, vitesse, texture, 0, nb_color_balls, num_balls_list);
    }
}

void deleteBall(Ball ball, int *num_balls_list, Vect *nb_color_balls)
{
    num_balls_list[ball.num] = 0;
    if (ball.color == 0)
    {
        nb_color_balls->x -= 1;
    }
    else
    {
        nb_color_balls->y -= 1;
    }
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
    Vect p1 = posCenter(b1);
    Vect p2 = posCenter(b2);
    Vect delta_p = {p1.x - p2.x, p1.y - p2.y};

    double d = norm(delta_p);

    delta_p.x /= d; // Normalisation
    delta_p.y /= d;

    double p = 2 * (dot(b1->speed, delta_p) - dot(b2->speed, delta_p)) / (b1->mass + b2->mass);

    vA_f->x = b1->speed.x - p * b2->mass * delta_p.x;
    vA_f->y = b1->speed.y - p * b2->mass * delta_p.y;

    vB_f->x = b2->speed.x + p * b1->mass * delta_p.x;
    vB_f->y = b2->speed.y + p * b1->mass * delta_p.y;
}

bool canAppear(Vect position, int width, int height, Ball *balls, int *num_balls_list)
{
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
        if (num_balls_list[i] == 0)
        {
            continue;
        }
        Ball *other = &balls[i];

        double collideDistance = ((double)width + (double)other->rect.w) / 2;
        Vect m_other = posCenter(other);
        Vect delta = {position.x - m_other.x, position.y - m_other.y};
        double dist = norm(delta);

        if (dist <= collideDistance)
        {
            return false;
        }
    }
    return true;
}

void updateBall(Uint64 dt, Ball *ball, Ball *balls, int num_boule_actuelle, int *num_balls_list, Vect *nb_color_balls)
{
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

    for (int i = num_boule_actuelle + 1; i < max_balls; i++)
    {
        if (num_balls_list[i] == 1)
        {
            Ball *other = &balls[i];

            double collideDistance = ((double)ball->rect.w + (double)other->rect.w) / 2;

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
                choc(ball, other, num_balls_list, nb_color_balls);
            }

            if (dist > collideDistance && (ball->colliding == i && other->colliding == num_boule_actuelle))
            {
                ball->colliding = -1;
                other->colliding = -1;
            }
        }
    }

    ball->position.x += ball->speed.x * dt / 1000;
    ball->position.y += ball->speed.y * dt / 1000;
}

void updateBalls(Uint64 elapsedTime, Ball *balls, int *num_balls_list, Vect *nb_color_balls)
{
    for (int i = 0; i < max_balls; i++)
    {
        if (num_balls_list[i] == 1)
        {
            updateBall(elapsedTime, &balls[i], balls, i, num_balls_list, nb_color_balls);
        }
    }
}

void choc(Ball *ball1, Ball *ball2, int *num_balls_list, Vect *nb_color_balls)
{
    int nb_aleat = rand() % 100;
    if (nb_aleat % 3 != 0 && ball1->texture != ball2->texture)
    { /* 1 chance sur 3 égalité */
        if (cineticEnergy(*ball1) * 100 / (cineticEnergy(*ball1) + cineticEnergy(*ball2)) > nb_aleat)
        {
            /* ball1 gagne */
            deleteBall(*ball2, num_balls_list, nb_color_balls);
            ball1->colliding = -1;
        }
        else
        {
            /* ball2 gagne */
            deleteBall(*ball1, num_balls_list, nb_color_balls);
            ball2->colliding = -1;
        }
    }
}

void drawBall(SDL_Renderer *renderer, Ball *ball)
{
    SDL_RenderCopy(renderer, ball->texture, NULL, &ball->rect);
}

void drawBalls(SDL_Renderer *renderer, Ball *balls, int *num_balls_list)
{
    for (int i = 0; i < max_balls; i++)
    {
        if (num_balls_list[i] == 1)
        {
            drawBall(renderer, &balls[i]);
        }
    }
}

void cleanup(SDL_Window *window, SDL_Renderer *renderer, int nb_textures, SDL_Texture **textures, Ball *balls)
{
    int i;
    if (textures != NULL)
    {
        for (i = 0; i < nb_textures; i++)
        {
            if (textures[i] != NULL)
            {
                SDL_DestroyTexture(textures[i]); // Correct: utiliser SDL_DestroyTexture
            }
        }
        free(textures);
    }
    if (renderer != NULL)
    {
        SDL_DestroyRenderer(renderer);
    }
    if (window != NULL)
    {
        SDL_DestroyWindow(window);
    }
    TTF_Quit();
    SDL_Quit();
}

double cineticEnergy(Ball ball)
{
    double speed = norm(ball.speed);
    return ball.mass * speed * speed;
}

int loadImagesFromDirectory(const char *directory, SDL_Renderer *renderer, SDL_Texture **textures, int max_textures)
{
    DIR *dir;
    struct dirent *ent;
    struct stat *st = malloc(sizeof(struct stat));
    int texture_count = 0;

    printf("Chargement des images du répertoire: %s\n", directory);

    if ((dir = opendir(directory)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL && texture_count < max_textures)
        {

            char filepath[512];
            sprintf(filepath, "%s/%s", directory, ent->d_name);

            if (stat(filepath, st) == -1)
            {
                fprintf(stderr, "Erreur lors de la récupération des informations sur le fichier: %s\n", ent->d_name);
                continue;
            }

            if (S_ISREG(st->st_mode))
            {

                SDL_Surface *surf = SDL_LoadBMP(filepath);
                if (!surf)
                {
                    fprintf(stderr, "Erreur de chargement de l'image: %s\n", SDL_GetError());
                    continue;
                }

                SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_FreeSurface(surf);
                if (imageTexture)
                {
                    /* L'indice de texture est le premier chiffre du nom du fichier (peut prendre plusieurs chiffres) */

                    int texture_index = partialAtoi(ent->d_name);

                    if (texture_index < 0 || texture_index >= max_textures)
                    {
                        fprintf(stderr, "Indice de texture invalide: %d\n", texture_index);
                        continue;
                    }

                    textures[texture_index] = imageTexture;

                    texture_count++;
                }
                else
                {
                    fprintf(stderr, "Erreur lors de la création de la texture: %s\n", SDL_GetError());
                }
                printf("Chargement de l'image: %s\n", filepath);
            }
        }
        closedir(dir);
    }
    else
    {
        fprintf(stderr, "Erreur lors de l'ouverture du répertoire: %s\n", directory);
        return -1;
    }

    free(st);

    return texture_count;
}
