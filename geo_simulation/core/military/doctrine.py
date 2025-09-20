"""Military strategy based on government"""

# Module implementation

# doctrine.py
from enum import Enum
from typing import Dict, List, Set
from dataclasses import dataclass
import yaml

class DoctrineType(Enum):
    OFFENSIVE = "offensive"
    DEFENSIVE = "defensive"
    BALANCED = "balanced"
    SPECIALIZED = "specialized"

@dataclass
class DoctrineEffect:
    combat_bonus: float
    movement_bonus: float
    supply_efficiency: float
    morale_impact: float
    special_abilities: List[str]

class MilitaryDoctrine:
    def __init__(self, doctrine_file: str = None):
        self.doctrines = {}
        self.active_doctrine = None
        self.doctrine_history = []
        
        if doctrine_file:
            self.load_doctrines(doctrine_file)
    
    def load_doctrines(self, file_path: str):
        """Load doctrines from YAML configuration file"""
        try:
            with open(file_path, 'r') as f:
                doctrine_data = yaml.safe_load(f)
                
            for name, data in doctrine_data.items():
                self.doctrines[name] = DoctrineEffect(
                    combat_bonus=data.get('combat_bonus', 1.0),
                    movement_bonus=data.get('movement_bonus', 1.0),
                    supply_efficiency=data.get('supply_efficiency', 1.0),
                    morale_impact=data.get('morale_impact', 0.0),
                    special_abilities=data.get('special_abilities', [])
                )
        except Exception as e:
            print(f"Error loading doctrines: {e}")
    
    def set_doctrine(self, doctrine_name: str) -> bool:
        """Set the active military doctrine"""
        if doctrine_name in self.doctrines:
            self.active_doctrine = doctrine_name
            self.doctrine_history.append(doctrine_name)
            return True
        return False
    
    def get_effects(self) -> DoctrineEffect:
        """Get effects of the active doctrine"""
        return self.doctrines.get(self.active_doctrine, DoctrineEffect(1.0, 1.0, 1.0, 0.0, []))
    
    def develop_doctrine(self, name: str, effects: Dict, requirements: List[str] = None):
        """Develop a new custom doctrine"""
        if requirements and not self._check_requirements(requirements):
            return False
            
        self.doctrines[name] = DoctrineEffect(
            combat_bonus=effects.get('combat_bonus', 1.0),
            movement_bonus=effects.get('movement_bonus', 1.0),
            supply_efficiency=effects.get('supply_efficiency', 1.0),
            morale_impact=effects.get('morale_impact', 0.0),
            special_abilities=effects.get('special_abilities', [])
        )
        return True
    
    def _check_requirements(self, requirements: List[str]) -> bool:
        """Check if doctrine development requirements are met"""
        # This would check technology, experience, etc.
        return True
    
    def get_compatible_units(self, unit_types: List[str]) -> List[str]:
        """Get units that benefit from the active doctrine"""
        if not self.active_doctrine:
            return unit_types
            
        doctrine = self.doctrines[self.active_doctrine]
        # Filter units based on doctrine special abilities
        compatible_units = []
        for unit in unit_types:
            # Simple compatibility check - would be more complex in reality
            if "armor" in unit and "armored_warfare" in doctrine.special_abilities:
                compatible_units.append(unit)
            elif "infantry" in unit and "infantry_tactics" in doctrine.special_abilities:
                compatible_units.append(unit)
            else:
                compatible_units.append(unit)  # All units get basic benefits
                
        return compatible_units