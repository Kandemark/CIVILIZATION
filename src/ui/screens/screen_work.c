#include "display/theme.h"
#include "ui/screens/screens.h"
#include "core/character.h"
#include "engine/renderer.h"
#include "ui/ui_common.h"
#include <stdio.h>

void civ_screen_work_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f,
                             int x, int y, int w, int h, int sh,
                             civ_input_state_t *in, const char *cur, const char *sym) {
  if (!g->player_character) return;
  civ_character_t *pc = (civ_character_t *)g->player_character;
  int dy=y, lx=x+4; char buf[128];
  civ_font_render_aligned(r,f,"WORK & EMPLOYMENT",lx,dy,w-8,22,CIV_COLOR_PRIMARY,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=26;
  const char *ranks[]={"Entry","Junior","Senior","Lead","Executive"};
  snprintf(buf,sizeof(buf),"Career: %s  Salary: %s%.0f/%s/mo",ranks[pc->career_rank%5],sym,pc->monthly_salary,cur);
  civ_font_render_aligned(r,f,buf,lx,dy,w-8,18,g_theme.text_secondary,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=24;
  float col = civ_market_cost_of_living((civ_market_engine_t*)g->market, cur);
  struct { const char *t; civ_skill_t s; int m; } j[] = {
    {"Farmhand",CIV_SKILL_SURVIVAL,30},{"Trader",CIV_SKILL_TRADE,50},{"Artisan",CIV_SKILL_CRAFTSMANSHIP,60},
    {"Teacher",CIV_SKILL_SCHOLARSHIP,70},{"Guard",CIV_SKILL_COMBAT,40},{"Clerk",CIV_SKILL_ADMINISTRATION,55},
    {"Diplomat",CIV_SKILL_DIPLOMACY,75},{"Manager",CIV_SKILL_STEWARDSHIP,80}};
  for (int jj=0; jj<8; jj++) { bool qual=pc->skills[j[jj].s]>=j[jj].m; bool hov=civ_input_is_mouse_over(in,lx,dy,w-8,30);
    civ_render_rect_filled_alpha(r,lx,dy,w-8,28,hov?0x162033:(qual?0x080C18:0x040810),200);
    float pay=j[jj].m*col*(1+pc->skills[j[jj].s]/100.0f);
    snprintf(buf,sizeof(buf),"%s — %s%.0f/%s [%s %d/%d]",j[jj].t,qual?"":"(!) ",pay,cur,civ_skill_name(j[jj].s),pc->skills[j[jj].s],j[jj].m);
    civ_font_render_aligned(r,f,buf,lx+10,dy,w-20,26,qual?(hov?0xFFFFFF:g_theme.text_secondary):g_theme.text_dim,CIV_ALIGN_LEFT,CIV_VALIGN_MIDDLE);
    if(hov&&in->mouse_left_pressed&&qual){pc->monthly_salary=pay;pc->personal_wealth+=pay;pc->reputation+=2;if(pc->career_rank<4)pc->career_rank++;} dy+=32; }
  (void)sym;(void)h;(void)sh;
}
