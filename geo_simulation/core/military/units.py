"""Military units"""

# Module implementation

# units.py
from typing import Dict, List, Optional
from dataclasses import dataclass
from enum import Enum
import numpy as np

class UnitType(Enum):
    INFANTRY = "infantry"
    CAVALRY = "cavalry"
    ARTILLERY = "artillery"
    ARMOR = "armor"
    SUPPORT = "support"
    SPECIAL_FORCES = "special_forces"

@dataclass
class Unit:
    id: str
    name: str
    unit_type: UnitType
    combat_strength: float
    movement_speed: float
    supply_consumption: float
    fuel_consumption: float
    ammunition_consumption: float
    morale: float
    experience: float
    equipment_quality: float
    current_strength: int
    max_strength: int

class UnitManager:
    def __init__(self):
        self.units: Dict[str, Unit] = {}
        self.unit_templates = self._initialize_templates()
        self.organization = {}  # Unit hierarchy and organization
        self.reinforcement_pool = {}
    
    def _initialize_templates(self) -> Dict[str, Dict]:
        """Initialize unit templates for different types"""
        return {
            "light_infantry": {
                "combat_strength": 10,
                "movement_speed": 5,
                "supply_consumption": 1.0,
                "fuel_consumption": 0.1,
                "ammunition_consumption": 2.0,
                "equipment_cost": 50
            },
            "heavy_infantry": {
                "combat_strength": 15,
                "movement_speed": 3,
                "supply_consumption": 1.5,
                "fuel_consumption": 0.2,
                "ammunition_consumption": 3.0,
                "equipment_cost": 100
            },
            "cavalry": {
                "combat_strength": 20,
                "movement_speed": 8,
                "supply_consumption": 2.0,
                "fuel_consumption": 1.0,
                "ammunition_consumption": 2.5,
                "equipment_cost": 200
            },
            "artillery": {
                "combat_strength": 25,
                "movement_speed": 2,
                "supply_consumption": 3.0,
                "fuel_consumption": 1.5,
                "ammunition_consumption": 10.0,
                "equipment_cost": 500
            }
        }
    
    def create_unit(self, template_name: str, name: str, size: int = 1000) -> Optional[Unit]:
        """Create a new unit from a template"""
        if template_name not in self.unit_templates:
            return None
        
        template = self.unit_templates[template_name]
        unit_id = f"unit_{len(self.units) + 1}"
        
        new_unit = Unit(
            id=unit_id,
            name=name,
            unit_type=UnitType(template_name.split('_')[-1]),
            combat_strength=template["combat_strength"],
            movement_speed=template["movement_speed"],
            supply_consumption=template["supply_consumption"],
            fuel_consumption=template["fuel_consumption"],
            ammunition_consumption=template["ammunition_consumption"],
            morale=0.6,
            experience=0.0,
            equipment_quality=1.0,
            current_strength=size,
            max_strength=size
        )
        
        self.units[unit_id] = new_unit
        return new_unit
    
    def update_unit_strength(self, unit_id: str, casualties: int, prisoners: int = 0) -> bool:
        """Update unit strength after combat"""
        if unit_id not in self.units:
            return False
        
        unit = self.units[unit_id]
        total_losses = casualties + prisoners
        unit.current_strength = max(0, unit.current_strength - total_losses)
        
        # Update morale based on casualties
        casualty_ratio = total_losses / unit.max_strength if unit.max_strength > 0 else 0
        unit.morale = max(0.1, unit.morale - casualty_ratio * 0.3)
        
        # Gain experience from combat
        unit.experience = min(1.0, unit.experience + casualty_ratio * 0.1)
        
        return True
    
    def reinforce_unit(self, unit_id: str, reinforcements: int) -> bool:
        """Reinforce a unit with new troops"""
        if unit_id not in self.units:
            return False
        
        unit = self.units[unit_id]
        unit.current_strength = min(unit.max_strength, unit.current_strength + reinforcements)
        
        # New troops dilute experience and morale
        reinforcement_ratio = reinforcements / unit.max_strength
        unit.experience *= (1 - reinforcement_ratio)
        unit.morale = (unit.morale * (unit.current_strength - reinforcements) + 
                      0.5 * reinforcements) / unit.current_strength
        
        return True
    
    def upgrade_equipment(self, unit_id: str, quality_improvement: float, cost: Dict[str, float]) -> bool:
        """Upgrade unit equipment"""
        if unit_id not in self.units:
            return False
        
        unit = self.units[unit_id]
        unit.equipment_quality += quality_improvement
        unit.combat_strength *= (1 + quality_improvement * 0.2)
        
        return True
    
    def organize_units(self, parent_unit: str, child_units: List[str]):
        """Organize units into a hierarchy"""
        if parent_unit not in self.organization:
            self.organization[parent_unit] = []
        
        self.organization[parent_unit].extend(child_units)
    
    def calculate_combat_power(self, unit_ids: List[str]) -> Dict[str, float]:
        """Calculate combat power for a group of units"""
        total_power = 0.0
        detailed_power = {}
        
        for unit_id in unit_ids:
            if unit_id in self.units:
                unit = self.units[unit_id]
                power = (unit.combat_strength * unit.current_strength / unit.max_strength *
                        unit.morale * (1 + unit.experience * 0.5) * unit.equipment_quality)
                total_power += power
                detailed_power[unit_id] = power
        
        return {"total": total_power, "detailed": detailed_power}
    
    def get_supply_requirements(self, unit_ids: List[str]) -> Dict[str, float]:
        """Get supply requirements for a group of units"""
        requirements = {
            "food": 0.0,
            "fuel": 0.0,
            "ammunition": 0.0,
            "medical": 0.0
        }
        
        for unit_id in unit_ids:
            if unit_id in self.units:
                unit = self.units[unit_id]
                requirements["food"] += unit.supply_consumption * 0.6
                requirements["fuel"] += unit.fuel_consumption
                requirements["ammunition"] += unit.ammunition_consumption
                requirements["medical"] += unit.supply_consumption * 0.1
        
        return requirements
