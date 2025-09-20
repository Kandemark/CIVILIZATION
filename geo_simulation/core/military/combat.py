"""Combat mechanics"""

# Module implementation

# combat.py
import numpy as np
from enum import Enum
from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass
import logging

logger = logging.getLogger(__name__)

class CombatPhase(Enum):
    PREP = "preparation"
    RANGED = "ranged"
    MELEE = "melee"
    PURSUIT = "pursuit"
    AFTERMATH = "aftermath"

@dataclass
class CombatResult:
    victor: str
    casualties_attacker: int
    casualties_defender: int
    prisoners: int
    territory_gained: bool
    resources_captured: Dict[str, float]
    duration: int  # in rounds

class CombatSystem:
    def __init__(self):
        self.terrain_modifiers = {
            "plains": 1.0,
            "forest": 0.8,
            "mountains": 0.6,
            "urban": 0.7,
            "river": 0.75,
            "fortified": 0.5
        }
        self.weather_modifiers = {
            "clear": 1.0,
            "rain": 0.9,
            "fog": 0.8,
            "snow": 0.7,
            "storm": 0.6
        }
        self.combat_history = []
        
    def calculate_combat_effectiveness(self, unit, terrain: str, weather: str) -> float:
        """Calculate unit effectiveness based on terrain and weather"""
        base = unit.combat_strength
        terrain_mod = self.terrain_modifiers.get(terrain, 1.0)
        weather_mod = self.weather_modifiers.get(weather, 1.0)
        
        # Apply unit-specific terrain bonuses
        if hasattr(unit, 'terrain_bonuses'):
            terrain_mod *= unit.terrain_bonuses.get(terrain, 1.0)
            
        return base * terrain_mod * weather_mod * unit.morale
    
    def simulate_engagement(self, attacker_units, defender_units, 
                          terrain: str = "plains", weather: str = "clear",
                          attacker_doctrine: Optional[str] = None,
                          defender_doctrine: Optional[str] = None) -> CombatResult:
        """
        Simulate a combat engagement between attacker and defender forces
        """
        logger.info(f"Combat initiated on {terrain} terrain with {weather} weather")
        
        # Calculate initial effectiveness
        att_strength = sum(self.calculate_combat_effectiveness(u, terrain, weather) 
                          for u in attacker_units)
        def_strength = sum(self.calculate_combat_effectiveness(u, terrain, weather) 
                          for u in defender_units)
        
        # Apply doctrine modifiers
        if attacker_doctrine:
            att_strength *= self._apply_doctrine_modifiers(attacker_doctrine, "offensive")
        if defender_doctrine:
            def_strength *= self._apply_doctrine_modifiers(defender_doctrine, "defensive")
        
        # Initial force ratio
        force_ratio = att_strength / def_strength if def_strength > 0 else float('inf')
        
        # Simulate combat rounds
        rounds = 0
        max_rounds = 20  # Prevent infinite combat
        att_casualties, def_casualties = 0, 0
        
        while rounds < max_rounds and att_strength > 0 and def_strength > 0:
            rounds += 1
            
            # Calculate damage exchange
            att_damage = att_strength * np.random.normal(0.1, 0.02)
            def_damage = def_strength * np.random.normal(0.08, 0.02)
            
            # Apply damage
            def_strength = max(0, def_strength - att_damage)
            att_strength = max(0, att_strength - def_damage)
            
            # Track casualties
            def_casualties += att_damage
            att_casualties += def_damage
            
            # Check for morale break
            if self._check_morale_break(attacker_units, def_casualties / def_strength if def_strength > 0 else 1.0):
                break
            if self._check_morale_break(defender_units, att_casualties / att_strength if att_strength > 0 else 1.0):
                break
        
        # Determine victor
        victor = "attacker" if att_strength > def_strength else "defender"
        
        # Calculate prisoners
        prisoners = int(def_casualties * 0.2) if victor == "attacker" else int(att_casualties * 0.1)
        
        result = CombatResult(
            victor=victor,
            casualties_attacker=int(att_casualties),
            casualties_defender=int(def_casualties),
            prisoners=prisoners,
            territory_gained=victor == "attacker",
            resources_captured=self._calculate_captured_resources(attacker_units if victor == "attacker" else defender_units),
            duration=rounds
        )
        
        self.combat_history.append(result)
        return result
    
    def _apply_doctrine_modifiers(self, doctrine: str, role: str) -> float:
        """Apply doctrine-specific combat modifiers"""
        doctrine_modifiers = {
            "blitzkrieg": {"offensive": 1.3, "defensive": 0.9},
            "defense_in_depth": {"offensive": 0.9, "defensive": 1.3},
            "maneuver_warfare": {"offensive": 1.2, "defensive": 1.0},
            "attrition_warfare": {"offensive": 1.1, "defensive": 1.1},
        }
        return doctrine_modifiers.get(doctrine, {}).get(role, 1.0)
    
    def _check_morale_break(self, units, casualty_ratio: float) -> bool:
        """Check if units break from morale loss"""
        avg_morale = sum(u.morale for u in units) / len(units) if units else 0
        break_chance = casualty_ratio * (1 - avg_morale)
        return np.random.random() < break_chance
    
    def _calculate_captured_resources(self, victorious_units) -> Dict[str, float]:
        """Calculate resources captured after victory"""
        # Base resources based on unit types
        resources = {
            "food": sum(u.supply_consumption * 0.5 for u in victorious_units),
            "fuel": sum(u.fuel_consumption * 0.3 for u in victorious_units),
            "ammunition": sum(u.ammunition_consumption * 0.4 for u in victorious_units)
        }
        return resources