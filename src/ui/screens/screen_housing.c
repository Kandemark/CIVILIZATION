#include "ui/screens/screens.h"
#include "core/character.h"
#include "engine/renderer.h"
#include "ui/ui_common.h"
#include <stdio.h>

void civ_screen_housing_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f,
                                int x, int y, int w, int h, int sh,
                                civ_input_state_t *in, const char *cur, const char *sym) {
  int dy=y, lx=x+4; char buf[96];
  civ_font_render_aligned(r,f,"HOUSING & PROPERTY",lx,dy,w-8,22,CIV_COLOR_PRIMARY,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=26;
  civ_character_t *pc=(civ_character_t*)g->player_character; const char *hs[]={"Homeless","Shelter","Renting","Owning"};
  if(pc){snprintf(buf,sizeof(buf),"Current: %s (%.0f/mo)",hs[pc->housing_level%4],pc->housing_cost); civ_font_render_aligned(r,f,buf,lx,dy,w-8,18,0xAABBCC,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=24;}
  float col = g->market ? civ_market_cost_of_living((civ_market_engine_t*)g->market, cur) : 1.0f;
  char ha[6][48];
  snprintf(ha[0],48,"Find Shelter (free)");
  snprintf(ha[1],48,"Rent (-%s%.0f/mo)",sym,50.0f*col);
  snprintf(ha[2],48,"Buy House (-%s%.0f)",sym,2000.0f*col);
  snprintf(ha[3],48,"Mortgage (-%s%.0f down)",sym,200.0f*col);
  snprintf(ha[4],48,"Sell Property (+%s%.0f)",sym,1500.0f*col);
  snprintf(ha[5],48,"Upgrade (-%s%.0f)",sym,5000.0f*col);
  for(int i=0;i<6;i++){bool hov=civ_input_is_mouse_over(in,lx,dy,w-8,26); civ_render_rect_filled_alpha(r,lx,dy,w-8,24,hov?0x162033:0x080C18,200); civ_font_render_aligned(r,f,ha[i],lx+10,dy,w-20,24,hov?0xFFFFFF:0xAABBCC,CIV_ALIGN_LEFT,CIV_VALIGN_MIDDLE); dy+=28;}
  (void)cur;(void)sym;(void)h;(void)sh;
}
