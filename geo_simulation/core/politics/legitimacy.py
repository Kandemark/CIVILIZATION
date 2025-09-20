"""Government legitimacy metrics"""

# Module implementation

# politics/legitimacy.py
from enum import Enum, auto
from typing import Dict, List, Set, Optional
from dataclasses import dataclass, field
import numpy as np
from datetime import datetime, timedelta
import logging

logger = logging.getLogger(__name__)

class LegitimacySource(Enum):
    """Sources of political legitimacy"""
    DEMOCRATIC_ELECTION = auto()
    DIVINE_RIGHT = auto()
    REVOLUTIONARY = auto()
    TRADITIONAL = auto()
    CHARISMATIC = auto()
    TECHNOCRATIC = auto()
    MILITARY_VICTORY = auto()
    ECONOMIC_SUCCESS = auto()

class LegitimacyLevel(Enum):
    """Levels of political legitimacy"""
    ILLEGITIMATE = 0.0    # < 0.2
    WEAK = 0.2            # 0.2 - 0.4
    CONTESTED = 0.4       # 0.4 - 0.6
    MODERATE = 0.6        # 0.6 - 0.8
    STRONG = 0.8          # 0.8 - 1.0

@dataclass
class LegitimacyEvent:
    """An event that affects legitimacy"""
    type: str
    magnitude: float
    duration: timedelta
    description: str
    source: LegitimacySource
    start_time: datetime = field(default_factory=datetime.now)

class LegitimacySystem:
    """Manages political legitimacy for rulers and governments"""
    
    def __init__(self):
        self.legitimacy_score: float = 0.5  # 0.0 to 1.0
        self.sources: Set[LegitimacySource] = set()
        self.active_events: List[LegitimacyEvent] = []
        self.historical_events: List[LegitimacyEvent] = []
        self.decay_rate: float = 0.01  # Daily legitimacy decay
        
    def add_legitimacy_source(self, source: LegitimacySource, strength: float = 0.1) -> None:
        """Add a source of legitimacy"""
        self.sources.add(source)
        self.legitimacy_score = min(1.0, self.legitimacy_score + strength)
        logger.info(f"Added legitimacy source: {source.name} (+{strength})")
    
    def remove_legitimacy_source(self, source: LegitimacySource, penalty: float = 0.1) -> None:
        """Remove a source of legitimacy"""
        if source in self.sources:
            self.sources.remove(source)
            self.legitimacy_score = max(0.0, self.legitimacy_score - penalty)
            logger.info(f"Removed legitimacy source: {source.name} (-{penalty})")
    
    def add_legitimacy_event(self, event: LegitimacyEvent) -> None:
        """Add a legitimacy-affecting event"""
        self.active_events.append(event)
        self.legitimacy_score = np.clip(
            self.legitimacy_score + event.magnitude, 0.0, 1.0
        )
        logger.info(f"Legitimacy event: {event.description} ({event.magnitude:+.2f})")
    
    def update_legitimacy(self, time_delta: float = 1.0) -> None:
        """Update legitimacy over time"""
        current_time = datetime.now()
        
        # Process active events
        remaining_events = []
        for event in self.active_events:
            if current_time - event.start_time < event.duration:
                remaining_events.append(event)
            else:
                self.historical_events.append(event)
                logger.debug(f"Legitimacy event expired: {event.description}")
        
        self.active_events = remaining_events
        
        # Apply natural decay
        decay = self.decay_rate * time_delta
        self.legitimacy_score = max(0.0, self.legitimacy_score - decay)
        
        # Base legitimacy from sources
        base_legitimacy = len(self.sources) * 0.1
        self.legitimacy_score = np.clip(
            self.legitimacy_score + base_legitimacy * 0.01 * time_delta, 0.0, 1.0
        )
    
    def get_legitimacy_level(self) -> LegitimacyLevel:
        """Get the current legitimacy level"""
        if self.legitimacy_score < 0.2:
            return LegitimacyLevel.ILLEGITIMATE
        elif self.legitimacy_score < 0.4:
            return LegitimacyLevel.WEAK
        elif self.legitimacy_score < 0.6:
            return LegitimacyLevel.CONTESTED
        elif self.legitimacy_score < 0.8:
            return LegitimacyLevel.MODERATE
        else:
            return LegitimacyLevel.STRONG
    
    def get_crisis_risk(self) -> float:
        """Calculate the risk of political crisis based on legitimacy"""
        level = self.get_legitimacy_level()
        
        risk_factors = {
            LegitimacyLevel.ILLEGITIMATE: 0.8,
            LegitimacyLevel.WEAK: 0.5,
            LegitimacyLevel.CONTESTED: 0.3,
            LegitimacyLevel.MODERATE: 0.1,
            LegitimacyLevel.STRONG: 0.01
        }
        
        return risk_factors[level]
    
    def can_govern_effectively(self) -> bool:
        """Check if the government can govern effectively"""
        return self.legitimacy_score >= 0.4
    
    def get_public_support(self) -> float:
        """Estimate public support based on legitimacy"""
        # Public support is generally higher than raw legitimacy score
        return min(1.0, self.legitimacy_score * 1.2)
    
    def trigger_corruption_scandal(self, severity: float = 0.3) -> None:
        """Trigger a corruption scandal that reduces legitimacy"""
        event = LegitimacyEvent(
            type="corruption_scandal",
            magnitude=-severity,
            duration=timedelta(days=30 * severity),
            description=f"Corruption scandal (severity: {severity})",
            source=LegitimacySource.DEMOCRATIC_ELECTION
        )
        self.add_legitimacy_event(event)
    
    def trigger_economic_boom(self, magnitude: float = 0.2) -> None:
        """Trigger economic success that increases legitimacy"""
        event = LegitimacyEvent(
            type="economic_boom",
            magnitude=magnitude,
            duration=timedelta(days=60),
            description=f"Economic prosperity boost",
            source=LegitimacySource.ECONOMIC_SUCCESS
        )
        self.add_legitimacy_event(event)
    
    def hold_election(self, fairness: float = 0.8) -> None:
        """Hold an election to gain democratic legitimacy"""
        # Fairness affects the legitimacy gain
        gain = 0.3 * fairness
        event = LegitimacyEvent(
            type="election",
            magnitude=gain,
            duration=timedelta(days=365),  # Election legitimacy lasts ~1 year
            description=f"Democratic election (fairness: {fairness:.1f})",
            source=LegitimacySource.DEMOCRATIC_ELECTION
        )
        self.add_legitimacy_event(event)