/**
 * @file camera.h
 * @brief Map camera with smooth zoom/pan and world-to-screen transforms
 */
#ifndef CIV_DISPLAY_CAMERA_H
#define CIV_DISPLAY_CAMERA_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  float x, y;
  float zoom;
  float target_x, target_y;
  float target_zoom;
  float lerp_speed;
  float min_zoom, max_zoom;
  int32_t map_width, map_height;
} civ_camera_t;

void civ_camera_init(civ_camera_t *cam, int32_t map_w, int32_t map_h);
void civ_camera_update(civ_camera_t *cam, float dt);

void civ_camera_pan(civ_camera_t *cam, float dx, float dy);
void civ_camera_zoom(civ_camera_t *cam, float factor, float cx, float cy);

void civ_camera_world_to_screen(const civ_camera_t *cam, int win_w, int win_h,
                                float wx, float wy, float *sx, float *sy);
void civ_camera_screen_to_world(const civ_camera_t *cam, int win_w, int win_h,
                                int sx, int sy, float *wx, float *wy);

bool civ_camera_is_visible(const civ_camera_t *cam, int win_w, int win_h,
                           float wx, float wy, float radius);

#ifdef __cplusplus
}
#endif
#endif
