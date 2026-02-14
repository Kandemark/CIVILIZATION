/**
 * @file renderer.c
 * @brief SDL3 rendering utilities implementation
 */

#include "../../include/engine/renderer.h"
#include "../../include/utils/noise.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

SDL_Color civ_color_from_rgb(uint32_t color) {
  SDL_Color c;
  c.r = (color >> 16) & 0xFF;
  c.g = (color >> 8) & 0xFF;
  c.b = color & 0xFF;
  c.a = 255;
  return c;
}

void civ_render_rect_filled(SDL_Renderer *renderer, int x, int y, int w, int h,
                            uint32_t color) {
  civ_render_rect_filled_alpha(renderer, x, y, w, h, color, 255);
}

void civ_render_rect_filled_alpha(SDL_Renderer *renderer, int x, int y, int w,
                                  int h, uint32_t color, uint8_t alpha) {
  if (!renderer)
    return;

  SDL_Color c = civ_color_from_rgb(color);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, alpha);

  SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
  SDL_RenderFillRect(renderer, &rect);
}

void civ_render_rect_outline(SDL_Renderer *renderer, int x, int y, int w, int h,
                             uint32_t color, int thickness) {
  if (!renderer)
    return;

  SDL_Color c = civ_color_from_rgb(color);
  SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

  for (int i = 0; i < thickness; i++) {
    SDL_FRect rect = {(float)(x + i), (float)(y + i), (float)(w - 2 * i),
                      (float)(h - 2 * i)};
    SDL_RenderRect(renderer, &rect);
  }
}

void civ_render_rounded_rect(SDL_Renderer *renderer, int x, int y, int w, int h,
                             int radius, uint32_t color) {
  if (!renderer)
    return;

  SDL_Color c = civ_color_from_rgb(color);
  SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

  /* Simple approximation: draw main rect + circles at corners */
  SDL_FRect main_rect = {(float)(x + radius), (float)y, (float)(w - 2 * radius),
                         (float)h};
  SDL_RenderFillRect(renderer, &main_rect);

  SDL_FRect left_rect = {(float)x, (float)(y + radius), (float)radius,
                         (float)(h - 2 * radius)};
  SDL_RenderFillRect(renderer, &left_rect);

  SDL_FRect right_rect = {(float)(x + w - radius), (float)(y + radius),
                          (float)radius, (float)(h - 2 * radius)};
  SDL_RenderFillRect(renderer, &right_rect);

  /* For simplicity, skip rounded corners - just use sharp corners for now */
  SDL_FRect tl = {(float)x, (float)y, (float)radius, (float)radius};
  SDL_FRect tr = {(float)(x + w - radius), (float)y, (float)radius,
                  (float)radius};
  SDL_FRect bl = {(float)x, (float)(y + h - radius), (float)radius,
                  (float)radius};
  SDL_FRect br = {(float)(x + w - radius), (float)(y + h - radius),
                  (float)radius, (float)radius};

  SDL_RenderFillRect(renderer, &tl);
  SDL_RenderFillRect(renderer, &tr);
  SDL_RenderFillRect(renderer, &bl);
  SDL_RenderFillRect(renderer, &br);
}

void civ_render_line(SDL_Renderer *renderer, int x1, int y1, int x2, int y2,
                     uint32_t color) {
  if (!renderer)
    return;

  SDL_Color c = civ_color_from_rgb(color);
  SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
  SDL_RenderLine(renderer, (float)x1, (float)y1, (float)x2, (float)y2);
}

void civ_render_gradient_vertical(SDL_Renderer *renderer, int x, int y, int w,
                                  int h, uint32_t color_top,
                                  uint32_t color_bottom) {
  if (!renderer)
    return;

  SDL_Color c_top = civ_color_from_rgb(color_top);
  SDL_Color c_bot = civ_color_from_rgb(color_bottom);

  /* Draw lines with interpolated colors */
  for (int i = 0; i < h; i++) {
    float t = (float)i / (float)h;
    uint8_t r = (uint8_t)(c_top.r + (c_bot.r - c_top.r) * t);
    uint8_t g = (uint8_t)(c_top.g + (c_bot.g - c_top.g) * t);
    uint8_t b = (uint8_t)(c_top.b + (c_bot.b - c_top.b) * t);

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderLine(renderer, (float)x, (float)(y + i), (float)(x + w),
                   (float)(y + i));
  }
}

