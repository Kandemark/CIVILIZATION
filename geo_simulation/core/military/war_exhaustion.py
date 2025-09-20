"""Cost of prolonged conflict"""

# Module implementation

# war_exhaustion.py
from typing import Dict, List
from dataclasses import dataclass
from enum import Enum
import numpy as np

class WarExhaustionLevel(Enum):
    NONE = 0.0
    LOW = 0.2
    MODERATE = 0.4
    HIGH = 0.6
    SEVERE = 0.8
    CRITICAL = 1.0

@dataclass
class WarExhaustionFactor:
    type: str
    weight: float
    current_value: float
    description: str

class WarExhaustionSystem:
    def __init__(self):
        self.exhaustion_level = 0.0
        self.factors: Dict[str, WarExhaustionFactor] = {}
        self.historical_exhaustion = []
        self.mitigation_efforts = 0.0
        self._initialize_factors()
    
    def _initialize_factors(self):
        """Initialize war exhaustion factors"""
        factors = {
            "casualties": WarExhaustionFactor("casualties", 0.3, 0.0, "Military losses"),
            "duration": WarExhaustionFactor("duration", 0.2, 0.0, "Length of the war"),
            "economic_impact": WarExhaustionFactor("economic_impact", 0.25, 0.0, "Economic costs"),
            "public_support": WarExhaustionFactor("public_support", 0.15, 1.0, "Public opinion"),
            "territorial_losses": WarExhaustionFactor("territorial_losses", 0.1, 0.0, "Lost territory")
        }
        self.factors = factors
    
    def update_exhaustion(self, time_delta: float, new_casualties: int = 0,
                        economic_costs: float = 0.0, territories_lost: int = 0):
        """Update war exhaustion level"""
        # Update factor values
        self.factors["casualties"].current_value += new_casualties * 0.0001
        self.factors["duration"].current_value += time_delta * 0.01
        self.factors["economic_impact"].current_value += economic_costs * 0.000001
        self.factors["territorial_losses"].current_value += territories_lost * 0.1
        
        # Calculate weighted exhaustion
        total_exhaustion = 0.0
        for factor in self.factors.values():
            total_exhaustion += factor.current_value * factor.weight
        
        # Apply mitigation
        total_exhaustion = max(0, total_exhaustion - self.mitigation_efforts)
        
        self.exhaustion_level = min(1.0, total_exhaustion)
        self.historical_exhaustion.append(self.exhaustion_level)
        
        return self.exhaustion_level
    
    def get_exhaustion_level(self) -> WarExhaustionLevel:
        """Get the current war exhaustion level"""
        if self.exhaustion_level <= 0.2:
            return WarExhaustionLevel.NONE
        elif self.exhaustion_level <= 0.4:
            return WarExhaustionLevel.LOW
        elif self.exhaustion_level <= 0.6:
            return WarExhaustionLevel.MODERATE
        elif self.exhaustion_level <= 0.8:
            return WarExhaustionLevel.HIGH
        elif self.exhaustion_level <= 0.9:
            return WarExhaustionLevel.SEVERE
        else:
            return WarExhaustionLevel.CRITICAL
    
    def apply_mitigation(self, propaganda_effort: float, economic_relief: float,
                       military_success: float) -> float:
        """Apply efforts to mitigate war exhaustion"""
        mitigation = (propaganda_effort * 0.4 + economic_relief * 0.3 + 
                     military_success * 0.3)
        self.mitigation_efforts = min(0.5, mitigation)  # Cap mitigation
        
        return self.mitigation_efforts
    
    def get_effects(self) -> Dict[str, float]:
        """Get effects of current war exhaustion level"""
        level = self.get_exhaustion_level()
        
        effects = {
            "recruitment_rate": 1.0 - self.exhaustion_level * 0.5,
            "economic_output": 1.0 - self.exhaustion_level * 0.3,
            "morale": 1.0 - self.exhaustion_level * 0.4,
            "research_speed": 1.0 - self.exhaustion_level * 0.2,
            "rebellion_risk": self.exhaustion_level * 2.0
        }
        
        if level == WarExhaustionLevel.CRITICAL:
            effects["surrender_risk"] = 0.1
            effects["military_coup_risk"] = 0.05
        
        return effects
    
    def implement_policy_change(self, policy_type: str, effectiveness: float) -> bool:
        """Implement policy changes to address war exhaustion"""
        policy_effects = {
            "austerity_measures": {"economic_impact": -0.1, "public_support": -0.05},
            "propaganda_campaign": {"public_support": 0.15},
            "veteran_support": {"casualties": -0.08, "public_support": 0.05},
            "diplomatic_efforts": {"duration": -0.1, "public_support": 0.1}
        }
        
        if policy_type not in policy_effects:
            return False
        
        for factor, effect in policy_effects[policy_type].items():
            if factor in self.factors:
                self.factors[factor].current_value = max(
                    0, self.factors[factor].current_value + effect * effectiveness
                )
        
        return True