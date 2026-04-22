/**
 * @file paths.c
 * @brief Asset path resolution implementation
 */
#include "utils/paths.h"
#include <stdio.h>
#include <string.h>

static char g_asset_root[512] = "";

void civ_path_init(const char *base_path) {
  if (!base_path) { g_asset_root[0] = '\0'; return; }
  size_t len = strlen(base_path);
  if (len >= sizeof(g_asset_root)) len = sizeof(g_asset_root) - 1;
  memcpy(g_asset_root, base_path, len);
  g_asset_root[len] = '\0';
  /* Strip trailing slash if present */
  if (len > 0 && g_asset_root[len - 1] == '/')
    g_asset_root[len - 1] = '\0';
}

void civ_path_resolve(const char *relative, char *out, size_t out_sz) {
  if (!relative || !out || out_sz == 0) return;
  if (g_asset_root[0])
    snprintf(out, out_sz, "%s/%s", g_asset_root, relative);
  else
    snprintf(out, out_sz, "%s", relative);
}
