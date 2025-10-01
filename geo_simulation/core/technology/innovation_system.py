"""Research and discovery processes"""

# Module implementation

"""
Enhanced Innovation System with Technology Trees and Breakthroughs
"""

from typing import Dict, List, Set, Optional, Tuple
from datetime import datetime, timedelta
import networkx as nx
import random
import math
from enum import Enum
from dataclasses import dataclass
from .innovation import Innovation, InnovationType, DevelopmentStage


class TechCategory(Enum):
    """Categories of technology."""
    AGRICULTURE = "agriculture"
    INDUSTRY = "industry"
    MILITARY = "military"
    SCIENCE = "science"
    MEDICINE = "medicine"
    TRANSPORTATION = "transportation"
    COMMUNICATION = "communication"
    ENERGY = "energy"
    COMPUTING = "computing"
    BIOTECH = "biotech"


class TechEra(Enum):
    """Technology eras with progression thresholds."""
    PREHISTORIC = (0, "prehistoric")
    ANCIENT = (1, "ancient")
    CLASSICAL = (2, "classical")
    MEDIEVAL = (3, "medieval")
    RENAISSANCE = (4, "renaissance")
    INDUSTRIAL = (5, "industrial")
    MODERN = (6, "modern")
    INFORMATION = (7, "information")
    FUTURE = (8, "future")
    
    def __init__(self, level: int, label: str):
        self.level = level
        self.label = label


class BreakthroughType(Enum):
    """Types of technological breakthroughs."""
    SCIENTIFIC_REVOLUTION = "scientific_revolution"
    INDUSTRIAL_REVOLUTION = "industrial_revolution"
    DIGITAL_REVOLUTION = "digital_revolution"
    BIOTECH_REVOLUTION = "biotech_revolution"
    QUANTUM_LEAP = "quantum_leap"
    PARADIGM_SHIFT = "paradigm_shift"


@dataclass
class TechnologyNode:
    """Represents a technology node in the tech tree."""
    id: str
    name: str
    description: str
    category: TechCategory
    era: TechEra
    base_research_cost: float
    prerequisites: Set[str]  # IDs of prerequisite technologies
    effects: Dict[str, float]  # Effects on various systems
    breakthrough_potential: float  # 0.0 to 1.0
    synergy_techs: Set[str]  # Technologies that synergize with this one
    
    def __post_init__(self):
        """Validate technology node data."""
        assert self.base_research_cost > 0, "Research cost must be positive"
        assert 0.0 <= self.breakthrough_potential <= 1.0, "Breakthrough potential must be between 0 and 1"


class ResearchTeam:
    """Represents a research team with specialized capabilities."""
    
    def __init__(self, team_id: str, size: int, expertise: Dict[TechCategory, float], 
                 focus_area: Optional[TechCategory] = None):
        self.team_id = team_id
        self.size = size
        self.expertise = expertise
        self.focus_area = focus_area
        self.current_project: Optional[str] = None  # Technology ID
        self.progress: float = 0.0
        self.funding: float = 0.0
        self.efficiency: float = 1.0
        self.experience: float = 0.0
        
    def assign_project(self, tech_id: str, funding: float) -> bool:
        """Assign a research project to this team."""
        self.current_project = tech_id
        self.progress = 0.0
        self.funding = funding
        return True
    
    def research_tick(self, time_delta: float, tech_node: TechnologyNode) -> Tuple[float, bool]:
        """
        Make research progress.
        
        Returns:
            Tuple of (progress_made, is_completed)
        """
        if not self.current_project:
            return 0.0, False
        
        # Calculate research efficiency
        category_expertise = self.expertise.get(tech_node.category, 0.3)
        focus_bonus = 1.2 if self.focus_area == tech_node.category else 1.0
        funding_efficiency = min(1.5, math.log(1 + self.funding / 1000))  # Diminishing returns
        experience_bonus = 1.0 + (self.experience * 0.01)
        
        research_rate = (
            self.size * 0.1 *
            category_expertise *
            focus_bonus *
            funding_efficiency *
            experience_bonus *
            self.efficiency *
            time_delta
        )
        
        self.progress += research_rate
        self.experience += research_rate * 0.01
        
        completed = self.progress >= tech_node.base_research_cost
        if completed:
            self.current_project = None
            self.progress = 0.0
            
        return research_rate, completed


