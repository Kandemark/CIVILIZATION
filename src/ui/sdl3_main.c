/**
 * @file sdl3_main.c
 * @brief Single entry point delegating lifecycle to app controller.
 */

#include "../../include/ui/app_controller.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  civ_app_controller_t app;
  civ_result_t init_result = civ_app_controller_init(&app, argc, argv);
  if (CIV_FAILED(init_result)) {
    fprintf(stderr, "Application initialization failed: %s\n",
            init_result.message ? init_result.message : "Unknown error");
    return 1;
  }

  civ_app_controller_run(&app);
  civ_app_controller_shutdown(&app);
  return 0;
}
