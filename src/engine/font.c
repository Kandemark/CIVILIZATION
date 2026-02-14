/**
 * @file font.c
 * @brief SDL3_ttf font rendering implementation using system fonts
 */

#include "../../include/engine/font.h"
#include "../../include/engine/renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

struct civ_font {
  TTF_Font *ttf_font;
  int size;
  char name[256];
};

/* System font detection for Windows */
static char *find_system_font_windows(const char *font_name) {
#ifdef _WIN32
  static char font_path[512];
  char windows_dir[256];

  GetWindowsDirectoryA(windows_dir, sizeof(windows_dir));

  /* Map common font names to filenames */
  const char *filename = NULL;
  if (strcmp(font_name, "Segoe UI") == 0) {
    filename = "segoeui.ttf";
  } else if (strcmp(font_name, "Arial") == 0) {
    filename = "arial.ttf";
  } else if (strcmp(font_name, "Consolas") == 0) {
    filename = "consola.ttf";
  } else if (strcmp(font_name, "Calibri") == 0) {
    filename = "calibri.ttf";
  } else if (strcmp(font_name, "Verdana") == 0) {
    filename = "verdana.ttf";
  } else {
    /* Default to Arial */
    filename = "arial.ttf";
  }

  snprintf(font_path, sizeof(font_path), "%s\\Fonts\\%s", windows_dir,
           filename);
  return font_path;
#else
  return NULL;
#endif
}

bool civ_font_system_init(void) {
  if (!TTF_Init()) {
    fprintf(stderr, "Failed to initialize SDL_ttf: %s\n", SDL_GetError());
    return false;
  }
  printf("SDL_ttf initialized successfully\n");
  return true;
}

void civ_font_system_shutdown(void) { TTF_Quit(); }

civ_font_t *civ_font_load_system(const char *name, int size) {
  char *font_path = find_system_font_windows(name);
  if (!font_path) {
    fprintf(stderr, "Failed to find system font: %s\n", name);
    return NULL;
  }

  return civ_font_load(font_path, size);
}

civ_font_t *civ_font_load(const char *path, int size) {
  civ_font_t *font = (civ_font_t *)malloc(sizeof(civ_font_t));
  if (!font)
    return NULL;

  font->ttf_font = TTF_OpenFont(path, size);
  if (!font->ttf_font) {
    fprintf(stderr, "Failed to load font from %s: %s\n", path, SDL_GetError());
    free(font);
    return NULL;
  }

  font->size = size;
  strncpy(font->name, path, sizeof(font->name) - 1);
  font->name[sizeof(font->name) - 1] = '\0';

  printf("Loaded font: %s (size %d)\n", path, size);
  return font;
}

void civ_font_destroy(civ_font_t *font) {
  if (!font)
    return;

  if (font->ttf_font) {
    TTF_CloseFont(font->ttf_font);
  }
  free(font);
}

void civ_font_render(SDL_Renderer *renderer, civ_font_t *font, const char *text,
                     int x, int y, uint32_t color) {
  if (!renderer || !font || !font->ttf_font || !text)
    return;

  SDL_Color c = civ_color_from_rgb(color);
  SDL_Surface *surface =
      TTF_RenderText_Blended(font->ttf_font, text, strlen(text), c);
  if (!surface)
    return;

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_DestroySurface(surface);

  if (!texture)
    return;

  float w_f, h_f;
  SDL_GetTextureSize(texture, &w_f, &h_f);
  int w = (int)w_f;
  int h = (int)h_f;

  SDL_FRect dest = {(float)x, (float)y, (float)w, (float)h};
  SDL_RenderTexture(renderer, texture, NULL, &dest);

  SDL_DestroyTexture(texture);
}

void civ_font_render_aligned(SDL_Renderer *renderer, civ_font_t *font,
                             const char *text, int x, int y, int w, int h,
                             uint32_t color, civ_text_align_t align,
                             civ_text_valign_t valign) {
  if (!renderer || !font || !text)
    return;

  int text_w, text_h;
  civ_font_get_text_size(font, text, &text_w, &text_h);

  /* Calculate position based on alignment */
  int pos_x = x;
  int pos_y = y;

  switch (align) {
  case CIV_ALIGN_LEFT:
    pos_x = x;
    break;
  case CIV_ALIGN_CENTER:
    pos_x = x + (w - text_w) / 2;
    break;
  case CIV_ALIGN_RIGHT:
    pos_x = x + w - text_w;
    break;
  }

  switch (valign) {
  case CIV_VALIGN_TOP:
    pos_y = y;
    break;
  case CIV_VALIGN_MIDDLE:
    pos_y = y + (h - text_h) / 2;
    break;
  case CIV_VALIGN_BOTTOM:
    pos_y = y + h - text_h;
    break;
  }

  civ_font_render(renderer, font, text, pos_x, pos_y, color);
}

void civ_font_render_alpha(SDL_Renderer *renderer, civ_font_t *font,
                           const char *text, int x, int y, uint32_t color,
                           uint8_t alpha) {
  if (!renderer || !font || !font->ttf_font || !text)
    return;

  SDL_Color c = civ_color_from_rgb(color);
  c.a = alpha;

  SDL_Surface *surface =
      TTF_RenderText_Blended(font->ttf_font, text, strlen(text), c);
  if (!surface)
    return;

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_DestroySurface(surface);

  if (!texture)
    return;

  float w_f, h_f;
  SDL_GetTextureSize(texture, &w_f, &h_f);
  int w = (int)w_f;
  int h = (int)h_f;
  SDL_SetTextureAlphaMod(texture, alpha);

  SDL_FRect dest = {(float)x, (float)y, (float)w, (float)h};
  SDL_RenderTexture(renderer, texture, NULL, &dest);

  SDL_DestroyTexture(texture);
}

void civ_font_get_text_size(civ_font_t *font, const char *text, int *w,
                            int *h) {
  if (!font || !font->ttf_font || !text) {
    if (w)
      *w = 0;
    if (h)
      *h = 0;
    return;
  }

  /* SDL3_ttf: TTF_GetStringSize returns bool and takes int* */
  TTF_GetStringSize(font->ttf_font, text, strlen(text), w, h);
}

int civ_font_get_height(civ_font_t *font) {
  if (!font || !font->ttf_font)
    return 0;
  return TTF_GetFontHeight(font->ttf_font);
}
