/**
 * @file paths.c
 * @brief Asset path resolution — tries multiple base directories
 */
#include "utils/paths.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char g_asset_roots[3][512];
static int  g_asset_root_count = 0;

void civ_path_init(const char *base_path) {
  g_asset_root_count = 0;

  if (base_path && base_path[0]) {
    char buf[512];
    size_t len = strlen(base_path);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, base_path, len);
    buf[len] = '\0';
    if (len > 0 && buf[len - 1] == '/') buf[--len] = '\0';

    /* 1. Exe parent (for dev: build/dominion -> project root) */
    char *slash = strrchr(buf, '/');
    if (slash) {
      *slash = '\0';
      snprintf(g_asset_roots[g_asset_root_count++], 512, "%s", buf);
      *slash = '/';
    }

    /* 2. Exe directory itself */
    snprintf(g_asset_roots[g_asset_root_count++], 512, "%s", buf);
  }

  /* 3. CWD */
  char cwd[512];
  if (getcwd(cwd, sizeof(cwd)))
    snprintf(g_asset_roots[g_asset_root_count++], 512, "%s", cwd);
}

void civ_path_resolve(const char *relative, char *out, size_t out_sz) {
  if (!relative || !out || out_sz == 0) return;

  for (int i = 0; i < g_asset_root_count; i++) {
    snprintf(out, out_sz, "%s/%s", g_asset_roots[i], relative);
    if (access(out, F_OK) == 0) return;
  }

  /* No file found — return CWD-relative path */
  snprintf(out, out_sz, "%s", relative);
}
