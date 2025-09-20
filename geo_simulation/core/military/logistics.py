"""Supply lines and resources"""

# Module implementation

# logistics.py
from typing import Dict, List, Set, Tuple
from dataclasses import dataclass
import numpy as np
from enum import Enum
import logging

logger = logging.getLogger(__name__)

class SupplyType(Enum):
    FOOD = "food"
    FUEL = "fuel"
    AMMUNITION = "ammunition"
    MEDICAL = "medical"
    EQUIPMENT = "equipment"

@dataclass
class SupplyStatus:
    current_levels: Dict[SupplyType, float]
    consumption_rates: Dict[SupplyType, float]
    resupply_priority: Dict[SupplyType, int]
    last_resupply: Dict[SupplyType, float]

class LogisticsSystem:
    def __init__(self, initial_supplies: Dict[SupplyType, float] = None):
        self.supply_lines = []
        self.depots = {}
        self.supply_status = SupplyStatus(
            current_levels=initial_supplies or {st: 1000.0 for st in SupplyType},
            consumption_rates={st: 1.0 for st in SupplyType},
            resupply_priority={SupplyType.AMMUNITION: 1, SupplyType.FUEL: 2, 
                             SupplyType.FOOD: 3, SupplyType.MEDICAL: 4, SupplyType.EQUIPMENT: 5},
            last_resupply={st: 0.0 for st in SupplyType}
        )
        self.transport_capacity = 1000
        self.supply_efficiency = 1.0
    
    def update_consumption(self, units: List, distance: float, terrain: str) -> Dict[SupplyType, float]:
        """Update consumption rates based on units and conditions"""
        total_consumption = {st: 0.0 for st in SupplyType}
        
        for unit in units:
            for supply_type in SupplyType:
                consumption_rate = getattr(unit, f"{supply_type.value}_consumption", 0.0)
                # Adjust for distance and terrain
                adjusted_rate = consumption_rate * distance * self._terrain_modifier(terrain)
                total_consumption[supply_type] += adjusted_rate
        
        self.supply_status.consumption_rates = total_consumption
        return total_consumption
    
    def _terrain_modifier(self, terrain: str) -> float:
        """Get consumption modifier based on terrain"""
        modifiers = {
            "plains": 1.0,
            "forest": 1.2,
            "mountains": 1.5,
            "desert": 1.3,
            "urban": 1.1
        }
        return modifiers.get(terrain, 1.0)
    
    def consume_supplies(self, time_delta: float) -> bool:
        """Consume supplies over time and check if exhausted"""
        for supply_type in SupplyType:
            consumption = self.supply_status.consumption_rates[supply_type] * time_delta
            self.supply_status.current_levels[supply_type] -= consumption
            
            if self.supply_status.current_levels[supply_type] <= 0:
                logger.warning(f"{supply_type.value} supplies exhausted!")
                if supply_type in [SupplyType.AMMUNITION, SupplyType.FUEL]:
                    return False  # Critical failure
        
        return True
    
    def plan_resupply(self, available_resources: Dict[SupplyType, float], distance_to_base: float) -> Dict[SupplyType, float]:
        """Plan optimal resupply based on available resources and distance"""
        resupply_plan = {}
        remaining_capacity = self.transport_capacity
        
        # Prioritize by resupply priority
        for priority in sorted(set(self.supply_status.resupply_priority.values())):
            for supply_type, prio in self.supply_status.resupply_priority.items():
                if prio == priority:
                    needed = self._calculate_needed_supplies(supply_type, distance_to_base)
                    available = available_resources.get(supply_type, 0.0)
                    
                    amount = min(needed, available, remaining_capacity)
                    resupply_plan[supply_type] = amount
                    remaining_capacity -= amount
        
        return resupply_plan
    
    def _calculate_needed_supplies(self, supply_type: SupplyType, distance: float) -> float:
        """Calculate needed supplies considering consumption during transit"""
        current_level = self.supply_status.current_levels[supply_type]
        consumption_rate = self.supply_status.consumption_rates[supply_type]
        
        # Estimate consumption during resupply journey
        transit_consumption = consumption_rate * distance * 2  # Round trip
        
        needed = max(0, transit_consumption - current_level)
        return needed * 1.2  # Safety margin
    
    def execute_resupply(self, supplies: Dict[SupplyType, float]):
        """Execute a resupply operation"""
        for supply_type, amount in supplies.items():
            self.supply_status.current_levels[supply_type] += amount
            self.supply_status.last_resupply[supply_type] = amount
        
        logger.info(f"Resupply executed: {supplies}")
    
    def get_supply_status(self) -> Dict[SupplyType, Tuple[float, float]]:
        """Get current supply status as (current, ideal) tuples"""
        status = {}
        for supply_type in SupplyType:
            current = self.supply_status.current_levels[supply_type]
            ideal = self.supply_status.consumption_rates[supply_type] * 7  # 7 days worth
            status[supply_type] = (current, ideal)
        
        return status
    
    def optimize_supply_lines(self, frontlines: List[Tuple[float, float]], base_locations: List[Tuple[float, float]]):
        """Optimize supply lines based on frontlines and base locations"""
        # This would implement sophisticated logistics optimization
        # For now, use simple distance-based optimization
        optimized_lines = []
        
        for frontline in frontlines:
            best_base = min(base_locations, 
                          key=lambda base: np.linalg.norm(np.array(base) - np.array(frontline)))
            optimized_lines.append((best_base, frontline))
        
        self.supply_lines = optimized_lines
        return optimized_lines