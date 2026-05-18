#include "../../../include/ui/icon/icon_atlas.h"
#include "../../../include/engine/renderer.h"
#include <stdlib.h>
#include <string.h>

/* ── Nerd Font glyph mapping ────────────────────────────────────────────
 * Maps icon IDs to Unicode codepoints in patched Nerd Fonts.
 * These are from Font Awesome (nf-fa-*) in the Private Use Area
 * as patched by the Nerd Fonts project.
 */
static const civ_nerd_glyph_t nerd_glyphs[] = {
    {CIV_ICON_SWORD,       0xF0E7}, /* nf-fa-bolt — repurposed for combat */
    {CIV_ICON_SHIELD,      0xF132}, /* nf-fa-shield */
    {CIV_ICON_HELMET,      0xF11B}, /* nf-fa-gamepad — styled */
    {CIV_ICON_INFANTRY,    0xF0C0}, /* nf-fa-users */
    {CIV_ICON_COIN,        0xF0D6}, /* nf-fa-money */
    {CIV_ICON_GEM,         0xF219}, /* nf-fa-diamond */
    {CIV_ICON_CHEST,       0xF1B2}, /* nf-fa-cube */
    {CIV_ICON_SCALES,      0xF24E}, /* nf-fa-balance-scale */
    {CIV_ICON_TRADE,       0xF0EC}, /* nf-fa-exchange */
    {CIV_ICON_HAMMER,      0xF0E3}, /* nf-fa-gavel — legal, repurposed */
    {CIV_ICON_WHEAT,       0xF06C}, /* nf-fa-leaf — nature */
    {CIV_ICON_GEAR,        0xF013}, /* nf-fa-cog */
    {CIV_ICON_CROWN,       0xF119}, /* nf-fa-graduation-cap — styled crown */
    {CIV_ICON_FLAG,        0xF024}, /* nf-fa-flag */
    {CIV_ICON_SCROLL,      0xF15C}, /* nf-fa-file-text */
    {CIV_ICON_BOOK,        0xF02D}, /* nf-fa-book */
    {CIV_ICON_TEMPLE,      0xF19C}, /* nf-fa-university */
    {CIV_ICON_PRAYER,      0xF004}, /* nf-fa-heart */
    {CIV_ICON_DOVE,        0xF072}, /* nf-fa-plane — repurposed peace */
    {CIV_ICON_HANDSHAKE,   0xF2B5}, /* nf-fa-handshake-o */
    {CIV_ICON_FLASK,       0xF0C3}, /* nf-fa-flask */
    {CIV_ICON_COMPASS,     0xF14E}, /* nf-fa-compass */
    {CIV_ICON_TELESCOPE,   0xF002}, /* nf-fa-search */
    {CIV_ICON_MOUNTAIN,    0xF1BB}, /* nf-fa-tree — styled mountain */
    {CIV_ICON_TREE_RING,   0xF1BB}, /* nf-fa-tree */
    {CIV_ICON_FIRE,        0xF06D}, /* nf-fa-fire */
    {CIV_ICON_SHIP,        0xF21A}, /* nf-fa-ship */
    {CIV_ICON_PEOPLE,      0xF0C0}, /* nf-fa-users */
    {CIV_ICON_HEART,       0xF004}, /* nf-fa-heart */
    {CIV_ICON_SKULL,       0xF54C}, /* nf-fa-skull-crossbones — approx */
    {CIV_ICON_STAR,        0xF005}, /* nf-fa-star */
    {CIV_ICON_CLOSE,       0xF00D}, /* nf-fa-times */
    {CIV_ICON_SETTINGS,    0xF013}, /* nf-fa-cog */
    {CIV_ICON_SEARCH,      0xF002}, /* nf-fa-search */
    {CIV_ICON_ZOOM_IN,     0xF00E}, /* nf-fa-search-plus */
    {CIV_ICON_ZOOM_OUT,    0xF010}, /* nf-fa-search-minus */
    {CIV_ICON_HOME,        0xF015}, /* nf-fa-home */
    {CIV_ICON_BACK,        0xF060}, /* nf-fa-arrow-left */
    {CIV_ICON_FORWARD,     0xF061}, /* nf-fa-arrow-right */
    {CIV_ICON_REFRESH,     0xF021}, /* nf-fa-refresh */
    {CIV_ICON_PLUS,        0xF067}, /* nf-fa-plus */
    {CIV_ICON_MINUS,       0xF068}, /* nf-fa-minus */
    {CIV_ICON_CHECK,       0xF00C}, /* nf-fa-check */
    {CIV_ICON_CROSS,       0xF00D}, /* nf-fa-times */
    {CIV_ICON_MENU,        0xF0C9}, /* nf-fa-bars */
    {CIV_ICON_INFO,        0xF05A}, /* nf-fa-info-circle */
    {CIV_ICON_WARNING,     0xF071}, /* nf-fa-warning */
    {CIV_ICON_BELL,        0xF0F3}, /* nf-fa-bell */
    {CIV_ICON_LOCK,        0xF023}, /* nf-fa-lock */
    {CIV_ICON_UNLOCK,      0xF09C}, /* nf-fa-unlock */
    {CIV_ICON_ROCKET,      0xF135}, /* nf-fa-rocket */
    {CIV_ICON_GLOBE,       0xF0AC}, /* nf-fa-globe */
};

