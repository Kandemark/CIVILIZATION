#include "display/theme.h"
#include "ui/screens/screens.h"
#include "core/character.h"
#include "engine/renderer.h"
#include "ui/ui_common.h"

void civ_screen_network_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f,
                                int x, int y, int w, int h, int sh,
                                civ_input_state_t *in, const char *cur, const char *sym) {
  int dy=y, lx=x+4;
  civ_font_render_aligned(r,f,"NETWORK & CONNECTIONS",lx,dy,w-8,22,CIV_COLOR_PRIMARY,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=26;
  float col = g->market ? civ_market_cost_of_living((civ_market_engine_t*)g->market, cur) : 1.0f;
  char na[5][64];
  snprintf(na[0],64,"Attend Gathering (+2 rep)");
  snprintf(na[1],64,"Meet Traders (+3 rep)");
  snprintf(na[2],64,"Volunteer (+1 rep +1 inf)");
  snprintf(na[3],64,"Host Event (-%s%.0f, +5 rep)",sym,30.0f*col);
  snprintf(na[4],64,"Seek Mentor (+2 scholarship)");
  for(int i=0;i<5;i++){bool hov=civ_input_is_mouse_over(in,lx,dy,w-8,26); civ_render_rect_filled_alpha(r,lx,dy,w-8,24,hov?0x162033:0x080C18,200); civ_font_render_aligned(r,f,na[i],lx+10,dy,w-20,24,hov?0xFFFFFF:g_theme.text_secondary,CIV_ALIGN_LEFT,CIV_VALIGN_MIDDLE);
    if(hov&&in->mouse_left_pressed&&g->player_character){civ_character_t*pc=(civ_character_t*)g->player_character; if(i==0)pc->reputation+=2;else if(i==1)pc->reputation+=3;else if(i==2){pc->reputation+=1;pc->political_influence+=1;}else if(i==3&&pc->personal_wealth>=30){pc->personal_wealth-=30;pc->reputation+=5;}else if(i==4)pc->skills[CIV_SKILL_SCHOLARSHIP]+=2;} dy+=28;}
  (void)cur;(void)sym;(void)h;(void)sh;
}
