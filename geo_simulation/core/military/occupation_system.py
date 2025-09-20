"""Managing conquered territories"""

# Module implementation

# occupation_system.py
from typing import Dict, List, Tuple
from dataclasses import dataclass
from enum import Enum
import numpy as np

class ResistanceLevel(Enum):
    NONE = 0.0
    LOW = 0.2
    MODERATE = 0.4
    HIGH = 0.6
    INTENSE = 0.8
    TOTAL = 1.0

@dataclass
class OccupationPolicy:
    name: str
    garrison_requirements: float  # troops per population
    resistance_change: float
    resource_extraction: float
    intelligence_gathering: float

class OccupationManager:
    def __init__(self):
        self.occupied_territories: Dict[str, Dict] = {}
        self.occupation_policies = self._initialize_policies()
        self.active_policy = "standard"
        self.resistance_events = []
    
    def _initialize_policies(self) -> Dict[str, OccupationPolicy]:
        """Initialize available occupation policies"""
        return {
            "lenient": OccupationPolicy(
                name="Lenient Occupation",
                garrison_requirements=0.001,
                resistance_change=-0.01,
                resource_extraction=0.3,
                intelligence_gathering=0.4
            ),
            "standard": OccupationPolicy(
                name="Standard Occupation",
                garrison_requirements=0.002,
                resistance_change=0.0,
                resource_extraction=0.6,
                intelligence_gathering=0.6
            ),
            "harsh": OccupationPolicy(
                name="Harsh Occupation",
                garrison_requirements=0.004,
                resistance_change=0.02,
                resource_extraction=0.9,
                intelligence_gathering=0.8
            ),
            "brutal": OccupationPolicy(
                name="Brutal Occupation",
                garrison_requirements=0.008,
                resistance_change=0.05,
                resource_extraction=1.2,
                intelligence_gathering=1.0
            )
        }
    
    def occupy_territory(self, territory_id: str, population: int, 
                        resources: Dict[str, float], initial_resistance: float = 0.0):
        """Occupy a new territory"""
        self.occupied_territories[territory_id] = {
            'population': population,
            'resources': resources,
            'resistance': initial_resistance,
            'garrison_size': 0,
            'last_updated': 0,
            'resource_extraction': 0.0
        }
    
    def update_occupation(self, time_delta: float, available_troops: Dict[str, int]):
        """Update occupation status for all territories"""
        total_resources = {}
        
        for territory_id, data in self.occupied_territories.items():
            # Update resistance
            policy = self.occupation_policies[self.active_policy]
            garrison_ratio = data['garrison_size'] / data['population'] if data['population'] > 0 else 0
            
            resistance_change = policy.resistance_change
            resistance_change -= garrison_ratio * 0.1  # More troops reduce resistance growth
            
            data['resistance'] = np.clip(data['resistance'] + resistance_change * time_delta, 0.0, 1.0)
            
            # Extract resources
            extraction_efficiency = policy.resource_extraction * (1 - data['resistance'] * 0.5)
            for resource, amount in data['resources'].items():
                extracted = amount * extraction_efficiency * time_delta
                total_resources[resource] = total_resources.get(resource, 0.0) + extracted
                data['resources'][resource] = max(0, amount - extracted)
            
            # Check for resistance events
            if data['resistance'] > 0.6 and np.random.random() < data['resistance'] * 0.1:
                self._trigger_resistance_event(territory_id, data['resistance'])
        
        return total_resources
    
    def _trigger_resistance_event(self, territory_id: str, resistance_level: float):
        """Trigger a resistance event"""
        event_severity = resistance_level
        event_types = [
            "sabotage", "ambush", "propaganda", "strike", "assassination"
        ]
        
        event_type = np.random.choice(event_types)
        severity = event_severity * np.random.uniform(0.5, 1.5)
        
        self.resistance_events.append({
            'territory': territory_id,
            'type': event_type,
            'severity': severity,
            'time': len(self.resistance_events)
        })
    
    def set_occupation_policy(self, policy_name: str) -> bool:
        """Set the active occupation policy"""
        if policy_name in self.occupation_policies:
            self.active_policy = policy_name
            return True
        return False
    
    def get_garrison_requirements(self) -> Dict[str, int]:
        """Calculate garrison requirements for all territories"""
        requirements = {}
        policy = self.occupation_policies[self.active_policy]
        
        for territory_id, data in self.occupied_territories.items():
            required = int(data['population'] * policy.garrison_requirements)
            requirements[territory_id] = max(100, required)  # Minimum garrison
        
        return requirements
    
    def deploy_garrison(self, territory_id: str, troops: int):
        """Deploy troops to garrison a territory"""
        if territory_id in self.occupied_territories:
            self.occupied_territories[territory_id]['garrison_size'] = troops
    
    def get_resistance_level(self, territory_id: str) -> ResistanceLevel:
        """Get the resistance level for a territory"""
        if territory_id not in self.occupied_territories:
            return ResistanceLevel.NONE
        
        resistance = self.occupied_territories[territory_id]['resistance']
        if resistance <= 0.1:
            return ResistanceLevel.NONE
        elif resistance <= 0.3:
            return ResistanceLevel.LOW
        elif resistance <= 0.5:
            return ResistanceLevel.MODERATE
        elif resistance <= 0.7:
            return ResistanceLevel.HIGH
        elif resistance <= 0.9:
            return ResistanceLevel.INTENSE
        else:
            return ResistanceLevel.TOTAL