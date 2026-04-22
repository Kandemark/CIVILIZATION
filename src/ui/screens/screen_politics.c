#include "display/theme.h"
#include "ui/screens/screens.h"
#include "core/character.h"
#include "engine/renderer.h"
#include "ui/ui_common.h"
#include <stdio.h>

void civ_screen_politics_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f,
                                 int x, int y, int w, int h, int sh,
                                 civ_input_state_t *in, const char *cur, const char *sym) {
  int dy=y, lx=x+4; char buf[128];
  civ_font_render_aligned(r,f,"POLITICS & CIVIC LIFE",lx,dy,w-8,22,CIV_COLOR_PRIMARY,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=26;
  snprintf(buf,sizeof(buf),"Influence:%.0f Party:%s Approval:%.0f%% Boss:%.0f%%",g->player_character?((civ_character_t*)g->player_character)->political_influence:0,g->player_role.party_name[0]?g->player_role.party_name:"None",g->player_role.public_approval*100,g->player_role.boss_trust*100);
  civ_font_render_aligned(r,f,buf,lx,dy,w-8,18,g_theme.text_secondary,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=24;
  float col = g->market ? civ_market_cost_of_living((civ_market_engine_t*)g->market, cur) : 1.0f;
  char pa[5][64];
  snprintf(pa[0],64,"Join Party (+5 inf)");
  snprintf(pa[1],64,"Donate %s%.0f (+10 inf)",sym,20.0f*col);
  snprintf(pa[2],64,"Apply Gov Position");
  snprintf(pa[3],64,"Protest (+3 rep)");
  snprintf(pa[4],64,"Run for Office (-%s%.0f)",sym,50.0f*col);
  for(int i=0;i<5;i++){bool hov=civ_input_is_mouse_over(in,lx,dy,w-8,26); civ_render_rect_filled_alpha(r,lx,dy,w-8,24,hov?0x162033:0x080C18,200); civ_font_render_aligned(r,f,pa[i],lx+10,dy,w-20,24,hov?0xFFFFFF:g_theme.text_secondary,CIV_ALIGN_LEFT,CIV_VALIGN_MIDDLE);
    if(hov&&in->mouse_left_pressed&&g->player_character){civ_character_t*pc=(civ_character_t*)g->player_character; if(i==0)pc->political_influence+=5;else if(i==1&&pc->personal_wealth>=20){pc->personal_wealth-=20;pc->political_influence+=10;}else if(i==2)civ_role_set(&g->player_role,&civ_role_cabinet_economics,g->player_role.nation_id);else if(i==3){pc->reputation+=3;pc->political_influence+=2;}else if(i==4&&pc->reputation>=30&&pc->personal_wealth>=50){pc->personal_wealth-=50;pc->political_influence+=15;}} dy+=28;}
  (void)cur;(void)sym;(void)h;(void)sh;
}
