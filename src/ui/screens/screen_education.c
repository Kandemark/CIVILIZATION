#include "ui/screens/screens.h"
#include "core/character.h"
#include "engine/renderer.h"
#include "ui/ui_common.h"
#include <stdio.h>

void civ_screen_education_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f,
                                  int x, int y, int w, int h, int sh,
                                  civ_input_state_t *in, const char *cur, const char *sym) {
  int dy=y, lx=x+4;
  civ_font_render_aligned(r,f,"EDUCATION & LEARNING",lx,dy,w-8,22,CIV_COLOR_PRIMARY,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=26;
  civ_character_t *pc=(civ_character_t*)g->player_character; const char *lv[]={"None","Primary","Secondary","Bachelors","Masters","Doctorate"}; char buf[96];
  if(pc){snprintf(buf,sizeof(buf),"Level: %s  Debt: %.0f",lv[pc->education_level%6],pc->education_debt); civ_font_render_aligned(r,f,buf,lx,dy,w-8,18,0xCCCCCC,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=24;}
  float col = g->market ? civ_market_cost_of_living((civ_market_engine_t*)g->market, cur) : 1.0f;
  char ea[4][48];
  snprintf(ea[0],48,"Enroll (-%s%.0f)",sym,200.0f*col);
  snprintf(ea[1],48,"Study (+progress)");
  snprintf(ea[2],48,"Student Loan (+%s%.0f)",sym,500.0f*col);
  snprintf(ea[3],48,"Pay Tuition (-%s%.0f)",sym,300.0f*col);
  for(int i=0;i<4;i++){bool hov=civ_input_is_mouse_over(in,lx,dy,w-8,26); civ_render_rect_filled_alpha(r,lx,dy,w-8,24,hov?0x162033:0x080C18,200); civ_font_render_aligned(r,f,ea[i],lx+10,dy,w-20,24,hov?0xFFFFFF:0xAABBCC,CIV_ALIGN_LEFT,CIV_VALIGN_MIDDLE); dy+=28;}
  (void)cur;(void)sym;(void)h;(void)sh;
}
