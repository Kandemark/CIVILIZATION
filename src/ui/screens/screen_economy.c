/**
 * @file screen_economy.c
 * @brief Economy screen — global markets, forex, commodities, companies
 *
 * Redesigned with themed sections, per-nation economy header,
 * graph-based data visualization, and consistent spacing.
 */
#include "ui/screens/screens.h"
#include "core/economy/financial_markets.h"
#include "core/world/nation.h"
#include "core/constitution.h"
#include "display/theme.h"
#include "engine/renderer.h"
#include "ui/graph/graph.h"
#include "ui/ui_common.h"
#include <stdio.h>

void civ_screen_economy_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f,
                               int x, int y, int w, int h, int sh,
                               civ_input_state_t *in, const char *cur,
                               const char *sym) {
  (void)cur; (void)sym; (void)sh;
  int dy = y, lx = x + g_theme.space_sm;
  int cw = w - g_theme.space_md;

  /* ── Header: player nation economy ─────────────────────────── */
  civ_font_render_aligned(r, f, "ECONOMY", lx, dy, cw, 24,
      CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  dy += 28;

  /* Show player nation economic data if available */
  if (g->nation_manager) {
    civ_nation_manager_t *nm = (civ_nation_manager_t *)g->nation_manager;
    int pi = nm->player_nation_index;
    if (pi >= 0 && pi < nm->count) {
      civ_nation_t *pn = &nm->nations[pi];
      char buf[192];
      snprintf(buf, sizeof(buf),
          "%s  |  GDP: $%.0fM  +%.1f%%  |  Unemp: %.1f%%  |  Infl: %.1f%%",
          pn->name, pn->economy.gdp, pn->economy.gdp_growth * 100.0f,
          pn->economy.unemployment * 100.0f, pn->economy.inflation * 100.0f);
      civ_font_render_aligned(r, f, buf, lx, dy, cw, 16,
          g_theme.text_secondary, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      dy += 22;
    }
  }

  if (!g->market) {
    civ_font_render_aligned(r, f, "Market data unavailable", lx, dy, cw, 20,
        g_theme.text_dim, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    return;
  }

  civ_market_engine_t *mkt = g->market;
  char buf[256];

  /* ── Overview bar ──────────────────────────────────────────── */
  {
    float avg_inf = 0;
    for (int i = 0; i < 5 && i < mkt->currency_count; i++)
      avg_inf += mkt->currencies[i].inflation;
    avg_inf /= 5.0f;

    civ_render_rect_filled_alpha(r, lx, dy, cw, 30,
        g_theme.bg_dark, 200);
    snprintf(buf, sizeof(buf),
        "Inflation: %.1f%%  |  Commodities: %d  |  Currencies: %d  |  Companies: %d",
        avg_inf * 100.0f, mkt->commodity_count,
        mkt->currency_count, mkt->company_count);
    civ_font_render_aligned(r, f, buf, lx + 10, dy + 7, cw - 20, 16,
        g_theme.warning, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
    dy += 36;
  }

  /* ── FOREX ─────────────────────────────────────────────────── */
  {
    civ_render_line(r, lx, dy, lx + cw, dy, g_theme.hud_border);
    dy += g_theme.space_sm;
    civ_font_render_aligned(r, f, "FOREX", lx, dy, 160, 16,
        g_theme.warning, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    civ_font_render_aligned(r, f, "1 USD", lx + 60, dy, 80, 16,
        g_theme.text_dim, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    dy += 20;

    /* Column headers */
    civ_font_render_aligned(r, f, "ISO", lx, dy, 36, 12,
        g_theme.text_dim, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    civ_font_render_aligned(r, f, "Rate", lx + 170, dy, 80, 12,
        g_theme.text_dim, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    civ_font_render_aligned(r, f, "Change", lx + 250, dy, 70, 12,
        g_theme.text_dim, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    civ_font_render_aligned(r, f, "Name", lx + 320, dy, cw - 320, 12,
        g_theme.text_dim, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    dy += 14;

    int fx[] = {0, 1, 2, 3, 4, 11, 13, 16, 18, 19,
                5, 6, 7, 8, 9, 10, 12, 14, 15, 17};
    for (int ci = 0; ci < 20 && dy < y + h - 200; ci++) {
      int idx = fx[ci];
      if (idx >= mkt->currency_count) continue;
      civ_market_currency_t *mc = &mkt->currencies[idx];

      float tr[] = { mc->current_rate * 0.97f, mc->current_rate * 0.99f,
                     mc->current_rate, mc->current_rate * 1.02f,
                     mc->current_rate * 1.01f, mc->current_rate * 0.98f };

      snprintf(buf, sizeof(buf), "%-4s", mc->iso);
      civ_font_render_aligned(r, f, buf, lx, dy, 36, 14,
          g_theme.text_secondary, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

      SDL_Texture *sp = civ_graph_sparkline(r, lx + 40, dy, 120, 12,
          tr, 6, mc->current_rate > mc->base_rate ? g_theme.success
                                                   : g_theme.danger);
      if (sp) {
        SDL_FRect dr = { (float)(lx + 40), (float)dy, 120.0f, 12.0f };
        SDL_RenderTexture(r, sp, NULL, &dr);
        SDL_DestroyTexture(sp);
      }

      snprintf(buf, sizeof(buf), "%10.4f %+6.1f%% %s",
          mc->current_rate,
          (mc->current_rate / mc->base_rate - 1.0f) * 100.0f,
          mc->name);
      civ_font_render_aligned(r, f, buf, lx + 168, dy, cw - 170, 14,
          g_theme.text_dim, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      dy += 16;
    }
  }

  /* ── Companies ──────────────────────────────────────────────── */
  dy += g_theme.space_sm;
  civ_render_line(r, lx, dy, lx + cw, dy, g_theme.hud_border);
  dy += g_theme.space_sm;
  civ_font_render_aligned(r, f, "STOCK MARKET", lx, dy, 200, 16,
      g_theme.warning, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  dy += 22;

  int listed = 0;
  for (int ci = 0; ci < mkt->company_count && listed < 8; ci++) {
    civ_company_t *co = civ_market_get_company(mkt, ci);
    if (!co || !co->is_public) continue;

    bool hov = civ_input_is_mouse_over(in, lx, dy, cw, 28);
    civ_render_rect_filled_alpha(r, lx, dy, cw, 28,
        hov ? g_theme.bg_light : g_theme.bg_dark, 200);
    civ_render_rect_outline(r, lx, dy, cw, 28,
        g_theme.hud_border, 1);

    snprintf(buf, sizeof(buf), "%-20s %-12s Stock: $%.2f  Emp: %d  Rev: $%.1fM  Gr: %+.0f%%",
        co->name, co->industry, co->stock_price,
        co->employees, co->revenue / 1000000.0f, co->growth_rate * 100.0f);
    civ_font_render_aligned(r, f, buf, lx + g_theme.space_sm, dy, cw - g_theme.space_md,
        16, hov ? g_theme.text_primary : g_theme.hud_text,
        CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
    dy += 32;
    listed++;
  }

  /* ── Commodities ────────────────────────────────────────────── */
  dy += g_theme.space_sm;
  civ_render_line(r, lx, dy, lx + cw, dy, g_theme.hud_border);
  dy += g_theme.space_sm;
  civ_font_render_aligned(r, f, "COMMODITIES", lx, dy, 200, 16,
      g_theme.warning, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  dy += 22;

  if (mkt->commodity_count > 0) {
    civ_graph_bar_t bars[12];
    float maxp = 0;
    for (int i = 0; i < mkt->commodity_count && i < 12; i++) {
      bars[i].label = mkt->commodities[i].name;
      bars[i].value = mkt->commodities[i].price_per_unit;
      bars[i].color = g_graph_palette_default[i % 12];
      if (bars[i].value > maxp) maxp = bars[i].value;
    }
    for (int i = 0; i < mkt->commodity_count && i < 12; i++)
      bars[i].value = bars[i].value / (maxp > 0 ? maxp : 1.0f);

    civ_graph_ctx_t gctx;
    civ_graph_ctx_init(&gctx, lx, dy, cw, 90);
    gctx.show_grid = false;
    gctx.x_max = (float)mkt->commodity_count;
    SDL_Texture *bt = civ_graph_bar(r, &gctx, bars,
        mkt->commodity_count < 12 ? mkt->commodity_count : 12, false);
    if (bt) {
      SDL_FRect dr = { (float)lx, (float)dy, (float)cw, 90.0f };
      SDL_RenderTexture(r, bt, NULL, &dr);
      SDL_DestroyTexture(bt);
    }
    dy += 100;

    /* Price list */
    for (int i = 0; i < mkt->commodity_count && i < 12; i++) {
      snprintf(buf, sizeof(buf), "%-14s $%8.1f %s",
          mkt->commodities[i].name,
          mkt->commodities[i].price_per_unit,
          mkt->commodities[i].unit);
      int col = i % 2;
      civ_font_render_aligned(r, f, buf, lx + col * (cw / 2), dy,
          cw / 2 - g_theme.space_sm, 14,
          g_theme.text_dim, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      if (col == 1) dy += 16;
    }
  }

  (void)in;
}