class InnovationSystem:
    """
    Enhanced innovation system with technology trees, research teams, and breakthrough mechanics.
    """
    
    def __init__(self, name: str, description: str = ""):
        self.name = name
        self.description = description
        self.creation_date = datetime.now()
        
        # Core data structures
        self.innovations: Dict[str, Innovation] = {}
        self.technology_tree: Dict[str, TechnologyNode] = {}
        self.research_teams: Dict[str, ResearchTeam] = {}
        self.researched_techs: Set[str] = set()
        
        # Graph structures
        self.innovation_graph = nx.DiGraph()
        self.tech_tree_graph = nx.DiGraph()
        
        # System state
        self.research_budget: float = 0.0
        self.tech_level: float = 0.0
        self.scientific_tradition: float = 0.0
        self.breakthrough_counter: int = 0
        
        # Configuration
        self.innovation_chance: float = 0.01
        self.breakthrough_chance_base: float = 0.001
        
        self._initialize_technology_tree()
    
    def _initialize_technology_tree(self):
        """Initialize the core technology tree."""
        # Ancient era technologies
        self._add_technology_node(TechnologyNode(
            id="basic_agriculture",
            name="Basic Agriculture",
            description="Domestication of plants and animals",
            category=TechCategory.AGRICULTURE,
            era=TechEra.ANCIENT,
            base_research_cost=100,
            prerequisites=set(),
            effects={"food_production": 0.3, "population_growth": 0.1},
            breakthrough_potential=0.1,
            synergy_techs=set()
        ))
        
        self._add_technology_node(TechnologyNode(
            id="writing",
            name="Writing System",
            description="Development of written language",
            category=TechCategory.SCIENCE,
            era=TechEra.ANCIENT,
            base_research_cost=150,
            prerequisites=set(),
            effects={"research_speed": 0.2, "knowledge_preservation": 0.4},
            breakthrough_potential=0.3,
            synergy_techs=set()
        ))
        
        # Classical era
        self._add_technology_node(TechnologyNode(
            id="mathematics",
            name="Mathematics",
            description="Development of numerical systems and calculations",
            category=TechCategory.SCIENCE,
            era=TechEra.CLASSICAL,
            base_research_cost=200,
            prerequisites={"writing"},
            effects={"research_speed": 0.3, "engineering": 0.2},
            breakthrough_potential=0.4,
            synergy_techs={"architecture", "astronomy"}
        ))
        
        self._add_technology_node(TechnologyNode(
            id="iron_working",
            name="Iron Working",
            description="Smelting and working of iron",
            category=TechCategory.INDUSTRY,
            era=TechEra.CLASSICAL,
            base_research_cost=250,
            prerequisites=set(),
            effects={"production": 0.4, "military_strength": 0.3},
            breakthrough_potential=0.2,
            synergy_techs=set()
        ))
        
        # Industrial era
        self._add_technology_node(TechnologyNode(
            id="steam_engine",
            name="Steam Engine",
            description="Harnessing steam power for mechanical work",
            category=TechCategory.ENERGY,
            era=TechEra.INDUSTRIAL,
            base_research_cost=500,
            prerequisites={"mathematics", "iron_working"},
            effects={"production": 0.8, "transportation": 0.6, "energy_production": 0.7},
            breakthrough_potential=0.8,
            synergy_techs={"railroads", "factory_system"}
        ))
        
        # Information era
        self._add_technology_node(TechnologyNode(
            id="transistor",
            name="Transistor",
            description="Semiconductor device for amplification and switching",
            category=TechCategory.COMPUTING,
            era=TechEra.INFORMATION,
            base_research_cost=800,
            prerequisites={"mathematics", "electronics"},
            effects={"research_speed": 0.6, "computing_power": 0.9, "communication": 0.5},
            breakthrough_potential=0.9,
            synergy_techs={"integrated_circuit", "computer_science"}
        ))
    
    def _add_technology_node(self, tech_node: TechnologyNode):
        """Add a technology node to the tree."""
        self.technology_tree[tech_node.id] = tech_node
        self.tech_tree_graph.add_node(tech_node.id, tech_node=tech_node)
        
        # Add prerequisite edges
        for prereq_id in tech_node.prerequisites:
            if prereq_id in self.technology_tree:
                self.tech_tree_graph.add_edge(prereq_id, tech_node.id)
    
    def add_research_team(self, team_id: str, size: int, 
                         expertise: Dict[TechCategory, float],
                         focus_area: Optional[TechCategory] = None) -> bool:
        """Add a research team to the system."""
        if team_id in self.research_teams:
            return False
            
        self.research_teams[team_id] = ResearchTeam(team_id, size, expertise, focus_area)
        return True
    
    def remove_research_team(self, team_id: str) -> bool:
        """Remove a research team."""
        if team_id not in self.research_teams:
            return False
            
        del self.research_teams[team_id]
        return True
    
    def assign_research_project(self, team_id: str, tech_id: str, funding: float) -> bool:
        """Assign a research project to a team."""
        if team_id not in self.research_teams or tech_id not in self.technology_tree:
            return False
            
        if not self.can_research_technology(tech_id):
            return False
            
        return self.research_teams[team_id].assign_project(tech_id, funding)
    
    def update_research(self, time_delta: float = 1.0) -> List[Tuple[str, str]]:
        """
        Update research progress across all teams.
        
        Returns:
            List of (team_id, tech_id) for completed research
        """
        completed_projects = []
        
        # Distribute budget to active teams
        active_teams = [team for team in self.research_teams.values() if team.current_project]
        if active_teams:
            budget_per_team = self.research_budget / len(active_teams)
            for team in active_teams:
                team.funding = budget_per_team
        
        # Process research for each team
        for team_id, team in self.research_teams.items():
            if team.current_project:
                tech_node = self.technology_tree[team.current_project]
                progress_made, completed = team.research_tick(time_delta, tech_node)
                
                if completed:
                    self.researched_techs.add(tech_node.id)
                    completed_projects.append((team_id, tech_node.id))
                    self._on_technology_researched(tech_node)
        
        # Check for random innovations
        self._check_random_innovations(time_delta)
        
        # Update system metrics
        self._update_tech_level()
        self._update_scientific_tradition()
        
        return completed_projects
    
    def _on_technology_researched(self, tech_node: TechnologyNode):
        """Handle effects of researching a new technology."""
        # Apply technology effects
        for effect, value in tech_node.effects.items():
            # In a full implementation, these would modify system-wide parameters
            pass
        
        # Check for breakthrough
        self._check_breakthrough(tech_node)
        
        # Unlock synergies
        for synergy_id in tech_node.synergy_techs:
            if synergy_id in self.technology_tree:
                # Boost research speed for synergistic technologies
                pass
    
    def _check_breakthrough(self, tech_node: TechnologyNode):
        """Check if a technology research triggers a breakthrough."""
        breakthrough_chance = (
            self.breakthrough_chance_base *
            tech_node.breakthrough_potential *
            self.scientific_tradition
        )
        
        if random.random() < breakthrough_chance:
            self._trigger_breakthrough(tech_node)
    
    def _trigger_breakthrough(self, tech_node: TechnologyNode):
        """Trigger a technological breakthrough."""
        self.breakthrough_counter += 1
        
        # Determine breakthrough type based on technology category
        breakthrough_type = self._determine_breakthrough_type(tech_node.category)
        
        # Apply breakthrough effects
        breakthrough_effects = self._generate_breakthrough_effects(breakthrough_type, tech_node.era)
        
        # Boost scientific tradition
        self.scientific_tradition += 0.1
        
        # Log breakthrough
        print(f"BREAKTHROUGH! {tech_node.name} led to {breakthrough_type.value}")
        
        return breakthrough_effects
    
    def _determine_breakthrough_type(self, category: TechCategory) -> BreakthroughType:
        """Determine the type of breakthrough based on technology category."""
        mapping = {
            TechCategory.SCIENCE: BreakthroughType.SCIENTIFIC_REVOLUTION,
            TechCategory.INDUSTRY: BreakthroughType.INDUSTRIAL_REVOLUTION,
            TechCategory.COMPUTING: BreakthroughType.DIGITAL_REVOLUTION,
            TechCategory.BIOTECH: BreakthroughType.BIOTECH_REVOLUTION,
            TechCategory.ENERGY: BreakthroughType.QUANTUM_LEAP,
        }
        return mapping.get(category, BreakthroughType.PARADIGM_SHIFT)
    
    def _generate_breakthrough_effects(self, breakthrough_type: BreakthroughType, era: TechEra) -> Dict[str, float]:
        """Generate effects for a breakthrough."""
        base_multiplier = 1.0 + (era.level * 0.2)
        
        effects_map = {
            BreakthroughType.SCIENTIFIC_REVOLUTION: {
                "research_speed": 0.5 * base_multiplier,
                "innovation_chance": 0.3 * base_multiplier
            },
            BreakthroughType.INDUSTRIAL_REVOLUTION: {
                "production": 0.8 * base_multiplier,
                "efficiency": 0.4 * base_multiplier
            },
            BreakthroughType.DIGITAL_REVOLUTION: {
                "computing_power": 1.0 * base_multiplier,
                "communication": 0.6 * base_multiplier
            },
            BreakthroughType.BIOTECH_REVOLUTION: {
                "health": 0.7 * base_multiplier,
                "food_production": 0.5 * base_multiplier
            },
            BreakthroughType.QUANTUM_LEAP: {
                "research_speed": 0.9 * base_multiplier,
                "energy_production": 0.8 * base_multiplier
            },
            BreakthroughType.PARADIGM_SHIFT: {
                "all_metrics": 0.3 * base_multiplier
            }
        }
        
        return effects_map.get(breakthrough_type, {})
    
    def _check_random_innovations(self, time_delta: float):
        """Check for random innovations outside the tech tree."""
        innovation_chance = (
            self.innovation_chance *
            self.tech_level *
            self.scientific_tradition *
            time_delta
        )
        
        if random.random() < innovation_chance:
            self._generate_random_innovation()
    
    def _generate_random_innovation(self):
        """Generate a random innovation."""
        # This would create a new Innovation object and add it to the system
        # For now, we'll just increment a counter
        pass
    
    def can_research_technology(self, tech_id: str) -> bool:
        """Check if a technology can be researched."""
        if tech_id not in self.technology_tree:
            return False
            
        tech_node = self.technology_tree[tech_id]
        return all(prereq in self.researched_techs for prereq in tech_node.prerequisites)
    
    def get_available_technologies(self) -> List[TechnologyNode]:
        """Get list of technologies available for research."""
        available = []
        for tech_id, tech_node in self.technology_tree.items():
            if tech_id not in self.researched_techs and self.can_research_technology(tech_id):
                available.append(tech_node)
        return available
    
    def get_tech_tree_path(self, target_tech_id: str) -> List[List[TechnologyNode]]:
        """Get research paths to reach a target technology."""
        if target_tech_id not in self.tech_tree_graph:
            return []
            
        all_paths = []
        root_nodes = [node for node in self.tech_tree_graph.nodes() 
                     if self.tech_tree_graph.in_degree(node) == 0]
        
        for root in root_nodes:
            try:
                paths = nx.all_simple_paths(self.tech_tree_graph, root, target_tech_id)
                for path in paths:
                    tech_path = [self.technology_tree[node_id] for node_id in path 
                               if node_id in self.technology_tree]
                    if tech_path:
                        all_paths.append(tech_path)
            except nx.NetworkXNoPath:
                continue
                
        return all_paths
    
    def _update_tech_level(self):
        """Update overall technology level."""
        if not self.researched_techs:
            self.tech_level = 0.0
            return
            
        total_value = 0.0
        for tech_id in self.researched_techs:
            tech_node = self.technology_tree[tech_id]
            era_multiplier = (tech_node.era.level + 1) * 0.5
            total_value += tech_node.base_research_cost * era_multiplier
        
        max_possible = sum(
            tech.base_research_cost * ((tech.era.level + 1) * 0.5)
            for tech in self.technology_tree.values()
        )
        
        self.tech_level = total_value / max(1, max_possible)
    
    def _update_scientific_tradition(self):
        """Update scientific tradition based on research activity."""
        base_growth = len(self.researched_techs) * 0.01
        breakthrough_bonus = self.breakthrough_counter * 0.05
        research_activity = len([t for t in self.research_teams.values() if t.current_project]) * 0.02
        
        self.scientific_tradition = min(1.0, 
            self.scientific_tradition + base_growth + breakthrough_bonus + research_activity
        )
    
    def get_research_bonuses(self) -> Dict[str, float]:
        """Get cumulative bonuses from all researched technologies."""
        bonuses = {
            "food_production": 0.0,
            "production": 0.0,
            "research_speed": 0.0,
            "military_strength": 0.0,
            "health": 0.0,
            "culture": 0.0,
            "energy_production": 0.0,
            "computing_power": 0.0,
            "communication": 0.0,
            "efficiency": 0.0
        }
        
        for tech_id in self.researched_techs:
            tech_node = self.technology_tree[tech_id]
            for effect, value in tech_node.effects.items():
                if effect in bonuses:
                    bonuses[effect] += value
        
        return bonuses
    
    def get_system_metrics(self) -> Dict[str, float]:
        """Get comprehensive system metrics."""
        metrics = {
            'tech_level': self.tech_level,
            'scientific_tradition': self.scientific_tradition,
            'breakthroughs': self.breakthrough_counter,
            'researched_techs': len(self.researched_techs),
            'available_techs': len(self.get_available_technologies()),
            'total_techs': len(self.technology_tree),
            'research_teams': len(self.research_teams),
            'active_projects': len([t for t in self.research_teams.values() if t.current_project]),
            'research_budget': self.research_budget,
        }
        
        # Add research bonuses
        metrics.update(self.get_research_bonuses())
        
        return metrics
    
    def get_research_recommendations(self, max_recommendations: int = 5) -> List[Dict]:
        """Get technology research recommendations."""
        recommendations = []
        available_techs = self.get_available_technologies()
        
        for tech in available_techs:
            # Calculate priority score
            era_priority = (tech.era.level + 1) * 0.2
            breakthrough_priority = tech.breakthrough_potential * 0.3
            synergy_priority = len(tech.synergy_techs) * 0.1
            
            # Team expertise matching
            team_expertise = 0.0
            for team in self.research_teams.values():
                if not team.current_project:
                    team_expertise = max(team_expertise, team.expertise.get(tech.category, 0.0))
            
            priority_score = era_priority + breakthrough_priority + synergy_priority + team_expertise
            
            recommendation = {
                'tech_id': tech.id,
                'tech_name': tech.name,
                'category': tech.category.value,
                'era': tech.era.label,
                'research_cost': tech.base_research_cost,
                'breakthrough_potential': tech.breakthrough_potential,
                'priority_score': priority_score,
                'effects': tech.effects
            }
            recommendations.append(recommendation)
        
        recommendations.sort(key=lambda x: x['priority_score'], reverse=True)
        return recommendations[:max_recommendations]