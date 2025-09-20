"""Main game controller"""

"""
Enhanced main game controller with better system integration and event handling.
"""

import time
from typing import Dict, Any, List, Optional
import json
from datetime import datetime
from enum import Enum
import random

from core.simulation_engine.time_manager import TimeManager, TimeScale
from core.abstracts.soft_metrics import SoftMetricsManager, Mood
from core.dependency_mapping.dependency_graph import DependencyGraph, create_default_dependency_graph
from data.serialization.json_serializer import JsonSerializer

# Imports from correct paths according to structure
from core.population.demographics import PopulationManager
from core.economy.market_dynamics import MarketEconomy
from core.technology.innovation_system import TechnologyTree
from core.military.combat import CombatSystem
from core.diplomacy.relations import DiplomacySystem, RelationLevel
from core.military.combat import CombatSystem
from core.diplomacy.relations import DiplomacySystem
from core.military.units import UnitType
from core.military.formations import FormationType, Formation

class GameState(Enum):
    """Possible states of the game."""
    INITIALIZING = "initializing"
    RUNNING = "running"
    PAUSED = "paused"
    SAVING = "saving"
    LOADING = "loading"
    SHUTTING_DOWN = "shutting_down"

class CivilizationGame:
    """Enhanced main game controller class."""
    
    def __init__(self):
        self.time_manager = TimeManager()
        self.soft_metrics = SoftMetricsManager()
        self.dependency_graph = create_default_dependency_graph()
        self.serializer = JsonSerializer()
        self.state = GameState.INITIALIZING
        self.systems = {}
        self.event_log = []
        self.performance_metrics = {
            'update_count': 0,
            'avg_update_time': 0,
            'last_update_time': 0
        }
        self.config = {}
        self.is_running = True   # or False depending on startup behavior
        self.is_paused = False   # optional if you plan to support pause
        
        # Register time listeners
        self.time_manager.add_time_listener(self._on_time_event)
        self.population_manager = PopulationManager()
        self.market_economy = MarketEconomy()
        self.technology_tree = TechnologyTree()

        self.military_system = CombatSystem()
        self.diplomacy_system = DiplomacySystem()
        
        self.is_running = False
        self.event_log = []
        self.nations = ['player_nation', 'ai_nation_1', 'ai_nation_2']  # Example nations
    
    def initialize_game(self, config: Dict[str, Any] = None):
        """Initialize the game with optional configuration."""
        self.state = GameState.INITIALIZING
        
        if config is None:
            config = self._load_default_config()
        
        self.config = config
        
        print("🚀 Initializing Civilization Game...")
        print("=" * 50)
        
        # Initialize all systems
        self._initialize_systems(config)

        print("✅ All systems initialized:")
        for system_name in ['Time', 'Population', 'Economy', 'Technology', 'Military', 'Diplomacy']:
            print(f"  - {system_name}")
        
        # Set initial time scale from config
        time_scale_name = config.get('game', {}).get('time_scale', 'NORMAL')
        try:
            self.time_manager.set_time_scale(TimeScale[time_scale_name])
        except KeyError:
            print(f"Warning: Unknown time scale '{time_scale_name}', using NORMAL")
            self.time_manager.set_time_scale(TimeScale.NORMAL)
        
        # Analyze system dependencies
        if config.get('simulation', {}).get('enable_dependency_tracking', True):
            self._analyze_system_dependencies()
        
        self.state = GameState.RUNNING
        print("✅ Game initialization complete!")
        print("=" * 50)
    
    def _load_default_config(self) -> Dict[str, Any]:
        """Load default game configuration."""
        return {
            'game': {
                'name': 'Civilization Simulation',
                'version': '0.1.0',
                'start_year': 1,
                'time_scale': 'NORMAL',
                'difficulty': 'medium'
            },
            'simulation': {
                'enable_fuzzy_logic': True,
                'enable_dependency_tracking': True
            }
        }
    
    def _initialize_systems(self, config: Dict[str, Any]):
        """Initialize all game systems."""
        # Initialize core systems
        self.systems = {
            'time': self.time_manager,
            'soft_metrics': self.soft_metrics,
            'dependency_graph': self.dependency_graph
        }
        
        # Initialize fuzzy logic if enabled
        if config.get('simulation', {}).get('enable_fuzzy_logic', True):
            from core.abstracts.fuzzy_logic import create_happiness_fuzzy_system
            self.systems['fuzzy_inference'] = create_happiness_fuzzy_system()
        
        print("✅ Systems initialized:")
        for system_name in self.systems.keys():
            print(f"  - {system_name}")

    def _initialize_military_forces(self):
        """Initialize military forces for all nations."""
        for nation in self.nations:
            # Add some basic military units for each nation
            self.military_system.recruit_units(nation, UnitType.INFANTRY, 1000, 0.7)
            self.military_system.recruit_units(nation, UnitType.ARCHER, 500, 0.6)
            self.military_system.recruit_units(nation, UnitType.CAVALRY, 300, 0.8)
    
    def _analyze_system_dependencies(self):
        """Analyze and report on system dependencies."""
        print("\n📊 System Dependency Analysis:")
        print("-" * 40)
        
        centrality = self.dependency_graph.calculate_system_centrality()
        print("System Centrality (importance):")
        for system, score in sorted(centrality.items(), key=lambda x: x[1], reverse=True):
            print(f"  {system}: {score:.3f}")
        
        critical_paths = self.dependency_graph.find_critical_paths()
        if critical_paths:
            print(f"\n🔍 Critical Paths found: {len(critical_paths)}")
            for i, path in enumerate(critical_paths[:3]):
                print(f"  Path {i+1}: {' → '.join(path)}")
    
    def _on_time_event(self, event_type: str, calendar):
        """Handle time-based events."""
        event = {
            'type': f'time_{event_type}',
            'timestamp': datetime.now().isoformat(),
            'game_date': calendar.get_date_string(),
            'description': f"Advanced to {calendar.get_date_string()}",
            'importance': 0.1  # Low importance for regular time events
        }
        self.event_log.append(event)
        
        # Keep event log manageable
        max_events = self.config.get('performance', {}).get('max_event_log', 1000)
        if len(self.event_log) > max_events:
            self.event_log = self.event_log[-max_events:]
        
        if event_type == 'year':
            print(f"\n🎉 New Year: {calendar.year}")
            self._on_new_year()
        elif event_type == 'month':
            self._on_new_month()
        elif event_type == 'day':
            self._on_new_day()
    
    def _on_new_year(self):
        """Handle new year events."""
        # Update soft metrics based on yearly changes
        economic_data = self._get_economic_data()
        self.soft_metrics.update_from_economy(economic_data)
        
        # Generate random events for the year
        yearly_events = self._generate_yearly_events()
        self.soft_metrics.update_from_events(yearly_events)
        
        # Print yearly report
        self._print_yearly_report()
    
    def _on_new_month(self):
        """Handle new month events."""
        # Monthly economic fluctuations
        economic_shift = (random.random() - 0.5) * 0.1
        self.soft_metrics.happiness_metrics.add_change(economic_shift)
    
    def _on_new_day(self):
        """Handle new day events."""
        # Daily minor fluctuations
        daily_shift = (random.random() - 0.5) * 0.01
        self.soft_metrics.happiness_metrics.add_change(daily_shift)
    
    def _get_economic_data(self) -> Dict[str, float]:
        """Get current economic data with realistic fluctuations."""
        base_gdp = 0.6
        base_unemployment = 0.3
        base_growth = 0.02
        
        # Add some realistic fluctuations
        year_factor = self.time_manager.calendar.year / 100
        seasonal_factor = 0.1 * (self.time_manager.calendar.month / 6 - 1)  # -0.1 to 0.1
        
        return {
            'gdp_per_capita': max(0.1, min(1.0, base_gdp + year_factor * 0.1 + seasonal_factor)),
            'unemployment': max(0.1, min(0.8, base_unemployment - year_factor * 0.05 + seasonal_factor * 0.5)),
            'growth_rate': max(-0.05, min(0.1, base_growth + year_factor * 0.01 + random.random() * 0.02))
        }
    
    def _generate_yearly_events(self) -> List[Dict]:
        """Generate random events for the year."""
        events = []
        
        # Economic events
        if random.random() < 0.3:
            events.append({
                'type': 'economic_boom',
                'happiness_impact': 0.1,
                'legitimacy_impact': 0.05,
                'importance': 0.7
            })
        
        # Natural events
        if random.random() < 0.2:
            events.append({
                'type': 'good_harvest',
                'happiness_impact': 0.08,
                'importance': 0.5
            })
        
        return events
    
    def _print_yearly_report(self):
        """Print a comprehensive yearly status report."""
        metrics = self.soft_metrics.to_dict()
        fuzzy_assessment = self.soft_metrics.get_fuzzy_assessment()
        mood = self.soft_metrics.happiness_metrics.get_mood()
        
        print("\n📊 Yearly Report:")
        print("=" * 40)
        print(f"Year: {self.time_manager.calendar.year}")
        print(f"Season: {self.time_manager.calendar.get_season()}")
        print(f"Total Days: {self.time_manager.calendar.total_days}")
        print()
        
        print("📈 Metrics:")
        print(f"  Happiness: {metrics['happiness']:.3f} ({mood.value}/5 - {mood.name})")
        print(f"  Legitimacy: {metrics['legitimacy']:.3f}")
        print(f"  Prestige: {metrics['prestige']:.3f}")
        print()
        
        print("🎯 Fuzzy Assessment:")
        for metric, level in fuzzy_assessment.items():
            print(f"  {metric.capitalize()}: {level}")
        print()
        
        print("⚡ Performance:")
        print(f"  Updates: {self.performance_metrics['update_count']}")
        print(f"  Avg Update Time: {self.performance_metrics['avg_update_time']:.3f}ms")
    
    def run(self):
        """Main game loop with enhanced performance tracking."""
        self.state = GameState.RUNNING
        
        print("\n🎮 Starting game loop...")
        print("Press Ctrl+C to stop or 'p' to pause")
        
        update_times = []
        last_autosave = time.time()
        autosave_interval = self.config.get('performance', {}).get('autosave_interval', 300)
        
        try:
            while self.state == GameState.RUNNING:
                start_time = time.time()
                
                # Update all systems
                self.update()
                
                # Calculate update time
                update_time = (time.time() - start_time) * 1000  # Convert to ms
                update_times.append(update_time)
                
                # Keep only recent timings for average
                if len(update_times) > 100:
                    update_times = update_times[-100:]
                
                # Update performance metrics
                self.performance_metrics['update_count'] += 1
                self.performance_metrics['last_update_time'] = update_time
                self.performance_metrics['avg_update_time'] = sum(update_times) / len(update_times)
                
                # Check for autosave
                current_time = time.time()
                if current_time - last_autosave >= autosave_interval:
                    self._autosave()
                    last_autosave = current_time
                
                # Small delay to prevent CPU overload
                time.sleep(0.01)
                
        except KeyboardInterrupt:
            print("\n🛑 Game stopped by user")
        finally:
            self.shutdown()
    
    def update(self):
        """Update all game systems with error handling."""
        try:
            # Update time first
            time_delta = self.time_manager.update()
            
            if time_delta > 0:
                # Update other systems based on time progression
                self._update_systems(time_delta)
                
        except Exception as e:
            print(f"❌ Error in game update: {e}")
            # Log the error but don't crash the game
            self.event_log.append({
                'type': 'system_error',
                'timestamp': datetime.now().isoformat(),
                'error': str(e),
                'importance': 0.8
            })
    
    def _update_systems(self, time_delta: float):
        """Update all game systems based on time progression."""
        # Update soft metrics with current economic data
        economic_data = self._get_economic_data()
        self.soft_metrics.update_from_economy(economic_data)
        
        # Run fuzzy inference if enabled
        if 'fuzzy_inference' in self.systems:
            fuzzy_inputs = {
                'economy': economic_data['gdp_per_capita'],
                'safety': 0.7  # Placeholder for actual safety data
            }
            fuzzy_result = self.systems['fuzzy_inference'].infer(fuzzy_inputs)
            
            # Use fuzzy results to adjust metrics
            if 'happiness' in fuzzy_result:
                happiness_adjustment = (fuzzy_result['happiness'] - 0.5) * 0.1
                self.soft_metrics.happiness_metrics.add_change(happiness_adjustment)
    
    def _autosave(self):
        """Perform automatic save of the game state."""
        if self.state != GameState.RUNNING:
            return
        
        try:
            self.state = GameState.SAVING
            filename = f"autosave_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
            self.save_game(filename)
            print(f"💾 Autosave completed: {filename}")
        except Exception as e:
            print(f"❌ Autosave failed: {e}")
        finally:
            self.state = GameState.RUNNING
    
    def save_game(self, filename: str):
        """Save the game state to a file."""
        game_state = {
            'time': self.time_manager.to_dict(),
            'soft_metrics': self.soft_metrics.to_dict(),
            'version': self.config.get('game', {}).get('version', '1.0.0'),
            'save_date': datetime.now().isoformat(),
            'performance_metrics': self.performance_metrics
        }
        
        self.serializer.save_to_file(game_state, filename)
        print(f"✅ Game saved to {filename}")
    
    def load_game(self, filename: str):
        """Load a game state from a file."""
        self.state = GameState.LOADING
        
        try:
            game_state = self.serializer.load_from_file(filename)
            
            if game_state:
                self.time_manager = TimeManager.from_dict(game_state['time'])
                # Would load other systems here
                
                # Update performance metrics if available
                if 'performance_metrics' in game_state:
                    self.performance_metrics.update(game_state['performance_metrics'])
                
                print(f"✅ Game loaded from {filename}")
                
        except Exception as e:
            raise ValueError(f"Error loading game: {e}")
        finally:
            self.state = GameState.RUNNING
    
    def pause(self):
        """Pause the game."""
        if self.state == GameState.RUNNING:
            self.state = GameState.PAUSED
            self.time_manager.set_time_scale(TimeScale.PAUSED)
            print("⏸️  Game paused")
    
    def resume(self):
        """Resume the game."""
        if self.state == GameState.PAUSED:
            self.state = GameState.RUNNING
            self.time_manager.set_time_scale(TimeScale.NORMAL)
            print("▶️  Game resumed")
    
    def shutdown(self):
        """Clean shutdown of the game."""
        self.state = GameState.SHUTTING_DOWN
        print("🛑 Shutting down game...")
        
        # Perform final autosave
        try:
            self._autosave()
        except Exception as e:
            print(f"❌ Final autosave failed: {e}")
        
        print("✅ Game shutdown complete")
    
    def get_status(self) -> Dict[str, Any]:
        """Get comprehensive game status."""
        metrics = self.soft_metrics.to_dict()
        mood = self.soft_metrics.happiness_metrics.get_mood()
        
        return {
            'state': self.state.value,
            'running': self.state == GameState.RUNNING,
            'game_date': self.time_manager.calendar.get_date_string(),
            'time_scale': self.time_manager.time_scale.name,
            'game_speed': self.time_manager.game_speed,
            'metrics': metrics,
            'mood': mood.name,
            'mood_value': mood.value,
            'events_count': len(self.event_log),
            'performance': self.performance_metrics,

            'population': self.population_manager.to_dict(),
            'economy': self.market_economy.get_economic_report(),
            'technology': {
                'level': self.technology_tree.tech_level,
                'researched_count': len(self.technology_tree.researched_techs),
                'research_budget': self.technology_tree.research_budget
            },
        }
    
    def _initialize_systems(self, config: Dict[str, Any]):
        """Initialize all game systems with new additions."""
        self.systems = {
            'time': self.time_manager,
            'soft_metrics': self.soft_metrics,
            'dependency_graph': self.dependency_graph
        }
        
        # Initialize new systems
        self.systems['population'] = PopulationManager()
        self.systems['economy'] = MarketEconomy()
        self.systems['technology'] = TechnologyTree()
        
        # Initialize regions
        self._initialize_regions(config)
        
        # Initialize fuzzy logic if enabled
        if config.get('simulation', {}).get('enable_fuzzy_logic', True):
            from core.abstracts.fuzzy_logic import create_happiness_fuzzy_system
            self.systems['fuzzy_inference'] = create_happiness_fuzzy_system()
        
        print("✅ Systems initialized:")
        for system_name in self.systems.keys():
            print(f"  - {system_name}")
    
    def _initialize_regions(self, config: Dict[str, Any]):
        """Initialize game regions with populations."""
        region_config = config.get('world', {}).get('regions', {})
        
        for region_id, region_data in region_config.items():
            population = region_data.get('initial_population', 1000)
            self.systems['population'].initialize_region(region_id, population)
    
    def _update_systems(self, time_delta: float):
        """Update all game systems with new integrations."""
        # Get current population data
        population_data = self.systems['population'].demographics['capital'].to_dict()
        
        # Update economy with population data and tech level
        tech_level = self.systems['technology'].tech_level
        economic_report = self.systems['economy'].update_market(time_delta, population_data, tech_level)
        
        # Update population with economic conditions
        economic_conditions = {
            'unemployment': economic_report['unemployment_rate'],
            'gdp_per_capita': economic_report['gdp_per_capita'],
            'healthcare_quality': 0.6 + tech_level * 0.3,
            'food_supply': 0.8 - economic_report['inflation_rate'] * 0.5
        }
        self.systems['population'].demographics['capital'].update_population(
            time_delta, economic_conditions)
        
        # Update technology research
        completed_techs = self.systems['technology'].update_research(time_delta)
        for tech in completed_techs:
            print(f"🔬 Technology researched: {tech.name}")
        
        # Update soft metrics with current data
        self.soft_metrics.update_from_economy(economic_report)
        
        # Check for innovations
        innovation = self.systems['technology'].innovation_system.check_innovation(
            tech_level, self.systems['technology'].research_budget
        )
        if innovation:
            print(f"💡 Innovation: {innovation['type']} (+{innovation['magnitude']*100:.1f}% {innovation['effect']})")
    
    def get_comprehensive_status(self) -> Dict[str, Any]:
        """Get comprehensive status of all systems."""
        base_status = self.get_status()
        
        # Add economic data
        economic_data = self.systems['economy'].get_economic_report()
        
        # Add population data
        population_data = self.systems['population'].demographics['capital'].to_dict()
        
        # Add technology data
        tech_data = {
            'tech_level': self.systems['technology'].tech_level,
            'researched_technologies': [tech.name for tech in self.systems['technology'].researched_technologies],
            'current_research': self.systems['technology'].current_research.name if self.systems['technology'].current_research else None,
            'research_progress': self.systems['technology'].research_progress,
            'research_budget': self.systems['technology'].research_budget
        }
        
        # Add soft metrics data
        soft_metrics_data = {
            'happiness': self.soft_metrics.happiness,
            'stability': self.soft_metrics.stability,
            'cultural_cohesion': self.soft_metrics.cultural_cohesion,
            'environmental_health': self.soft_metrics.environmental_health
        }
        
        # Combine all data into comprehensive status
        comprehensive_status = {
            **base_status,
            'economy': economic_data,
            'population': population_data,
            'technology': tech_data,
            'soft_metrics': soft_metrics_data,
            'timestamp': self.time_manager.get_formatted_time(),
            'total_population': self.systems['population'].get_total_population(),
            'regions': list(self.systems['population'].demographics.keys())
        }
        
        return comprehensive_status
    
    def get_total_population(self) -> int:
        """Get total population across all regions."""
        return self.systems['population'].get_total_population()
    
    def set_research_focus(self, technology_name: str, budget_allocation: float = 0.2):
        """Set research focus and budget allocation."""
        self.systems['technology'].set_research_focus(technology_name, budget_allocation)
        print(f"🔬 Research focus set to: {technology_name} (Budget: {budget_allocation*100:.1f}%)")
    
    def adjust_economic_policy(self, policy_type: str, value: float):
        """Adjust economic policy parameters."""
        if policy_type == 'tax_rate':
            self.systems['economy'].tax_rate = value
            print(f"💰 Tax rate adjusted to: {value*100:.1f}%")
        elif policy_type == 'government_spending':
            self.systems['economy'].government_spending = value
            print(f"🏛️ Government spending adjusted to: {value*100:.1f}%")
        else:
            print(f"⚠️ Unknown policy type: {policy_type}")

    def _print_status_update(self):
        """Print a status update."""
        pop_data = self.population_manager.to_dict()
        econ_data = self.market_economy.get_economic_report()
        tech_data = self.technology_tree.get_research_bonuses()
        
        print(f"\n📊 Status Update - {self.time_manager.calendar.get_date_string()}")
        print(f"🏛️  Population: {pop_data['total_population']:,} (Workforce: {pop_data['workforce_size']:,})")
        print(f"💰 Economy: GDP ${econ_data['gdp']:,.0f} (${econ_data['gdp_per_capita']:,.1f}/capita)")
        print(f"🔬 Technology: Level {self.technology_tree.tech_level:.2f}")
        print(f"   Bonuses: Food +{tech_data['food_production']*100:.1f}%, Research +{tech_data['research_speed']*100:.1f}%")
    
    def _on_time_event(self, event_type: str, calendar):
        """Handle time-based events."""
        event = {
            'type': f'time_{event_type}',
            'timestamp': time.time(),
            'game_date': calendar.get_date_string()
        }
        self.event_log.append(event)
        
        if event_type == 'year':
            print(f"\n🎉 New Year: {calendar.year}")
            self._on_new_year()
    
    def _on_new_year(self):
        """Handle new year events."""
        # Annual report
        self._print_annual_report()
    
    def _print_annual_report(self):
        """Print annual status report."""
        pop_data = self.population_manager.to_dict()
        econ_data = self.market_economy.get_economic_report()
        
        print("\n📈 Annual Report:")
        print("=" * 40)
        print(f"Year: {self.time_manager.calendar.year}")
        print(f"Population: {pop_data['total_population']:,} (+{pop_data.get('growth', 0):+})")
        print(f"GDP: ${econ_data['gdp']:,.0f} (${econ_data['gdp_per_capita']:,.1f}/capita)")
        print(f"Tech Level: {self.technology_tree.tech_level:.3f}")
        print(f"Researched Technologies: {len(self.technology_tree.researched_techs)}")
        
    def shutdown(self):
        """Clean shutdown of the game."""
        self.is_running = False
        print("Shutting down game...")

    def _handle_military_event(self):
        """Handle random military events."""
        if len(self.nations) >= 2 and random.random() < 0.2:
            attacker, defender = random.sample(self.nations, 2)
            
            # Check if nations are not allied
            relations = self.diplomacy_system.relations[attacker][defender]
            if relations.relation_level.value <= RelationLevel.NEUTRAL.value:
                
                battle_result = self.military_system.simulate_battle(
                    attacker, defender, random.choice(['plains', 'forest', 'mountains'])
                )
                
                print(f"⚔️  Battle between {attacker} and {defender}: {battle_result['result']}")
                self.event_log.append({
                    'type': 'battle',
                    'attacker': attacker,
                    'defender': defender,
                    'result': battle_result['result'],
                    'casualties': {
                        'attacker': battle_result['attacker_casualties'],
                        'defender': battle_result['defender_casualties']
                    }
                })

def main():
    """Main entry point for the game."""
    game = CivilizationGame()
    game.initialize_game()
    game.run()

if __name__ == "__main__":
    main()