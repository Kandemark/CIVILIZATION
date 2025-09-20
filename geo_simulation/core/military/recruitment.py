"""Unit recruitment based on population"""

# Module implementation

# recruitment.py
from typing import Dict, List, Tuple
from dataclasses import dataclass
from enum import Enum
import numpy as np

class RecruitmentPool(Enum):
    VOLUNTEERS = "volunteers"
    CONSCRIPTS = "conscripts"
    PROFESSIONALS = "professionals"
    MERCENARIES = "mercenaries"
    SPECIALISTS = "specialists"

@dataclass
class RecruitmentCenter:
    level: int
    capacity: int
    training_quality: float
    specialization: str
    current_training: Dict[str, int]

class RecruitmentSystem:
    def __init__(self):
        self.recruitment_centers: Dict[str, RecruitmentCenter] = {}
        self.available_pools: Dict[RecruitmentPool, int] = {
            pool: 0 for pool in RecruitmentPool
        }
        self.training_programs: Dict[str, Dict] = {}
        self.recruitment_policies = {
            "volunteer_only": {"conscription": False, "mercenaries": False},
            "limited_conscription": {"conscription": True, "rate": 0.01},
            "total_war": {"conscription": True, "rate": 0.05, "mercenaries": True}
        }
        self.active_policy = "volunteer_only"
    
    def update_recruitment_pools(self, population: int, economic_development: float,
                               patriotism: float, war_support: float):
        """Update available recruitment pools based on various factors"""
        # Volunteers based on patriotism and war support
        self.available_pools[RecruitmentPool.VOLUNTEERS] = int(
            population * patriotism * war_support * 0.01
        )
        
        # Professionals based on economic development
        self.available_pools[RecruitmentPool.PROFESSIONALS] = int(
            population * economic_development * 0.005
        )
        
        # Apply conscription if active
        if self.recruitment_policies[self.active_policy]["conscription"]:
            conscription_rate = self.recruitment_policies[self.active_policy]["rate"]
            self.available_pools[RecruitmentPool.CONSCRIPTS] = int(
                population * conscription_rate
            )
        else:
            self.available_pools[RecruitmentPool.CONSCRIPTS] = 0
        
        # Mercenaries based on economic factors
        self.available_pools[RecruitmentPool.MERCENARIES] = int(
            economic_development * 1000  # Simplified
        )
    
    def recruit_units(self, unit_type: str, quantity: int, 
                     recruitment_center_id: str) -> bool:
        """Recruit units of a specific type"""
        if recruitment_center_id not in self.recruitment_centers:
            return False
        
        center = self.recruitment_centers[recruitment_center_id]
        
        # Check if center has capacity
        total_training = sum(center.current_training.values())
        if total_training + quantity > center.capacity:
            return False
        
        # Check if recruitment pools are sufficient
        pool_requirements = self._get_pool_requirements(unit_type, quantity)
        for pool, required in pool_requirements.items():
            if self.available_pools[pool] < required:
                return False
        
        # Deduct from pools
        for pool, required in pool_requirements.items():
            self.available_pools[pool] -= required
        
        # Add to training
        center.current_training[unit_type] = center.current_training.get(unit_type, 0) + quantity
        
        return True
    
    def _get_pool_requirements(self, unit_type: str, quantity: int) -> Dict[RecruitmentPool, int]:
        """Get pool requirements for recruiting a unit type"""
        # This would be based on unit type characteristics
        requirements = {
            RecruitmentPool.VOLUNTEERS: quantity * 1,
            RecruitmentPool.PROFESSIONALS: quantity * 0.5,
            RecruitmentPool.CONSCRIPTS: quantity * 0.5
        }
        return {k: v for k, v in requirements.items() if v > 0}
    
    def update_training(self, time_delta: float):
        """Update training progress for all centers"""
        trained_units = {}
        
        for center_id, center in self.recruitment_centers.items():
            training_rate = center.training_quality * time_delta
            
            for unit_type, quantity in list(center.current_training.items()):
                trained = int(quantity * training_rate)
                if trained > 0:
                    center.current_training[unit_type] -= trained
                    trained_units[unit_type] = trained_units.get(unit_type, 0) + trained
        
        return trained_units
    
    def build_recruitment_center(self, location: str, level: int = 1, 
                               specialization: str = "general") -> str:
        """Build a new recruitment center"""
        center_id = f"center_{len(self.recruitment_centers) + 1}"
        
        new_center = RecruitmentCenter(
            level=level,
            capacity=level * 1000,
            training_quality=0.5 + level * 0.1,
            specialization=specialization,
            current_training={}
        )
        
        self.recruitment_centers[center_id] = new_center
        return center_id
    
    def upgrade_recruitment_center(self, center_id: str) -> bool:
        """Upgrade a recruitment center"""
        if center_id not in self.recruitment_centers:
            return False
        
        center = self.recruitment_centers[center_id]
        center.level += 1
        center.capacity = center.level * 1000
        center.training_quality = 0.5 + center.level * 0.1
        
        return True
    
    def set_recruitment_policy(self, policy_name: str) -> bool:
        """Set the active recruitment policy"""
        if policy_name in self.recruitment_policies:
            self.active_policy = policy_name
            return True
        return False
    
    def get_recruitment_costs(self, unit_type: str, quantity: int) -> Dict[str, float]:
        """Get resource costs for recruitment"""
        # This would be based on unit type
        base_costs = {
            "infantry": {"money": 100, "equipment": 50, "time": 30},
            "cavalry": {"money": 200, "equipment": 100, "time": 60},
            "artillery": {"money": 500, "equipment": 300, "time": 90},
            "armor": {"money": 1000, "equipment": 800, "time": 120}
        }
        
        costs = base_costs.get(unit_type, {"money": 0, "equipment": 0, "time": 0})
        return {k: v * quantity for k, v in costs.items()}