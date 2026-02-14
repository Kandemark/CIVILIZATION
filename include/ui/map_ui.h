#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

void renderDropdownMenu(SDL_Renderer* renderer, SDL_TTF_Font* font, int x, int y);
void handleDropdownEvent(SDL_Event* event);
void renderMap(SDL_Renderer* renderer);
