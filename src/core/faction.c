/**
 * @file faction.c
 * @brief 8 faction archetypes — starting index positions, no gameplay modifiers
 *
 * Each faction defines where you begin on continuous measurement scales.
 * After turn 1, everything is driven by simulation systems and your decisions.
 * There are no "+25% military" bonuses — comparative position is computed
 * dynamically against global averages.
 */
#include "../../include/core/faction.h"
#include <string.h>

/* Default sector weights — allocated to give each faction a distinct shape */
static const civ_faction_t s_factions[CIV_FACTION_COUNT] = {
    /* ── Imperial Dominion ───────────────────────────────── */
    {
        .id = "imperial", .name = "Imperial Dominion",
        .description = "A centralized state built through consolidation of "
                       "surrounding territories. High executive authority, "
                       "military tradition, but internal integration is ongoing.",
        .color = 0xCC2200, .color_accent = 0xFF6644,
        .icon = CIV_ICON_CROWN,
        .start = {
            .tech_index = 180, .military_index = 340,
            .economic_index = 220, .cultural_index = 140,
            .stability_index = -50, .population_index = 280,
            .sector_weights = {0.18f, 0.15f, 0.22f, 0.14f, 0.08f, 0.10f, 0.05f, 0.08f},
            .executive_power = 0.80f, .legislative_power = 0.15f, .judicial_power = 0.05f,
            .preferred_terrain = "Plains",
            .preferred_latitude = 35,
            .starting_settlements = 3, .starting_territory_radius = 3.0f,
        },
    },
    /* ── Mercantile League ───────────────────────────────── */
    {
        .id = "mercantile", .name = "Mercantile League",
        .description = "A coastal network of trade ports united by commerce "
                       "rather than territory. Wealth flows through markets, "
                       "but military power is distributed and defensive.",
        .color = 0xDAA520, .color_accent = 0xFFD700,
        .icon = CIV_ICON_COIN,
        .start = {
            .tech_index = 200, .military_index = 120,
            .economic_index = 380, .cultural_index = 220,
            .stability_index = 40, .population_index = 180,
            .sector_weights = {0.08f, 0.05f, 0.12f, 0.22f, 0.18f, 0.25f, 0.06f, 0.04f},
            .executive_power = 0.35f, .legislative_power = 0.55f, .judicial_power = 0.10f,
            .preferred_terrain = "Coastal",
            .preferred_latitude = 25,
            .starting_settlements = 4, .starting_territory_radius = 2.0f,
        },
    },
    /* ── Theocratic Order ─────────────────────────────────── */
    {
        .id = "theocratic", .name = "Theocratic Order",
        .description = "Faith and doctrine form the foundation of law and "
                       "identity. Cultural unity is high, but the weight of "
                       "tradition shapes the pace of innovation.",
        .color = 0x8B00CC, .color_accent = 0xCC66FF,
        .icon = CIV_ICON_TEMPLE,
        .start = {
            .tech_index = 80, .military_index = 160,
            .economic_index = 180, .cultural_index = 420,
            .stability_index = 180, .population_index = 240,
            .sector_weights = {0.22f, 0.08f, 0.14f, 0.12f, 0.06f, 0.05f, 0.04f, 0.29f},
            .executive_power = 0.70f, .legislative_power = 0.10f, .judicial_power = 0.20f,
            .preferred_terrain = "River valleys",
            .preferred_latitude = 30,
            .starting_settlements = 2, .starting_territory_radius = 2.5f,
        },
    },
    /* ── Democratic Federation ────────────────────────────── */
    {
        .id = "democratic", .name = "Democratic Federation",
        .description = "Power is distributed broadly through representative "
                       "institutions. Decision-making is slow but enjoys "
                       "broad legitimacy. Cultural dynamism is high.",
        .color = 0x2288FF, .color_accent = 0x66BBFF,
        .icon = CIV_ICON_FLAG,
        .start = {
            .tech_index = 260, .military_index = 100,
            .economic_index = 240, .cultural_index = 340,
            .stability_index = 220, .population_index = 220,
            .sector_weights = {0.14f, 0.06f, 0.16f, 0.20f, 0.12f, 0.14f, 0.10f, 0.08f},
            .executive_power = 0.25f, .legislative_power = 0.55f, .judicial_power = 0.20f,
            .preferred_terrain = "Fertile valleys",
            .preferred_latitude = 40,
            .starting_settlements = 3, .starting_territory_radius = 2.0f,
        },
    },
    /* ── Technocratic Union ───────────────────────────────── */
    {
        .id = "technocratic", .name = "Technocratic Union",
        .description = "Governance by expertise. Research institutions hold "
                       "political weight. Scientific output is unmatched, but "
                       "the population is concentrated in research centers.",
        .color = 0x00AACC, .color_accent = 0x44EEFF,
        .icon = CIV_ICON_FLASK,
        .start = {
            .tech_index = 480, .military_index = 90,
            .economic_index = 260, .cultural_index = 180,
            .stability_index = 60, .population_index = 120,
            .sector_weights = {0.06f, 0.08f, 0.20f, 0.16f, 0.08f, 0.06f, 0.30f, 0.06f},
            .executive_power = 0.45f, .legislative_power = 0.30f, .judicial_power = 0.25f,
            .preferred_terrain = "Mountains",
            .preferred_latitude = 45,
            .starting_settlements = 1, .starting_territory_radius = 2.0f,
        },
    },
    /* ── Martial Horde ────────────────────────────────────── */
    {
        .id = "martial", .name = "Martial Horde",
        .description = "A mobile military society organized around command "
                       "structure. Rapid territorial control through force, "
                       "but economic infrastructure follows slowly behind.",
        .color = 0xCC0000, .color_accent = 0xFF3333,
        .icon = CIV_ICON_SWORD,
        .start = {
            .tech_index = 60, .military_index = 460,
            .economic_index = 100, .cultural_index = 100,
            .stability_index = -80, .population_index = 320,
            .sector_weights = {0.10f, 0.20f, 0.30f, 0.08f, 0.02f, 0.05f, 0.02f, 0.23f},
            .executive_power = 0.90f, .legislative_power = 0.05f, .judicial_power = 0.05f,
            .preferred_terrain = "Steppe",
            .preferred_latitude = 45,
            .starting_settlements = 5, .starting_territory_radius = 4.0f,
        },
    },
    /* ── Agricultural Collective ──────────────────────────── */
    {
        .id = "agricultural", .name = "Agricultural Collective",
        .description = "Land and labor form the basis of society. Population "
                       "is distributed across fertile regions. Steady growth "
                       "comes from the soil, not the sword.",
        .color = 0x228800, .color_accent = 0x44DD44,
        .icon = CIV_ICON_WHEAT,
        .start = {
            .tech_index = 140, .military_index = 80,
            .economic_index = 200, .cultural_index = 200,
            .stability_index = 100, .population_index = 400,
            .sector_weights = {0.35f, 0.12f, 0.10f, 0.10f, 0.04f, 0.06f, 0.04f, 0.19f},
            .executive_power = 0.30f, .legislative_power = 0.45f, .judicial_power = 0.25f,
            .preferred_terrain = "Grassland",
            .preferred_latitude = 35,
            .starting_settlements = 4, .starting_territory_radius = 2.5f,
        },
    },
    /* ── Stewards of the Wild ─────────────────────────────── */
    {
        .id = "stewards", .name = "Stewards of the Wild",
        .description = "Living in balance with the natural world. Deep "
                       "understanding of terrain yields advantages in difficult "
                       "landscapes, but industrial output is deliberately limited.",
        .color = 0x228844, .color_accent = 0x66CC88,
        .icon = CIV_ICON_TREE_RING,
        .start = {
            .tech_index = 160, .military_index = 110,
            .economic_index = 140, .cultural_index = 360,
            .stability_index = 160, .population_index = 160,
            .sector_weights = {0.20f, 0.18f, 0.06f, 0.14f, 0.02f, 0.08f, 0.10f, 0.22f},
            .executive_power = 0.20f, .legislative_power = 0.50f, .judicial_power = 0.30f,
            .preferred_terrain = "Forest",
            .preferred_latitude = 20,
            .starting_settlements = 2, .starting_territory_radius = 3.0f,
        },
    },
};

const civ_faction_t *civ_faction_get_all(void) { return s_factions; }
const civ_faction_t *civ_faction_get(int index) {
  if (index < 0 || index >= CIV_FACTION_COUNT) return NULL;
  return &s_factions[index];
}
const civ_faction_t *civ_faction_get_by_id(const char *id) {
  for (int i = 0; i < CIV_FACTION_COUNT; i++)
    if (strcmp(s_factions[i].id, id) == 0) return &s_factions[i];
  return NULL;
}
int civ_faction_count(void) { return CIV_FACTION_COUNT; }
