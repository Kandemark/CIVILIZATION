"""Happiness, legitimacy, prestige metrics"""

# Module implementation

"""
Soft metrics system for intangible concepts like happiness, legitimacy, and prestige.
Uses fuzzy logic and probabilistic models to simulate abstract concepts.
"""

from typing import Dict, List, Tuple, Optional
from enum import Enum
import random
from dataclasses import dataclass
from .fuzzy_logic import FuzzySet, FuzzyVariable

class Mood(Enum):
    """Emotional states of populations."""
    ECSTATIC = 5
    HAPPY = 4
    CONTENT = 3
    DISCONTENT = 2
    UNHAPPY = 1
    REBELLIOUS = 0

@dataclass
class HappinessMetrics:
    """Comprehensive happiness measurement system."""
    base_happiness: float = 0.5  # 0.0 to 1.0
    stability: float = 0.5       # 0.0 to 1.0
    loyalty: float = 0.5         # 0.0 to 1.0
    recent_changes: List[float] = None
    
    def __post_init__(self):
        if self.recent_changes is None:
            self.recent_changes = []
    
    @property
    def overall_happiness(self) -> float:
        """Calculate overall happiness score."""
        base = self.base_happiness
        stability_factor = self.stability * 0.3
        loyalty_factor = self.loyalty * 0.2
        
        # Consider recent changes (weighted average)
        recent_impact = 0
        if self.recent_changes:
            weights = [0.5, 0.3, 0.2]  # Weight recent changes more heavily
            for i, change in enumerate(self.recent_changes[-3:]):  # Last 3 changes
                if i < len(weights):
                    recent_impact += change * weights[i]
        
        return max(0, min(1, base + stability_factor + loyalty_factor + recent_impact * 0.2))
    
    def get_mood(self) -> Mood:
        """Convert happiness score to mood enum."""
        score = self.overall_happiness
        if score >= 0.9: return Mood.ECSTATIC
        if score >= 0.7: return Mood.HAPPY
        if score >= 0.5: return Mood.CONTENT
        if score >= 0.3: return Mood.DISCONTENT
        if score >= 0.1: return Mood.UNHAPPY
        return Mood.REBELLIOUS
    
    def add_change(self, change: float):
        """Record a happiness change."""
        self.recent_changes.append(change)
        # Keep only recent changes
        if len(self.recent_changes) > 10:
            self.recent_changes = self.recent_changes[-10:]

class LegitimacySystem:
    """System for measuring government legitimacy and political stability."""
    
    def __init__(self):
        self.legitimacy = 0.7  # 0.0 to 1.0
        self.political_stability = 0.6
        self.corruption_level = 0.3
        self.government_approval = 0.5
        
    def calculate_legitimacy_score(self) -> float:
        """Calculate overall legitimacy score."""
        factors = {
            'stability': self.political_stability * 0.4,
            'corruption': (1 - self.corruption_level) * 0.3,
            'approval': self.government_approval * 0.3
        }
        
        return sum(factors.values())
    
    def update_from_events(self, events: List[Dict]) -> float:
        """Update legitimacy based on recent events."""
        event_impact = 0
        
        for event in events:
            impact = event.get('legitimacy_impact', 0)
            # Scale impact based on importance and recency
            scaled_impact = impact * event.get('importance', 1) * event.get('recency_factor', 1)
            event_impact += scaled_impact
        
        # Apply dampened effect of events
        self.legitimacy = max(0, min(1, self.legitimacy + event_impact * 0.1))
        return self.legitimacy

