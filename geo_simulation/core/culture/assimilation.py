"""Cultural assimilation processes"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
import random
import math

@dataclass
class AssimilationPressure:
    source_culture: str
    target_culture: str
    pressure_strength: float
    pressure_type: str  # "military", "economic", "cultural", "demographic"
    duration: int

class InfiniteAssimilationSystem:
    """Handles cultural assimilation without predefined outcomes"""
    
    def __init__(self):
        self.active_pressures: Dict[str, List[AssimilationPressure]] = {}  # target -> pressures
        self.assimilation_history: List[Dict] = []
        self.resistance_factors: Dict[str, float] = {}  # culture -> resistance
        
    def apply_assimilation_pressure(self, source_culture: str, target_culture: str, pressure_type: str, strength: float, duration: int):
        """Apply assimilation pressure between cultures"""
        pressure = AssimilationPressure(
            source_culture=source_culture,
            target_culture=target_culture,
            pressure_strength=strength,
            pressure_type=pressure_type,
            duration=duration
        )
        
        if target_culture not in self.active_pressures:
            self.active_pressures[target_culture] = []
        
        self.active_pressures[target_culture].append(pressure)
    
    def simulate_assimilation_turn(self, cultures: Dict[str, any]) -> Dict[str, List[Dict]]:
        """Simulate one turn of cultural assimilation"""
        assimilation_results = {}
        
        for target_culture_id, pressures in self.active_pressures.items():
            if target_culture_id not in cultures:
                continue
                
            target_culture = cultures[target_culture_id]
            culture_results = []
            
            for pressure in pressures[:]:  # Use slice to avoid modification during iteration
                result = self._process_assimilation_pressure(target_culture, pressure, cultures)
                culture_results.append(result)
                
                # Decrement duration and remove expired pressures
                pressure.duration -= 1
                if pressure.duration <= 0:
                    self.active_pressures[target_culture_id].remove(pressure)
            
            assimilation_results[target_culture_id] = culture_results
        
        return assimilation_results
    
    def _process_assimilation_pressure(self, target_culture, pressure: AssimilationPressure, cultures: Dict[str, any]) -> Dict:
        """Process a single assimilation pressure"""
        source_culture = cultures.get(pressure.source_culture)
        if not source_culture:
            return {"assimilated": False, "reason": "source_culture_not_found"}
        
        # Calculate assimilation probability
        base_probability = pressure.pressure_strength
        
        # Modify by culture traits
        resistance = self._get_culture_resistance(target_culture)
        attractiveness = self._get_culture_attractiveness(source_culture)
        
        # Pressure type modifiers
        type_modifiers = {
            "military": 1.5,    # Military conquest highly effective
            "economic": 1.2,    # Economic dominance moderately effective
            "cultural": 0.8,    # Cultural appeal less direct
            "demographic": 1.0  # Population pressure neutral
        }
        
        type_modifier = type_modifiers.get(pressure.pressure_type, 1.0)
        
        final_probability = base_probability * type_modifier * attractiveness / resistance
        
        # Check if assimilation occurs
        if random.random() < final_probability:
            return self._execute_assimilation(target_culture, source_culture, pressure)
        else:
            return {
                "assimilated": False,
                "pressure_type": pressure.pressure_type,
                "probability": final_probability,
                "resistance": resistance
            }
    
    def _get_culture_resistance(self, culture) -> float:
        """Calculate a culture's resistance to assimilation"""
        # Base resistance from cultural cohesion
        base_resistance = getattr(culture, 'cohesion', 0.5)
        
        # Resistance increases with civilization index
        civ_index = getattr(culture, 'civilization_index', 1.0)
        civ_resistance = math.log(1.0 + civ_index) * 0.1
        
        # Cultural complexity provides resistance
        complexity = getattr(culture, 'cultural_complexity', 1.0)
        complexity_resistance = math.log(complexity) * 0.05
        
        total_resistance = base_resistance + civ_resistance + complexity_resistance
        return min(2.0, total_resistance)  # Cap at 2.0 for balance
    
    def _get_culture_attractiveness(self, culture) -> float:
        """Calculate how attractive a culture is to others"""
        # Prestige and development make culture more attractive
        prestige = getattr(culture, 'prestige', 0.5)
        civ_index = getattr(culture, 'civilization_index', 1.0)
        
        # Technological and artistic sophistication increases attractiveness
        tech_mastery = getattr(culture, 'technological_mastery', 1.0)
        artistic_sophistication = getattr(culture, 'aesthetic_sophistication', 1.0)
        
        attractiveness = (prestige * 0.4 + 
                         math.log(1.0 + civ_index) * 0.3 +
                         math.log(tech_mastery) * 0.2 +
                         math.log(artistic_sophistication) * 0.1)
        
        return min(2.0, attractiveness)
    
    def _execute_assimilation(self, target_culture, source_culture, pressure: AssimilationPressure) -> Dict:
        """Execute cultural assimilation"""
        assimilation_strength = pressure.pressure_strength
        
        # Determine what gets assimilated
        traits_assimilated = []
        traditions_adopted = []
        
        # Assimilate cultural traits
        if hasattr(source_culture, 'emergent_traits') and hasattr(target_culture, 'emergent_traits'):
            for trait, strength in source_culture.emergent_traits.items():
                if random.random() < assimilation_strength * 0.3:
                    current = target_culture.emergent_traits.get(trait, 0.0)
                    assimilation_amount = strength * assimilation_strength * 0.1
                    target_culture.emergent_traits[trait] = min(1.0, current + assimilation_amount)
                    traits_assimilated.append(trait)
        
        # Adopt traditions
        if hasattr(source_culture, 'generated_traditions') and hasattr(target_culture, 'generated_traditions'):
            for tradition in source_culture.generated_traditions:
                if (random.random() < assimilation_strength * 0.2 and 
                    tradition not in target_culture.generated_traditions):
                    target_culture.generated_traditions.add(tradition)
                    traditions_adopted.append(tradition)
        
        # Cultural prestige transfer
        if hasattr(source_culture, 'prestige') and hasattr(target_culture, 'prestige'):
            prestige_transfer = assimilation_strength * 0.1
            source_culture.prestige = max(0.0, source_culture.prestige - prestige_transfer * 0.5)
            target_culture.prestige = min(1.0, target_culture.prestige + prestige_transfer)
        
        result = {
            "assimilated": True,
            "pressure_type": pressure.pressure_type,
            "traits_assimilated": traits_assimilated,
            "traditions_adopted": traditions_adopted,
            "assimilation_strength": assimilation_strength
        }
        
        # Record in history
        self.assimilation_history.append({
            "turn": len(self.assimilation_history),
            "source": pressure.source_culture,
            "target": pressure.target_culture,
            "result": result
        })
        
        return result
    
    def get_assimilation_risk(self, culture_id: str, cultures: Dict[str, any]) -> float:
        """Calculate a culture's risk of being assimilated"""
        if culture_id not in self.active_pressures:
            return 0.0
        
        total_risk = 0.0
        culture = cultures.get(culture_id)
        if not culture:
            return 0.0
        
        resistance = self._get_culture_resistance(culture)
        
        for pressure in self.active_pressures[culture_id]:
            source_culture = cultures.get(pressure.source_culture)
            if source_culture:
                attractiveness = self._get_culture_attractiveness(source_culture)
                risk = pressure.pressure_strength * attractiveness / resistance
                total_risk += risk
        
        return min(1.0, total_risk)