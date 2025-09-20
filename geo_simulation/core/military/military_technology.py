"""Weapons and defense systems"""

# Module implementation

# military_technology.py
from typing import Dict, List, Set
from dataclasses import dataclass
from enum import Enum
import yaml

class TechCategory(Enum):
    INFANTRY = "infantry"
    ARMOR = "armor"
    ARTILLERY = "artillery"
    NAVAL = "naval"
    AERIAL = "aerial"
    LOGISTICS = "logistics"
    COMMUNICATIONS = "communications"
    MEDICINE = "medicine"

@dataclass
class Technology:
    name: str
    category: TechCategory
    research_cost: int
    prerequisites: List[str]
    effects: Dict[str, float]
    era: int

class TechnologyTree:
    def __init__(self):
        self.technologies: Dict[str, Technology] = {}
        self.researched_tech: Set[str] = set()
        self.research_progress: Dict[str, float] = {}
        self.current_era = 1
    
    def load_tech_tree(self, file_path: str):
        """Load technology tree from YAML file"""
        try:
            with open(file_path, 'r') as f:
                tech_data = yaml.safe_load(f)
            
            for name, data in tech_data.items():
                self.technologies[name] = Technology(
                    name=name,
                    category=TechCategory(data['category']),
                    research_cost=data['research_cost'],
                    prerequisites=data.get('prerequisites', []),
                    effects=data.get('effects', {}),
                    era=data.get('era', 1)
                )
        except Exception as e:
            print(f"Error loading technology tree: {e}")
    
    def research_technology(self, tech_name: str, research_points: float) -> bool:
        """Research a technology"""
        if tech_name not in self.technologies:
            return False
        
        if tech_name in self.researched_tech:
            return True  # Already researched
        
        # Check prerequisites
        tech = self.technologies[tech_name]
        if not all(preq in self.researched_tech for preq in tech.prerequisites):
            return False
        
        # Update research progress
        current_progress = self.research_progress.get(tech_name, 0.0)
        new_progress = current_progress + research_points
        self.research_progress[tech_name] = new_progress
        
        if new_progress >= tech.research_cost:
            self.researched_tech.add(tech_name)
            # Apply technology effects
            self._apply_tech_effects(tech)
            return True
        
        return False
    
    def _apply_tech_effects(self, tech: Technology):
        """Apply effects of a researched technology"""
        # This would update various game systems with the new technology benefits
        print(f"Applied technology effects: {tech.name} - {tech.effects}")
    
    def get_available_techs(self) -> List[str]:
        """Get technologies available for research"""
        available = []
        for name, tech in self.technologies.items():
            if (name not in self.researched_tech and 
                all(preq in self.researched_tech for preq in tech.prerequisites) and
                tech.era <= self.current_era):
                available.append(name)
        return available
    
    def get_tech_effects(self, tech_name: str) -> Dict[str, float]:
        """Get effects of a specific technology"""
        if tech_name in self.technologies:
            return self.technologies[tech_name].effects
        return {}
    
    def advance_era(self):
        """Advance to the next technological era"""
        self.current_era += 1
    
    def get_era_techs(self, era: int) -> List[str]:
        """Get all technologies from a specific era"""
        return [name for name, tech in self.technologies.items() if tech.era == era]