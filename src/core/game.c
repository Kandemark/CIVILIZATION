/**
 * @file game.c
 * @brief Implementation of the main game controller
 */

#include "../../include/core/game.h"
#include "../../include/common.h"
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/* Forward declarations */
static void initialize_systems(civ_game_t* game, const civ_game_config_t* config);
static void update_systems(civ_game_t* game, civ_float_t time_delta);

civ_game_t* civ_game_create(void) {
    civ_game_t* game = (civ_game_t*)CIV_MALLOC(sizeof(civ_game_t));
    if (!game) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate game");
        return NULL;
    }
    
    memset(game, 0, sizeof(civ_game_t));
    game->state = CIV_GAME_STATE_INITIALIZING;
    game->is_running = false;
    game->is_paused = false;
    game->event_log_capacity = 1000;
    game->event_log = (civ_game_event_t*)CIV_CALLOC(game->event_log_capacity, sizeof(civ_game_event_t));
    
    if (!game->event_log) {
        CIV_FREE(game);
        return NULL;
    }
    
    return game;
}

void civ_game_destroy(civ_game_t* game) {
    if (!game) return;
    
    if (game->time_manager) {
        civ_time_manager_destroy(game->time_manager);
    }
    if (game->population_manager) {
        civ_population_manager_destroy(game->population_manager);
    }
    if (game->market_economy) {
        civ_market_dynamics_destroy(game->market_economy);
    }
    if (game->technology_tree) {
        civ_innovation_system_destroy(game->technology_tree);
    }
    if (game->military_system) {
        civ_combat_system_destroy(game->military_system);
    }
    if (game->unit_manager) {
        civ_unit_manager_destroy(game->unit_manager);
    }
    if (game->diplomacy_system) {
        civ_diplomacy_system_destroy(game->diplomacy_system);
    }
    if (game->soft_metrics) {
        civ_soft_metrics_manager_destroy(game->soft_metrics);
    }
    if (game->event_manager) {
        civ_event_manager_destroy(game->event_manager);
    }
    if (game->dynamic_borders) {
        civ_dynamic_borders_destroy(game->dynamic_borders);
    }
    if (game->government) {
        civ_government_destroy(game->government);
    }
    if (game->geography) {
        civ_geography_destroy(game->geography);
    }
    if (game->culture_system) {
        civ_culture_system_destroy(game->culture_system);
    }
    if (game->ai_system) {
        civ_ai_system_destroy(game->ai_system);
    }
    if (game->politics_system) {
        civ_politics_system_destroy(game->politics_system);
    }
    if (game->subunit_manager) {
        civ_subunit_manager_destroy(game->subunit_manager);
    }
    if (game->world_map) {
        civ_map_destroy(game->world_map);
    }
    if (game->map_view_manager) {
        civ_map_view_manager_destroy(game->map_view_manager);
    }
    if (game->territory_manager) {
        civ_territory_manager_destroy(game->territory_manager);
    }
    if (game->custom_governance_manager) {
        civ_custom_governance_manager_destroy(game->custom_governance_manager);
    }
    if (game->conquest_system) {
        civ_conquest_system_destroy(game->conquest_system);
    }
    if (game->cultural_display) {
        civ_cultural_display_destroy(game->cultural_display);
    }
    if (game->system_orchestrator) {
        civ_system_orchestrator_destroy(game->system_orchestrator);
    }
    if (game->performance_optimizer) {
        civ_performance_optimizer_destroy(game->performance_optimizer);
    }
    if (game->config_manager) {
        civ_config_manager_destroy(game->config_manager);
    }
    if (game->cache) {
        civ_cache_destroy(game->cache);
    }
    if (game->memory_pool) {
        civ_memory_pool_manager_destroy(game->memory_pool);
    }
    
    if (game->nations) {
        for (size_t i = 0; i < game->nation_count; i++) {
            CIV_FREE(game->nations[i]);
        }
        CIV_FREE(game->nations);
    }
    
    CIV_FREE(game->event_log);
    CIV_FREE(game);
}

