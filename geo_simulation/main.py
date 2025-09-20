"""Game entry point"""

#!/usr/bin/env python3
"""
Civilization Game - Main Entry Point
A sophisticated geopolitical simulation game with dynamic systems.
"""

import argparse
import sys
import os
from pathlib import Path

# Add the project root to Python path
sys.path.insert(0, str(Path(__file__).parent.parent))

from geo_simulation.game import CivilizationGame
from geo_simulation.data.serialization.json_serializer import JsonSerializer
from geo_simulation.core.dependency_mapping.dependency_graph import create_default_dependency_graph, DependencyGraph

def setup_arg_parser():
    """Set up command line argument parser."""
    parser = argparse.ArgumentParser(description="Civilization Game Simulation")
    
    parser.add_argument(
        '--new', 
        action='store_true',
        help='Start a new game'
    )
    
    parser.add_argument(
        '--load', 
        type=str,
        help='Load a saved game from the specified file'
    )
    
    parser.add_argument(
        '--config', 
        type=str,
        default='config/default.json',
        help='Path to configuration file'
    )
    
    parser.add_argument(
        '--speed', 
        type=float,
        default=1.0,
        help='Initial game speed multiplier'
    )
    
    parser.add_argument(
        '--headless',
        action='store_true',
        help='Run in headless mode (no UI)'
    )
    
    parser.add_argument(
        '--benchmark',
        action='store_true',
        help='Run performance benchmark mode'
    )
    
    parser.add_argument(
        '--visualize-dependencies',
        action='store_true',
        help='Generate dependency graph visualization and exit'
    )
    
    return parser

def load_configuration(config_path: str) -> dict:
    """Load game configuration from file."""
    serializer = JsonSerializer()
    
    if not os.path.exists(config_path):
        print(f"Configuration file {config_path} not found. Using defaults.")
        return get_default_config()
    
    try:
        return serializer.load_from_file(config_path)
    except Exception as e:
        print(f"Error loading configuration: {e}")
        return get_default_config()

def get_default_config() -> dict:
    """Return default game configuration."""
    return {
        'game': {
            'name': 'Civilization Simulation',
            'version': '0.1.0',
            'start_year': 1,
            'time_scale': 'NORMAL',
            'difficulty': 'medium'
        },
        'world': {
            'size': 'medium',
            'climate_zones': True,
            'natural_disasters': True,
            'resource_distribution': 'balanced'
        },
        'simulation': {
            'enable_fuzzy_logic': True,
            'enable_dependency_tracking': True,
            'max_population': 10000000,
            'tech_progression': 'realistic'
        },
        'ai': {
            'aggressiveness': 0.5,
            'diplomacy_weight': 0.7,
            'expansion_weight': 0.6,
            'research_focus': 'balanced'
        },
        'performance': {
            'autosave_interval': 300,  # seconds
            'max_event_log': 1000,
            'cache_size': 1000
        }
    }

def visualize_dependencies():
    """Generate and display dependency graph visualization."""
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("Matplotlib is required for dependency visualization.")
        print("Install it with: pip install matplotlib")
        return
    
    print("Generating dependency graph visualization...")
    
    graph = create_default_dependency_graph()
    
    # Analyze the graph
    centrality = graph.calculate_system_centrality()
    critical_paths = graph.find_critical_paths()
    feedback_loops = graph.analyze_feedback_loops()
    
    print("\n📊 System Centrality Analysis:")
    print("-" * 40)
    for system, score in sorted(centrality.items(), key=lambda x: x[1], reverse=True):
        print(f"  {system:15}: {score:.3f}")
    
    print(f"\n🔍 Found {len(critical_paths)} critical paths:")
    for i, path in enumerate(critical_paths[:3]):
        print(f"  Path {i+1}: {' → '.join(path)}")
    
    print(f"\n🔄 Found {len(feedback_loops)} feedback loops:")
    for i, loop in enumerate(feedback_loops[:3]):
        print(f"  Loop {i+1}: {' → '.join(loop)}")
    
    # Generate visualization
    graph.visualize("dependency_graph.png")
    print(f"\n📈 Visualization saved as 'dependency_graph.png'")
    
    # Simulate some impacts
    print("\n🎯 Impact Simulation Examples:")
    print("-" * 40)
    
    test_systems = ['economy', 'technology', 'population']
    for system in test_systems:
        impacts = graph.simulate_impact(system, 0.5)
        print(f"\nImpact of {system} change:")
        for affected, impact in sorted(impacts.items(), key=lambda x: x[1], reverse=True)[:5]:
            if affected != system:
                print(f"  {affected:15}: {impact:.3f}")

