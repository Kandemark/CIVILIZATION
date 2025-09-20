"""Resistance and insurgency"""

# Module implementation

# rebellion_system.py
from typing import Dict, List, Tuple
from dataclasses import dataclass
from enum import Enum
import numpy as np

class RebellionStage(Enum):
    DORMANT = 0
    AGITATING = 1
    ORGANIZING = 2
    ACTIVE = 3
    CIVIL_WAR = 4

@dataclass
class RebelFaction:
    ideology: str
    strength: float
    support: float
    organization: float
    goals: List[str]
    controlled_territory: List[str]

class RebellionSystem:
    def __init__(self):
        self.rebel_factions: Dict[str, RebelFaction] = {}
        self.rebellion_risk: Dict[str, float] = {}
        self.suppression_efforts: Dict[str, float] = {}
        self.historical_grievances: Dict[str, List[Tuple]] = {}
    
    def add_grievance(self, region_id: str, grievance_type: str, severity: float, 
                     description: str):
        """Add a historical grievance to a region"""
        if region_id not in self.historical_grievances:
            self.historical_grievances[region_id] = []
        
        self.historical_grievances[region_id].append(
            (grievance_type, severity, description)
        )
    
    def calculate_rebellion_risk(self, region_id: str, oppression_level: float,
                               economic_conditions: float, ethnic_tensions: float,
                               government_legitimacy: float) -> float:
        """Calculate rebellion risk for a region"""
        base_risk = 0.0
        
        # Base factors
        base_risk += oppression_level * 0.4
        base_risk += (1 - economic_conditions) * 0.3
        base_risk += ethnic_tensions * 0.2
        base_risk += (1 - government_legitimacy) * 0.3
        
        # Historical grievances
        if region_id in self.historical_grievances:
            grievance_severity = sum(sev for _, sev, _ in self.historical_grievances[region_id])
            base_risk += grievance_severity * 0.1
        
        # Suppression efforts reduce risk
        suppression = self.suppression_efforts.get(region_id, 0.0)
        base_risk = max(0, base_risk - suppression * 0.2)
        
        self.rebellion_risk[region_id] = base_risk
        return base_risk
    
    def update_rebel_factions(self, time_delta: float):
        """Update all rebel factions"""
        for faction_id, faction in self.rebel_factions.items():
            # Faction growth based on support and organization
            growth_factor = faction.support * faction.organization * 0.1
            
            # Increase strength and organization
            faction.strength += growth_factor * time_delta
            faction.organization = min(1.0, faction.organization + growth_factor * 0.1 * time_delta)
            
            # Check for faction actions
            if faction.organization > 0.7 and np.random.random() < 0.1:
                self._trigger_rebel_action(faction_id)
    
    def _trigger_rebel_action(self, faction_id: str):
        """Trigger a rebel action based on faction strength and organization"""
        faction = self.rebel_factions[faction_id]
        action_types = []
        
        if faction.organization < 0.3:
            action_types = ["propaganda", "recruitment", "small_attack"]
        elif faction.organization < 0.6:
            action_types = ["raid", "sabotage", "assassination", "demonstration"]
        else:
            action_types = ["uprising", "territory_capture", "major_offensive"]
        
        action = np.random.choice(action_types)
        success_chance = faction.organization * faction.strength
        
        # TODO: Implement specific action effects
        print(f"Rebel faction {faction_id} executed {action} with {success_chance:.1%} success chance")
    
    def create_rebel_faction(self, region_id: str, ideology: str, 
                           initial_support: float = 0.1) -> str:
        """Create a new rebel faction"""
        faction_id = f"rebel_{len(self.rebel_factions) + 1}"
        
        new_faction = RebelFaction(
            ideology=ideology,
            strength=initial_support * 10,
            support=initial_support,
            organization=0.1,
            goals=[f"liberate_{region_id}"],
            controlled_territory=[]
        )
        
        self.rebel_factions[faction_id] = new_faction
        return faction_id
    
    def suppress_rebellion(self, region_id: str, suppression_force: float,
                         intelligence_ops: float) -> float:
        """Apply suppression efforts to a region"""
        current_suppression = self.suppression_efforts.get(region_id, 0.0)
        new_suppression = suppression_force * 0.5 + intelligence_ops * 0.8
        
        # Diminishing returns
        effective_suppression = min(1.0, current_suppression + new_suppression)
        self.suppression_efforts[region_id] = effective_suppression
        
        # Reduce rebel support and organization
        for faction_id, faction in self.rebel_factions.items():
            if region_id in faction.controlled_territory:
                faction.support = max(0, faction.support - effective_suppression * 0.1)
                faction.organization = max(0, faction.organization - effective_suppression * 0.05)
        
        return effective_suppression
    
    def get_rebellion_stage(self, region_id: str) -> RebellionStage:
        """Get the current rebellion stage for a region"""
        risk = self.rebellion_risk.get(region_id, 0.0)
        
        if risk < 0.2:
            return RebellionStage.DORMANT
        elif risk < 0.4:
            return RebellionStage.AGITATING
        elif risk < 0.6:
            return RebellionStage.ORGANIZING
        elif risk < 0.8:
            return RebellionStage.ACTIVE
        else:
            return RebellionStage.CIVIL_WAR