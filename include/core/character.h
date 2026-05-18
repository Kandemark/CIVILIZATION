/**
 * @file character.h
 * @brief Player character — a person in the world, not the nation itself
 *
 * The player starts as an individual with a birthplace, family background,
 * skills, and resources. They can pursue any path: trade, agriculture,
 * military service, scholarship, or eventually political office.
 *
 * The nation-state simulation runs independently. The player is one
 * actor within it — influential, but not omnipotent.
 */
#ifndef CIV_CORE_CHARACTER_H
#define CIV_CORE_CHARACTER_H

#include "../common.h"
#include "../ui/icon/icon_atlas.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_CHAR_NAME_MAX    64
#define CIV_CHAR_BG_MAX      32
#define CIV_CHAR_ROLE_MAX    64
#define CIV_CHAR_LOC_MAX     64
#define CIV_CHAR_SKILL_COUNT 10

/* ── Family backgrounds ─────────────────────────────────────────────── */
typedef enum {
  CIV_BG_PEASANT,
  CIV_BG_MERCHANT,
  CIV_BG_MILITARY,
  CIV_BG_BUREAUCRATIC,
  CIV_BG_ACADEMIC,
  CIV_BG_ARTISAN,
  CIV_BG_NOMADIC,
  CIV_BG_ARISTOCRATIC,
  CIV_BG_COUNT
} civ_background_t;

/* ── Skills (indices, not levels) ───────────────────────────────────── */
typedef enum {
  CIV_SKILL_TRADE,        /* commerce, negotiation */
  CIV_SKILL_COMBAT,       /* personal fighting ability */
  CIV_SKILL_LEADERSHIP,   /* commanding others */
  CIV_SKILL_SCHOLARSHIP,  /* learning, research */
  CIV_SKILL_CRAFTSMANSHIP,/* production, building */
  CIV_SKILL_DIPLOMACY,    /* persuasion, relations */
  CIV_SKILL_STEWARDSHIP,  /* resource management */
  CIV_SKILL_SURVIVAL,     /* terrain, foraging */
  CIV_SKILL_INTRIGUE,     /* information, influence */
  CIV_SKILL_ADMINISTRATION,/* governance, bureaucracy */
} civ_skill_t;

/* ── Player character ────────────────────────────────────────────────── */
typedef struct {
  char             name[CIV_CHAR_NAME_MAX];
  char             birthplace[CIV_CHAR_LOC_MAX];  /* settlement or region name */
  float            birth_lat, birth_lon;
  civ_background_t background;
  char             current_role[CIV_CHAR_ROLE_MAX]; /* what they do now */
  char             current_location[CIV_CHAR_LOC_MAX];

  /* Skill indices (starting values from background, grow with experience) */
  int32_t          skills[CIV_CHAR_SKILL_COUNT];

  /* Resources */
  float            personal_wealth;
  float            land_owned;
  float            reputation;       /* local fame/influence */
  float            political_influence; /* ability to affect governance */

  /* Current political position (empty string = private citizen) */
  char             political_position[64];

  /* Deep life state */
  int32_t          housing_level;   /* 0=homeless,1=shelter,2=renting,3=owning */
  float            housing_cost;    /* monthly */
  int32_t          education_level; /* 0=none,1=primary,2=secondary,3=bachelors,4=masters,5=doctorate */
  float            education_debt;  /* student loans */
  float            health;          /* 0-100 */
  float            healthcare_cost; /* monthly */
  float            savings_balance; /* bank savings with interest */
  float            loan_balance;    /* outstanding loans */
  float            loan_rate;       /* interest rate on loans */
  int32_t          career_rank;     /* 0=entry,1=junior,2=senior,3=lead,4=exec */
  float            monthly_salary;  /* from current job */
  float            monthly_expenses;/* total monthly cost */
  int32_t          relationship_count; /* friends/contacts */
  char             partner_name[48];
  bool             has_partner;

  /* Relationships */
  char             patron_id[32];
  char             rival_id[32];
} civ_character_t;

/* ── API ─────────────────────────────────────────────────────────────── */
civ_character_t *civ_character_create(const char *name);
void             civ_character_destroy(civ_character_t *c);

const char      *civ_background_name(civ_background_t bg);
const char      *civ_background_description(civ_background_t bg);
civ_icon_id_t    civ_background_icon(civ_background_t bg);
const char      *civ_skill_name(civ_skill_t skill);

/* Initialize character skills from background choice */
void civ_character_apply_background(civ_character_t *c, civ_background_t bg);
void civ_character_dynamic_title(civ_character_t *c, char *out, size_t size);

#ifdef __cplusplus
}
#endif
#endif