void civ_render_set_alpha(SDL_Renderer *renderer, uint8_t alpha) {
  uint8_t r, g, b;
  SDL_GetRenderDrawColor(renderer, &r, &g, &b, NULL);
  SDL_SetRenderDrawColor(renderer, r, g, b, alpha);
}

/* Map rendering context */

civ_render_map_context_t *
civ_render_map_context_create(SDL_Renderer *renderer, int fb_width,
                              int fb_height, int map_width, int map_height) {
  if (!renderer)
    return NULL;

  civ_render_map_context_t *ctx =
      (civ_render_map_context_t *)malloc(sizeof(civ_render_map_context_t));
  if (!ctx)
    return NULL;

  ctx->buffer_width = fb_width;
  ctx->buffer_height = fb_height;
  ctx->map_width = map_width;
  ctx->map_height = map_height;
  ctx->view_x = (float)map_width / 2.0f;
  ctx->view_y = (float)map_height / 2.0f;
  ctx->zoom = 0.12f;

  /* Create pixel buffer */
  ctx->pixel_buffer =
      (uint32_t *)malloc(fb_width * fb_height * sizeof(uint32_t));
  if (!ctx->pixel_buffer) {
    free(ctx);
    return NULL;
  }

  /* Create streaming texture */
  ctx->map_texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, fb_width, fb_height);
  if (!ctx->map_texture) {
    free(ctx->pixel_buffer);
    free(ctx);
    return NULL;
  }

  return ctx;
}

void civ_render_map_context_destroy(civ_render_map_context_t *ctx) {
  if (!ctx)
    return;

  if (ctx->map_texture) {
    SDL_DestroyTexture(ctx->map_texture);
  }
  if (ctx->pixel_buffer) {
    free(ctx->pixel_buffer);
  }
  free(ctx);
}

/* Helper: Get map tile color */
static uint32_t get_map_color(const civ_map_tile_t *tile) {
  if (!tile)
    return 0xFF010204;

  float e = tile->elevation;
  float t = tile->temperature;
  float m = tile->moisture;

  /* 1. Deep Ocean & Coastal Waters */
  if (e < 0.52f) {
    if (e < 0.35f)
      return 0xFF030A1C; /* Abyssal */
    if (e < 0.45f)
      return 0xFF081A36; /* Deep */
    return 0xFF12406A;   /* Coastal */
  }

  /* 2. Land Biomes (Elevation > 0.52) */
  /* Ice Caps */
  if (t < 0.15f)
    return 0xFFF0F5FF;

  /* Desert (High temp, low moisture) */
  if (t > 0.65f && m < 0.22f)
    return 0xFFD9B97E;

  /* Savanna (High temp, medium moisture) */
  if (t > 0.60f && m < 0.45f)
    return 0xFFB5AD5D;

  /* Mountains & High Hills */
  if (e > 0.82f) {
    if (e > 0.90f && t < 0.4f)
      return 0xFFFFFFFF; /* Snowy peaks */
    return 0xFF6A6560;   /* Stone */
  }
  if (e > 0.72f)
    return 0xFF8A7E6A; /* High Hills */

  /* Lush Jungles & Forests */
  if (m > 0.75f) {
    if (t > 0.6f)
      return 0xFF0A3D12; /* Jungle */
    return 0xFF144D1B;   /* Boreal Forest */
  }

  /* Woodlands */
  if (m > 0.55f)
    return 0xFF2A6B2F;

  /* Grassland (Default) */
  uint32_t color = 0xFF567E34;

  /* Apply Visibility Shaders */
  if (!tile->is_explored) {
    return 0xFF010204; /* Black/Unexplored */
  }

  if (!tile->is_visible) {
    /* Shrouded: Darken significantly */
    uint8_t r = ((color >> 16) & 0xFF) / 3;
    uint8_t g = ((color >> 8) & 0xFF) / 3;
    uint8_t b = (color & 0xFF) / 3;
    return 0xFF000000 | (r << 16) | (g << 8) | b;
  }

  return color;
}

