/*
 * @file win32_app.c
 * @brief Native Windows GUI with customizable navigation and professional map
 * rendering.
 */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../../include/common.h"
#include "../../include/core/events/game_events.h"
#include "../../include/core/game.h"
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>

/* -------------------------------------------------------------------------- */
/* Global state --------------------------------------------------------------
 */
static civ_game_t *g_game = NULL;
static bool g_running = true;

/* Window dimensions ----------------------------------------------------------
 */
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 800

/* Color scheme --------------------------------------------------------------
 */
#define COLOR_PRIMARY RGB(70, 130, 220)
#define COLOR_PRIMARY_DARK RGB(50, 110, 190)
#define COLOR_SUCCESS RGB(34, 139, 34)
#define COLOR_TEXT RGB(30, 30, 40)
#define COLOR_TEXT_LIGHT RGB(100, 100, 110)
#define COLOR_BORDER RGB(200, 200, 210)
#define COLOR_NAV_BG RGB(240, 240, 245)
#define COLOR_NAV_HOVER RGB(220, 230, 250)
#define COLOR_NAV_ACTIVE RGB(200, 220, 255)
#define COLOR_BG_PANEL RGB(250, 250, 250)
#define COLOR_BG RGB(255, 255, 255)

/* -------------------------------------------------------------------------- */
/* UI enums ------------------------------------------------------------------
 */
typedef enum {
  TAB_DASHBOARD = 0,
  TAB_MAP,
  TAB_ECONOMY,
  TAB_POPULATION,
  TAB_TECHNOLOGY,
  TAB_MILITARY,
  TAB_DIPLOMACY,
  TAB_POLITICS,
  TAB_ENVIRONMENT,
  TAB_SETTINGS,
  TAB_COUNT
} UITab;

typedef enum {
  NAV_POS_LEFT,
  NAV_POS_TOP,
  NAV_POS_RIGHT,
  NAV_POS_BOTTOM
} NavPosition;

typedef enum { MAP_MODE_TERRAIN = 0, MAP_MODE_POLITICAL } MapMode;

/* UI settings ---------------------------------------------------------------
 */
typedef struct {
  NavPosition nav_position;
  bool show_minimap;
  int nav_width;    /* for side navigation */
  int nav_height;   /* for top/bottom navigation */
  MapMode map_mode; /* current map rendering mode */
} UISettings;
static UISettings g_settings = {NAV_POS_LEFT, true, 200, 50, MAP_MODE_TERRAIN};

/* UI state ------------------------------------------------------------------
 */
typedef struct {
  UITab active_tab;
  int hover_tab;
  int hover_group;
  bool needs_redraw;
  RECT nav_rect;
  RECT content_rect;
} UIState;
static UIState g_ui_state = {TAB_DASHBOARD, -1, -1, true, {0}, {0}};

/* Navigation groups ----------------------------------------------------------
 */
typedef struct {
  const char *name;
  UITab tabs[5];
  int tab_count;
  bool expanded;
} NavGroup;
#define GROUP_COUNT 5
static NavGroup g_nav_groups[GROUP_COUNT] = {
    {"Dashboard", {TAB_DASHBOARD}, 1, true},
    {"World", {TAB_MAP}, 1, true},
    {"Empire", {TAB_ECONOMY, TAB_POPULATION, TAB_TECHNOLOGY}, 3, true},
    {"State", {TAB_MILITARY, TAB_DIPLOMACY, TAB_POLITICS}, 3, true},
    {"System", {TAB_ENVIRONMENT, TAB_SETTINGS}, 2, true}};

static const char *TAB_NAMES[TAB_COUNT] = {
    "Dashboard", "World Map", "Economy",  "Population",  "Technology",
    "Military",  "Diplomacy", "Politics", "Environment", "Settings"};

/* Double buffering ----------------------------------------------------------
 */
static HDC g_backbuffer_dc = NULL;
static HBITMAP g_backbuffer_bitmap = NULL;
static HBITMAP g_old_bitmap = NULL;

/* Fonts ---------------------------------------------------------------------
 */
