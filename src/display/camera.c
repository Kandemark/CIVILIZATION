#include "../../include/display/camera.h"
#include <math.h>

#define WORLD_UNIT_SIZE 4.0f

void civ_camera_init(civ_camera_t *cam, int32_t map_w, int32_t map_h) {
  cam->x = (float)map_w / 2.0f;
  cam->y = (float)map_h / 2.0f;
  cam->zoom = 0.12f;
  cam->target_x = cam->x;
  cam->target_y = cam->y;
  cam->target_zoom = cam->zoom;
  cam->lerp_speed = 8.0f;
  cam->map_width = map_w;
  cam->map_height = map_h;
  cam->min_zoom = 0.005f;
  cam->max_zoom = 2.0f;
}

void civ_camera_update(civ_camera_t *cam, float dt) {
  float t = 1.0f - expf(-cam->lerp_speed * dt);
  cam->x += (cam->target_x - cam->x) * t;
  cam->y += (cam->target_y - cam->y) * t;
  cam->zoom += (cam->target_zoom - cam->zoom) * t;

  float half_h = (cam->map_height * 0.5f);
  if (cam->y < half_h * 0.1f) cam->y = half_h * 0.1f;
  if (cam->y > cam->map_height - half_h * 0.1f)
    cam->y = cam->map_height - half_h * 0.1f;
}

void civ_camera_pan(civ_camera_t *cam, float dx, float dy) {
  cam->target_x += dx / (cam->target_zoom * WORLD_UNIT_SIZE);
  cam->target_y += dy / (cam->target_zoom * WORLD_UNIT_SIZE);
}

void civ_camera_zoom(civ_camera_t *cam, float factor, float cx, float cy) {
  float old_zoom = cam->target_zoom;
  cam->target_zoom *= factor;
  if (cam->target_zoom < cam->min_zoom) cam->target_zoom = cam->min_zoom;
  if (cam->target_zoom > cam->max_zoom) cam->target_zoom = cam->max_zoom;

  float ratio = old_zoom / cam->target_zoom;
  cam->target_x += (cx - cam->target_x) * (1.0f - ratio);
  cam->target_y += (cy - cam->target_y) * (1.0f - ratio);
}

void civ_camera_world_to_screen(const civ_camera_t *cam, int win_w, int win_h,
                                float wx, float wy, float *sx, float *sy) {
  *sx = (float)win_w / 2.0f + (wx - cam->x) * cam->zoom * WORLD_UNIT_SIZE;
  *sy = (float)win_h / 2.0f + (wy - cam->y) * cam->zoom * WORLD_UNIT_SIZE;
}

void civ_camera_screen_to_world(const civ_camera_t *cam, int win_w, int win_h,
                                int sx, int sy, float *wx, float *wy) {
  *wx = cam->x + (sx - win_w / 2.0f) / (cam->zoom * WORLD_UNIT_SIZE);
  *wy = cam->y + (sy - win_h / 2.0f) / (cam->zoom * WORLD_UNIT_SIZE);
}

bool civ_camera_is_visible(const civ_camera_t *cam, int win_w, int win_h,
                           float wx, float wy, float radius) {
  float sx, sy;
  civ_camera_world_to_screen(cam, win_w, win_h, wx, wy, &sx, &sy);
  float margin = radius * cam->zoom * WORLD_UNIT_SIZE + 32.0f;
  return sx > -margin && sx < (float)win_w + margin && sy > -margin &&
         sy < (float)win_h + margin;
}