void civ_render_map(SDL_Renderer *renderer, civ_render_map_context_t *ctx,
                    civ_map_t *map, int fb_width, int fb_height) {
  if (!renderer || !ctx || !map || !ctx->pixel_buffer)
    return;

  const float WORLD_UNIT_SIZE = 4.0f;
  float minZ = (float)fb_height / ((float)ctx->map_height * WORLD_UNIT_SIZE);
  if (ctx->zoom < minZ)
    ctx->zoom = minZ;

  float inv_scale = 1.0f / (ctx->zoom * WORLD_UNIT_SIZE);
  float half_h = (fb_height * 0.5f) * inv_scale;

  /* Clamp view_y */
  if (ctx->view_y < half_h)
    ctx->view_y = half_h;
  if (ctx->view_y > ctx->map_height - half_h)
    ctx->view_y = ctx->map_height - half_h;

  /* Render map to pixel buffer */
  for (int y = 0; y < fb_height; y++) {
    uint32_t *row = &ctx->pixel_buffer[y * fb_width];

    for (int x = 0; x < fb_width; x++) {
      /* Calculate world coordinates from view context */
      /* MASTERPIECE 2.0: Correct coordinate spacing with WORLD_UNIT_SIZE */
      float fx =
          ctx->view_x + (x - fb_width / 2.0f) / (ctx->zoom * WORLD_UNIT_SIZE);
      float fy =
          ctx->view_y + (y - fb_height / 2.0f) / (ctx->zoom * WORLD_UNIT_SIZE);

      /* MASTERPIECE 2.0: Spherical Realism (Equatorial Wrap, Polar Clamp) */
      /* 1. East-West Circumnavigation (Wrapping) */
      fx = fmodf(fx, (float)ctx->map_width);
      if (fx < 0)
        fx += (float)ctx->map_width;

      /* 2. North-South Stability (Clamping) */
      /* If we are past the poles, render deep space / ocean depth */
      if (fy < 0 || fy >= (float)ctx->map_height) {
        row[x] = 0xFF020408; /* Deep Space Black */
        continue;
      }

      int32_t wx = (int32_t)fx;
      int32_t wy = (int32_t)fy;

      civ_map_tile_t *tile = civ_map_get_tile(map, wx, wy);
      if (!tile) {
        row[x] = 0x00000000;
        continue;
      }

      uint32_t color = get_map_color(tile);

      /* MASTERPIECE 2.0: Procedural Grain & Detail */
      float detail_noise =
          (float)civ_noise_perlin(fx * 50.0f, fy * 50.0f, map->seed + 50);
      float detail_weight = 0.05f;

      /* Biome-specific detail adjustments */
      if (tile->land_use == CIV_LAND_USE_WATER) {
        /* Water ripples */
        detail_noise = (float)civ_noise_perlin(
            fx * 20.0f, fy * 20.0f + SDL_GetTicks() * 0.001f, map->seed + 51);
        detail_weight = 0.08f;
      } else if (tile->land_use == CIV_LAND_USE_DESERT) {
        /* Sand ripples */
        detail_noise =
            (float)civ_noise_perlin(fx * 30.0f, fy * 5.0f, map->seed + 52);
        detail_weight = 0.12f;
      }

      /* Highlight Rivers */
      if (tile->has_river && tile->elevation >= map->sea_level) {
        color = 0xFF2A8AE0; /* River blue */
      }

      /* MASTERPIECE SHADING KERNEL: Slope-based realistic lighting */
      /* Wrap neighbor coordinates for seamless shading */
      civ_map_tile_t *east =
          civ_map_get_tile(map, (wx + 1) % ctx->map_width, wy);
      civ_map_tile_t *south =
          civ_map_get_tile(map, wx, (wy + 1) % ctx->map_height);

      float dx = east ? (tile->elevation - east->elevation) : 0.0f;
      float dy = south ? (tile->elevation - south->elevation) : 0.0f;

      /* Composite Normal approximation */
      float lit = 1.0f + (dx + dy) * 12.0f; /* High contrast slope lighting */
      lit += detail_noise * detail_weight;  /* Add procedural micro-shading */
      lit = fmaxf(0.6f, fminf(1.4f, lit)); /* Expanded stable range for drama */

      /* MASTERPIECE 2.0: Atmospheric Cloud Shadows */
      float cloud_x = fx * 0.02f + SDL_GetTicks() * 0.00005f;
      float cloud_y = fy * 0.02f + SDL_GetTicks() * 0.00003f;
      float clouds = (float)civ_noise_perlin(cloud_x, cloud_y, map->seed + 99);
      if (clouds > 0.4f) {
        lit *= (1.0f - (clouds - 0.4f) * 0.5f); /* Subtle darken for clouds */
      }

      /* Coastal Highlight (Foam) */
      if (tile->elevation >= 0.52f && tile->elevation < 0.54f) {
        lit *= 1.25f;
      }

      uint8_t r = (uint8_t)(((color >> 16) & 0xFF) * lit);
      uint8_t g = (uint8_t)(((color >> 8) & 0xFF) * lit);
      uint8_t b = (uint8_t)((color & 0xFF) * lit);

      /* Resource "Glimmer" Overlay */
      if (tile->has_resource) {
        float pulse = (sinf(SDL_GetTicks() * 0.005f + wx + wy) * 0.5f + 0.5f);
        r = (uint8_t)fminf(255, r + 50 * pulse);
        g = (uint8_t)fminf(255, g + 40 * pulse);
        b = (uint8_t)fmaxf(0, b - 20 * pulse);
      }

      row[x] = 0xFF000000 | (r << 16) | (g << 8) | b;
    }
  }

  /* Upload to texture */
  SDL_UpdateTexture(ctx->map_texture, NULL, ctx->pixel_buffer,
                    fb_width * sizeof(uint32_t));

  /* Render texture to screen */
  SDL_FRect dst_rect = {0, 0, (float)fb_width, (float)fb_height};
  SDL_RenderTexture(renderer, ctx->map_texture, NULL, &dst_rect);

  /* MASTERPIECE 2.0: Latitude/Longitude Indicators */
  float lat = 90.0f - (ctx->view_y / (float)ctx->map_height) * 180.0f;
  float lon = (ctx->view_x / (float)ctx->map_width) * 360.0f - 180.0f;

  /* Wrap longitude display */
  while (lon < -180.0f)
    lon += 360.0f;
  while (lon > 180.0f)
    lon -= 360.0f;

  char lat_buf[32], lon_buf[32];
  sprintf(lat_buf, "LAT: %.2f%c", fabsf(lat), lat >= 0 ? 'N' : 'S');
  sprintf(lon_buf, "LON: %.2f%c", fabsf(lon), lon >= 0 ? 'E' : 'W');

  /* Note: Drawing indicators requires a font, which we don't have passed here.
     Typically we'd render UI on top in the scene_*.c files.
     However, for "Masterpiece 2.0", I will ensure the scene rendering adds
     these. */
}