civ_result_t civ_game_initialize(civ_game_t* game, const civ_game_config_t* config) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!game) {
        result.error = CIV_ERROR_NULL_POINTER;
        result.message = "Game pointer is null";
        return result;
    }
    
    game->state = CIV_GAME_STATE_INITIALIZING;
    
    /* Copy configuration */
    if (config) {
        memcpy(&game->config, config, sizeof(civ_game_config_t));
    } else {
        civ_game_get_default_config(&game->config);
    }
    
    /* Initialize time manager */
    game->time_manager = civ_time_manager_create();
    if (!game->time_manager) {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
        result.message = "Failed to create time manager";
        return result;
    }
    
    /* Set initial time scale */
    civ_time_manager_set_time_scale(game->time_manager, CIV_TIME_SCALE_NORMAL);
    
    /* Initialize all systems */
    initialize_systems(game, config);
    
    game->state = CIV_GAME_STATE_RUNNING;
    game->is_running = true;
    
    civ_log(CIV_LOG_INFO, "Game initialized successfully");
    
    return result;
}

void civ_game_run(civ_game_t* game) {
    if (!game || !game->is_running) return;
    
    civ_log(CIV_LOG_INFO, "Starting game loop...");
    
    while (game->is_running && game->state == CIV_GAME_STATE_RUNNING) {
        civ_game_update(game);
        
        /* Small delay to prevent CPU overload */
        #ifdef _WIN32
        Sleep(10);
        #else
        usleep(10000);  /* 10ms */
        #endif
    }
}

void civ_game_update(civ_game_t* game) {
    if (!game || game->state != CIV_GAME_STATE_RUNNING) return;
    
    clock_t start_time = clock();
    
    /* Update time manager */
    if (game->time_manager) {
        civ_float_t time_delta = civ_time_manager_update(game->time_manager);
        
        if (time_delta > 0.0f) {
            update_systems(game, time_delta);
        }
    }
    
    /* Update performance metrics */
    clock_t end_time = clock();
    civ_float_t update_time = ((civ_float_t)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0f; /* ms */
    
    game->performance.update_count++;
    game->performance.last_update_time = update_time;
    game->performance.avg_update_time = 
        (game->performance.avg_update_time * (game->performance.update_count - 1) + update_time) /
        game->performance.update_count;
    
    /* Check for autosave */
    static time_t last_autosave = 0;
    time_t current_time = time(NULL);
    if (game->config.autosave_interval > 0 && 
        current_time - last_autosave >= game->config.autosave_interval) {
        char filename[STRING_MAX_LEN];
        snprintf(filename, sizeof(filename), "autosave_%ld.json", current_time);
        civ_game_save(game, filename);
        last_autosave = current_time;
    }
}

void civ_game_pause(civ_game_t* game) {
    if (!game) return;
    
    if (game->state == CIV_GAME_STATE_RUNNING) {
        game->state = CIV_GAME_STATE_PAUSED;
        game->is_paused = true;
        if (game->time_manager) {
            civ_time_manager_set_time_scale(game->time_manager, CIV_TIME_SCALE_PAUSED);
        }
        civ_log(CIV_LOG_INFO, "Game paused");
    }
}

void civ_game_resume(civ_game_t* game) {
    if (!game) return;
    
    if (game->state == CIV_GAME_STATE_PAUSED) {
        game->state = CIV_GAME_STATE_RUNNING;
        game->is_paused = false;
        if (game->time_manager) {
            civ_time_manager_set_time_scale(game->time_manager, CIV_TIME_SCALE_NORMAL);
        }
        civ_log(CIV_LOG_INFO, "Game resumed");
    }
}

void civ_game_shutdown(civ_game_t* game) {
    if (!game) return;
    
    game->state = CIV_GAME_STATE_SHUTTING_DOWN;
    game->is_running = false;
    
    /* Perform final autosave */
    civ_game_save(game, "autosave_final.json");
    
    civ_log(CIV_LOG_INFO, "Game shutdown complete");
}

civ_result_t civ_game_save(civ_game_t* game, const char* filename) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!game || !filename) {
        result.error = CIV_ERROR_NULL_POINTER;
        result.message = "Null pointer argument";
        return result;
    }
    
    game->state = CIV_GAME_STATE_SAVING;
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        result.error = CIV_ERROR_IO;
        result.message = "Failed to open file for writing";
        game->state = CIV_GAME_STATE_RUNNING;
        return result;
    }
    
    /* Serialize time manager */
    if (game->time_manager) {
        char* time_json = civ_time_manager_to_json(game->time_manager);
        if (time_json) {
            fprintf(file, "{\"time\":%s,\"version\":\"%s\"}\n", time_json, game->config.version);
            CIV_FREE(time_json);
        }
    }
    
    fclose(file);
    game->state = CIV_GAME_STATE_RUNNING;
    
    civ_log(CIV_LOG_INFO, "Game saved to %s", filename);
    
    return result;
}

