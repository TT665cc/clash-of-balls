/* gcc -o clash_of_balls.o src/*.c -lSDL2 -lSDL2_ttf -lm */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../include/utils.h"
#include "../include/texture.h"
#include "../include/physics.h"
#include "../include/server.h"
#include "../include/client.h"

// Déclarations et définitions inchangées

const int ARENA_WIDTH = 480;   // Largeur de l'arène
const int ARENA_HEIGHT = 854;  // Hauteur de l'arène
const Vect ARENA_DIMENSIONS = {ARENA_WIDTH, ARENA_HEIGHT};
const int SCREEN_WIDTH = 1400; // Largeur de la fenêtre
const int SCREEN_HEIGHT = 950; // Hauteur de la fenêtre
const int FPS = 100;           // Nombre d'images par seconde

typedef struct Text
{
    SDL_Texture *texture;
    int width;
    int height;
    TTF_Font *font;
    SDL_Color color;
} Text;

typedef struct Card
{
    SDL_Texture *texture;
    SDL_Rect rect;
    bool is_selected;
    bool exist;
    int type;
} Card;

void drawObject(SDL_Renderer *renderer, Object *object);
void drawObjects(SDL_Renderer *renderer, Object *objects, int *num_object_list);
void cleanup(SDL_Window *window, SDL_Renderer *renderer, int nb_textures, SDL_Texture **textures);
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
    // Initialisation de la graine pour les nombres aléatoires
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

void aiPlay(SDL_Renderer *renderer, int *num_objects_list, Object *balls, Vect *nb_color_balls)
{
    int max_speed = 500;
    Vect position = (Vect){100, rand() % ARENA_HEIGHT};
    Vect vitesse = (Vect){rand() % max_speed, -(rand() % max_speed)};
    if (canAppear(position, 40, 40, balls, num_objects_list, 0, ARENA_DIMENSIONS))
    {
        createBall(renderer, balls, 30, 60, position, vitesse, 0, nb_color_balls, num_objects_list, false);
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
    createBall(renderer, objects, 30, 20, (Vect){300.0, 200.0}, (Vect){0.0, 0.0}, 1, &nb_color_balls, num_objects_list, true);
    createBall(renderer, objects, 45, 800, (Vect){400.0, 300.0}, (Vect){-100.0, 40.0}, 0, &nb_color_balls, num_objects_list, true);

    // Ajouter quelques murs
    createWall(renderer, objects, 180, 20, (Vect){ARENA_WIDTH / 2, ARENA_HEIGHT / 2}, 1, num_objects_list);

    Vect mousePos = {0, 0};

    // Boucle principale

    bool isRunning = true;
    SDL_Event event;
    Uint64 lastTime = SDL_GetTicks64();
    Uint64 lastTime_cartes = SDL_GetTicks64();
    Uint64 frameTime = SDL_GetTicks64();
    Uint64 lastTimeAI = SDL_GetTicks64();
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
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
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
                            canAppear((Vect){mouseXInArena, mouseYInArena}, t_ball.w, t_ball.h, objects, num_objects_list, cards[i].type, ARENA_DIMENSIONS))
                        {
                            /* Créer une boule avec la position de la souris et une vitesse aléatoire */
                            Vect speed = (Vect){rand() % 1000 - 500, rand() % 1000 - 500};



                            createBall(renderer, objects, t_ball.w / 2, 20,
                                       (Vect){mouseXInArena, mouseYInArena},
                                       speed, 1, &nb_color_balls, num_objects_list, false);
                            cards[i].is_selected = false;
                            cards[i].exist = false;
                            cards[i].texture = textures[0];
                            draw_t_ball = false;
                        }
                    }
                }
            }
            else if (event.type == SDL_MOUSEWHEEL || (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN)))
            {
                if (event.wheel.y < 0) // make the image smaller
                {
                    t_ball.w -= 10;
                    t_ball.h -= 10;
                }
                else if (event.wheel.y > 0) // make the image bigger
                {
                    t_ball.w += 10;
                    t_ball.h += 10;
                }

                t_ball.w = (t_ball.w - 10) % 100 + 10;
                t_ball.h = (t_ball.h - 10) % 100 + 10;

                int mouseXInArena = mousePos.x - (SCREEN_WIDTH - ARENA_WIDTH) / 10;
                int mouseYInArena = mousePos.y - (SCREEN_HEIGHT - ARENA_HEIGHT) / 2;

                t_ball.x = mouseXInArena - t_ball.w / 2;
                t_ball.y = mouseYInArena - t_ball.h / 2;
            }
            else if (event.type == SDL_MOUSEMOTION)
            {

                mousePos.x = event.motion.x;
                mousePos.y = event.motion.y;

                int mouseXInArena = mousePos.x - (SCREEN_WIDTH - ARENA_WIDTH) / 10;
                int mouseYInArena = mousePos.y - (SCREEN_HEIGHT - ARENA_HEIGHT) / 2;

                t_ball.x = mouseXInArena - t_ball.w / 2;
                t_ball.y = mouseYInArena - t_ball.h / 2;
            }
        }

        SDL_RenderPresent(renderer);

        // Gestion du framerate
        Uint64 currentTime = SDL_GetTicks64();
        elapsedTime = currentTime - lastTime;
        lastTime = SDL_GetTicks64();
        frameCount++;

        if (currentTime - lastTime_cartes >= 5000 && (!cards[3].exist))
        {
            lastTime_cartes = currentTime;
            cards[3].texture = textures[3];
            cards[3].exist = true;
        }

        if (currentTime - lastTimeAI >= 2000)
        {
            printf("FPS: %d\n", frameCount / 2);
            frameCount = 0;
            lastTimeAI = currentTime;

            // Probabilité d'apparition d'une boule blanche (par rapport au nombre de boules blanches et noires)

            double proba = sqrt((double)(nb_color_balls.y) / (double)(nb_color_balls.x + nb_color_balls.y));

            if (rand() % 100 < proba * 100)
            {
                aiPlay(renderer, num_objects_list, objects, &nb_color_balls);
            }
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
        updateObjects(elapsedTime, objects, num_objects_list, &nb_color_balls, ARENA_DIMENSIONS);



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
            if (event.type == SDL_QUIT || ( event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
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
