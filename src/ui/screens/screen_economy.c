#include "../../../include/ui/screens/screens.h"
#include "../../../include/core/constitution.h"
#include "../../../include/core/npc_engine.h"
#include "../../../include/display/theme.h"
#include "../../../include/engine/renderer.h"
#include "../../../include/ui/ui_common.h"
#include "../../../include/ui/graph/graph.h"
#include "../../../include/ui/ui_common.h"
#include <stdio.h>

void civ_screen_economy_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f,
                               int x, int y, int w, int h, int sh,
                               civ_input_state_t *in, const char *cur, const char *sym) {
  (void)cur; (void)sym;
  int dy = y, lx = x + 4;
  civ_font_render_aligned(r, f, "GLOBAL ECONOMY", lx, dy, w-8, 22, CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy+=26;
  if (!g->market) { civ_font_render_aligned(r, f, "Market data unavailable", lx, dy, w-8, 20, 0x556677, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); return; }
  civ_market_engine_t *mkt = (civ_market_engine_t *)g->market;
  char buf[192];
  /* Overview bar */
  civ_render_rect_filled_alpha(r, lx, dy, w-8, 28, 0x080C18, 200);
  float avg_inf = 0; for (int i=0; i<5; i++) avg_inf += mkt->currencies[i].inflation; avg_inf /= 5;
  snprintf(buf, sizeof(buf), "Inflation: %.1f%% | Commodities: %d | Forex: %d pairs | Companies: %d",
           avg_inf*100, mkt->commodity_count, mkt->currency_count, mkt->company_count);
  civ_font_render_aligned(r, f, buf, lx+10, dy+6, w-20, 16, 0xFFCC00, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE); dy+=34;

  /* Forex */
  civ_font_render_aligned(r, f, "FOREX — 1 USD", lx, dy, 200, 16, 0xFFCC00, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy+=18;
  int fx[] = {0,1,2,3,4,11,13,16,18,19,5,6,7,8,9,10,12,14,15,17};
  for (int ci=0; ci<20 && dy<sh-280; ci++) { int idx=fx[ci]; if(idx>=mkt->currency_count)continue; civ_market_currency_t*mc=&mkt->currencies[idx];
    float tr[]={mc->current_rate*0.97f,mc->current_rate*0.99f,mc->current_rate,mc->current_rate*1.02f,mc->current_rate*1.01f,mc->current_rate*0.98f};
    snprintf(buf,sizeof(buf),"%-4s",mc->iso); civ_font_render_aligned(r,f,buf,lx,dy,40,14,0xAABBCC,CIV_ALIGN_LEFT,CIV_VALIGN_TOP);
    SDL_Texture*sp=civ_graph_sparkline(r,lx+44,dy,120,12,tr,6,mc->current_rate>mc->base_rate?0x44FF44:0xFF6644);
    if(sp){SDL_FRect dr={(float)(lx+44),(float)dy,120,12};SDL_RenderTexture(r,sp,NULL,&dr);SDL_DestroyTexture(sp);}
    snprintf(buf,sizeof(buf),"%10.4f %+.1f%% %s",mc->current_rate,(mc->current_rate/mc->base_rate-1)*100,mc->name);
    civ_font_render_aligned(r,f,buf,lx+170,dy,w-178,14,0x667788,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=16; }

  /* Stock Market */
  dy+=6; civ_render_line(r,lx,dy,lx+w-8,dy,0x1A2A3A); dy+=8;
  civ_font_render_aligned(r,f,"STOCK MARKET",lx,dy,200,16,0xFFCC00,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=20;
  int listed=0;
  for (int ci=0; ci<mkt->company_count && listed<8; ci++) { civ_company_t*co=civ_market_get_company(mkt,ci);
    if(!co||!co->is_public)continue;
    bool hov=civ_input_is_mouse_over(in,lx,dy,w-8,30); civ_render_rect_filled_alpha(r,lx,dy,w-8,28,hov?0x162033:0x080C18,200);
    snprintf(buf,sizeof(buf),"%-20s %-12s Stock:%.2f Emp:%d Rev:%.1fM Gr:%+.0f%%",co->name,co->industry,co->stock_price,co->employees,co->revenue/1000000,co->growth_rate*100);
    civ_font_render_aligned(r,f,buf,lx+8,dy,w-16,16,hov?0xFFFFFF:0x8899AA,CIV_ALIGN_LEFT,CIV_VALIGN_MIDDLE); dy+=32; listed++; }

  /* Commodities */
  dy+=6; civ_render_line(r,lx,dy,lx+w-8,dy,0x1A2A3A); dy+=8;
  civ_font_render_aligned(r,f,"COMMODITIES",lx,dy,200,16,0xFFCC00,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=20;
  civ_graph_bar_t bars[12]; float maxp=0;
  for(int i=0;i<mkt->commodity_count;i++){bars[i].label=mkt->commodities[i].name;bars[i].value=mkt->commodities[i].price_per_unit;bars[i].color=g_graph_palette_default[i%12];if(bars[i].value>maxp)maxp=bars[i].value;}
  for(int i=0;i<mkt->commodity_count;i++)bars[i].value=bars[i].value/(maxp>0?maxp:1);
  civ_graph_ctx_t gctx; civ_graph_ctx_init(&gctx,lx,dy,w-8,90); gctx.show_grid=false; gctx.x_max=mkt->commodity_count;
  SDL_Texture*bt=civ_graph_bar(r,&gctx,bars,mkt->commodity_count,false);
  if(bt){SDL_FRect dr={(float)lx,(float)dy,(float)(w-8),90};SDL_RenderTexture(r,bt,NULL,&dr);SDL_DestroyTexture(bt);} dy+=100;

  /* Prices */
  for(int i=0;i<mkt->commodity_count&&i<12;i++){snprintf(buf,sizeof(buf),"%-14s %8.1f %s",mkt->commodities[i].name,mkt->commodities[i].price_per_unit,mkt->commodities[i].unit); civ_font_render_aligned(r,f,buf,lx+(i%2)*(w/2),dy,w/2-8,14,0x667788,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); if(i%2==1)dy+=16;}
  (void)in; (void)sh;
}
