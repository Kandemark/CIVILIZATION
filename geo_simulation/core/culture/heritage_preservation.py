"""Maintaining cultural traditions"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
import random
import math

@dataclass
class CulturalHeritage:
    heritage_id: str
    culture_id: str
    heritage_type: str  # "artifact", "tradition", "knowledge", "site"
    significance: float
    preservation_status: float  # 0-1
    age: int
    rediscovery_chance: float

class InfiniteHeritageSystem:
    """Manages cultural heritage that can be preserved, lost, or rediscovered infinitely"""
    
    def __init__(self):
        self.active_heritage: Dict[str, CulturalHeritage] = {}
        self.lost_heritage: Dict[str, CulturalHeritage] = {}
        self.preservation_efforts: Dict[str, float] = {}  # culture -> effort
        
    def generate_heritage(self, culture_id: str, heritage_type: str, significance: float) -> str:
        """Generate new cultural heritage"""
        heritage_id = f"heritage_{culture_id}_{len(self.active_heritage)}"
        
        heritage = CulturalHeritage(
            heritage_id=heritage_id,
            culture_id=culture_id,
            heritage_type=heritage_type,
            significance=significance,
            preservation_status=1.0,  # New heritage is well-preserved
            age=0,
            rediscovery_chance=0.1
        )
        
        self.active_heritage[heritage_id] = heritage
        return heritage_id
    
    def simulate_heritage_turn(self, cultures: Dict[str, any]) -> Dict[str, List[Dict]]:
        """Simulate one turn of heritage preservation and loss"""
        heritage_results = {}
        
        # Age and degrade heritage
        for heritage_id, heritage in list(self.active_heritage.items()):
            culture_id = heritage.culture_id
            if culture_id not in heritage_results:
                heritage_results[culture_id] = []
            
            # Natural degradation
            degradation_rate = self._calculate_degradation_rate(heritage, cultures.get(culture_id))
            heritage.preservation_status = max(0.0, heritage.preservation_status - degradation_rate)
            heritage.age += 1
            
            # Check for loss
            if heritage.preservation_status <= 0.1 and random.random() < 0.1:
                self._lose_heritage(heritage_id)
                heritage_results[culture_id].append({
                    "type": "heritage_lost",
                    "heritage_id": heritage_id,
                    "heritage_type": heritage.heritage_type,
                    "significance": heritage.significance
                })
            else:
                # Preservation effects
                preservation_effect = self._apply_preservation_efforts(heritage, cultures.get(culture_id))
                if preservation_effect > 0:
                    heritage_results[culture_id].append({
                        "type": "heritage_preserved",
                        "heritage_id": heritage_id,
                        "improvement": preservation_effect
                    })
        
        # Rediscovery of lost heritage
        rediscovery_results = self._simulate_rediscovery()
        for culture_id, rediscoveries in rediscovery_results.items():
            if culture_id not in heritage_results:
                heritage_results[culture_id] = []
            heritage_results[culture_id].extend(rediscoveries)
        
        return heritage_results
    
    def _calculate_degradation_rate(self, heritage: CulturalHeritage, culture) -> float:
        """Calculate how quickly heritage degrades"""
        base_degradation = 0.01
        
        # Older heritage degrades faster
        age_factor = math.log(1.0 + heritage.age) * 0.005
        
        # Significance affects degradation (very significant heritage is better maintained)
        significance_factor = (1.0 - heritage.significance) * 0.02
        
        # Cultural factors
        cultural_preservation = getattr(culture, 'cultural_complexity', 1.0)
        preservation_bonus = math.log(cultural_preservation) * 0.001
        
        total_degradation = base_degradation + age_factor + significance_factor - preservation_bonus
        return max(0.001, total_degradation)
    
    def _apply_preservation_efforts(self, heritage: CulturalHeritage, culture) -> float:
        """Apply cultural preservation efforts to heritage"""
        if not culture:
            return 0.0
        
        # Base preservation from cultural complexity
        preservation_power = getattr(culture, 'cultural_complexity', 1.0) * 0.001
        
        # Additional effort from conscious preservation
        conscious_effort = self.preservation_efforts.get(heritage.culture_id, 0.0) * 0.01
        
        total_preservation = preservation_power + conscious_effort
        
        # Apply preservation
        improvement = total_preservation * heritage.significance
        heritage.preservation_status = min(1.0, heritage.preservation_status + improvement)
        
        return improvement
    
    def _lose_heritage(self, heritage_id: str):
        """Move heritage from active to lost"""
        heritage = self.active_heritage[heritage_id]
        self.lost_heritage[heritage_id] = heritage
        del self.active_heritage[heritage_id]
        
        # Increase rediscovery chance for significant heritage
        if heritage.significance > 0.7:
            heritage.rediscovery_chance += 0.1
    
    def _simulate_rediscovery(self) -> Dict[str, List[Dict]]:
        """Simulate rediscovery of lost heritage"""
        rediscovery_results = {}
        
        for heritage_id, heritage in list(self.lost_heritage.items()):
            if random.random() < heritage.rediscovery_chance:
                # Rediscovery occurs!
                self.active_heritage[heritage_id] = heritage
                del self.lost_heritage[heritage_id]
                
                # Reset preservation status
                heritage.preservation_status = 0.5  # Partially restored
                
                if heritage.culture_id not in rediscovery_results:
                    rediscovery_results[heritage.culture_id] = []
                
                rediscovery_results[heritage.culture_id].append({
                    "type": "heritage_rediscovered",
                    "heritage_id": heritage_id,
                    "heritage_type": heritage.heritage_type,
                    "significance": heritage.significance,
                    "restored_preservation": heritage.preservation_status
                })
                
                # Decrease rediscovery chance after successful rediscovery
                heritage.rediscovery_chance *= 0.5
        
        return rediscovery_results
    
    def invest_preservation_effort(self, culture_id: str, effort: float):
        """A culture invests effort in preserving its heritage"""
        if culture_id not in self.preservation_efforts:
            self.preservation_efforts[culture_id] = 0.0
        
        self.preservation_efforts[culture_id] = min(1.0, 
            self.preservation_efforts[culture_id] + effort)
    
    def calculate_cultural_continuity(self, culture_id: str) -> float:
        """Calculate a culture's continuity based on preserved heritage"""
        active_heritage = [h for h in self.active_heritage.values() 
                          if h.culture_id == culture_id]
        
        if not active_heritage:
            return 0.0
        
        total_significance = sum(h.significance * h.preservation_status 
                               for h in active_heritage)
        avg_preservation = sum(h.preservation_status for h in active_heritage) / len(active_heritage)
        
        # Heritage age contributes to continuity
        avg_age = sum(h.age for h in active_heritage) / len(active_heritage)
        age_factor = math.log(1.0 + avg_age) * 0.1
        
        continuity = (total_significance * 0.6 + avg_preservation * 0.3 + age_factor * 0.1)
        return min(1.0, continuity)