#ifndef TEXTURE_H

#define TEXTURE_H

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

/**
 * @brief Load a texture from a file
 * 
 * @param filename File to load the texture from
 * @param renderer Renderer to create the texture
 * 
 * @return SDL_Texture* Loaded texture
 */
SDL_Texture *loadTexture(const char *filename, SDL_Renderer *renderer);

/**
 * @brief Create a circle texture
 * 
 * @param renderer Renderer to create the texture
 * @param radius Radius of the circle
 * @param color Color of the circle
 * 
 * @return SDL_Texture* Created texture
 */
SDL_Texture *createCircleTexture(SDL_Renderer *renderer, int radius, int color);

/**
 * @brief Create a rectangle texture
 * 
 * @param renderer Renderer to create the texture
 * @param width Width of the rectangle
 * @param height Height of the rectangle
 * @param color Color of the rectangle
 * 
 * @return SDL_Texture* Created texture
 */
SDL_Texture *createRectangleTexture(SDL_Renderer *renderer, int width, int height, int color);

#endif