civ_result_t civ_game_load(civ_game_t* game, const char* filename) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!game || !filename) {
        result.error = CIV_ERROR_NULL_POINTER;
        result.message = "Null pointer argument";
        return result;
    }
    
    game->state = CIV_GAME_STATE_LOADING;
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        result.error = CIV_ERROR_IO;
        result.message = "Failed to open file for reading";
        game->state = CIV_GAME_STATE_RUNNING;
        return result;
    }
    
    /* Simple JSON loading (in production, use a proper JSON library) */
    char buffer[4096];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, file);
    buffer[bytes_read] = '\0';
    
    /* TODO: Parse JSON and restore game state */
    
    fclose(file);
    game->state = CIV_GAME_STATE_RUNNING;
    
    civ_log(CIV_LOG_INFO, "Game loaded from %s", filename);
    
    return result;
}

void civ_game_get_default_config(civ_game_config_t* config) {
    if (!config) return;
    
    memset(config, 0, sizeof(civ_game_config_t));
    strcpy(config->name, "Civilization Simulation");
    strcpy(config->version, "0.1.0");
    config->start_year = 1;
    config->difficulty = 1; /* Medium */
    config->enable_fuzzy_logic = true;
    config->enable_dependency_tracking = true;
    config->autosave_interval = 300; /* 5 minutes */
    config->max_event_log = 1000;
}

void civ_game_add_event(civ_game_t* game, const char* type, const char* description, civ_float_t importance) {
    if (!game || !type || !description) return;
    
    if (game->event_log_size >= game->event_log_capacity) {
        /* Remove oldest event */
        memmove(game->event_log, game->event_log + 1, 
                (game->event_log_capacity - 1) * sizeof(civ_game_event_t));
        game->event_log_size--;
    }
    
    civ_game_event_t* event = &game->event_log[game->event_log_size];
    strncpy(event->type, type, STRING_SHORT_LEN - 1);
    event->type[STRING_SHORT_LEN - 1] = '\0';
    strncpy(event->description, description, STRING_MAX_LEN - 1);
    event->description[STRING_MAX_LEN - 1] = '\0';
    event->importance = importance;
    
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    event->timestamp.year = tm_info->tm_year + 1900;
    event->timestamp.month = tm_info->tm_mon + 1;
    event->timestamp.day = tm_info->tm_mday;
    event->timestamp.hour = tm_info->tm_hour;
    event->timestamp.minute = tm_info->tm_min;
    event->timestamp.second = tm_info->tm_sec;
    
    game->event_log_size++;
}

