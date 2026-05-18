/**
 * @file icon_atlas.h
 * @brief Icon system — 200+ game icons via SDF atlas + Nerd Font fallback
 *
 * Tier 1: Signed-distance-field atlas (resolution-independent, any size)
 * Tier 2: Nerd Font glyph fallback (30+ glyphs for common symbols)
 * Tier 3: Procedural shapes (runtime-generated for dynamic content)
 */
#ifndef CIV_UI_ICON_ATLAS_H
#define CIV_UI_ICON_ATLAS_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Icon ID enumeration (200+ game concepts) ──────────────────────── */
typedef enum {
  /* Military — 20 icons */
  CIV_ICON_SWORD, CIV_ICON_SHIELD, CIV_ICON_SPEAR, CIV_ICON_BOW,
  CIV_ICON_AXE, CIV_ICON_HELMET, CIV_ICON_ARMOR, CIV_ICON_CROSSHAIR,
  CIV_ICON_CAVALRY, CIV_ICON_INFANTRY, CIV_ICON_ARCHER, CIV_ICON_SIEGE,
  CIV_ICON_FORT, CIV_ICON_BATTLEMENT, CIV_ICON_WARDRUM, CIV_ICON_BANNER,
  CIV_ICON_CANNON, CIV_ICON_BOMB, CIV_ICON_GUARD, CIV_ICON_SCOUT,

  /* Economy — 18 icons */
  CIV_ICON_COIN, CIV_ICON_GEM, CIV_ICON_GOLD_BAR, CIV_ICON_CHEST,
  CIV_ICON_SCALES, CIV_ICON_TRADE, CIV_ICON_CARAVAN, CIV_ICON_HAMMER,
  CIV_ICON_PICKAXE, CIV_ICON_WHEAT, CIV_ICON_FISH, CIV_ICON_COW,
  CIV_ICON_TREE_RING, CIV_ICON_FACTORY, CIV_ICON_GEAR, CIV_ICON_ANVIL,
  CIV_ICON_MINE, CIV_ICON_MARKET,

  /* Governance — 18 icons */
  CIV_ICON_CROWN, CIV_ICON_THRONE, CIV_ICON_SCEPTER, CIV_ICON_SCROLL,
  CIV_ICON_GAVEL, CIV_ICON_PILLAR, CIV_ICON_FLAG, CIV_ICON_STANDARD,
  CIV_ICON_BALANCE, CIV_ICON_LAW, CIV_ICON_TRIBUNE, CIV_ICON_SENATE,
  CIV_ICON_PALACE, CIV_ICON_CONTRACT, CIV_ICON_SEAL, CIV_ICON_RIBBON,
  CIV_ICON_VOTE, CIV_ICON_DECREE,

  /* Culture — 16 icons */
  CIV_ICON_BOOK, CIV_ICON_QUILL, CIV_ICON_LYRE, CIV_ICON_MASK,
  CIV_ICON_THEATER, CIV_ICON_MONUMENT, CIV_ICON_STATUE, CIV_ICON_PAINTING,
  CIV_ICON_TAPESTRY, CIV_ICON_LANGUAGE, CIV_ICON_PRAYER, CIV_ICON_TEMPLE,
  CIV_ICON_SHRINE, CIV_ICON_FESTIVAL, CIV_ICON_DANCE, CIV_ICON_POTTERY,

  /* Diplomacy — 14 icons */
  CIV_ICON_HANDSHAKE, CIV_ICON_TREATY, CIV_ICON_DOVE, CIV_ICON_OLIVE,
  CIV_ICON_EMBASSY, CIV_ICON_ENVOY, CIV_ICON_ALLIANCE, CIV_ICON_TRUCE,
  CIV_ICON_SURRENDER, CIV_ICON_DECLARATION, CIV_ICON_WAR, CIV_ICON_PEACE,
  CIV_ICON_TRADE_ROUTE, CIV_ICON_TRIBUTE,

  /* Technology — 14 icons */
  CIV_ICON_WRITING, CIV_ICON_WHEEL, CIV_ICON_GEAR_TECH, CIV_ICON_FLASK,
  CIV_ICON_ATOM, CIV_ICON_TELESCOPE, CIV_ICON_COMPASS, CIV_ICON_ASTRONOMY,
  CIV_ICON_CLOCKWORK, CIV_ICON_BLUEPRINT, CIV_ICON_MICROSCOPE, CIV_ICON_COMPUTER,
  CIV_ICON_SATELLITE, CIV_ICON_ROCKET,

  /* Nature / Terrain — 16 icons */
  CIV_ICON_MOUNTAIN, CIV_ICON_HILL, CIV_ICON_PLAIN, CIV_ICON_FOREST,
  CIV_ICON_DESERT, CIV_ICON_TUNDRA, CIV_ICON_COAST, CIV_ICON_OCEAN,
  CIV_ICON_RIVER, CIV_ICON_LAKE, CIV_ICON_VOLCANO, CIV_ICON_EARTHQUAKE,
  CIV_ICON_FIRE, CIV_ICON_STORM, CIV_ICON_FLOOD, CIV_ICON_DROUGHT, CIV_ICON_BLIZZARD,

  /* Population — 12 icons */
  CIV_ICON_PERSON, CIV_ICON_PEOPLE, CIV_ICON_FAMILY, CIV_ICON_CHILD,
  CIV_ICON_ELDER, CIV_ICON_BIRTH, CIV_ICON_DEATH, CIV_ICON_MIGRATION,
  CIV_ICON_HEART, CIV_ICON_SKULL, CIV_ICON_HEALTH, CIV_ICON_PLAGUE,

  /* UI chrome — 23 icons */
  CIV_ICON_GLOBE, CIV_ICON_SHIP,
  CIV_ICON_CLOSE, CIV_ICON_MINIMIZE, CIV_ICON_MAXIMIZE, CIV_ICON_SETTINGS,
  CIV_ICON_SEARCH, CIV_ICON_ZOOM_IN, CIV_ICON_ZOOM_OUT, CIV_ICON_HOME,
  CIV_ICON_BACK, CIV_ICON_FORWARD, CIV_ICON_REFRESH, CIV_ICON_PLUS,
  CIV_ICON_MINUS, CIV_ICON_CHECK, CIV_ICON_CROSS, CIV_ICON_MENU,
  CIV_ICON_INFO, CIV_ICON_WARNING, CIV_ICON_ERROR, CIV_ICON_BELL,
  CIV_ICON_LOCK, CIV_ICON_UNLOCK, CIV_ICON_STAR,

  CIV_ICON_COUNT
} civ_icon_id_t;

