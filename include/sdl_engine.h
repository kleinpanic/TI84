#ifndef SDL_ENGINE_H
#define SDL_ENGINE_H

#include <SDL2/SDL.h>

// Declare global variables as extern
extern SDL_Window* window;
extern SDL_Renderer* renderer;

// Declare functions
int init_sdl();
void close_sdl();
void render_calculator();
void handle_input(int* quit);

#endif
