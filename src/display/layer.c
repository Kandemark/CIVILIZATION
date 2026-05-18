#include "../../include/display/layer.h"
#include <string.h>

void civ_layer_stack_init(civ_layer_stack_t *stack) {
  memset(stack, 0, sizeof(*stack));
  for (int i = 0; i < CIV_LAYER_COUNT; i++) {
    stack->layers[i].id = (civ_layer_id_t)i;
    stack->layers[i].enabled = true;
  }
}

void civ_layer_stack_render(civ_layer_stack_t *stack, SDL_Renderer *r, int w,
                            int h) {
  for (int i = 0; i < CIV_LAYER_COUNT; i++) {
    civ_layer_t *layer = &stack->layers[i];
    if (layer->enabled && layer->render) {
      layer->render(r, w, h, layer->userdata);
    }
  }
}
