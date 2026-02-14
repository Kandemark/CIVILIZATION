/**
 * @file window.c
 * @brief SDL3 window management implementation
 */

#include "../../include/engine/window.h"
#include <stdio.h>
#include <stdlib.h>

struct civ_window {
  SDL_Window *sdl_window;
  SDL_Renderer *renderer;
  int width;
  int height;
};

civ_window_t *civ_window_create(const char *title, int width, int height,
                                uint32_t flags) {
  civ_window_t *window = (civ_window_t *)malloc(sizeof(civ_window_t));
  if (!window) {
    fprintf(stderr, "Failed to allocate window structure\n");
    return NULL;
  }

  /* Get desktop resolution if width/height are 0 */
  if (width == 0 || height == 0) {
    SDL_DisplayID display_id = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(display_id);
    if (mode) {
      width = mode->w;
      height = mode->h;
    } else {
      width = 1920;
      height = 1080;
    }
  }

  window->width = width;
  window->height = height;

  /* Convert flags to SDL flags */
  Uint32 sdl_flags = 0;
  if (flags & CIV_WINDOW_FULLSCREEN) {
    sdl_flags |= SDL_WINDOW_FULLSCREEN;
  }
  if (flags & CIV_WINDOW_FULLSCREEN_DESKTOP) {
    sdl_flags |= SDL_WINDOW_FULLSCREEN;
  }
  if (flags & CIV_WINDOW_RESIZABLE) {
    sdl_flags |= SDL_WINDOW_RESIZABLE;
  }
  if (flags & CIV_WINDOW_BORDERLESS) {
    sdl_flags |= SDL_WINDOW_BORDERLESS;
  }
  if (flags & CIV_WINDOW_MAXIMIZED) {
    sdl_flags |= SDL_WINDOW_MAXIMIZED;
  }

  /* Create SDL window */
  window->sdl_window = SDL_CreateWindow(title, width, height, sdl_flags);
  if (!window->sdl_window) {
    fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
    free(window);
    return NULL;
  }

  /* Create renderer with VSync and hardware acceleration */
  window->renderer = SDL_CreateRenderer(window->sdl_window, NULL);
  if (!window->renderer) {
    fprintf(stderr, "Failed to create SDL renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(window->sdl_window);
    free(window);
    return NULL;
  }

  /* Enable VSync */
  SDL_SetRenderVSync(window->renderer, 1);

  printf("Window created: %dx%d\n", width, height);
  return window;
}

void civ_window_destroy(civ_window_t *window) {
  if (!window)
    return;

  if (window->renderer) {
    SDL_DestroyRenderer(window->renderer);
  }
  if (window->sdl_window) {
    SDL_DestroyWindow(window->sdl_window);
  }
  free(window);
}

SDL_Renderer *civ_window_get_renderer(civ_window_t *window) {
  return window ? window->renderer : NULL;
}

SDL_Window *civ_window_get_sdl_window(civ_window_t *window) {
  return window ? window->sdl_window : NULL;
}

void civ_window_clear(civ_window_t *window, uint32_t color) {
  if (!window || !window->renderer)
    return;

  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;

  SDL_SetRenderDrawColor(window->renderer, r, g, b, 255);
  SDL_RenderClear(window->renderer);
}

void civ_window_present(civ_window_t *window) {
  if (!window || !window->renderer)
    return;
  SDL_RenderPresent(window->renderer);
}

void civ_window_get_size(civ_window_t *window, int *w, int *h) {
  if (!window || !window->sdl_window) {
    if (w)
      *w = 0;
    if (h)
      *h = 0;
    return;
  }
  SDL_GetWindowSize(window->sdl_window, w, h);
}

void civ_window_get_drawable_size(civ_window_t *window, int *w, int *h) {
  if (!window || !window->renderer) {
    if (w)
      *w = 0;
    if (h)
      *h = 0;
    return;
  }
  SDL_GetRenderOutputSize(window->renderer, w, h);
}

void civ_window_set_title(civ_window_t *window, const char *title) {
  if (!window || !window->sdl_window)
    return;
  SDL_SetWindowTitle(window->sdl_window, title);
}

void civ_window_set_fullscreen(civ_window_t *window, bool fullscreen) {
  if (!window || !window->sdl_window)
    return;
  SDL_SetWindowFullscreen(window->sdl_window, fullscreen ? true : false);
}
