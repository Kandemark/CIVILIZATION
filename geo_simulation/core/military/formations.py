"""Unit formations"""

# Module implementation

# formations.py
from enum import Enum
from typing import Dict, List, Tuple
import numpy as np
from dataclasses import dataclass

class FormationType(Enum):
    LINE = "line"
    COLUMN = "column"
    WEDGE = "wedge"
    ECHELON = "echelon"
    SKIRMISH = "skirmish"
    SQUARE = "square"
    PHALANX = "phalanx"

@dataclass
class FormationStats:
    attack_bonus: float
    defense_bonus: float
    movement_penalty: float
    flanking_vulnerability: float
    supply_consumption: float

class FormationSystem:
    def __init__(self):
        self.formations = self._initialize_formations()
        self.current_formation = FormationType.LINE
        self.formation_history = []
    
    def _initialize_formations(self) -> Dict[FormationType, FormationStats]:
        """Initialize all available formations with their stats"""
        return {
            FormationType.LINE: FormationStats(1.0, 1.2, 0.0, 0.3, 1.0),
            FormationType.COLUMN: FormationStats(0.8, 0.7, 0.2, 0.5, 0.9),
            FormationType.WEDGE: FormationStats(1.3, 0.9, -0.1, 0.4, 1.1),
            FormationType.ECHELON: FormationStats(1.1, 1.0, 0.1, 0.2, 1.0),
            FormationType.SKIRMISH: FormationStats(0.9, 0.8, 0.3, 0.6, 0.8),
            FormationType.SQUARE: FormationStats(0.7, 1.5, -0.3, 0.1, 1.2),
            FormationType.PHALANX: FormationStats(0.9, 1.4, -0.2, 0.2, 1.1)
        }
    
    def change_formation(self, new_formation: FormationType) -> bool:
        """Change to a new formation"""
        if new_formation in self.formations:
            self.formation_history.append((self.current_formation, new_formation))
            self.current_formation = new_formation
            return True
        return False
    
    def get_current_stats(self) -> FormationStats:
        """Get stats for the current formation"""
        return self.formations[self.current_formation]
    
    def calculate_formation_bonus(self, unit_type: str, terrain: str) -> float:
        """Calculate formation bonus based on unit type and terrain"""
        stats = self.get_current_stats()
        base_bonus = (stats.attack_bonus + stats.defense_bonus) / 2
        
        # Adjust for unit type compatibility
        unit_modifiers = {
            "infantry": 1.0,
            "cavalry": 0.8 if self.current_formation == FormationType.PHALANX else 1.2,
            "artillery": 0.7 if self.current_formation in [FormationType.WEDGE, FormationType.COLUMN] else 1.0,
            "armor": 1.1 if self.current_formation in [FormationType.WEDGE, FormationType.ECHELON] else 0.9
        }
        
        # Adjust for terrain compatibility
        terrain_modifiers = {
            "plains": 1.0,
            "forest": 0.8 if self.current_formation in [FormationType.LINE, FormationType.PHALANX] else 0.9,
            "mountains": 0.6 if self.current_formation == FormationType.COLUMN else 0.8,
            "urban": 0.7 if self.current_formation == FormationType.SKIRMISH else 0.9
        }
        
        unit_mod = unit_modifiers.get(unit_type, 1.0)
        terrain_mod = terrain_modifiers.get(terrain, 1.0)
        
        return base_bonus * unit_mod * terrain_mod
    
    def optimize_formation(self, unit_composition: Dict[str, int], terrain: str, mission_type: str) -> FormationType:
        """Recommend the best formation based on situation"""
        best_formation = FormationType.LINE
        best_score = -float('inf')
        
        for formation in FormationType:
            self.current_formation = formation
            score = self._calculate_formation_score(unit_composition, terrain, mission_type)
            
            if score > best_score:
                best_score = score
                best_formation = formation
        
        self.current_formation = best_formation
        return best_formation
    
    def _calculate_formation_score(self, unit_composition: Dict[str, int], terrain: str, mission_type: str) -> float:
        """Calculate a score for how good this formation is for the situation"""
        stats = self.get_current_stats()
        score = 0
        
        # Mission type weights
        mission_weights = {
            "attack": stats.attack_bonus * 1.5 + stats.defense_bonus * 0.5,
            "defend": stats.attack_bonus * 0.5 + stats.defense_bonus * 1.5,
            "maneuver": (1 - stats.movement_penalty) * 2.0,
            "retreat": (1 - stats.movement_penalty) * 1.5 + stats.defense_bonus * 0.5
        }
        
        score += mission_weights.get(mission_type, 1.0)
        
        # Unit composition adjustment
        for unit_type, count in unit_composition.items():
            unit_score = self.calculate_formation_bonus(unit_type, terrain)
            score += unit_score * count
        
        return score