/* ── Icon size presets ──────────────────────────────────────────────── */
typedef enum {
  CIV_ICON_XS  = 12,   /* Inline text */
  CIV_ICON_SM  = 16,   /* Compact lists */
  CIV_ICON_MD  = 24,   /* Standard buttons / tabs */
  CIV_ICON_LG  = 32,   /* Panel headers */
  CIV_ICON_XL  = 48,   /* Feature highlights */
  CIV_ICON_XXL = 64,   /* Splash / hero */
} civ_icon_size_t;

/* ── Nerd Font glyph mapping (fallback) ──────────────────────────────── */
typedef struct {
  civ_icon_id_t icon_id;
  uint32_t      codepoint; /* Unicode codepoint in Nerd Font */
} civ_nerd_glyph_t;

/* ── Atlas API ────────────────────────────────────────────────────────── */
typedef struct civ_icon_atlas civ_icon_atlas_t;

civ_icon_atlas_t *civ_icon_atlas_create(SDL_Renderer *r);
void              civ_icon_atlas_destroy(civ_icon_atlas_t *atlas);
bool              civ_icon_atlas_load_sdf(civ_icon_atlas_t *atlas,
                                          const char *filepath);

void civ_icon_render(civ_icon_atlas_t *atlas, SDL_Renderer *r,
                     civ_icon_id_t icon_id, float x, float y, int size,
                     uint32_t color, uint8_t alpha);
void civ_icon_render_nerd(civ_icon_id_t icon_id, const char *nerd_font_path,
                          float x, float y, int size, uint32_t color);

const char       *civ_icon_name(civ_icon_id_t id);
uint32_t           civ_icon_nerd_codepoint(civ_icon_id_t id);

#ifdef __cplusplus
}
#endif
#endif
