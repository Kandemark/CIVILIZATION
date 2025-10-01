"""Revolutions and civil wars"""

# Module implementation

# politics/political_crises.py
from enum import Enum, auto
from typing import Dict, List, Optional, Set
from dataclasses import dataclass, field
import numpy as np
from datetime import datetime, timedelta
import logging

logger = logging.getLogger(__name__)

class CrisisType(Enum):
    """Types of political crises"""
    CONSTITUTIONAL = auto()
    SUCCESSION = auto()
    LEGITIMACY = auto()
    ECONOMIC = auto()
    MILITARY = auto()
    DIPLOMATIC = auto()
    SOCIAL_UNREST = auto()
    NATURAL_DISASTER = auto()

class CrisisSeverity(Enum):
    """Severity levels of political crises"""
    MINOR = 0.2
    MODERATE = 0.4
    SERIOUS = 0.6
    SEVERE = 0.8
    CATASTROPHIC = 1.0

@dataclass
class PoliticalCrisis:
    """Represents a political crisis"""
    id: str
    type: CrisisType
    severity: CrisisSeverity
    start_date: datetime
    duration: timedelta
    affected_factions: Set[str]
    description: str
    resolution_actions: List[str] = field(default_factory=list)
    resolved: bool = False
    resolution_date: Optional[datetime] = None

class CrisisSystem:
    """Manages political crises and their resolution"""
    
    def __init__(self):
        self.active_crises: Dict[str, PoliticalCrisis] = {}
        self.resolved_crises: List[PoliticalCrisis] = []
        self.crisis_risk_factors: Dict[CrisisType, float] = {
            crisis_type: 0.1 for crisis_type in CrisisType
        }
    
    def trigger_crisis(self, crisis_type: CrisisType, severity: CrisisSeverity, description: str, duration_days: int = 30) -> Optional[PoliticalCrisis]:
        """Trigger a new political crisis"""
        crisis_id = f"crisis_{len(self.active_crises) + len(self.resolved_crises) + 1}"
        
        crisis = PoliticalCrisis(
            id=crisis_id,
            type=crisis_type,
            severity=severity,
            start_date=datetime.now(),
            duration=timedelta(days=duration_days),
            affected_factions=set(),
            description=description
        )
        
        self.active_crises[crisis_id] = crisis
        logger.warning(f"Political crisis triggered: {description} (Severity: {severity.name})")
        return crisis
    
    def resolve_crisis(self, crisis_id: str, success: bool = True) -> bool:
        """Resolve a political crisis"""
        if crisis_id not in self.active_crises:
            return False
        
        crisis = self.active_crises[crisis_id]
        crisis.resolved = True
        crisis.resolution_date = datetime.now()
        
        self.resolved_crises.append(crisis)
        del self.active_crises[crisis_id]
        
        outcome = "successfully" if success else "unsuccessfully"
        logger.info(f"Crisis {crisis_id} resolved {outcome}: {crisis.description}")
        return True
    
    def update_crises(self, time_delta: float = 1.0):
        """Update all active crises"""
        current_time = datetime.now()
        crises_to_resolve = []
        
        for crisis_id, crisis in self.active_crises.items():
            # Check if crisis has expired
            if current_time - crisis.start_date >= crisis.duration:
                crises_to_resolve.append(crisis_id)
            else:
                # Crisis intensifies over time if not addressed
                pass
        
        # Resolve expired crises (usually unsuccessfully)
        for crisis_id in crises_to_resolve:
            self.resolve_crisis(crisis_id, success=False)
    
    def calculate_crisis_risk(self, legitimacy: float, faction_stability: float, economic_health: float) -> Dict[CrisisType, float]:
        """Calculate risk of different crisis types"""
        risks = {}
        
        # Base risks modified by various factors
        for crisis_type, base_risk in self.crisis_risk_factors.items():
            risk = base_risk
            
            # Modify based on legitimacy
            if crisis_type in [CrisisType.LEGITIMACY, CrisisType.CONSTITUTIONAL]:
                risk *= (1.0 - legitimacy) * 2.0
            
            # Modify based on faction stability
            if crisis_type in [CrisisType.SOCIAL_UNREST, CrisisType.SUCCESSION]:
                risk *= (1.0 - faction_stability) * 1.5
            
            # Modify based on economic health
            if crisis_type == CrisisType.ECONOMIC:
                risk *= (1.0 - economic_health) * 3.0
            
            risks[crisis_type] = min(1.0, risk)
        
        return risks
    
    def add_resolution_action(self, crisis_id: str, action: str) -> bool:
        """Add a resolution action to a crisis"""
        if crisis_id not in self.active_crises:
            return False
        
        self.active_crises[crisis_id].resolution_actions.append(action)
        return True
    
    def get_crisis_impact(self, crisis_id: str) -> Dict[str, float]:
        """Calculate the impact of a crisis"""
        if crisis_id not in self.active_crises:
            return {}
        
        crisis = self.active_crises[crisis_id]
        severity_value = crisis.severity.value
        
        impact = {
            'stability': -severity_value * 0.3,
            'economy': -severity_value * 0.2 if crisis.type == CrisisType.ECONOMIC else -severity_value * 0.1,
            'legitimacy': -severity_value * 0.4 if crisis.type == CrisisType.LEGITIMACY else -severity_value * 0.2,
            'military': -severity_value * 0.3 if crisis.type == CrisisType.MILITARY else 0.0
        }
        
        return impact
    
    def trigger_economic_crisis(self, severity: CrisisSeverity, cause: str = "Market collapse"):
        """Trigger an economic crisis"""
        return self.trigger_crisis(
            CrisisType.ECONOMIC,
            severity,
            f"Economic Crisis: {cause}",
            duration_days=90
        )
    
    def trigger_social_unrest(self, severity: CrisisSeverity, cause: str = "Popular discontent"):
        """Trigger social unrest"""
        return self.trigger_crisis(
            CrisisType.SOCIAL_UNREST,
            severity,
            f"Social Unrest: {cause}",
            duration_days=60
        )