void civ_render_minimap(SDL_Renderer *renderer, int x, int y, int w, int h,
                        civ_map_t *map, civ_render_map_context_t *ctx) {
  if (!renderer || !map || !ctx)
    return;

  /* Draw background */
  civ_render_rect_filled_alpha(renderer, x, y, w, h, 0x010204, 200);
  civ_render_rect_outline(renderer, x, y, w, h, 0x1A2A3A, 1);

  /* Render simplified tiles */
  float step_x = (float)map->width / (float)w;
  float step_y = (float)map->height / (float)h;

  for (int py = 0; py < h; py++) {
    for (int px = 0; px < w; px++) {
      int32_t mx = (int32_t)(px * step_x);
      int32_t my = (int32_t)(py * step_y);
      civ_map_tile_t *tile = civ_map_get_tile(map, mx, my);
      if (tile) {
        uint32_t color = get_map_color(tile);
        /* Use a simple pixel or small rect for minimap */
        SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF,
                               (color >> 8) & 0xFF, color & 0xFF, 255);
        SDL_RenderPoint(renderer, (float)(x + px), (float)(y + py));
      }
    }
  }

  /* Draw view rectangle */
  const float WORLD_UNIT_SIZE = 4.0f;
  float inv_scale = 1.0f / (ctx->zoom * WORLD_UNIT_SIZE);
  float view_w_world = ctx->buffer_width * inv_scale;
  float view_h_world = ctx->buffer_height * inv_scale;

  float rect_w = (view_w_world / (float)map->width) * (float)w;
  float rect_h = (view_h_world / (float)map->height) * (float)h;
  float rect_x =
      (ctx->view_x / (float)map->width) * (float)w + (float)x - rect_w / 2.0f;
  float rect_y =
      (ctx->view_y / (float)map->height) * (float)h + (float)y - rect_h / 2.0f;

  civ_render_rect_outline(renderer, (int)rect_x, (int)rect_y, (int)rect_w,
                          (int)rect_h, 0xFFFFFF, 1);
}

