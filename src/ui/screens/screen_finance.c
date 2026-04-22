#include "ui/screens/screens.h"
#include "core/character.h"
#include "engine/renderer.h"
#include "ui/ui_common.h"
#include <stdio.h>

void civ_screen_finance_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f,
                                int x, int y, int w, int h, int sh,
                                civ_input_state_t *in, const char *cur, const char *sym) {
  if (!g->player_character) return;
  civ_character_t *pc = (civ_character_t *)g->player_character;
  int dy=y, lx=x+4; char buf[128];
  civ_font_render_aligned(r,f,"FINANCE & BANKING",lx,dy,w-8,22,CIV_COLOR_PRIMARY,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=26;
  float ir = g->market?((civ_market_engine_t*)g->market)->currencies[0].inflation*1.5f:0.03f;
  snprintf(buf,sizeof(buf),"Savings:%.0f (+%.0f/yr) Loans:%.0f (-%.0f/yr) Net:%.0f",pc->savings_balance,pc->savings_balance*ir,pc->loan_balance,pc->loan_balance*pc->loan_rate,pc->savings_balance-pc->loan_balance+pc->personal_wealth);
  civ_font_render_aligned(r,f,buf,lx,dy,w-8,18,0xCCCCCC,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); dy+=26;
  const char *acts[]={"Deposit 100","Withdraw 50","Take Loan (500)","Repay 100"};
  for(int a=0;a<4;a++){bool hov=civ_input_is_mouse_over(in,lx+(a%2)*(w/2),dy,w/2-10,26); civ_render_rect_filled_alpha(r,lx+(a%2)*(w/2),dy,w/2-14,24,hov?0x162033:0x080C18,200); civ_font_render_aligned(r,f,acts[a],lx+(a%2)*(w/2)+8,dy,w/2-20,24,hov?0xFFFFFF:0xAABBCC,CIV_ALIGN_LEFT,CIV_VALIGN_MIDDLE);
    if(hov&&in->mouse_left_pressed){if(a==0&&pc->personal_wealth>=100){pc->personal_wealth-=100;pc->savings_balance+=100;}else if(a==1&&pc->savings_balance>=50){pc->savings_balance-=50;pc->personal_wealth+=50;}else if(a==2){pc->loan_balance+=500;pc->personal_wealth+=500;pc->loan_rate=0.05f;}else if(a==3&&pc->personal_wealth>=100){pc->personal_wealth-=100;pc->loan_balance-=100;}} if(a%2==1)dy+=28;}
  /* Wallet */
  dy+=6;civ_render_line(r,lx,dy,lx+w-8,dy,0x1A2A3A);dy+=6;
  snprintf(buf,sizeof(buf),"Wallet: %.0f USD (%d currencies)",civ_wallet_total(&g->wallet,(civ_market_engine_t*)g->market),g->wallet.count);
  civ_font_render_aligned(r,f,buf,lx,dy,w-8,16,0xFFCC00,CIV_ALIGN_LEFT,CIV_VALIGN_TOP);dy+=18;
  for(int wi=0;wi<g->wallet.count&&wi<6;wi++){civ_market_currency_t*mc=civ_market_get_currency((civ_market_engine_t*)g->market,g->wallet.slots[wi].currency_iso); snprintf(buf,sizeof(buf),"%-4s %12.2f %s",g->wallet.slots[wi].currency_iso,g->wallet.slots[wi].balance,mc?mc->name:""); civ_font_render_aligned(r,f,buf,lx+(wi%2)*(w/2),dy,w/2-8,14,0x667788,CIV_ALIGN_LEFT,CIV_VALIGN_TOP); if(wi%2==1)dy+=16;}
  (void)cur;(void)sym;(void)h;(void)sh;
}