/* Initialize all game systems */
static void initialize_systems(civ_game_t* game, const civ_game_config_t* config) {
    /* Initialize population manager */
    game->population_manager = civ_population_manager_create();
    if (game->population_manager) {
        civ_population_manager_initialize_region(game->population_manager, "capital", 1000);
    }
    
    /* Initialize economy */
    game->market_economy = civ_market_dynamics_create();
    
    /* Initialize technology */
    game->technology_tree = civ_innovation_system_create();
    if (game->technology_tree) {
        civ_innovation_system_set_research_budget(game->technology_tree, 100.0f);
    }
    
    /* Initialize military */
    game->military_system = civ_combat_system_create();
    game->unit_manager = civ_unit_manager_create();
    
    /* Initialize diplomacy */
    game->diplomacy_system = civ_diplomacy_system_create();
    
    /* Initialize soft metrics */
    game->soft_metrics = civ_soft_metrics_manager_create();
    
    /* Initialize event manager */
    game->event_manager = civ_event_manager_create();
    
    /* Initialize dynamic borders */
    game->dynamic_borders = civ_dynamic_borders_create();
    
    /* Initialize government */
    game->government = civ_government_create("Default Government", CIV_GOV_TYPE_DEMOCRACY);
    
    /* Initialize geography */
    civ_coordinate_t sw = {0.0f, 0.0f};
    civ_coordinate_t ne = {100.0f, 100.0f};
    game->geography = civ_geography_create("Default Region", sw, ne);
    game->culture_system = civ_culture_system_create();
    game->ai_system = civ_ai_system_create();
    game->politics_system = civ_politics_system_create();
    game->subunit_manager = civ_subunit_manager_create();
    
    /* Generate 2D world map */
    civ_map_gen_params_t map_params = {0};
    map_params.width = 200;
    map_params.height = 200;
    map_params.sea_level = 0.3f;
    map_params.land_ratio = 0.7f;
    map_params.seed = (uint32_t)time(NULL);
    map_params.generate_rivers = true;
    map_params.generate_mountains = true;
    game->world_map = civ_map_create(map_params.width, map_params.height, map_params.seed);
    if (game->world_map) {
        civ_map_generate(game->world_map, &map_params);
    }
    
    /* Initialize map view manager */
    game->map_view_manager = civ_map_view_manager_create(game->world_map);
    
    /* Initialize territory manager */
    game->territory_manager = civ_territory_manager_create();
    
    /* Initialize custom governance manager */
    game->custom_governance_manager = civ_custom_governance_manager_create();
    
    /* Initialize conquest system */
    game->conquest_system = civ_conquest_system_create();
    
    /* Initialize cultural display */
    game->cultural_display = civ_cultural_display_create();
    
    /* Initialize performance and modularity systems */
    game->system_orchestrator = civ_system_orchestrator_create();
    game->performance_optimizer = civ_performance_optimizer_create();
    game->config_manager = civ_config_manager_create();
    game->cache = civ_cache_create(1000, 10 * 1024 * 1024, 3600);  /* 1000 entries, 10MB, 1hr TTL */
    game->memory_pool = civ_memory_pool_manager_create(1024, 100);  /* 1KB blocks, 100 blocks */
    
    /* Enable profiling if in debug mode */
    #ifdef DEBUG
    if (game->performance_optimizer) {
        civ_performance_optimizer_enable_profiling(game->performance_optimizer, true);
    }
    #endif
    
    /* Initialize nations */
    game->nation_capacity = 10;
    game->nations = (char**)CIV_CALLOC(game->nation_capacity, sizeof(char*));
    if (game->nations) {
        const char* default_nations[] = {"player_nation", "ai_nation_1", "ai_nation_2"};
        for (size_t i = 0; i < 3 && i < game->nation_capacity; i++) {
            game->nations[i] = (char*)CIV_MALLOC(strlen(default_nations[i]) + 1);
            if (game->nations[i]) {
                strcpy(game->nations[i], default_nations[i]);
                game->nation_count++;
            }
        }
        
        /* Initialize diplomatic relations */
        if (game->diplomacy_system && game->nation_count > 0) {
            civ_diplomacy_system_initialize_relations(game->diplomacy_system, 
                                                     (const char**)game->nations, 
                                                     game->nation_count);
        }
    }
    
    civ_log(CIV_LOG_INFO, "All systems initialized successfully");
}