void civ_render_settlements(SDL_Renderer *renderer,
                            civ_render_map_context_t *ctx,
                            civ_settlement_manager_t *manager, int win_w,
                            int win_h) {
  if (!renderer || !ctx || !manager)
    return;

  const float WORLD_UNIT_SIZE = 4.0f;

  for (size_t i = 0; i < manager->settlement_count; i++) {
    civ_settlement_t *s = &manager->settlements[i];

    /* Convert world coords to screen coords */
    float screen_x = (float)win_w / 2.0f +
                     (s->x - ctx->view_x) * ctx->zoom * WORLD_UNIT_SIZE;
    float screen_y = (float)win_h / 2.0f +
                     (s->y - ctx->view_y) * ctx->zoom * WORLD_UNIT_SIZE;

    float size = 40.0f * ctx->zoom;
    if (size < 4.0f)
      size = 4.0f;

    /* Culling */
    if (screen_x + size < 0 || screen_x - size > win_w || screen_y + size < 0 ||
        screen_y - size > win_h)
      continue;

    /* Draw Settlement Icon (Castle-like shape) */
    uint32_t icon_color = 0xFFCCCCCC; /* Silver/Stone */
    if (s->tier >= CIV_SETTLEMENT_CITY)
      icon_color = 0xFFFFD700; /* Gold for cities */

    civ_render_rect_filled(renderer, (int)(screen_x - size / 2),
                           (int)(screen_y - size / 2), (int)size, (int)size,
                           icon_color);
    civ_render_rect_outline(renderer, (int)(screen_x - size / 2),
                            (int)(screen_y - size / 2), (int)size, (int)size,
                            0xFF000000, 1);

    /* Draw battlements */
    float b_size = size / 3.0f;
    civ_render_rect_filled(renderer, (int)(screen_x - size / 2),
                           (int)(screen_y - size / 2 - b_size), (int)b_size,
                           (int)b_size, icon_color);
    civ_render_rect_filled(renderer, (int)(screen_x + size / 2 - b_size),
                           (int)(screen_y - size / 2 - b_size), (int)b_size,
                           (int)b_size, icon_color);

    /* Note: Text labels (city name/pop) are usually rendered via civ_font
       which isn't available in renderer.c easily.
       We'll handle the labels in scene_game.c instead. */
  }
}
