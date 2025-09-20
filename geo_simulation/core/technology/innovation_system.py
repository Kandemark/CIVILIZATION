"""Research and discovery processes"""

# Module implementation

"""
Advanced technology research and innovation system with tech trees and breakthroughs.
"""

from typing import Dict, List, Tuple, Optional, Set
from dataclasses import dataclass
from enum import Enum
import random
import math

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

class TechEra(Enum):
    """Technology eras."""
    ANCIENT = 0
    CLASSICAL = 1
    MEDIEVAL = 2
    RENAISSANCE = 3
    INDUSTRIAL = 4
    MODERN = 5
    INFORMATION = 6
    FUTURE = 7

@dataclass
class Technology:
    """Represents a specific technology."""
    id: str
    name: str
    category: TechCategory
    era: TechEra
    base_research_cost: float
    prerequisites: List[str]  # List of technology IDs
    effects: Dict[str, float]  # Effects on various systems
    
    def is_researchable(self, researched_techs: Set[str]) -> bool:
        """Check if this technology can be researched."""
        return all(prereq in researched_techs for prereq in self.prerequisites)

class ResearchTeam:
    """Represents a research team working on technology."""
    
    def __init__(self, size: int, expertise: Dict[TechCategory, float]):
        self.size = size
        self.expertise = expertise
        self.current_project: Optional[Technology] = None
        self.progress = 0.0
        self.funding = 0.0
        
    def assign_project(self, technology: Technology, funding: float):
        """Assign a new research project."""
        self.current_project = technology
        self.progress = 0.0
        self.funding = funding
        
    def research_tick(self, time_delta: float) -> Optional[Technology]:
        """Make research progress and return completed technology if done."""
        if not self.current_project:
            return None
            
        # Calculate research efficiency
        expertise_bonus = self.expertise.get(self.current_project.category, 0.5)
        funding_efficiency = min(1.0, self.funding / 1000.0)  # Diminishing returns
        
        research_rate = (
            self.size * 0.1 * 
            expertise_bonus * 
            funding_efficiency * 
            time_delta
        )
        
        self.progress += research_rate
        
        if self.progress >= self.current_project.base_research_cost:
            completed_tech = self.current_project
            self.current_project = None
            self.progress = 0.0
            return completed_tech
            
        return None

