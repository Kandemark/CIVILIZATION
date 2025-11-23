/**
 * @file win32_app.c
 * @brief Native Windows GUI Entry Point
 */

#define _CRT_SECURE_NO_WARNINGS
#include "../../include/common.h"
#include "../../include/core/events/game_events.h"
#include "../../include/core/game.h"
#include <stdio.h>
#include <windows.h>

/* Global Game State */
static civ_game_t *g_game = NULL;
static bool g_running = true;

/* Window Dimensions */
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

/* Helper to render text */
void DrawTextLine(HDC hdc, int x, int *y, const char *format, ...) {
  char buffer[1024];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  TextOutA(hdc, x, *y, buffer, (int)strlen(buffer));
  *y += 20; /* Line height */
}

/* Render Game State */
void RenderGame(HDC hdc) {
  if (!g_game)
    return;

  int y = 10;
  int x = 10;

  /* Title */
  SetTextColor(hdc, RGB(0, 0, 128));
  DrawTextLine(hdc, x, &y, "CIVILIZATION SIMULATION (Win32 Native)");
  y += 10;

  /* Date */
  SetTextColor(hdc, RGB(0, 0, 0));
  if (g_game->time_manager) {
    char date_str[64];
    civ_calendar_get_date_string(&g_game->time_manager->calendar, date_str, 64);
    DrawTextLine(hdc, x, &y, "Date: %s", date_str);
  }

  /* Economy */
  y += 10;
  SetTextColor(hdc, RGB(0, 100, 0));
  DrawTextLine(hdc, x, &y, "[ECONOMY]");
  if (g_game->currency_manager &&
      g_game->currency_manager->currency_count > 0) {
    civ_currency_t *c = &g_game->currency_manager->currencies[0];
    DrawTextLine(hdc, x, &y, "Currency: %s (%s)", c->name, c->symbol);
    DrawTextLine(hdc, x, &y, "Rate: %.4f  Inflation: %.2f%%", c->exchange_rate,
                 c->inflation_rate * 100.0f);
  }

  /* Disasters */
  y += 10;
  SetTextColor(hdc, RGB(200, 0, 0));
  DrawTextLine(hdc, x, &y, "[ACTIVE DISASTERS]");
  if (g_game->disaster_manager) {
    int count = 0;
    for (size_t i = 0; i < g_game->disaster_manager->disaster_count; i++) {
      if (g_game->disaster_manager->active_disasters[i].active) {
        DrawTextLine(hdc, x, &y, "! %s (Sev: %.1f)",
                     g_game->disaster_manager->active_disasters[i].name,
                     g_game->disaster_manager->active_disasters[i].severity);
        count++;
      }
    }
    if (count == 0)
      DrawTextLine(hdc, x, &y, "None");
  }

  /* Events */
  y += 10;
  SetTextColor(hdc, RGB(50, 50, 50));
  DrawTextLine(hdc, x, &y, "[LATEST EVENTS]");
  if (g_game->event_log_size > 0) {
    int start = g_game->event_log_size > 10 ? g_game->event_log_size - 10 : 0;
    for (int i = start; i < (int)g_game->event_log_size; i++) {
      DrawTextLine(hdc, x, &y, "- %s: %s", g_game->event_log[i].type,
                   g_game->event_log[i].description);
    }
  }

  /* Controls Help */
  y = WINDOW_HEIGHT - 50;
  SetTextColor(hdc, RGB(100, 100, 100));
  DrawTextLine(
      hdc, x, &y,
      "CONTROLS: Space=Pause/Resume  E=Econ Crisis  D=Disaster  Esc=Quit");
}

/* Window Procedure */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    /* Double buffering could be added here, but simple GDI for now */
    RenderGame(hdc);

    EndPaint(hwnd, &ps);
  } break;

  case WM_KEYDOWN: {
    switch (wParam) {
    case VK_ESCAPE:
      DestroyWindow(hwnd);
      break;
    case VK_SPACE:
      if (g_game) {
        if (g_game->is_paused)
          civ_game_resume(g_game);
        else
          civ_game_pause(g_game);
      }
      break;
    case 'E':
      if (g_game)
        civ_trigger_economic_crisis(g_game);
      break;
    case 'D':
      if (g_game)
        civ_trigger_natural_disaster(g_game, CIV_DISASTER_EARTHQUAKE);
      break;
    }
  } break;

  case WM_DESTROY:
    PostQuitMessage(0);
    g_running = false;
    break;

  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

/* Entry Point */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  /* Register Window Class */
  WNDCLASSEX wc;
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = 0;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = "CivSimWindowClass";
  wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  if (!RegisterClassEx(&wc)) {
    MessageBox(NULL, "Window Registration Failed!", "Error",
               MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }

  /* Create Window */
  HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "CivSimWindowClass",
                             "Civilization Simulation", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH,
                             WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

  if (hwnd == NULL) {
    MessageBox(NULL, "Window Creation Failed!", "Error",
               MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  /* Initialize Game */
  g_game = civ_game_create();
  if (g_game) {
    civ_game_config_t config;
    civ_game_get_default_config(&config);
    civ_game_initialize(g_game, &config);
  } else {
    MessageBox(NULL, "Failed to initialize game engine!", "Error",
               MB_ICONERROR | MB_OK);
    return 1;
  }

  /* Message Loop */
  MSG msg;
  while (g_running) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT)
        break;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else {
      /* Game Loop */
      if (g_game && g_game->is_running) {
        civ_game_update(g_game);

        /* Force repaint */
        InvalidateRect(hwnd, NULL, TRUE);

        /* Small delay to cap framerate roughly */
        Sleep(16);
      }
    }
  }

  /* Cleanup */
  if (g_game) {
    civ_game_shutdown(g_game);
    civ_game_destroy(g_game);
  }

  return msg.wParam;
}
