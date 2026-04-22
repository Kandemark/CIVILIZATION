/**
 * @file character.c
 * @brief Player character — a person in the simulated world
 */
#include "core/character.h"
#include <stdlib.h>
#include <string.h>

static const char *s_bg_names[CIV_BG_COUNT] = {
    "Peasant", "Merchant", "Military", "Bureaucratic",
    "Academic", "Artisan", "Nomadic", "Aristocratic",
};

static const char *s_bg_descs[CIV_BG_COUNT] = {
    "Born to the land. Your family has worked the soil for generations. "
    "You know the rhythms of seasons, the value of labor, and the hunger "
    "that drives men to action.",
    "Born to the market. Your family trades goods across regions. You "
    "understand currency, negotiation, and the networks that move wealth.",
    "Born to the sword. Your family has served in armies for generations. "
    "Discipline, tactics, and the weight of command are in your blood.",
    "Born to the ledger. Your family serves the administrative apparatus. "
    "You understand how systems function, how records shape reality, and "
    "how the machinery of governance actually works.",
    "Born to the library. Your family values knowledge above all. You "
    "grew up among texts, debates, and the pursuit of understanding.",
    "Born to the workshop. Your family builds things — tools, structures, "
    "goods. You understand materials, processes, and the pride of creation.",
    "Born to the open road. Your family follows the seasons, the herds, "
    "the trade winds. You know terrain, survival, and the freedom of "
    "movement that settled peoples have forgotten.",
    "Born to the estate. Your family holds land, title, and influence. "
    "You grew up with access to power — and the expectations that come with it.",
};

static const civ_icon_id_t s_bg_icons[CIV_BG_COUNT] = {
    CIV_ICON_WHEAT,   CIV_ICON_COIN,     CIV_ICON_SWORD,    CIV_ICON_GAVEL,
    CIV_ICON_BOOK,    CIV_ICON_HAMMER,   CIV_ICON_SCOUT,    CIV_ICON_CROWN,
};

static const char *s_skill_names[CIV_CHAR_SKILL_COUNT] = {
    "Trade", "Combat", "Leadership", "Scholarship", "Craftsmanship",
    "Diplomacy", "Stewardship", "Survival", "Intrigue", "Administration",
};

/* Starting skill indices per background */
static const int32_t s_bg_skills[CIV_BG_COUNT][CIV_CHAR_SKILL_COUNT] = {
    /* Peasant */    {  40,  50,  20,  10,  60,  20,  80,  70,  10,  10},
    /* Merchant */   { 100,  20,  30,  40,  30,  70,  50,  30,  50,  40},
    /* Military */   {  20, 100,  80,  20,  30,  30,  40,  60,  30,  30},
    /* Bureaucratic */{ 40,  10,  30,  60,  20,  50,  70,  10,  70,  90},
    /* Academic */   {  20,  10,  20, 100,  30,  40,  30,  20,  30,  40},
    /* Artisan */    {  60,  30,  20,  30, 100,  30,  50,  30,  20,  20},
    /* Nomadic */    {  50,  70,  40,  20,  40,  40,  30, 100,  30,  10},
    /* Aristocratic */{ 60,  40,  70,  50,  20,  80,  60,  20,  60,  60},
};

civ_character_t *civ_character_create(const char *name) {
  civ_character_t *c = (civ_character_t *)malloc(sizeof(civ_character_t));
  if (!c) return NULL;
  memset(c, 0, sizeof(*c));
  if (name) strncpy(c->name, name, CIV_CHAR_NAME_MAX - 1);
  strncpy(c->current_role, "Private Citizen", CIV_CHAR_ROLE_MAX - 1);
  c->personal_wealth = 50.0f;
  c->reputation = 10.0f;
  c->political_influence = 0.0f;
  return c;
}

void civ_character_destroy(civ_character_t *c) { free(c); }

void civ_character_dynamic_title(civ_character_t *c, char *out, size_t size) {
  if (!c || !out) return;
  int best_skill = 0, best_val = 0;
  for (int i = 0; i < CIV_CHAR_SKILL_COUNT; i++)
    if (c->skills[i] > best_val) { best_val = c->skills[i]; best_skill = i; }
  const char *wealth = c->personal_wealth > 5000 ? "Wealthy" : c->personal_wealth > 1000 ? "Comfortable" : c->personal_wealth > 100 ? "Working" : "Struggling";
  const char *inf = c->political_influence > 50 ? "Influential" : c->political_influence > 20 ? "Connected" : "";
  const char *role_desc = c->political_influence > 30 ? "Political Figure" : civ_skill_name((civ_skill_t)best_skill);
  if (inf[0]) snprintf(out, size, "%s %s %s", wealth, inf, role_desc);
  else snprintf(out, size, "%s %s", wealth, role_desc);
}

void civ_character_apply_background(civ_character_t *c, civ_background_t bg) {
  if (!c || bg >= CIV_BG_COUNT) return;
  c->background = bg;
  for (int i = 0; i < CIV_CHAR_SKILL_COUNT; i++)
    c->skills[i] = s_bg_skills[bg][i];

  /* Starting wealth varies by background */
  float wealth[] = {20, 200, 60, 80, 40, 100, 30, 400};
  c->personal_wealth = wealth[bg];
  c->reputation = 10.0f + (float)(bg == CIV_BG_ARISTOCRATIC ? 30 : bg == CIV_BG_MERCHANT ? 15 : 0);
  c->political_influence = (bg == CIV_BG_ARISTOCRATIC || bg == CIV_BG_BUREAUCRATIC) ? 25.0f : 0.0f;
  /* Initialize deep life state */
  c->housing_level = (bg == CIV_BG_ARISTOCRATIC) ? 3 : (bg == CIV_BG_NOMADIC) ? 1 : 2;
  c->housing_cost = (c->housing_level == 3) ? 0 : (c->housing_level == 2) ? 50 : 10;
  c->education_level = (bg == CIV_BG_ACADEMIC) ? 4 : (bg == CIV_BG_BUREAUCRATIC) ? 3 : 2;
  c->health = 80.0f + (float)(rand() % 20);
  c->healthcare_cost = 10.0f;
  c->savings_balance = c->personal_wealth * 0.5f;
  c->monthly_salary = (bg == CIV_BG_MERCHANT) ? 200 : (bg == CIV_BG_ARISTOCRATIC) ? 300 : 80;
  c->monthly_expenses = c->housing_cost + c->healthcare_cost + 20;
  c->relationship_count = 2 + rand() % 5;
  c->career_rank = (bg == CIV_BG_ARISTOCRATIC || bg == CIV_BG_MERCHANT) ? 2 : 1;
}

const char *civ_background_name(civ_background_t bg) {
  return (bg < CIV_BG_COUNT) ? s_bg_names[bg] : "Unknown";
}
const char *civ_background_description(civ_background_t bg) {
  return (bg < CIV_BG_COUNT) ? s_bg_descs[bg] : "";
}
civ_icon_id_t civ_background_icon(civ_background_t bg) {
  return (bg < CIV_BG_COUNT) ? s_bg_icons[bg] : CIV_ICON_PERSON;
}
const char *civ_skill_name(civ_skill_t skill) {
  return (skill < CIV_CHAR_SKILL_COUNT) ? s_skill_names[skill] : "Unknown";
}