class TechnologyTree:
    """Manages the complete technology tree and research progress."""
    
    def __init__(self):
        self.technologies = self._create_technology_tree()
        self.researched_techs: Set[str] = set()
        self.research_teams: List[ResearchTeam] = []
        self.research_budget = 0.0
        self.tech_level = 0.0  # Overall technology level (0.0 to 1.0)
        
    def _create_technology_tree(self) -> Dict[str, Technology]:
        """Create the complete technology tree."""
        techs = {}
        
        # Ancient era technologies
        techs['agriculture_basic'] = Technology(
            'agriculture_basic', 'Basic Agriculture', TechCategory.AGRICULTURE, TechEra.ANCIENT,
            100, [], {'food_production': 0.2}
        )
        
        techs['writing'] = Technology(
            'writing', 'Writing System', TechCategory.SCIENCE, TechEra.ANCIENT,
            150, [], {'research_speed': 0.1, 'culture': 0.2}
        )
        
        # Classical era
        techs['mathematics'] = Technology(
            'mathematics', 'Mathematics', TechCategory.SCIENCE, TechEra.CLASSICAL,
            200, ['writing'], {'research_speed': 0.2, 'construction': 0.1}
        )
        
        techs['iron_working'] = Technology(
            'iron_working', 'Iron Working', TechCategory.INDUSTRY, TechEra.CLASSICAL,
            250, [], {'production': 0.3, 'military_strength': 0.2}
        )
        
        # Add more technologies for each era...
        
        return techs
    
    def initialize_research_teams(self, num_teams: int = 3):
        """Initialize research teams."""
        for i in range(num_teams):
            expertise = {
                TechCategory.AGRICULTURE: random.uniform(0.3, 0.8),
                TechCategory.INDUSTRY: random.uniform(0.3, 0.8),
                TechCategory.SCIENCE: random.uniform(0.4, 0.9),
                TechCategory.MILITARY: random.uniform(0.2, 0.7)
            }
            self.research_teams.append(ResearchTeam(random.randint(5, 15), expertise))
    
    def update_research(self, time_delta: float) -> List[Technology]:
        """Update research progress and return completed technologies."""
        completed_techs = []
        
        # Distribute budget to teams
        budget_per_team = self.research_budget / max(1, len(self.research_teams))
        
        for team in self.research_teams:
            if not team.current_project:
                # Assign new project if needed
                available_techs = self.get_available_technologies()
                if available_techs:
                    chosen_tech = random.choice(available_techs)
                    team.assign_project(chosen_tech, budget_per_team)
            
            # Make research progress
            completed_tech = team.research_tick(time_delta)
            if completed_tech:
                self.researched_techs.add(completed_tech.id)
                completed_techs.append(completed_tech)
                self._update_tech_level()
        
        return completed_techs
    
    def get_available_technologies(self) -> List[Technology]:
        """Get list of technologies that can be researched."""
        available = []
        for tech in self.technologies.values():
            if tech.id not in self.researched_techs and tech.is_researchable(self.researched_techs):
                available.append(tech)
        return available
    
    def _update_tech_level(self):
        """Update overall technology level based on researched technologies."""
        total_tech_value = 0.0
        for tech_id in self.researched_techs:
            tech = self.technologies[tech_id]
            # More advanced technologies contribute more
            era_multiplier = (tech.era.value + 1) * 0.5
            total_tech_value += tech.base_research_cost * era_multiplier
        
        # Normalize to 0.0-1.0 range
        max_possible_value = sum(tech.base_research_cost * ((tech.era.value + 1) * 0.5) 
                               for tech in self.technologies.values())
        self.tech_level = total_tech_value / max(1, max_possible_value)
    
    def get_research_bonuses(self) -> Dict[str, float]:
        """Get all active research bonuses from technologies."""
        bonuses = {
            'food_production': 0.0,
            'production': 0.0,
            'research_speed': 0.0,
            'military_strength': 0.0,
            'health': 0.0,
            'culture': 0.0
        }
        
        for tech_id in self.researched_techs:
            tech = self.technologies[tech_id]
            for effect, value in tech.effects.items():
                if effect in bonuses:
                    bonuses[effect] += value
        
        return bonuses
    
    def set_research_priority(self, category: TechCategory, priority: float):
        """Set research priority for a specific category."""
        # This would influence which technologies get researched
        pass

class InnovationSystem:
    """Handles random innovations and technological breakthroughs."""
    
    def __init__(self):
        self.innovation_chance = 0.01
        self.breakthrough_chance = 0.001
        self.scientific_tradition = 0.0
        
    def check_innovation(self, tech_level: float, research_investment: float) -> Optional[Dict]:
        """Check for random innovations or breakthroughs."""
        base_chance = self.innovation_chance * tech_level * (research_investment / 1000)
        
        if random.random() < base_chance:
            return self._generate_innovation(tech_level)
        
        if random.random() < self.breakthrough_chance * tech_level:
            return self._generate_breakthrough(tech_level)
        
        return None
    
    def _generate_innovation(self, tech_level: float) -> Dict:
        """Generate a random innovation."""
        innovations = [
            {'type': 'efficiency', 'effect': 'production', 'magnitude': 0.1 * tech_level},
            {'type': 'technique', 'effect': 'research_speed', 'magnitude': 0.05 * tech_level},
            {'type': 'process', 'effect': 'food_production', 'magnitude': 0.08 * tech_level}
        ]
        return random.choice(innovations)
    
    def _generate_breakthrough(self, tech_level: float) -> Dict:
        """Generate a major technological breakthrough."""
        breakthroughs = [
            {'type': 'scientific_revolution', 'effect': 'all', 'magnitude': 0.2 * tech_level},
            {'type': 'industrial_revolution', 'effect': 'production', 'magnitude': 0.4 * tech_level},
            {'type': 'digital_revolution', 'effect': 'research_speed', 'magnitude': 0.3 * tech_level}
        ]
        return random.choice(breakthroughs)