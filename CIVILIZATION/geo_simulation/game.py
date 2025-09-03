"""Main game controller"""

"""
Main game controller that orchestrates all simulation systems.
"""

import time
from typing import Dict, Any, List
import json
from datetime import datetime

from core.simulation_engine.time_manager import TimeManager, TimeScale
from core.abstracts.soft_metrics import SoftMetricsManager
from core.dependency_mapping.dependency_graph import DependencyGraph, create_default_dependency_graph
from data.serialization.json_serializer import JsonSerializer
import random

class CivilizationGame:
    """Main game controller class."""
    
    def __init__(self):
        self.time_manager = TimeManager()
        self.soft_metrics = SoftMetricsManager()
        self.dependency_graph = create_default_dependency_graph()
        self.serializer = JsonSerializer()
        self.is_running = False
        self.systems = {}
        self.event_log = []
        
        # Register time listeners
        self.time_manager.add_time_listener(self._on_time_event)
    
    def initialize_game(self, config: Dict[str, Any] = None):
        """Initialize the game with optional configuration."""
        if config is None:
            config = self._load_default_config()
        
        print("Initializing Civilization Game...")
        print("=" * 50)
        
        # Initialize all systems
        self._initialize_systems(config)
        
        # Analyze system dependencies
        self._analyze_system_dependencies()
        
        print("Game initialization complete!")
        print("=" * 50)
    
    def _load_default_config(self) -> Dict[str, Any]:
        """Load default game configuration."""
        return {
            'game_speed': 1.0,
            'start_year': 1,
            'difficulty': 'normal',
            'world_size': 'medium'
        }
    
    def _initialize_systems(self, config: Dict[str, Any]):
        """Initialize all game systems."""
        # This would initialize all the other systems in a real implementation
        self.systems = {
            'time': self.time_manager,
            'soft_metrics': self.soft_metrics,
            'dependency_graph': self.dependency_graph
            # Other systems would be added here
        }
        
        print("Systems initialized:")
        for system_name in self.systems.keys():
            print(f"  - {system_name}")
    
    def _analyze_system_dependencies(self):
        """Analyze and report on system dependencies."""
        print("\nSystem Dependency Analysis:")
        print("-" * 30)
        
        centrality = self.dependency_graph.calculate_system_centrality()
        print("System Centrality (importance):")
        for system, score in sorted(centrality.items(), key=lambda x: x[1], reverse=True):
            print(f"  {system}: {score:.3f}")
        
        critical_paths = self.dependency_graph.find_critical_paths()
        if critical_paths:
            print(f"\nCritical Paths found: {len(critical_paths)}")
            for i, path in enumerate(critical_paths[:3]):  # Show first 3
                print(f"  Path {i+1}: {' -> '.join(path)}")
    
    def _on_time_event(self, event_type: str, calendar):
        """Handle time-based events."""
        event = {
            'type': f'time_{event_type}',
            'timestamp': datetime.now().isoformat(),
            'game_date': calendar.get_date_string(),
            'description': f"Advanced to {calendar.get_date_string()}"
        }
        self.event_log.append(event)
        
        if event_type == 'year':
            print(f"\n🎉 New Year: {calendar.year}")
            self._on_new_year()
        elif event_type == 'month':
            self._on_new_month()
    
    def _on_new_year(self):
        """Handle new year events."""
        # Update soft metrics based on yearly changes
        economic_data = self._get_economic_data()  # Would be implemented
        self.soft_metrics.update_from_economy(economic_data)
        
        # Print yearly report
        self._print_yearly_report()
    
    def _on_new_month(self):
        """Handle new month events."""
        # Monthly updates would go here
        pass
    
    def _get_economic_data(self) -> Dict[str, float]:
        """Get current economic data (placeholder implementation)."""
        # In a real game, this would come from the economy system
        return {
            'gdp_per_capita': 0.6 + (random.random() * 0.2),
            'unemployment': 0.3 + (random.random() * 0.2),
            'growth_rate': 0.02 + (random.random() * 0.03)
        }
    
    def _print_yearly_report(self):
        """Print a yearly status report."""
        metrics = self.soft_metrics.to_dict()
        fuzzy_assessment = self.soft_metrics.get_fuzzy_assessment()
        
        print("\n📊 Yearly Report:")
        print("-" * 30)
        print(f"Happiness: {metrics['happiness']:.2f} ({metrics['mood']})")
        print(f"Legitimacy: {metrics['legitimacy']:.2f}")
        print(f"Prestige: {metrics['prestige']:.2f}")
        print(f"Fuzzy Assessment: {fuzzy_assessment}")
    
    def run(self):
        """Main game loop."""
        self.is_running = True
        print("\nStarting game loop...")
        print("Press Ctrl+C to stop")
        
        try:
            while self.is_running:
                # Update all systems
                self.update()
                
                # Small delay to prevent CPU overload
                time.sleep(0.01)
                
        except KeyboardInterrupt:
            print("\nGame stopped by user")
        finally:
            self.shutdown()
    
    def update(self):
        """Update all game systems."""
        # Update time first
        time_delta = self.time_manager.update()
        
        if time_delta > 0:
            # Update other systems based on time progression
            self._update_systems(time_delta)
    
    def _update_systems(self, time_delta: float):
        """Update all game systems based on time progression."""
        # This would update all systems in a real implementation
        pass
    
    def save_game(self, filename: str):
        """Save the game state to a file."""
        game_state = {
            'time': self.time_manager.to_dict(),
            'soft_metrics': self.soft_metrics.to_dict(),
            'version': '1.0.0',
            'save_date': datetime.now().isoformat()
        }
        
        self.serializer.save_to_file(game_state, filename)
        print(f"Game saved to {filename}")
    
    def load_game(self, filename: str):
        """Load a game state from a file."""
        game_state = self.serializer.load_from_file(filename)
        
        if game_state:
            self.time_manager = TimeManager.from_dict(game_state['time'])
            # Would load other systems here
            print(f"Game loaded from {filename}")
    
    def shutdown(self):
        """Clean shutdown of the game."""
        self.is_running = False
        print("Shutting down game...")
    
    def get_status(self) -> Dict[str, Any]:
        """Get current game status."""
        return {
            'running': self.is_running,
            'game_date': self.time_manager.calendar.get_date_string(),
            'time_scale': self.time_manager.time_scale.name,
            'metrics': self.soft_metrics.to_dict(),
            'events_count': len(self.event_log)
        }

def main():
    """Main entry point for the game."""
    game = CivilizationGame()
    
    # Initialize with default config
    game.initialize_game()
    
    # Start the game loop
    game.run()

if __name__ == "__main__":
    main()