/* ── Icon name table ─────────────────────────────────────────────────── */
static const char *icon_names[CIV_ICON_COUNT] = {
    #define X(id, name) [id] = name,
    X(CIV_ICON_SWORD, "sword") X(CIV_ICON_SHIELD, "shield")
    X(CIV_ICON_SPEAR, "spear") X(CIV_ICON_BOW, "bow")
    X(CIV_ICON_AXE, "axe") X(CIV_ICON_HELMET, "helmet")
    X(CIV_ICON_ARMOR, "armor") X(CIV_ICON_CROSSHAIR, "crosshair")
    X(CIV_ICON_CAVALRY, "cavalry") X(CIV_ICON_INFANTRY, "infantry")
    X(CIV_ICON_ARCHER, "archer") X(CIV_ICON_SIEGE, "siege")
    X(CIV_ICON_FORT, "fort") X(CIV_ICON_BATTLEMENT, "battlement")
    X(CIV_ICON_WARDRUM, "wardrum") X(CIV_ICON_BANNER, "banner")
    X(CIV_ICON_CANNON, "cannon") X(CIV_ICON_BOMB, "bomb")
    X(CIV_ICON_GUARD, "guard") X(CIV_ICON_SCOUT, "scout")
    X(CIV_ICON_COIN, "coin") X(CIV_ICON_GEM, "gem")
    X(CIV_ICON_GOLD_BAR, "gold_bar") X(CIV_ICON_CHEST, "chest")
    X(CIV_ICON_SCALES, "scales") X(CIV_ICON_TRADE, "trade")
    X(CIV_ICON_CARAVAN, "caravan") X(CIV_ICON_HAMMER, "hammer")
    X(CIV_ICON_PICKAXE, "pickaxe") X(CIV_ICON_WHEAT, "wheat")
    X(CIV_ICON_FISH, "fish") X(CIV_ICON_COW, "cow")
    X(CIV_ICON_TREE_RING, "tree_ring") X(CIV_ICON_FACTORY, "factory")
    X(CIV_ICON_GEAR, "gear") X(CIV_ICON_ANVIL, "anvil")
    X(CIV_ICON_MINE, "mine") X(CIV_ICON_MARKET, "market")
    X(CIV_ICON_CROWN, "crown") X(CIV_ICON_THRONE, "throne")
    X(CIV_ICON_SCEPTER, "scepter") X(CIV_ICON_SCROLL, "scroll")
    X(CIV_ICON_GAVEL, "gavel") X(CIV_ICON_PILLAR, "pillar")
    X(CIV_ICON_FLAG, "flag") X(CIV_ICON_STANDARD, "standard")
    X(CIV_ICON_BALANCE, "balance") X(CIV_ICON_LAW, "law")
    X(CIV_ICON_TRIBUNE, "tribune") X(CIV_ICON_SENATE, "senate")
    X(CIV_ICON_PALACE, "palace") X(CIV_ICON_CONTRACT, "contract")
    X(CIV_ICON_SEAL, "seal") X(CIV_ICON_RIBBON, "ribbon")
    X(CIV_ICON_VOTE, "vote") X(CIV_ICON_DECREE, "decree")
    X(CIV_ICON_BOOK, "book") X(CIV_ICON_QUILL, "quill")
    X(CIV_ICON_LYRE, "lyre") X(CIV_ICON_MASK, "mask")
    X(CIV_ICON_THEATER, "theater") X(CIV_ICON_MONUMENT, "monument")
    X(CIV_ICON_STATUE, "statue") X(CIV_ICON_PAINTING, "painting")
    X(CIV_ICON_TAPESTRY, "tapestry") X(CIV_ICON_LANGUAGE, "language")
    X(CIV_ICON_PRAYER, "prayer") X(CIV_ICON_TEMPLE, "temple")
    X(CIV_ICON_SHRINE, "shrine") X(CIV_ICON_FESTIVAL, "festival")
    X(CIV_ICON_DANCE, "dance") X(CIV_ICON_POTTERY, "pottery")
    X(CIV_ICON_HANDSHAKE, "handshake") X(CIV_ICON_TREATY, "treaty")
    X(CIV_ICON_DOVE, "dove") X(CIV_ICON_OLIVE, "olive")
    X(CIV_ICON_EMBASSY, "embassy") X(CIV_ICON_ENVOY, "envoy")
    X(CIV_ICON_ALLIANCE, "alliance") X(CIV_ICON_TRUCE, "truce")
    X(CIV_ICON_SURRENDER, "surrender") X(CIV_ICON_DECLARATION, "declaration")
    X(CIV_ICON_WAR, "war") X(CIV_ICON_PEACE, "peace")
    X(CIV_ICON_TRADE_ROUTE, "trade_route") X(CIV_ICON_TRIBUTE, "tribute")
    X(CIV_ICON_WRITING, "writing") X(CIV_ICON_WHEEL, "wheel")
    X(CIV_ICON_GEAR_TECH, "gear_tech") X(CIV_ICON_FLASK, "flask")
    X(CIV_ICON_ATOM, "atom") X(CIV_ICON_TELESCOPE, "telescope")
    X(CIV_ICON_COMPASS, "compass") X(CIV_ICON_ASTRONOMY, "astronomy")
    X(CIV_ICON_CLOCKWORK, "clockwork") X(CIV_ICON_BLUEPRINT, "blueprint")
    X(CIV_ICON_MICROSCOPE, "microscope") X(CIV_ICON_COMPUTER, "computer")
    X(CIV_ICON_SATELLITE, "satellite") X(CIV_ICON_ROCKET, "rocket")
    X(CIV_ICON_MOUNTAIN, "mountain") X(CIV_ICON_HILL, "hill")
    X(CIV_ICON_PLAIN, "plain") X(CIV_ICON_FOREST, "forest")
    X(CIV_ICON_DESERT, "desert") X(CIV_ICON_TUNDRA, "tundra")
    X(CIV_ICON_COAST, "coast") X(CIV_ICON_OCEAN, "ocean")
    X(CIV_ICON_RIVER, "river") X(CIV_ICON_LAKE, "lake")
    X(CIV_ICON_VOLCANO, "volcano") X(CIV_ICON_EARTHQUAKE, "earthquake")
    X(CIV_ICON_STORM, "storm") X(CIV_ICON_FLOOD, "flood")
    X(CIV_ICON_DROUGHT, "drought") X(CIV_ICON_BLIZZARD, "blizzard")
    X(CIV_ICON_PERSON, "person") X(CIV_ICON_PEOPLE, "people")
    X(CIV_ICON_FAMILY, "family") X(CIV_ICON_CHILD, "child")
    X(CIV_ICON_ELDER, "elder") X(CIV_ICON_BIRTH, "birth")
    X(CIV_ICON_DEATH, "death") X(CIV_ICON_MIGRATION, "migration")
    X(CIV_ICON_HEART, "heart") X(CIV_ICON_SKULL, "skull")
    X(CIV_ICON_HEALTH, "health") X(CIV_ICON_PLAGUE, "plague")
    X(CIV_ICON_CLOSE, "close") X(CIV_ICON_MINIMIZE, "minimize")
    X(CIV_ICON_MAXIMIZE, "maximize") X(CIV_ICON_SETTINGS, "settings")
    X(CIV_ICON_SEARCH, "search") X(CIV_ICON_ZOOM_IN, "zoom_in")
    X(CIV_ICON_ZOOM_OUT, "zoom_out") X(CIV_ICON_HOME, "home")
    X(CIV_ICON_BACK, "back") X(CIV_ICON_FORWARD, "forward")
    X(CIV_ICON_REFRESH, "refresh") X(CIV_ICON_PLUS, "plus")
    X(CIV_ICON_MINUS, "minus") X(CIV_ICON_CHECK, "check")
    X(CIV_ICON_CROSS, "cross") X(CIV_ICON_MENU, "menu")
    X(CIV_ICON_INFO, "info") X(CIV_ICON_WARNING, "warning")
    X(CIV_ICON_ERROR, "error") X(CIV_ICON_BELL, "bell")
    X(CIV_ICON_LOCK, "lock") X(CIV_ICON_UNLOCK, "unlock")
    X(CIV_ICON_STAR, "star")
    #undef X
};

