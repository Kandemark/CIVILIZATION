"""Unit morale based on population satisfaction"""

# Module implementation

# morale.py
from typing import Dict, List
from dataclasses import dataclass
from enum import Enum
import numpy as np

class MoraleState(Enum):
    ROUTED = 0.0
    BROKEN = 0.2
    LOW = 0.4
    STEADY = 0.6
    CONFIDENT = 0.8
    ELITE = 1.0

@dataclass
class MoraleEvent:
    type: str
    magnitude: float
    duration: float
    description: str

class MoraleSystem:
    def __init__(self, base_morale: float = 0.6):
        self.base_morale = base_morale
        self.current_morale = base_morale
        self.morale_history = []
        self.active_events: List[MoraleEvent] = []
        self.unit_cohesion = 1.0
        self.leadership_bonus = 0.0
    
    def update_morale(self, time_delta: float, casualties: float = 0.0, 
                     supply_status: float = 1.0, victories: int = 0, 
                     defeats: int = 0) -> float:
        """Update morale based on various factors"""
        # Base recovery
        recovery = 0.1 * time_delta
        
        # Casualty impact
        casualty_impact = -0.3 * casualties
        
        # Supply impact
        supply_impact = 0.2 * (supply_status - 0.5)
        
        # Combat results impact
        combat_impact = 0.1 * (victories - defeats)
        
        # Apply active events
        event_impact = 0.0
        remaining_events = []
        for event in self.active_events:
            event_impact += event.magnitude
            event.duration -= time_delta
            if event.duration > 0:
                remaining_events.append(event)
        self.active_events = remaining_events
        
        # Calculate total change
        total_change = (recovery + casualty_impact + supply_impact + 
                       combat_impact + event_impact + self.leadership_bonus)
        
        # Apply unit cohesion modifier
        total_change *= self.unit_cohesion
        
        # Update morale
        self.current_morale = np.clip(self.current_morale + total_change, 0.0, 1.0)
        self.morale_history.append(self.current_morale)
        
        return self.current_morale
    
    def add_morale_event(self, event_type: str, magnitude: float, 
                        duration: float, description: str):
        """Add a morale-affecting event"""
        event = MoraleEvent(
            type=event_type,
            magnitude=magnitude,
            duration=duration,
            description=description
        )
        self.active_events.append(event)
    
    def get_morale_state(self) -> MoraleState:
        """Get the current morale state"""
        if self.current_morale <= 0.2:
            return MoraleState.ROUTED
        elif self.current_morale <= 0.4:
            return MoraleState.BROKEN
        elif self.current_morale <= 0.6:
            return MoraleState.LOW
        elif self.current_morale <= 0.8:
            return MoraleState.STEADY
        else:
            return MoraleState.CONFIDENT
    
    def apply_combat_result(self, victory: bool, magnitude: float):
        """Apply combat result to morale"""
        impact = 0.1 * magnitude * (1 if victory else -1)
        self.current_morale = np.clip(self.current_morale + impact, 0.0, 1.0)
    
    def update_unit_cohesion(self, time_together: float, training_level: float,
                           casualty_rate: float) -> float:
        """Update unit cohesion based on various factors"""
        # Time together increases cohesion
        time_effect = 0.1 * np.log(1 + time_together)
        
        # Training increases cohesion
        training_effect = 0.2 * training_level
        
        # Casualties decrease cohesion
        casualty_effect = -0.3 * casualty_rate
        
        self.unit_cohesion = np.clip(self.unit_cohesion + time_effect + 
                                   training_effect + casualty_effect, 0.1, 1.0)
        
        return self.unit_cohesion
    
    def set_leadership_bonus(self, leadership_skill: float, leader_popularity: float):
        """Set leadership bonus based on leader characteristics"""
        self.leadership_bonus = 0.1 * leadership_skill + 0.05 * leader_popularity
    
    def get_combat_modifier(self) -> float:
        """Get combat effectiveness modifier based on morale"""
        state = self.get_morale_state()
        modifiers = {
            MoraleState.ROUTED: 0.3,
            MoraleState.BROKEN: 0.6,
            MoraleState.LOW: 0.8,
            MoraleState.STEADY: 1.0,
            MoraleState.CONFIDENT: 1.2,
            MoraleState.ELITE: 1.5
        }
        return modifiers[state]