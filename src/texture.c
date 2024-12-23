#include "../include/utils.h"
#include "../include/texture.h"
#include "../include/colors.h"
#include <SDL2/SDL.h>
#include <dirent.h>
#include <sys/stat.h>

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

SDL_Texture *loadTexture(const char *filename, SDL_Renderer *renderer)
{
    SDL_Surface *surf = SDL_LoadBMP(filename);
    if (!surf)
    {
        fprintf(stderr, "Erreur de chargement de l'image: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!imageTexture)
    {
        fprintf(stderr, "Erreur lors de la création de la texture: %s\n", SDL_GetError());
    }

    return imageTexture;
}

SDL_Texture *createCircleTexture(SDL_Renderer *renderer, int radius, int color)
{
    SDL_Surface *surf = SDL_CreateRGBSurface(0, radius * 2, radius * 2, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
    if (!surf)
    {
        fprintf(stderr, "Erreur lors de la création de la surface: %s\n", SDL_GetError());
        return NULL;
    }

    // Fill the circle with the color

    for (int x = -radius; x < radius; x++)
    {
        for (int y = -radius; y < radius; y++)
        {
            if (x * x + y * y <= radius * radius)
            {
                // Anti-aliasing

                // If the pixel is on the edge of the circle, we make it a bit transparent

                if (x * x + y * y > (radius - 1) * (radius - 1))
                {
                    Uint8 r, g, b, a;
                    SDL_GetRGBA(SDL_MapRGB(surf->format, lut[color].r, lut[color].g, lut[color].b), surf->format, &r, &g, &b, &a);
                    a = 255 - (255 * (x * x + y * y - (radius - 1) * (radius - 1))) / (radius * radius - (radius - 1) * (radius - 1));
                    SDL_Rect pixel = {x + radius, y + radius, 1, 1};
                    SDL_FillRect(surf, &pixel, SDL_MapRGBA(surf->format, r, g, b, a));
                }
                else
                {
                    SDL_Rect pixel = {x + radius, y + radius, 1, 1};
                    SDL_FillRect(surf, &pixel, SDL_MapRGB(surf->format, lut[color].r, lut[color].g, lut[color].b));
                }

            }
        }
    }

    SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!imageTexture)
    {
        fprintf(stderr, "Erreur lors de la création de la texture: %s\n", SDL_GetError());
    }

    return imageTexture;
}

SDL_Texture *createRectangleTexture(SDL_Renderer *renderer, int width, int height, int color)
{
    SDL_Surface *surf = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    if (!surf)
    {
        fprintf(stderr, "Erreur lors de la création de la surface: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_FillRect(surf, NULL, SDL_MapRGB(surf->format, lut[color].r, lut[color].g, lut[color].b));

    SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!imageTexture)
    {
        fprintf(stderr, "Erreur lors de la création de la texture: %s\n", SDL_GetError());
    }

    return imageTexture;
}