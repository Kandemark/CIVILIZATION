#include "../../../include/ui/screens/screens.h"
#include "../../../include/core/constitution.h"
#include "../../../include/core/world/nation.h"
#include "../../../include/engine/renderer.h"
#include "../../../include/ui/ui_common.h"
#include <stdio.h>

void civ_screen_constitution_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f,
                                     int x, int y, int w, int h, int sh,
                                     civ_input_state_t *in, const char *cur, const char *sym) {
  int dy=y, lx=x+4; char buf[128];
  civ_font_render_aligned(r,f,"NATIONAL CONSTITUTION",lx,dy,w-8,22,CIV_COLOR_PRIMARY,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=26;
  void *nc=NULL; if(g->nation_manager){civ_nation_t*pn=civ_nation_get_by_id((civ_nation_manager_t*)g->nation_manager,g->player_role.nation_id);if(pn)nc=pn->constitution;}
  civ_national_constitution_t fbc; if(!nc&&g->player_role.nation_id[0]){civ_national_constitution_init(&fbc,g->player_role.nation_id);nc=&fbc;}
  if(nc){civ_national_constitution_t*c=(civ_national_constitution_t*)nc; snprintf(buf,sizeof(buf),"%s",c->name); civ_font_render_aligned(r,f,buf,lx,dy,w-8,18,0xFFCC00,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=22;
    for(int rl=0;rl<CIV_RULE_COUNT;rl++){uint32_t pc=c->rules[rl].level==CIV_PERM_PROTECTED_RIGHT?0x44FF44:c->rules[rl].level==CIV_PERM_ALLOWED?0xAACCAA:c->rules[rl].level==CIV_PERM_RESTRICTED?0xFFCC00:0xFF4444;
      snprintf(buf,sizeof(buf),"%-22s %s",civ_rule_category_name((civ_rule_category_t)rl),civ_perm_level_name(c->rules[rl].level)); civ_font_render_aligned(r,f,buf,lx,dy,w/2-10,16,pc,CIV_ALIGN_LEFT,CIV_VALIGN_TOP);
      if(c->rules[rl].cost>0){snprintf(buf,sizeof(buf),"Cost:%.0f Age:%.0f",c->rules[rl].cost,c->rules[rl].minimum_age); civ_font_render_aligned(r,f,buf,lx+w/2,dy,w/2-10,16,0x667788,CIV_ALIGN_LEFT,CIV_VALIGN_TOP);} dy+=18;}
  }else{civ_font_render_aligned(r,f,"No constitution data",lx,dy,w-8,20,0x556677,CIV_ALIGN_LEFT,CIV_VALIGN_TOP);}
  (void)cur;(void)sym;(void)in;(void)h;(void)sh;
}
