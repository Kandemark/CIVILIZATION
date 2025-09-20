"""Internal conflicts and coups"""

# Module implementation

# politics/power_struggles.py
from enum import Enum, auto
from typing import Dict, List, Optional, Set
from dataclasses import dataclass, field
import numpy as np
from datetime import datetime
import logging

logger = logging.getLogger(__name__)

class StruggleType(Enum):
    """Types of power struggles"""
    LEADERSHIP_CONTEST = auto()
    POLICY_DISPUTE = auto()
    RESOURCE_ALLOCATION = auto()
    SUCCESSION_RIGHTS = auto()
    IDEOLOGICAL_CONFLICT = auto()
    PERSONAL_RIVALRY = auto()

class StruggleOutcome(Enum):
    """Possible outcomes of power struggles"""
    VICTORY = auto()
    COMPROMISE = auto()
    STALEMATE = auto()
    DEFEAT = auto()
    EXTERNAL_INTERVENTION = auto()

@dataclass
class PowerStruggle:
    """Represents a power struggle between political actors"""
    id: str
    type: StruggleType
    participants: Set[str]
    start_date: datetime
    intensity: float  # 0.0 to 1.0
    description: str
    resolved: bool = False
    outcome: Optional[StruggleOutcome] = None
    winner: Optional[str] = None
    concessions: Dict[str, float] = field(default_factory=dict)

class StruggleSystem:
    """Manages power struggles within and between factions"""
    
    def __init__(self):
        self.active_struggles: Dict[str, PowerStruggle] = {}
        self.resolved_struggles: List[PowerStruggle] = {}
        self.struggle_history: List[Dict] = []
    
    def initiate_struggle(self, struggle_type: StruggleType, participants: Set[str],
                         intensity: float, description: str) -> Optional[PowerStruggle]:
        """Initiate a new power struggle"""
        if len(participants) < 2:
            return None
        
        struggle_id = f"struggle_{len(self.active_struggles) + len(self.resolved_struggles) + 1}"
        
        struggle = PowerStruggle(
            id=struggle_id,
            type=struggle_type,
            participants=participants,
            start_date=datetime.now(),
            intensity=intensity,
            description=description
        )
        
        self.active_struggles[struggle_id] = struggle
        logger.info(f"Power struggle initiated: {description} (Intensity: {intensity:.1f})")
        return struggle
    
    def resolve_struggle(self, struggle_id: str, outcome: StruggleOutcome,
                        winner: Optional[str] = None, concessions: Optional[Dict[str, float]] = None) -> bool:
        """Resolve a power struggle"""
        if struggle_id not in self.active_struggles:
            return False
        
        struggle = self.active_struggles[struggle_id]
        struggle.resolved = True
        struggle.outcome = outcome
        struggle.winner = winner
        struggle.concessions = concessions or {}
        
        self.resolved_struggles[struggle_id] = struggle
        del self.active_struggles[struggle_id]
        
        logger.info(f"Power struggle resolved: {struggle.description} -> {outcome.name}")
        return True
    
    def simulate_struggle_outcome(self, struggle_id: str, participant_power: Dict[str, float]) -> StruggleOutcome:
        """Simulate the outcome of a power struggle based on participant power"""
        if struggle_id not in self.active_struggles:
            return StruggleOutcome.STALEMATE
        
        struggle = self.active_struggles[struggle_id]
        total_power = sum(participant_power.get(part, 0) for part in struggle.participants)
        
        if total_power == 0:
            return StruggleOutcome.STALEMATE
        
        # Calculate power ratios
        power_ratios = {
            part: participant_power.get(part, 0) / total_power
            for part in struggle.participants
        }
        
        # Determine outcome based on power distribution
        max_power = max(power_ratios.values())
        if max_power > 0.7:
            return StruggleOutcome.VICTORY
        elif max_power > 0.55:
            return StruggleOutcome.COMPROMISE
        else:
            return StruggleOutcome.STALEMATE
    
    def update_struggles(self, time_delta: float = 1.0):
        """Update all active power struggles"""
        for struggle in list(self.active_struggles.values()):
            # Struggles intensify over time
            struggle.intensity = min(1.0, struggle.intensity + 0.05 * time_delta)
            
            # High intensity struggles may resolve spontaneously
            if struggle.intensity > 0.8 and np.random.random() < 0.1:
                outcome = np.random.choice(list(StruggleOutcome))
                self.resolve_struggle(struggle.id, outcome)
    
    def get_struggle_impact(self, struggle_id: str, faction_cohesion: Dict[str, float]) -> Dict[str, float]:
        """Calculate the impact of a power struggle on factions"""
        if struggle_id not in self.active_struggles:
            return {}
        
        struggle = self.active_struggles[struggle_id]
        impact = {}
        
        for faction_id, cohesion in faction_cohesion.items():
            # Struggles reduce faction cohesion
            cohesion_loss = struggle.intensity * 0.2
            impact[faction_id] = max(0.0, cohesion - cohesion_loss)
        
        return impact
    
    def trigger_leadership_contest(self, faction_id: str, candidates: Set[str], intensity: float = 0.5):
        """Trigger a leadership contest within a faction"""
        return self.initiate_struggle(
            StruggleType.LEADERSHIP_CONTEST,
            candidates,
            intensity,
            f"Leadership contest in faction {faction_id}"
        )
    
    def trigger_ideological_conflict(self, factions: Set[str], intensity: float = 0.6):
        """Trigger an ideological conflict between factions"""
        return self.initiate_struggle(
            StruggleType.IDEOLOGICAL_CONFLICT,
            factions,
            intensity,
            f"Ideological conflict between {len(factions)} factions"
        )