def run_benchmark_mode():
    """Run the game in benchmark mode to test performance."""
    print("🚀 Starting performance benchmark...")
    print("-" * 50)
    
    from geo_simulation.core.simulation_engine.time_manager import TimeManager
    from geo_simulation.core.abstracts.soft_metrics import SoftMetricsManager
    import time
    
    # Create systems for benchmarking
    time_manager = TimeManager()
    soft_metrics = SoftMetricsManager()
    
    # Benchmark parameters
    test_duration = 10  # seconds
    update_count = 0
    
    print("Benchmarking system performance...")
    print(f"Test duration: {test_duration} seconds")
    print()
    
    start_time = time.time()
    end_time = start_time + test_duration
    
    # Performance test loop
    while time.time() < end_time:
        # Update time manager
        time_delta = time_manager.update()
        
        if time_delta > 0:
            # Update soft metrics with test data
            economic_data = {
                'gdp_per_capita': 0.6 + (update_count % 100) * 0.001,
                'unemployment': 0.3 + (update_count % 50) * 0.002,
                'growth_rate': 0.02 + (update_count % 20) * 0.001
            }
            soft_metrics.update_from_economy(economic_data)
            
            update_count += 1
    
    # Calculate results
    total_time = time.time() - start_time
    updates_per_second = update_count / total_time
    
    print("📊 Benchmark Results:")
    print("-" * 30)
    print(f"Total updates: {update_count}")
    print(f"Total time: {total_time:.2f} seconds")
    print(f"Updates per second: {updates_per_second:.2f}")
    print(f"Time per update: {(total_time/update_count)*1000:.3f} ms")
    
    # Show system status
    metrics = soft_metrics.to_dict()
    print(f"\nFinal happiness: {metrics['happiness']:.3f}")
    print(f"Final legitimacy: {metrics['legitimacy']:.3f}")
    print(f"Game date: {time_manager.calendar.get_date_string()}")

def main():
    """Main entry point for the Civilization game."""
    parser = setup_arg_parser()
    args = parser.parse_args()
    
    print("🌍 Civilization Game - Advanced Geopolitical Simulation")
    print("=" * 60)
    
    # Handle dependency visualization
    if args.visualize_dependencies:
        visualize_dependencies()
        return
    
    # Handle benchmark mode
    if args.benchmark:
        run_benchmark_mode()
        return
    
    # Load configuration
    config = load_configuration(args.config)
    
    # Create game instance
    game = CivilizationGame()
    
    # Initialize game
    if args.load:
        print(f"Loading game from: {args.load}")
        try:
            game.load_game(args.load)
            print("Game loaded successfully!")
        except Exception as e:
            print(f"Error loading game: {e}")
            print("Starting new game instead...")
            game.initialize_game(config)
    else:
        print("Starting new game...")
        game.initialize_game(config)
    
    # Set initial game speed
    if args.speed != 1.0:
        game.time_manager.adjust_game_speed(args.speed)
        print(f"Game speed set to: {args.speed}x")
    
    # Display initial status
    status = game.get_status()
    print(f"\n🏛️  Initial Game Status:")
    print("-" * 30)
    print(f"Game Date: {status['game_date']}")
    print(f"Time Scale: {status['time_scale']}")
    print(f"Happiness: {status['metrics']['happiness']:.2f}")
    print(f"Legitimacy: {status['metrics']['legitimacy']:.2f}")
    
    if args.headless:
        print("\n🚀 Running in headless mode...")
        print("Press Ctrl+C to stop the simulation")
        
        try:
            game.run()
        except KeyboardInterrupt:
            print("\nSimulation stopped by user.")
            
            # Ask about saving
            save = input("\n💾 Save game before exiting? (y/n): ").lower().strip()
            if save == 'y':
                filename = input("Enter save filename (default: autosave.json): ").strip()
                if not filename:
                    filename = "autosave.json"
                game.save_game(filename)
    else:
        print("\n🎮 Starting interactive mode...")
        # In a real implementation, this would launch the UI
        print("UI mode not yet implemented. Running in headless mode instead.")
        game.run()

if __name__ == "__main__":
    main()