static HFONT g_font_header = NULL;
static HFONT g_font_body = NULL;
static HFONT g_font_small = NULL;
static HFONT g_font_bold = NULL;
static HFONT g_font_group = NULL;

/* -------------------------------------------------------------------------- */
/* Helper functions -----------------------------------------------------------
 */
static void DrawTextLine(HDC hdc, int x, int *y, HFONT font, const char *fmt,
                         ...) {
  char buffer[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  HGDIOBJ old = SelectObject(hdc, font);
  TextOutA(hdc, x, *y, buffer, (int)strlen(buffer));
  SelectObject(hdc, old);
  *y += 22;
}

static void DrawGradient(HDC hdc, RECT rect, COLORREF c1, COLORREF c2,
                         bool vertical) {
  int r1 = GetRValue(c1), g1 = GetGValue(c1), b1 = GetBValue(c1);
  int r2 = GetRValue(c2), g2 = GetGValue(c2), b2 = GetBValue(c2);
  int w = rect.right - rect.left;
  int h = rect.bottom - rect.top;
  if (vertical) {
    for (int i = 0; i < h; ++i) {
      int r = r1 + (r2 - r1) * i / h;
      int g = g1 + (g2 - g1) * i / h;
      int b = b1 + (b2 - b1) * i / h;
      HBRUSH br = CreateSolidBrush(RGB(r, g, b));
      RECT line = {rect.left, rect.top + i, rect.right, rect.top + i + 1};
      FillRect(hdc, &line, br);
      DeleteObject(br);
    }
  } else {
    for (int i = 0; i < w; ++i) {
      int r = r1 + (r2 - r1) * i / w;
      int g = g1 + (g2 - g1) * i / w;
      int b = b1 + (b2 - b1) * i / w;
      HBRUSH br = CreateSolidBrush(RGB(r, g, b));
      RECT line = {rect.left + i, rect.top, rect.left + i + 1, rect.bottom};
      FillRect(hdc, &line, br);
      DeleteObject(br);
    }
  }
}

static void InitializeBackBuffer(HWND hwnd, int width, int height) {
  HDC hdc = GetDC(hwnd);
  if (g_backbuffer_dc) {
    SelectObject(g_backbuffer_dc, g_old_bitmap);
    DeleteObject(g_backbuffer_bitmap);
    DeleteDC(g_backbuffer_dc);
  }
  g_backbuffer_dc = CreateCompatibleDC(hdc);
  g_backbuffer_bitmap = CreateCompatibleBitmap(hdc, width, height);
  g_old_bitmap = (HBITMAP)SelectObject(g_backbuffer_dc, g_backbuffer_bitmap);
  ReleaseDC(hwnd, hdc);
}

void CleanupBackBuffer(void) {
  if (g_backbuffer_dc) {
    SelectObject(g_backbuffer_dc, g_old_bitmap);
    DeleteObject(g_backbuffer_bitmap);
    DeleteDC(g_backbuffer_dc);
    g_backbuffer_dc = NULL;
    g_backbuffer_bitmap = NULL;
  }
  if (g_font_header)
    DeleteObject(g_font_header);
  if (g_font_body)
    DeleteObject(g_font_body);
  if (g_font_small)
    DeleteObject(g_font_small);
  if (g_font_bold)
    DeleteObject(g_font_bold);
  if (g_font_group)
    DeleteObject(g_font_group);
}

static void InitializeFonts(void) {
  g_font_header =
      CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                 DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
  g_font_body =
      CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                 DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
  g_font_small =
      CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                 DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
  g_font_bold =
      CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                 DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
  g_font_group =
      CreateFont(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                 DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
}

static void CalculateLayout(int width, int height) {
  switch (g_settings.nav_position) {
  case NAV_POS_LEFT:
    g_ui_state.nav_rect = (RECT){0, 0, g_settings.nav_width, height};
    g_ui_state.content_rect = (RECT){g_settings.nav_width, 0, width, height};
    break;
  case NAV_POS_RIGHT:
    g_ui_state.content_rect =
        (RECT){0, 0, width - g_settings.nav_width, height};
    g_ui_state.nav_rect =
        (RECT){width - g_settings.nav_width, 0, width, height};
    break;
  case NAV_POS_TOP:
    g_ui_state.nav_rect = (RECT){0, 0, width, g_settings.nav_height};
    g_ui_state.content_rect = (RECT){0, g_settings.nav_height, width, height};
    break;
  case NAV_POS_BOTTOM:
    g_ui_state.content_rect =
        (RECT){0, 0, width, height - g_settings.nav_height};
    g_ui_state.nav_rect =
        (RECT){0, height - g_settings.nav_height, width, height};
    break;
  }
}

/* -------------------------------------------------------------------------- */
/* Rendering -----------------------------------------------------------------
 */
static void RenderNavigation(HDC hdc) {
  DrawGradient(hdc, g_ui_state.nav_rect, COLOR_NAV_BG, RGB(230, 230, 235),
               g_settings.nav_position == NAV_POS_LEFT ||
                   g_settings.nav_position == NAV_POS_RIGHT);
  HPEN pen = CreatePen(PS_SOLID, 1, COLOR_BORDER);
  HGDIOBJ oldPen = SelectObject(hdc, pen);
  // draw border depending on orientation
  if (g_settings.nav_position == NAV_POS_LEFT) {
    MoveToEx(hdc, g_ui_state.nav_rect.right - 1, 0, NULL);
    LineTo(hdc, g_ui_state.nav_rect.right - 1, g_ui_state.nav_rect.bottom);
  } else if (g_settings.nav_position == NAV_POS_RIGHT) {
    MoveToEx(hdc, g_ui_state.nav_rect.left, 0, NULL);
    LineTo(hdc, g_ui_state.nav_rect.left, g_ui_state.nav_rect.bottom);
  } else if (g_settings.nav_position == NAV_POS_TOP) {
    MoveToEx(hdc, 0, g_ui_state.nav_rect.bottom - 1, NULL);
    LineTo(hdc, g_ui_state.nav_rect.right, g_ui_state.nav_rect.bottom - 1);
  } else { // bottom
    MoveToEx(hdc, 0, g_ui_state.nav_rect.top, NULL);
    LineTo(hdc, g_ui_state.nav_rect.right, g_ui_state.nav_rect.top);
  }
  SelectObject(hdc, oldPen);
  DeleteObject(pen);

  bool vertical = (g_settings.nav_position == NAV_POS_LEFT ||
                   g_settings.nav_position == NAV_POS_RIGHT);
  int x = g_ui_state.nav_rect.left + 10;
  int y = g_ui_state.nav_rect.top + 10;
  int item_h = 30;
  int group_h = 25;
  if (vertical) {
    for (int g = 0; g < GROUP_COUNT; ++g) {
      SetTextColor(hdc, COLOR_TEXT_LIGHT);
      SetBkMode(hdc, TRANSPARENT);
      HGDIOBJ old = SelectObject(hdc, g_font_group);
      TextOutA(hdc, x, y, g_nav_groups[g].name,
               (int)strlen(g_nav_groups[g].name));
      SelectObject(hdc, old);
      y += group_h;
      for (int t = 0; t < g_nav_groups[g].tab_count; ++t) {
        UITab tab = g_nav_groups[g].tabs[t];
        RECT tabRect = {x, y, g_ui_state.nav_rect.right - 10, y + item_h};
        COLORREF bg = TRANSPARENT, fg = COLOR_TEXT;
        if (tab == g_ui_state.active_tab) {
          bg = COLOR_NAV_ACTIVE;
          fg = COLOR_PRIMARY_DARK;
        } else if (tab == g_ui_state.hover_tab) {
          bg = COLOR_NAV_HOVER;
          fg = COLOR_PRIMARY;
        }
        if (bg != TRANSPARENT) {
          HBRUSH br = CreateSolidBrush(bg);
          FillRect(hdc, &tabRect, br);
          DeleteObject(br);
        }
        SetTextColor(hdc, fg);
        HGDIOBJ oldF = SelectObject(hdc, g_font_body);
        DrawTextA(hdc, TAB_NAMES[tab], -1, &tabRect,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, oldF);
        y += item_h + 2;
      }
      y += 10; // spacing between groups
    }
  } else { // horizontal layout
    int tab_w = 120;
    x = 10;
    y = g_ui_state.nav_rect.top + (g_settings.nav_height - item_h) / 2;
    for (int i = 0; i < TAB_COUNT; ++i) {
      RECT tabRect = {x, y, x + tab_w, y + item_h};
      COLORREF bg = TRANSPARENT, fg = COLOR_TEXT;
      if (i == g_ui_state.active_tab) {
        bg = COLOR_NAV_ACTIVE;
        fg = COLOR_PRIMARY_DARK;
      } else if (i == g_ui_state.hover_tab) {
        bg = COLOR_NAV_HOVER;
        fg = COLOR_PRIMARY;
      }
      if (bg != TRANSPARENT) {
        HBRUSH br = CreateSolidBrush(bg);
        FillRect(hdc, &tabRect, br);
        DeleteObject(br);
      }
      SetTextColor(hdc, fg);
      HGDIOBJ oldF = SelectObject(hdc, g_font_body);
      DrawTextA(hdc, TAB_NAMES[i], -1, &tabRect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      SelectObject(hdc, oldF);
      x += tab_w + 5;
    }
  }
}

static void RenderMapPanel(HDC hdc, RECT rect) {
  // ocean background
  HBRUSH ocean = CreateSolidBrush(RGB(100, 150, 220));
  FillRect(hdc, &rect, ocean);
  DeleteObject(ocean);
  if (!g_game || !g_game->world_map)
    return;
  civ_map_t *map = g_game->world_map;
  int mapW = map->width, mapH = map->height;
  int rectW = rect.right - rect.left, rectH = rect.bottom - rect.top;
  int tileSizeX = rectW / mapW;
  int tileSizeY = rectH / mapH;
  int tileSize = (tileSizeX < tileSizeY) ? tileSizeX : tileSizeY;
  if (tileSize < 1)
    tileSize = 1;
  int offsetX = rect.left + (rectW - tileSize * mapW) / 2;
  int offsetY = rect.top + (rectH - tileSize * mapH) / 2;
  for (int y = 0; y < mapH; ++y) {
    for (int x = 0; x < mapW; ++x) {
      civ_map_tile_t *tile = civ_map_get_tile(map, x, y);
      if (!tile)
        continue;
      COLORREF color;
      if (g_settings.map_mode == MAP_MODE_TERRAIN) {
        if (tile->elevation < map->sea_level) {
          color = RGB(100, 150, 220); // water
        } else {
          switch (tile->terrain) {
          case CIV_TERRAIN_MOUNTAIN:
            color = RGB(180, 180, 180);
            break;
          case CIV_TERRAIN_HILL:
            color = RGB(120, 180, 120);
            break;
          case CIV_TERRAIN_PLAIN:
            color = RGB(180, 210, 140);
            break;
          case CIV_TERRAIN_VALLEY:
            color = RGB(150, 190, 120);
            break;
          case CIV_TERRAIN_PLATEAU:
            color = RGB(160, 170, 150);
            break;
          case CIV_TERRAIN_COASTAL:
            color = RGB(200, 200, 150);
            break;
          default:
            color = RGB(170, 200, 130);
            break;
          }
        }
      } else { // political mode – use land_use as placeholder owner color
        switch (tile->land_use) {
        case CIV_LAND_USE_FOREST:
          color = RGB(34, 139, 34);
          break;
        case CIV_LAND_USE_AGRICULTURE:
          color = RGB(85, 107, 47);
          break;
        case CIV_LAND_USE_URBAN:
          color = RGB(139, 69, 19);
          break;
        case CIV_LAND_USE_WETLAND:
          color = RGB(0, 128, 128);
          break;
        case CIV_LAND_USE_GRASSLAND:
          color = RGB(124, 252, 0);
          break;
        case CIV_LAND_USE_DESERT:
          color = RGB(210, 180, 140);
          break;
        case CIV_LAND_USE_WATER:
          color = RGB(70, 130, 180);
          break;
        case CIV_LAND_USE_TUNDRA:
          color = RGB(176, 196, 222);
          break;
        default:
          color = RGB(190, 190, 190);
          break;
        }
      }
      HBRUSH tileBrush = CreateSolidBrush(color);
      RECT tileRect = {offsetX + x * tileSize, offsetY + y * tileSize,
                       offsetX + (x + 1) * tileSize,
                       offsetY + (y + 1) * tileSize};
      FillRect(hdc, &tileRect, tileBrush);
      DeleteObject(tileBrush);
    }
  }
}

static void RenderSettingsPanel(HDC hdc, RECT rect) {
  int x = rect.left + 40;
  int y = rect.top + 40;
  SetTextColor(hdc, COLOR_PRIMARY);
  DrawTextLine(hdc, x, &y, g_font_header, "UI Customization");
  y += 20;
  SetTextColor(hdc, COLOR_TEXT);
  DrawTextLine(hdc, x, &y, g_font_bold, "Navigation Position:");
  y += 10;
  const char *posStr[4] = {"Left (Side)", "Top (Tabs)", "Right (Side)",
                           "Bottom (Dock)"};
  for (int i = 0; i < 4; ++i) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%s %s",
             (g_settings.nav_position == i) ? "(Selected)" : "[ ]", posStr[i]);
    SetTextColor(hdc,
                 (g_settings.nav_position == i) ? COLOR_SUCCESS : COLOR_TEXT);
    DrawTextLine(hdc, x + 20, &y, g_font_body, "%s", buf);
  }
  y += 20;
  SetTextColor(hdc, COLOR_TEXT);
  DrawTextLine(hdc, x, &y, g_font_bold, "Map Mode:");
  y += 10;
  const char *modeStr[2] = {"Terrain", "Political"};
  for (int i = 0; i < 2; ++i) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%s %s",
             (g_settings.map_mode == i) ? "(Selected)" : "[ ]", modeStr[i]);
    SetTextColor(hdc, (g_settings.map_mode == i) ? COLOR_SUCCESS : COLOR_TEXT);
    DrawTextLine(hdc, x + 20, &y, g_font_body, "%s", buf);
  }
  y += 20;
  SetTextColor(hdc, COLOR_TEXT_LIGHT);
  DrawTextLine(hdc, x, &y, g_font_small,
               "Press 'L' to cycle layout, 'M' to toggle map mode");
}

