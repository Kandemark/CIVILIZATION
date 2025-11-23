/**
 * @file main.c
 * @brief Main entry point for the Civilization simulation
 */

#include "../include/core/game.h"
#include "../include/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

static void print_usage(const char* program_name) {
    printf("Civilization Game - Advanced Geopolitical Simulation\n");
    printf("Usage: %s [options]\n\n", program_name);
    printf("Options:\n");
    printf("  --new              Start a new game\n");
    printf("  --load <file>      Load a saved game from file\n");
    printf("  --config <file>    Path to configuration file\n");
    printf("  --speed <float>    Initial game speed multiplier (default: 1.0)\n");
    printf("  --headless         Run in headless mode (no UI)\n");
    printf("  --benchmark        Run performance benchmark mode\n");
    printf("  --help             Show this help message\n");
}

static void run_benchmark(void) {
    printf("Starting performance benchmark...\n");
    printf("-----------------------------------\n");
    
    civ_game_t* game = civ_game_create();
    if (!game) {
        fprintf(stderr, "Failed to create game for benchmark\n");
        return;
    }
    
    civ_game_config_t config;
    civ_game_get_default_config(&config);
    civ_game_initialize(game, &config);
    
    printf("Benchmarking system performance...\n");
    printf("Test duration: 10 seconds\n\n");
    
    time_t start_time = time(NULL);
    time_t end_time = start_time + 10;
    uint64_t update_count = 0;
    
    while (time(NULL) < end_time) {
        civ_game_update(game);
        update_count++;
    }
    
    time_t total_time = time(NULL) - start_time;
    civ_float_t updates_per_second = (civ_float_t)update_count / (civ_float_t)total_time;
    
    printf("Benchmark Results:\n");
    printf("-------------------\n");
    printf("Total updates: %llu\n", (unsigned long long)update_count);
    printf("Total time: %ld seconds\n", total_time);
    printf("Updates per second: %.2f\n", updates_per_second);
    printf("Time per update: %.3f ms\n", 
           (civ_float_t)total_time * 1000.0f / (civ_float_t)update_count);
    
    if (game->time_manager) {
        char date_str[256];
        civ_calendar_get_date_string(&game->time_manager->calendar, date_str, sizeof(date_str));
        printf("Game date: %s\n", date_str);
    }
    
    civ_game_destroy(game);
}

int main(int argc, char* argv[]) {
    printf("Civilization Game - Advanced Geopolitical Simulation\n");
    printf("====================================================\n\n");
    
    /* Parse command line arguments */
    bool new_game = false;
    bool load_game = false;
    char* load_file = NULL;
    char* config_file = NULL;
    civ_float_t speed = 1.0f;
    bool headless = false;
    bool benchmark = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--new") == 0) {
            new_game = true;
        } else if (strcmp(argv[i], "--load") == 0 && i + 1 < argc) {
            load_game = true;
            load_file = argv[++i];
        } else if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            config_file = argv[++i];
        } else if (strcmp(argv[i], "--speed") == 0 && i + 1 < argc) {
            speed = (civ_float_t)atof(argv[++i]);
        } else if (strcmp(argv[i], "--headless") == 0) {
            headless = true;
        } else if (strcmp(argv[i], "--benchmark") == 0) {
            benchmark = true;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }
    
    /* Run benchmark mode */
    if (benchmark) {
        run_benchmark();
        return 0;
    }
    
    /* Create and initialize game */
    civ_game_t* game = civ_game_create();
    if (!game) {
        fprintf(stderr, "Failed to create game instance\n");
        return 1;
    }
    
    civ_game_config_t config;
    civ_game_get_default_config(&config);
    
    /* Load configuration from file if specified */
    if (config_file) {
        /* TODO: Load configuration from file */
        printf("Loading configuration from: %s\n", config_file);
    }
    
    /* Initialize game */
    civ_result_t result = civ_game_initialize(game, &config);
    if (CIV_FAILED(result)) {
        fprintf(stderr, "Failed to initialize game: %s\n", result.message);
        civ_game_destroy(game);
        return 1;
    }
    
    /* Set initial game speed */
    if (speed != 1.0f && game->time_manager) {
        civ_time_manager_adjust_speed(game->time_manager, speed);
        printf("Game speed set to: %.2fx\n", speed);
    }
    
    /* Load game if specified */
    if (load_game && load_file) {
        printf("Loading game from: %s\n", load_file);
        result = civ_game_load(game, load_file);
        if (CIV_FAILED(result)) {
            printf("Error loading game: %s\n", result.message);
            printf("Starting new game instead...\n");
        } else {
            printf("Game loaded successfully!\n");
        }
    } else {
        printf("Starting new game...\n");
    }
    
    /* Display initial status */
    if (game->time_manager) {
        char date_str[256];
        civ_calendar_get_date_string(&game->time_manager->calendar, date_str, sizeof(date_str));
        printf("\nInitial Game Status:\n");
        printf("-------------------\n");
        printf("Game Date: %s\n", date_str);
        printf("Time Scale: Normal\n");
    }
    
    if (headless) {
        printf("\nRunning in headless mode...\n");
        printf("Press Ctrl+C to stop the simulation\n\n");
        
        /* Run game loop */
        civ_game_run(game);
        
        printf("\nSimulation stopped by user.\n");
    } else {
        printf("\nStarting interactive mode...\n");
        printf("UI mode not yet implemented. Running in headless mode instead.\n");
        civ_game_run(game);
    }
    
    /* Cleanup */
    civ_game_shutdown(game);
    civ_game_destroy(game);
    
    return 0;
}

