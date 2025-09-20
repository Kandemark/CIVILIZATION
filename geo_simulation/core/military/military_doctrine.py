"""Strategic approaches to war"""

# Module implementation

# military_doctrine.py
from typing import Dict, List, Set
from dataclasses import dataclass
from enum import Enum
import yaml

class DoctrineEra(Enum):
    ANCIENT = "ancient"
    MEDIEVAL = "medieval"
    RENAISSANCE = "renaissance"
    INDUSTRIAL = "industrial"
    MODERN = "modern"
    FUTURISTIC = "futuristic"

@dataclass
class DoctrineTemplate:
    name: str
    era: DoctrineEra
    effects: Dict[str, float]
    requirements: List[str]
    compatible_units: List[str]
    special_rules: Dict[str, any]

class DoctrineManager:
    def __init__(self):
        self.available_doctrines: Dict[str, DoctrineTemplate] = {}
        self.active_doctrine: str = None
        self.doctrine_progress: Dict[str, float] = {}
        self.researched_doctrines: Set[str] = set()
        
    def load_doctrines(self, file_path: str):
        """Load doctrines from YAML configuration"""
        try:
            with open(file_path, 'r') as f:
                doctrines_data = yaml.safe_load(f)
            
            for name, data in doctrines_data.items():
                self.available_doctrines[name] = DoctrineTemplate(
                    name=name,
                    era=DoctrineEra(data['era']),
                    effects=data['effects'],
                    requirements=data.get('requirements', []),
                    compatible_units=data.get('compatible_units', []),
                    special_rules=data.get('special_rules', {})
                )
        except Exception as e:
            print(f"Error loading doctrines: {e}")
    
    def research_doctrine(self, doctrine_name: str, research_points: float) -> bool:
        """Research a new doctrine"""
        if doctrine_name not in self.available_doctrines:
            return False
        
        if doctrine_name in self.researched_doctrines:
            return True  # Already researched
        
        # Track research progress
        current_progress = self.doctrine_progress.get(doctrine_name, 0.0)
        required_points = self.available_doctrines[doctrine_name].effects.get('research_cost', 100)
        
        new_progress = current_progress + research_points
        self.doctrine_progress[doctrine_name] = new_progress
        
        if new_progress >= required_points:
            self.researched_doctrines.add(doctrine_name)
            return True
        
        return False
    
    def activate_doctrine(self, doctrine_name: str) -> bool:
        """Activate a researched doctrine"""
        if doctrine_name in self.researched_doctrines:
            self.active_doctrine = doctrine_name
            return True
        return False
    
    def get_active_effects(self) -> Dict[str, float]:
        """Get effects of the active doctrine"""
        if not self.active_doctrine:
            return {}
        
        return self.available_doctrines[self.active_doctrine].effects
    
    def get_doctrine_compatibility(self, unit_types: List[str]) -> List[str]:
        """Get units that are compatible with the active doctrine"""
        if not self.active_doctrine:
            return unit_types
        
        doctrine = self.available_doctrines[self.active_doctrine]
        compatible_units = []
        
        for unit_type in unit_types:
            if unit_type in doctrine.compatible_units or not doctrine.compatible_units:
                compatible_units.append(unit_type)
        
        return compatible_units
    
    def develop_custom_doctrine(self, name: str, effects: Dict[str, float], requirements: List[str], era: DoctrineEra) -> bool:
        """Develop a custom military doctrine"""
        # Check if requirements are met
        if not self._check_requirements(requirements):
            return False
        
        new_doctrine = DoctrineTemplate(
            name=name,
            era=era,
            effects=effects,
            requirements=requirements,
            compatible_units=[],
            special_rules={}
        )
        
        self.available_doctrines[name] = new_doctrine
        return True
    
    def _check_requirements(self, requirements: List[str]) -> bool:
        """Check if doctrine development requirements are met"""
        # This would check for required technologies, resources, etc.
        return all(req in self.researched_doctrines for req in requirements)
    
    def get_available_doctrines(self, current_era: DoctrineEra) -> List[str]:
        """Get doctrines available for research in the current era"""
        available = []
        for name, doctrine in self.available_doctrines.items():
            if doctrine.era.value <= current_era.value and name not in self.researched_doctrines:
                available.append(name)
        return available