/* Update all game systems */
static void update_systems(civ_game_t* game, civ_float_t time_delta) {
    /* Update population */
    if (game->population_manager) {
        /* Get economic conditions */
        civ_economic_report_t econ_report = {0};
        if (game->market_economy) {
            econ_report = civ_market_dynamics_get_report(game->market_economy);
        }
        
        /* Update population with economic conditions */
        civ_population_manager_update(game->population_manager, time_delta, &econ_report);
    }
    
    /* Update economy */
    if (game->market_economy && game->population_manager) {
        civ_float_t tech_level = 1.0f;
        if (game->technology_tree) {
            tech_level = civ_innovation_system_get_tech_level(game->technology_tree);
        }
        
        int64_t population = civ_population_manager_get_total(game->population_manager);
        void* pop_data = &population; /* Simplified */
        
        civ_market_dynamics_update(game->market_economy, time_delta, pop_data, tech_level);
    }
    
    /* Update technology */
    if (game->technology_tree) {
        civ_innovation_system_update(game->technology_tree, time_delta);
    }
    
    /* Update soft metrics */
    if (game->soft_metrics && game->market_economy) {
        civ_economic_report_t econ_report = civ_market_dynamics_get_report(game->market_economy);
        civ_soft_metrics_update_from_economy(game->soft_metrics, &econ_report);
    }
    
    /* Update diplomacy */
    if (game->diplomacy_system) {
        civ_diplomacy_system_update_relations(game->diplomacy_system, time(NULL));
    }
    
    /* Update events */
    if (game->event_manager) {
        civ_event_manager_update(game->event_manager, time_delta);
    }
    
    /* Update borders */
    if (game->dynamic_borders) {
        civ_dynamic_borders_update(game->dynamic_borders, time_delta);
    }
    
    /* Update culture system */
    if (game->culture_system) {
        civ_culture_system_update(game->culture_system, time_delta);
    }
    
    /* Update AI system */
    if (game->ai_system) {
        civ_ai_system_update(game->ai_system, time_delta);
    }
    
    /* Update politics system */
    if (game->politics_system) {
        civ_politics_system_update(game->politics_system, time_delta);
    }
    
    /* Update subunit manager */
    if (game->subunit_manager) {
        civ_subunit_manager_update(game->subunit_manager, time_delta);
    }
    
    /* Update territory manager */
    if (game->territory_manager) {
        civ_territory_manager_update(game->territory_manager, time_delta);
    }
    
    /* Update conquest system */
    if (game->conquest_system) {
        civ_conquest_update(game->conquest_system, time_delta);
    }
    
    /* Update cultural display */
    if (game->cultural_display && game->culture_system) {
        civ_cultural_display_update(game->cultural_display,
                                   game->culture_system->assimilation_tracker,
                                   game->culture_system->identity_manager);
    }
    
    /* Update system orchestrator (coordinates all systems) */
    if (game->system_orchestrator) {
        #ifdef DEBUG
        clock_t _profile_start = clock();
        #endif
        civ_system_orchestrator_update_all(game->system_orchestrator, time_delta);
        #ifdef DEBUG
        if (game->performance_optimizer && game->performance_optimizer->profiling_enabled) {
            clock_t _profile_end = clock();
            civ_float_t _profile_time = ((civ_float_t)(_profile_end - _profile_start)) / CLOCKS_PER_SEC * 1000.0f;
            civ_performance_optimizer_record_metric(game->performance_optimizer, "system_orchestrator_update", _profile_time, 0.0f);
        }
        #endif
    }
    
    /* Cleanup expired cache entries periodically */
    static time_t last_cache_cleanup = 0;
    time_t now = time(NULL);
    if (game->cache && now - last_cache_cleanup > 60) {  /* Every minute */
        civ_cache_cleanup_expired(game->cache);
        last_cache_cleanup = now;
    }
}

