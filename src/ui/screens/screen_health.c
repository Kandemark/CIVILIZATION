#include "../../../include/ui/screens/screens.h"
#include "../../../include/core/character.h"
#include "../../../include/engine/renderer.h"
#include "../../../include/ui/ui_common.h"
#include <stdio.h>

void civ_screen_health_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f,
                               int x, int y, int w, int h, int sh,
                               civ_input_state_t *in, const char *cur, const char *sym) {
  civ_character_t *pc=(civ_character_t*)g->player_character;
  int dy=y, lx=x+4; char buf[96];
  civ_font_render_aligned(r,f,"HEALTH & WELLNESS",lx,dy,w-8,22,CIV_COLOR_PRIMARY,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=26;
  if(pc){snprintf(buf,sizeof(buf),"Health:%.0f%% Cost:%.0f/mo",pc->health,pc->healthcare_cost); uint32_t hc=pc->health>70?0x44FF44:pc->health>40?0xFFCC00:0xFF4444; civ_font_render_aligned(r,f,buf,lx,dy,w-8,18,hc,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=24;}
  float col = g->market ? civ_market_cost_of_living((civ_market_engine_t*)g->market, cur) : 1.0f;
  char hl[5][48];
  snprintf(hl[0],48,"Visit Doctor (-%s%.0f)",sym,80.0f*col);
  snprintf(hl[1],48,"Medicine (-%s%.0f)",sym,30.0f*col);
  snprintf(hl[2],48,"Insurance (-%s%.0f/mo)",sym,20.0f*col);
  snprintf(hl[3],48,"Gym (-%s%.0f/mo)",sym,15.0f*col);
  snprintf(hl[4],48,"Surgery (-%s%.0f)",sym,500.0f*col);
  for(int i=0;i<5;i++){bool hov=civ_input_is_mouse_over(in,lx,dy,w-8,26); civ_render_rect_filled_alpha(r,lx,dy,w-8,24,hov?0x162033:0x080C18,200); civ_font_render_aligned(r,f,hl[i],lx+10,dy,w-20,24,hov?0xFFFFFF:0xAABBCC,CIV_ALIGN_LEFT,CIV_VALIGN_MIDDLE);
    if(hov&&in->mouse_left_pressed&&pc){if(i==0&&pc->personal_wealth>=80){pc->health+=15;pc->personal_wealth-=80;}else if(i==1&&pc->personal_wealth>=30){pc->health+=5;pc->personal_wealth-=30;}else if(i==2)pc->healthcare_cost=20;else if(i==3)pc->healthcare_cost=15;else if(i==4&&pc->personal_wealth>=500){pc->health=100;pc->personal_wealth-=500;}if(pc->health>100)pc->health=100;} dy+=28;}
  (void)cur;(void)sym;(void)h;(void)sh;
}
