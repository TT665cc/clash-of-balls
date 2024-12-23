#include <SDL2/SDL.h>


/**
 * @brief Load images from a directory
 * 
 * @param directory Directory to load images from
 * @param renderer Renderer to create textures
 * @param textures Array of textures to fill
 * @param max_textures Maximum number of textures to load
 * 
 * @return Number of textures loaded
 */
int loadImagesFromDirectory(const char *directory, SDL_Renderer *renderer, SDL_Texture **textures, int max_textures);

SDL_Texture *loadTexture(const char *filename, SDL_Renderer *renderer);

SDL_Texture *createCircleTexture(SDL_Renderer *renderer, int radius, int color);

SDL_Texture *createRectangleTexture(SDL_Renderer *renderer, int width, int height, int color);