class PrestigeSystem:
    """System for measuring national prestige and international reputation."""
    
    def __init__(self):
        self.prestige = 0.5  # 0.0 to 1.0
        self.international_relations = {}
        self.cultural_influence = 0.4
        self.technological_achievements = 0.3
        self.military_prowess = 0.5
        
    def calculate_prestige(self) -> float:
        """Calculate overall prestige score."""
        weights = {
            'cultural': 0.25,
            'technological': 0.25,
            'military': 0.25,
            'diplomatic': 0.25
        }
        
        # Calculate diplomatic prestige from relations
        diplomatic_score = sum(self.international_relations.values()) / max(1, len(self.international_relations))
        
        score = (
            self.cultural_influence * weights['cultural'] +
            self.technological_achievements * weights['technological'] +
            self.military_prowess * weights['military'] +
            diplomatic_score * weights['diplomatic']
        )
        
        return max(0, min(1, score))
    
    def update_relation(self, nation_id: str, relation: float):
        """Update relation with another nation."""
        self.international_relations[nation_id] = max(-1, min(1, relation))
    
    def add_achievement(self, achievement_type: str, magnitude: float):
        """Add an achievement that boosts prestige."""
        if achievement_type == 'cultural':
            self.cultural_influence = min(1, self.cultural_influence + magnitude * 0.1)
        elif achievement_type == 'technological':
            self.technological_achievements = min(1, self.technological_achievements + magnitude * 0.1)
        elif achievement_type == 'military':
            self.military_prowess = min(1, self.military_prowess + magnitude * 0.1)

class SoftMetricsManager:
    """Manages all soft metrics for the simulation."""
    
    def __init__(self):
        self.happiness_metrics = HappinessMetrics()
        self.legitimacy_system = LegitimacySystem()
        self.prestige_system = PrestigeSystem()
        self.fuzzy_variables = self._setup_fuzzy_variables()
        
    def _setup_fuzzy_variables(self) -> Dict[str, FuzzyVariable]:
        """Set up fuzzy variables for soft metrics."""
        # Happiness fuzzy sets
        happiness_var = FuzzyVariable("Happiness")
        happiness_var.add_set(FuzzySet("Low", 0.0, 0.0, 0.3, 0.5))
        happiness_var.add_set(FuzzySet("Medium", 0.3, 0.5, 0.7, 0.9))
        happiness_var.add_set(FuzzySet("High", 0.6, 0.8, 1.0, 1.0))

        # Legitimacy fuzzy sets
        legitimacy_var = FuzzyVariable("Legitimacy")
        legitimacy_var.add_set(FuzzySet("Illegitimate", 0.0, 0.0, 0.3, 0.4))
        legitimacy_var.add_set(FuzzySet("Questionable", 0.3, 0.5, 0.7, 0.9))
        legitimacy_var.add_set(FuzzySet("Legitimate", 0.6, 0.8, 1.0, 1.0))

        return {
            'happiness': happiness_var,
            'legitimacy': legitimacy_var
        }

    
    def update_from_economy(self, economic_data: Dict):
        """Update soft metrics based on economic conditions."""
        # Happiness affected by economy
        economic_wellbeing = economic_data.get('gdp_per_capita', 0.5)
        unemployment = economic_data.get('unemployment', 0.5)
        
        happiness_impact = (economic_wellbeing * 0.7) - (unemployment * 0.3)
        self.happiness_metrics.add_change(happiness_impact)
        
        # Legitimacy affected by economic performance
        economic_growth = economic_data.get('growth_rate', 0)
        legitimacy_impact = economic_growth * 0.2
        self.legitimacy_system.legitimacy = max(0, min(1, 
            self.legitimacy_system.legitimacy + legitimacy_impact))
    
    def update_from_events(self, events: List[Dict]):
        """Update soft metrics based on events."""
        self.legitimacy_system.update_from_events(events)
        
        for event in events:
            if 'happiness_impact' in event:
                self.happiness_metrics.add_change(event['happiness_impact'])
    
    def get_fuzzy_assessment(self) -> Dict[str, str]:
        """Get fuzzy assessment of all soft metrics."""
        happiness_level = self.fuzzy_variables['happiness'].fuzzify(
            self.happiness_metrics.overall_happiness)
        legitimacy_level = self.fuzzy_variables['legitimacy'].fuzzify(
            self.legitimacy_system.calculate_legitimacy_score())
        
        return {
            'happiness': happiness_level,
            'legitimacy': legitimacy_level,
            'prestige': self.prestige_system.calculate_prestige()
        }
    
    def to_dict(self) -> Dict:
        """Serialize soft metrics to dictionary."""
        return {
            'happiness': self.happiness_metrics.overall_happiness,
            'legitimacy': self.legitimacy_system.legitimacy,
            'prestige': self.prestige_system.calculate_prestige(),
        }