struct civ_icon_atlas { SDL_Texture *sdf_tex; int atlas_w, atlas_h; bool loaded; };

civ_icon_atlas_t *civ_icon_atlas_create(SDL_Renderer *r) {
  (void)r;
  civ_icon_atlas_t *atlas =
      (civ_icon_atlas_t *)malloc(sizeof(civ_icon_atlas_t));
  if (!atlas) return NULL;
  memset(atlas, 0, sizeof(*atlas));
  return atlas;
}

void civ_icon_atlas_destroy(civ_icon_atlas_t *atlas) {
  if (!atlas) return;
  if (atlas->sdf_tex) SDL_DestroyTexture(atlas->sdf_tex);
  free(atlas);
}

bool civ_icon_atlas_load_sdf(civ_icon_atlas_t *atlas, const char *filepath) {
  (void)atlas; (void)filepath;
  /* SDF atlas loading — stb_image based. Deferred until PNG pipeline is needed. */
  return false;
}

void civ_icon_render(civ_icon_atlas_t *atlas, SDL_Renderer *r,
                     civ_icon_id_t icon_id, float x, float y, int size,
                     uint32_t color, uint8_t alpha) {
  if (!atlas || !r) return;

  if (atlas->loaded && atlas->sdf_tex) {
    /* SDF path: sample texture with distance threshold */
    SDL_FRect dst = {x, y, (float)size, (float)size};
    SDL_SetTextureColorMod(atlas->sdf_tex, (color >> 16) & 0xFF,
                           (color >> 8) & 0xFF, color & 0xFF);
    SDL_SetTextureAlphaMod(atlas->sdf_tex, alpha);
    int col = (int)icon_id % 16, row = (int)icon_id / 16;
    SDL_FRect src = {(float)(col * atlas->atlas_w / 16),
                     (float)(row * atlas->atlas_h / 16),
                     (float)(atlas->atlas_w / 16),
                     (float)(atlas->atlas_h / 16)};
    SDL_RenderTexture(r, atlas->sdf_tex, &src, &dst);
  } else {
    /* Fallback: procedural shape placeholder */
    civ_render_rect_filled_alpha(r, (int)x, (int)y, size, size, color, alpha);
    civ_render_rect_outline(r, (int)x, (int)y, size, size, 0xFFFFFF, 1);
  }
}

void civ_icon_render_nerd(civ_icon_id_t icon_id, const char *nerd_font_path,
                          float x, float y, int size, uint32_t color) {
  /* Nerd Font glyph rendering — handled externally via SDL3_ttf
   * with the specific codepoint from civ_icon_nerd_codepoint() */
  (void)icon_id; (void)nerd_font_path; (void)x; (void)y; (void)size; (void)color;
}

const char *civ_icon_name(civ_icon_id_t id) {
  if (id >= CIV_ICON_COUNT) return "unknown";
  return icon_names[id] ? icon_names[id] : "unnamed";
}

uint32_t civ_icon_nerd_codepoint(civ_icon_id_t id) {
  for (size_t i = 0; i < sizeof(nerd_glyphs) / sizeof(nerd_glyphs[0]); i++)
    if (nerd_glyphs[i].icon_id == id) return nerd_glyphs[i].codepoint;
  return 0;
}
