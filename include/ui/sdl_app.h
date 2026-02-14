#pragma once
#include <SDL3/SDL.h>
int sdl_app_init(SDL_Window **window, SDL_Renderer **renderer);
void sdl_app_quit(SDL_Window *window, SDL_Renderer *renderer);
