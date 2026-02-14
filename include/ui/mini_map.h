#pragma once
#include <SDL3/SDL.h>
#include "../../include/core/world/map_view.h"

void renderMiniMap(SDL_Renderer* renderer, const civ_map_view_manager_t* manager, int x, int y, int width, int height);
