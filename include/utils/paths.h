/**
 * @file paths.h
 * @brief Asset path resolution from executable base directory
 *
 * At startup, civ_path_init() stores the executable's directory.
 * civ_path_resolve() builds absolute paths from relative asset paths.
 * No process-level side effects — uses SDL_GetBasePath once at init.
 */
#ifndef CIV_UTILS_PATHS_H
#define CIV_UTILS_PATHS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Call once at startup with SDL_GetBasePath() result */
void civ_path_init(const char *base_path);

/* Resolve a relative asset path to an absolute filesystem path.
   Writes at most out_sz bytes to out. Safe to call before SDL_Init. */
void civ_path_resolve(const char *relative, char *out, size_t out_sz);

#ifdef __cplusplus
}
#endif
#endif
