#include "../../../include/ui/screens/screens.h"
#include "../../../include/core/character.h"
#include "../../../include/engine/renderer.h"
#include "../../../include/ui/ui_common.h"
#include "../../../include/ui/graph/graph.h"
#include <stdio.h>

void civ_screen_dashboard_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f,
                                  int x, int y, int w, int h, int sh,
                                  civ_input_state_t *in, const char *cur, const char *sym) {
  if (!g->player_character) return;
  civ_character_t *pc = (civ_character_t *)g->player_character;
  int dy = y, lx = x + 4;
  civ_font_render_aligned(r, f, "PERSONAL DASHBOARD", lx, dy, w-8, 22, CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy+=26;
  char buf[192], dtitle[64]; civ_character_dynamic_title(pc, dtitle, sizeof(dtitle));
  /* Identity card */
  civ_render_rect_filled_alpha(r, lx, dy, w-8, 38, 0x080C18, 200);
  snprintf(buf, sizeof(buf), "%s  |  %s  |  %s", pc->name, g->player_role.nation_id, dtitle);
  civ_font_render_aligned(r, f, buf, lx+10, dy+10, w-20, 20, 0xFFFFFF, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE); dy+=44;
  /* 3 gauges */
  float mvals[]={pc->personal_wealth/1000,pc->reputation/100,pc->political_influence/100};
  const char *ml[]={"Wealth","Reputation","Influence"}; uint32_t mc[]={0xFFCC00,0x44BBFF,0xCC66FF};
  for (int mi=0; mi<3; mi++) { snprintf(buf,sizeof(buf),"%s",ml[mi]); civ_font_render_aligned(r,f,buf,lx+mi*(w/3),dy,80,14,0x667788,CIV_ALIGN_LEFT,CIV_VALIGN_TOP);
    civ_render_rect_filled(r,lx+mi*(w/3),dy+16,w/3-12,8,0x1A2A3A); float pct=mvals[mi]>1?1:(mvals[mi]<0?0:mvals[mi]);
    civ_render_rect_filled(r,lx+mi*(w/3),dy+16,(int)((w/3-12)*pct),8,mc[mi]);
    snprintf(buf,sizeof(buf),"%.0f",mvals[mi]*100); civ_font_render_aligned(r,f,buf,lx+mi*(w/3),dy+28,w/3-12,16,mc[mi],CIV_ALIGN_LEFT,CIV_VALIGN_TOP); } dy+=50;
  /* Life stats */
  const char *house[]={"Homeless","Shelter","Renting","Owning"}; const char *edu[]={"None","Prim","Sec","Bach","Mast","Doct"};
  snprintf(buf,sizeof(buf),"Housing:%s Edu:%s Health:%.0f%% Career:R%d",house[pc->housing_level%4],edu[pc->education_level%6],pc->health,pc->career_rank);
  civ_font_render_aligned(r,f,buf,lx,dy,w-8,16,0xAABBCC,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=20;
  float net=pc->savings_balance-pc->loan_balance+pc->personal_wealth;
  snprintf(buf,sizeof(buf),"Net:%.0f Sav:%.0f Loan:%.0f Inc:%+.0f/mo",net,pc->savings_balance,pc->loan_balance,pc->monthly_salary-pc->monthly_expenses);
  civ_font_render_aligned(r,f,buf,lx,dy,w-8,16,0x8899AA,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=24;
  /* 10 skill bars in 2 columns */
  civ_render_line(r,lx,dy,lx+w-8,dy,0x1A2A3A); dy+=4;
  for(int s=0;s<5;s++){snprintf(buf,sizeof(buf),"%-14s",civ_skill_name((civ_skill_t)s)); civ_font_render_aligned(r,f,buf,lx,dy,110,14,0x8899AA,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); civ_render_rect_filled(r,lx+112,dy+2,100,8,0x1A2A3A); float sp=pc->skills[s]/100.0f;if(sp>1)sp=1; civ_render_rect_filled(r,lx+112,dy+2,(int)(100*sp),8,g_graph_palette_default[s%12]); snprintf(buf,sizeof(buf),"%d",pc->skills[s]); civ_font_render_aligned(r,f,buf,lx+216,dy,30,14,0xAABBCC,CIV_ALIGN_RIGHT,CIV_VALIGN_TOP);
    int s2=s+5; snprintf(buf,sizeof(buf),"%-14s",civ_skill_name((civ_skill_t)s2)); civ_font_render_aligned(r,f,buf,lx+w/2,dy,110,14,0x8899AA,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); civ_render_rect_filled(r,lx+w/2+112,dy+2,100,8,0x1A2A3A); float sp2=pc->skills[s2]/100.0f;if(sp2>1)sp2=1; civ_render_rect_filled(r,lx+w/2+112,dy+2,(int)(100*sp2),8,g_graph_palette_default[s2%12]); snprintf(buf,sizeof(buf),"%d",pc->skills[s2]); civ_font_render_aligned(r,f,buf,lx+w/2+216,dy,30,14,0xAABBCC,CIV_ALIGN_RIGHT,CIV_VALIGN_TOP); dy+=16;}
  /* Wallet */
  if(g->wallet.count>0){dy+=4;civ_render_line(r,lx,dy,lx+w-8,dy,0x1A2A3A);dy+=4; snprintf(buf,sizeof(buf),"Wallet: %.0f USD (%d currencies)",civ_wallet_total(&g->wallet,(civ_market_engine_t*)g->market),g->wallet.count); civ_font_render_aligned(r,f,buf,lx,dy,w-8,16,0xFFCC00,CIV_ALIGN_LEFT,CIV_VALIGN_TOP);dy+=18;
    for(int wi=0;wi<g->wallet.count&&wi<3;wi++){civ_market_currency_t*mc=civ_market_get_currency((civ_market_engine_t*)g->market,g->wallet.slots[wi].currency_iso); snprintf(buf,sizeof(buf),"  %s%.2f %s",mc?mc->symbol:"",g->wallet.slots[wi].balance,g->wallet.slots[wi].currency_iso); civ_font_render_aligned(r,f,buf,lx,dy,w-8,14,0x667788,CIV_ALIGN_LEFT,CIV_VALIGN_TOP);dy+=16;}}
  (void)cur;(void)sym;(void)in;(void)h;(void)sh;
}