static void RenderContent(HDC hdc) {
  RECT rc = g_ui_state.content_rect;
  HBRUSH bg = CreateSolidBrush(COLOR_BG_PANEL);
  FillRect(hdc, &rc, bg);
  DeleteObject(bg);
  switch (g_ui_state.active_tab) {
  case TAB_MAP:
    RenderMapPanel(hdc, rc);
    break;
  case TAB_SETTINGS:
    RenderSettingsPanel(hdc, rc);
    break;
  default:
    SetTextColor(hdc, COLOR_PRIMARY);
    int x = rc.left + 20, y = rc.top + 20;
    DrawTextLine(hdc, x, &y, g_font_header, "%s",
                 TAB_NAMES[g_ui_state.active_tab]);
    SetTextColor(hdc, COLOR_TEXT);
    DrawTextLine(hdc, x, &y, g_font_body, "Content for %s",
                 TAB_NAMES[g_ui_state.active_tab]);
    break;
  }
}

static void HandleClick(int x, int y) {
  if (PtInRect(&g_ui_state.nav_rect, (POINT){x, y})) {
    bool vertical = (g_settings.nav_position == NAV_POS_LEFT ||
                     g_settings.nav_position == NAV_POS_RIGHT);
    if (vertical) {
      int curY = g_ui_state.nav_rect.top + 10;
      int item_h = 30, group_h = 25;
      for (int g = 0; g < GROUP_COUNT; ++g) {
        curY += group_h; // header
        for (int t = 0; t < g_nav_groups[g].tab_count; ++t) {
          if (y >= curY && y < curY + item_h) {
            g_ui_state.active_tab = g_nav_groups[g].tabs[t];
            g_ui_state.needs_redraw = true;
            return;
          }
          curY += item_h + 2;
        }
        curY += 10;
      }
    } else {
      int tab_w = 120;
      int curX = 10;
      int nav_y = g_ui_state.nav_rect.top + (g_settings.nav_height - 30) / 2;
      if (y >= nav_y && y < nav_y + 30) {
        int idx = (x - curX) / (tab_w + 5);
        if (idx >= 0 && idx < TAB_COUNT) {
          g_ui_state.active_tab = (UITab)idx;
          g_ui_state.needs_redraw = true;
        }
      }
    }
  }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam,
                                LPARAM lParam) {
  switch (msg) {
  case WM_CREATE: {
    RECT rc;
    GetClientRect(hwnd, &rc);
    InitializeBackBuffer(hwnd, rc.right, rc.bottom);
    InitializeFonts();
    CalculateLayout(rc.right, rc.bottom);
  } break;
  case WM_SIZE: {
    int w = LOWORD(lParam), h = HIWORD(lParam);
    InitializeBackBuffer(hwnd, w, h);
    CalculateLayout(w, h);
    g_ui_state.needs_redraw = true;
  } break;
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    if (g_ui_state.needs_redraw && g_backbuffer_dc) {
      // clear backbuffer
      RECT full = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
      HBRUSH bg = CreateSolidBrush(COLOR_BG);
      FillRect(g_backbuffer_dc, &full, bg);
      DeleteObject(bg);
      RenderNavigation(g_backbuffer_dc);
      RenderContent(g_backbuffer_dc);
      g_ui_state.needs_redraw = false;
    }
    if (g_backbuffer_dc) {
      RECT client;
      GetClientRect(hwnd, &client);
      BitBlt(hdc, 0, 0, client.right, client.bottom, g_backbuffer_dc, 0, 0,
             SRCCOPY);
    }
    EndPaint(hwnd, &ps);
  } break;
  case WM_LBUTTONDOWN: {
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);
    HandleClick(x, y);
    if (g_ui_state.needs_redraw)
      InvalidateRect(hwnd, NULL, FALSE);
  } break;
  case WM_KEYDOWN: {
    bool redraw = false;
    switch (wParam) {
    case 'L':
    case 'l':
      g_settings.nav_position =
          (NavPosition)((g_settings.nav_position + 1) % 4);
      {
        RECT rc;
        GetClientRect(hwnd, &rc);
        CalculateLayout(rc.right, rc.bottom);
      }
      redraw = true;
      break;
    case 'M':
    case 'm':
      g_settings.map_mode = (MapMode)((g_settings.map_mode + 1) % 2);
      redraw = true;
      break;
    case VK_ESCAPE:
      DestroyWindow(hwnd);
      break;
    }
    if (redraw) {
      g_ui_state.needs_redraw = true;
      InvalidateRect(hwnd, NULL, FALSE);
    }
  } break;
  case WM_DESTROY:
    CleanupBackBuffer();
    PostQuitMessage(0);
    g_running = false;
    break;
  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int nShow) {
  WNDCLASSEX wc = {0};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInst;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszClassName = "CivSimNavClass";
  if (!RegisterClassEx(&wc))
    return 0;
  HWND hwnd =
      CreateWindowEx(WS_EX_CLIENTEDGE, "CivSimNavClass",
                     "Civilization Simulation - Customizable UI",
                     WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                     WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInst, NULL);
  if (!hwnd)
    return 0;
  ShowWindow(hwnd, nShow);
  UpdateWindow(hwnd);
  // Initialize game core
  g_game = civ_game_create();
  if (g_game) {
    civ_game_config_t cfg;
    civ_game_get_default_config(&cfg);
    civ_game_initialize(g_game, &cfg);
  }
  MSG msg;
  while (g_running) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT)
        break;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else {
      Sleep(10);
    }
  }
  if (g_game) {
    civ_game_shutdown(g_game);
    civ_game_destroy(g_game);
  }
  return (int)msg.wParam;
}
