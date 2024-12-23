/* gcc -o clash_of_balls.o src/*.c -lSDL2 -lSDL2_ttf -lm */

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
#include "../include/texture.h"

// Déclarations et définitions inchangées

const int ARENA_WIDTH = 480;   // Largeur de l'arène
const int ARENA_HEIGHT = 854;  // Hauteur de l'arène
const int SCREEN_WIDTH = 1400; // Largeur de la fenêtre
const int SCREEN_HEIGHT = 950; // Hauteur de la fenêtre

#define max_objects 100

typedef struct
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
    int type; /* Balle : 0, Mur : 1 */
    bool invincible;
} Object;


typedef struct
{
    SDL_Texture *texture;
    int width;
    int height;
    TTF_Font *font;
    SDL_Color color;
} Text;

typedef struct
{
    SDL_Texture *texture;
    SDL_Rect rect;
    bool is_selected;
    bool exist;
    int type;
} Card;

int loadImagesFromDirectory(const char *directory, SDL_Renderer *renderer, SDL_Texture **textures, int max_textures);
void createBall(SDL_Renderer *renderer, Object *objects, int size, double mass, Vect position, Vect speed, int color, Vect *nb_color_balls, int *num_balls_list, bool invincible);
void updateObject(Uint64 dt, Object *object, Object *objects, int num_objet_actuel, int *num_objects_list, Vect *nb_color_balls);
void updateObjects(Uint64 elapsedTime, Object *objects, int *num_objects_list, Vect *nb_color_balls);
void drawObject(SDL_Renderer *renderer, Object *object);
void drawObjects(SDL_Renderer *renderer, Object *objects, int *num_object_list);
void cleanup(SDL_Window *window, SDL_Renderer *renderer, int nb_textures, SDL_Texture **textures);
void ballCollision(Object *o1, Object *o2);
void wallCollision(Object *ball, Object *wall);
bool canAppear(Vect position, int width, int height, Object *objects, int *num_objects_list, int type);
void deleteObject(Object object, int *num_objects_list, Vect *nb_color_balls);
void deleteAllObjects(Object *objects, int *num_objects_list, Vect *nb_color_balls);
double cineticEnergy(Object object);
void choc(Object *ball1, Object *ball2, int *num_objects_list, Vect *nb_color_balls);
void drawTransparentRectangle(SDL_Renderer *renderer, int x, int y, int width, int height, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha);
void aiPlay(SDL_Renderer *renderer, int *num_objects_list, Object *objects, Vect *nb_color_balls);
Text createText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color);
void drawText(SDL_Renderer *renderer, Text *text, SDL_Rect destRect);
void destroyText(Text *text);
int mainGame(SDL_Texture **textures, SDL_Renderer *renderer, SDL_Window *window, Text titleText);
bool welcomeScreen(SDL_Texture **textures, SDL_Renderer *renderer, SDL_Window *window, Text titleText);
bool replayScreen(SDL_Texture **textures, SDL_Renderer *renderer, SDL_Window *window, Text titleText, int winner);


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

    

    int max_textures = 5;

    SDL_Texture **textures = malloc(sizeof(SDL_Texture *) * max_textures);

    // Création de la fenêtre
    SDL_Window *window = SDL_CreateWindow("Clash_of_balls", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!window || !renderer)
    {
        printf("Erreur lors de la création de la fenêtre: %s\n", SDL_GetError());
        cleanup(window, renderer, 0, textures);
        return -1;
    }

    // Charger les textures
    int nb_textures = loadImagesFromDirectory("./img", renderer, textures, max_textures);


    TTF_Font *font = TTF_OpenFont("Jersey_Sharp.ttf", 32);
    if (!font)
    {
        printf("Erreur lors du chargement de la police: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Color blackColor = {0, 0, 0, 0};
    Text titleText = createText(renderer, font, "Clash of Balls", blackColor);
    

    bool start = welcomeScreen(textures, renderer, window, titleText);
    bool winner;

    while (start)
    {
        winner = mainGame(textures, renderer, window, titleText);
        start = replayScreen(textures, renderer, window, titleText, winner);
    }

    

    destroyText(&titleText);
    TTF_CloseFont(font);

    cleanup(window, renderer, nb_textures, textures);

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
    newText.font = font;
    newText.color = color;
    SDL_FreeSurface(textSurface);
    return newText;
}

void drawText(SDL_Renderer *renderer, Text *text, SDL_Rect destRect)
{
    if (text->texture)
    {
        SDL_Rect destRect2 = {destRect.x - destRect.w / 2, destRect.y - destRect2.h / 2, destRect.w, destRect.h};
        SDL_RenderCopy(renderer, text->texture, NULL, &destRect2);
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

void createBall(SDL_Renderer *renderer, Object *objects, int size, double mass, Vect position, Vect speed, int color, Vect *nb_color_balls, int *num_objects_list, bool invincible)
{
    for (int i = 0; i < max_objects; i++)
    {
        if (num_objects_list[i] == 0)
        {
            num_objects_list[i] = 1;
            objects[i].rect.w = size;
            objects[i].rect.h = size;
            objects[i].rect.x = floor(position.x - size / 2);
            objects[i].rect.y = floor(position.y - size / 2);
            objects[i].texture = createCircleTexture(renderer, size / 2, color);
            objects[i].position = position;
            objects[i].mass = mass;
            objects[i].colliding = -1;
            objects[i].num = i;
            objects[i].color = color;
            objects[i].type = 0;
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

void createWall(Object *objects, int width, int height, Vect position, SDL_Texture *texture, int *num_objects_list)
{
    for (int i = 0; i < max_objects; i++)
    {
        if (num_objects_list[i] == 0)
        {
            num_objects_list[i] = 1;
            objects[i].rect.w = width;
            objects[i].rect.h = height;
            objects[i].rect.x = floor(position.x - width / 2);
            objects[i].rect.y = floor(position.y - height / 2);
            objects[i].texture = texture;
            objects[i].position = position;
            objects[i].mass = 0;
            objects[i].colliding = -1;
            objects[i].num = i;
            objects[i].color = 0;
            objects[i].type = 1;
            objects[i].speed = (Vect){0, 0};
            break;
        }
    }
}

void aiPlay(SDL_Renderer *renderer, int *num_objects_list, Object *balls, Vect *nb_color_balls)
{
    int max_speed = 500;
    Vect position = (Vect){100, rand() % ARENA_HEIGHT};
    Vect vitesse = (Vect){rand() % max_speed, -(rand() % max_speed)};
    if (canAppear(position, 40, 40, balls, num_objects_list, 0))
    {
        createBall(renderer, balls, 60, 60, position, vitesse, 0, nb_color_balls, num_objects_list, false);
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

Vect posCenter(Object *object)
{
    Vect res;
    res.x = object->position.x + object->rect.w / 2;
    res.y = object->position.y + object->rect.h / 2;
    return res;
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

bool canAppear(Vect position, int width, int height, Object *objects, int *num_objects_list, int type)
{
    if ((position.x - width / 2 <= 0) || (position.x + width / 2 >= ARENA_WIDTH))
    {
        return false;
    }
    if ((position.y - height / 2 <= 0) || (position.y + height / 2 >= ARENA_HEIGHT))
    {
        return false;
    }

    SDL_FRect rect = { position.x, position.y, width, height };

    for (int i = 0; i < max_objects; i++)
    {
        if (num_objects_list[i] == 0)
        {
            continue;
        }
        Object *other = &objects[i];

        SDL_FRect rect2 = { other->position.x, other->position.y, other->rect.w, other->rect.h };

        if (__collides(rect, rect2, type | other->type<<1))
            return false;
    }
    return true;
}

double collides(Object *o1, Object *o2)
{
    SDL_FRect rect1 = {o1->position.x, o1->position.y, o1->rect.w, o1->rect.h};
    SDL_FRect rect2 = {o2->position.x, o2->position.y, o2->rect.w, o2->rect.h};

    return __collides(rect1, rect2, o1->type | o2->type<<1);
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

void updateObject(Uint64 dt, Object *object, Object *objects, int num_boule_actuelle, int *num_objects_list, Vect *nb_color_balls)
{

    if ((object->rect.x <= 0 && object->speed.x < 0) || (object->rect.x + object->rect.w >= ARENA_WIDTH && object->speed.x > 0))
    {
        object->speed.x *= -1;
    }
    if ((object->rect.y <= 0 && object->speed.y < 0) || (object->rect.y + object->rect.h >= ARENA_HEIGHT && object->speed.y > 0))
    {
        object->speed.y *= -1;
    }

    for (int i = num_boule_actuelle + 1; i < max_objects; i++)
    {
        if (num_objects_list[i] == 1)
        {
            Object *other = &objects[i];

            if(collides(object, other) && object->colliding == -1 && other->colliding == -1)
            {
                handleCollision(object, other);
                object->colliding = i;
                other->colliding = num_boule_actuelle;
                choc(object, other, num_objects_list, nb_color_balls);
            }

            if (!collides(object, other) && (object->colliding == i && other->colliding == num_boule_actuelle))
            {
                object->colliding = -1;
                other->colliding = -1;
            }
        }
    }
}

void updateObjects(Uint64 elapsedTime, Object *objects, int *num_objects_list, Vect *nb_color_balls)
{
    for (int i = 0; i < max_objects; i++)
    {
        if (num_objects_list[i] == 1)
        {
            updateObject(elapsedTime, &objects[i], objects, i, num_objects_list, nb_color_balls);
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

void drawObject(SDL_Renderer *renderer, Object *object)
{
    SDL_RenderCopy(renderer, object->texture, NULL, &object->rect);
}

void drawObjects(SDL_Renderer *renderer, Object *objects, int *num_objects_list)
{
    for (int i = 0; i < max_objects; i++)
    {
        if (num_objects_list[i] == 1)
        {
            drawObject(renderer, &objects[i]);
        }
    }
}

void cleanup(SDL_Window *window, SDL_Renderer *renderer, int nb_textures, SDL_Texture **textures)
{
    int i;
    if (textures != NULL)
    {
        for (i = 0; i < nb_textures; i++)
        {
            if (textures[i] != NULL)
            {
                SDL_DestroyTexture(textures[i]);
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

double cineticEnergy(Object object)
{
    double speed = norm(object.speed);
    return object.mass * speed * speed;
}

int mainGame(SDL_Texture **textures, SDL_Renderer *renderer, SDL_Window *window, Text titleText) {
    int num_objects_list[max_objects];
    Vect nb_color_balls = (Vect){0, 0}; /* x: blanches, y: noire*/
    for (int i = 0; i < max_objects; i++)
    {
        num_objects_list[i] = 0;
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

    SDL_Rect t_ball = {0, 0, 60, 60}; // Boule transparente (lorsqu'elle est sélectionnée)
    bool draw_t_ball = false;

    Object objects[max_objects];

    int nb_cards = 4;
    Card cards[nb_cards];

    for (int i = 0; i < nb_cards; i++)
    {
        int k = i % 3;
        cards[i].texture = textures[3];
        cards[i].rect.x = 900 - 375 * (k % 2) + 125 * k; // tkt c'est pas compliqué (première au centre, deuxième à gauche, troisième à droite)
        cards[i].rect.y = 150 + 400 * (i / 3);                       // on change de ligne tous les 3
        cards[i].rect.w = 150;
        cards[i].rect.h = 250;
        cards[i].exist = true;
        cards[i].is_selected = false;
    }

    // Ajouter quelques boules initiales
    //createBall(balls, 50, 10, (Vect){100.0, 100.0}, (Vect){600.0, 900.0}, textures[0], 0, &nb_color_balls, num_objects_list);
    createBall(renderer, objects, 60, 20, (Vect){300.0, 200.0}, (Vect){0.0, 0.0}, 1, &nb_color_balls, num_objects_list, true);
    createBall(renderer, objects, 90, 800, (Vect){400.0, 300.0}, (Vect){-100.0, 40.0}, 0, &nb_color_balls, num_objects_list, true);

    // Ajouter quelques murs
    createWall(objects, 180, 20, (Vect){ARENA_WIDTH / 2, ARENA_HEIGHT / 2}, textures[3], num_objects_list);

    Vect mousePos = {0, 0};

    // Boucle principale

    bool isRunning = true;
    SDL_Event event;
    Uint64 lastTime = SDL_GetTicks();
    Uint64 lastTime_cartes = SDL_GetTicks64();
    Uint64 frameTime = SDL_GetTicks();
    Uint64 elapsedTime = 0;
    int frameCount = 0;

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
                return -1;
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
                        cards[i].texture = textures[2];

                        for (int j = 0; j < nb_cards; j++)
                        {
                            if (j != i)
                            {
                                cards[j].is_selected = false;
                                if (cards[j].exist)
                                {
                                    cards[j].texture = textures[3];
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
                            canAppear((Vect){mouseXInArena - 30, mouseYInArena - 30}, 60, 60, objects, num_objects_list, cards[i].type))
                        {
                            /* Créer une boule avec la position de la souris et une vitesse aléatoire */
                            Vect speed = (Vect){rand() % 1000 - 500, rand() % 1000 - 500};

                            createBall(renderer, objects, 60, 20,
                                       (Vect){mouseXInArena, mouseYInArena},
                                       speed, 1, &nb_color_balls, num_objects_list, false);
                            cards[i].is_selected = false;
                            cards[i].exist = false;
                            cards[i].texture = textures[0];
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
            cards[3].texture = textures[3];
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
                aiPlay(renderer, num_objects_list, objects, &nb_color_balls);
            }
            printf("\nboules blanches: %d; boules noires: %d\n", (int)(nb_color_balls.x), (int)(nb_color_balls.y));
        }

        if (nb_color_balls.x == 0)
        {
            deleteAllObjects(objects, num_objects_list, &nb_color_balls);
            return 1;
        } else if (nb_color_balls.y == 0)
        {
            deleteAllObjects(objects, num_objects_list, &nb_color_balls);
            return 0;
        }

        

        // Mise à jour des boules
        updateObjects(elapsedTime, objects, num_objects_list, &nb_color_balls);



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

        // Dessiner les objets
        drawObjects(renderer, objects, num_objects_list);


        // Dessiner la boule transparente

        if (draw_t_ball)
        {
            SDL_RenderCopy(renderer, textures[4], NULL, &t_ball);
        }

        // Réinitialiser le viewport pour dessiner des éléments extérieurs
        SDL_RenderSetViewport(renderer, NULL);

        for (int i = 0; i < nb_cards; i++)
        {
            SDL_RenderCopy(renderer, cards[i].texture, NULL, &cards[i].rect);
        }
        // Le premier appel ne fonctionne pas, donc on le fait deux fois (bizarrerie de SDL (ou de moi))
        drawText(renderer, &titleText, (SDL_Rect){(SCREEN_WIDTH + ARENA_WIDTH) / 2, 50, titleText.width, titleText.height});
        drawText(renderer, &titleText, (SDL_Rect){(SCREEN_WIDTH + ARENA_WIDTH) / 2, 50, titleText.width, titleText.height});
    }

}

bool welcomeScreen(SDL_Texture **textures, SDL_Renderer *renderer, SDL_Window *window, Text titleText)
{
    bool isRunning = true;
    bool startGame = false;
    SDL_Event event;


    // Button at the low center of the screen (use SCREEN_WIDTH and SCREEN_HEIGHT)
    SDL_Rect playButton = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 200, 100};
    Text playText = createText(renderer, titleText.font, "Play", titleText.color);

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
            {
                isRunning = false;
                
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX = event.button.x;
                int mouseY = event.button.y;
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (isInBox((Vect){mouseX, mouseY}, playButton))
                    {
                        isRunning = false;
                        startGame = true;
                    }
                }
            }
        }

        SDL_RenderPresent(renderer);

        // Fond gris
        SDL_SetRenderDrawColor(renderer, 217, 217, 217, 255);
        SDL_RenderClear(renderer);

        // Bouton de jeu (Rectangle plus clair + texte)

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);

        SDL_RenderFillRect(renderer, &playButton);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        SDL_RenderDrawRect(renderer, &playButton);

        drawText(renderer, &titleText, (SDL_Rect){SCREEN_WIDTH / 2, 50, titleText.width, titleText.height}); 
        drawText(renderer, &titleText, (SDL_Rect){SCREEN_WIDTH / 2, 50, titleText.width, titleText.height});

        drawText(renderer, &playText, (SDL_Rect){playButton.x + playButton.w / 2, playButton.y + playButton.h / 2, playText.width, playText.height});
    }

    destroyText(&playText);

    return startGame;
}

bool replayScreen(SDL_Texture **textures, SDL_Renderer *renderer, SDL_Window *window, Text titleText, int winner)
{
    if (winner == -1)
    {
        return false;
    }
    
    bool isRunning = true;
    bool startGame = false;
    SDL_Event event;

    // Button at the low center of the screen (use SCREEN_WIDTH and SCREEN_HEIGHT)
    SDL_Rect replayButton = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 200, 100};
    Text replayText = createText(renderer, titleText.font, "Replay", titleText.color);
    Text winnerText;

    if (winner == 0)
    {
        winnerText = createText(renderer, titleText.font, "White wins !", titleText.color);
    }
    else if (winner == 1)
    {
        winnerText = createText(renderer, titleText.font, "Black wins !", titleText.color);
    }

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
            {
                isRunning = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX = event.button.x;
                int mouseY = event.button.y;
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (isInBox((Vect){mouseX, mouseY}, replayButton))
                    {
                        isRunning = false;
                        startGame = true;
                    }
                }
            }
        }

        SDL_RenderPresent(renderer);

        // Fond gris
        SDL_SetRenderDrawColor(renderer, 217, 217, 217, 255);
        SDL_RenderClear(renderer);

        // Bouton de jeu (Rectangle plus clair + texte)

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);

        SDL_RenderFillRect(renderer, &replayButton);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        SDL_RenderDrawRect(renderer, &replayButton);

        drawText(renderer, &titleText, (SDL_Rect){SCREEN_WIDTH / 2, 50, titleText.width, titleText.height});
        drawText(renderer, &titleText, (SDL_Rect){SCREEN_WIDTH / 2, 50, titleText.width, titleText.height});

        drawText(renderer, &winnerText, (SDL_Rect){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 200, winnerText.width, winnerText.height});

        drawText(renderer, &replayText, (SDL_Rect){replayButton.x + replayButton.w / 2, replayButton.y + replayButton.h / 2, replayText.width, replayText.height});
    }

    destroyText(&winnerText);
    destroyText(&replayText);

    return startGame;
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
