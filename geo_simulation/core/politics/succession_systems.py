"""Leadership transition methods"""

# Module implementation

"""
Succession Systems Module for Civilization Simulation

This module handles the complex processes of power transfer in various government systems.
It models different succession types, handles crises, and simulates realistic outcomes
based on historical precedents and game mechanics.

Features:
- Multiple succession systems (hereditary, elective, meritocratic, etc.)
- Succession crisis simulation and resolution
- Candidate evaluation with multiple factors
- Historical tracking and statistical analysis
- Integration with character and faction systems
- Dynamic crisis resolution with multiple outcomes
- Support for regency periods and minority rulers
"""

from enum import Enum, auto
from typing import Dict, List, Optional, Set, Tuple, Any, Callable, Union
from dataclasses import dataclass, field
import numpy as np
from datetime import datetime, timedelta
import logging
import random
from collections import defaultdict
import math

logger = logging.getLogger(__name__)


class SuccessionType(Enum):
    """Types of succession systems used in different government forms."""
    HEREDITARY_MONARCHY = auto()      # Traditional bloodline inheritance
    ELECTIVE_MONARCHY = auto()        # Ruler elected by nobility
    PRIMOGENITURE = auto()            # First-born child inherits
    TANISTRY = auto()                 # Celtic system - elected from extended family
    MERITOCRATIC = auto()             # Based on qualifications and achievements
    MILITARY_STRENGTH = auto()        # Strongest military leader takes power
    POPULAR_ACCLAIM = auto()          # Chosen by popular support
    DIVINE_SELECTION = auto()         # Religious or supernatural selection
    COUNCIL_APPOINTMENT = auto()      # Selected by ruling council
    LOTTERY = auto()                  # Random selection from eligible candidates
    DUEL = auto()                     # Combat decides the ruler
    APPOINTED_SUCCESSOR = auto()      # Current ruler appoints successor
    SENIORITY = auto()                # Eldest family member inherits


class SuccessionCrisis(Enum):
    """Types of succession crises that can occur."""
    MULTIPLE_CLAIMANTS = auto()       # Several strong claimants dispute succession
    FOREIGN_INTERVENTION = auto()     # External powers influence the process
    CIVIL_WAR = auto()                # Open conflict between factions
    REGENCY_CONFLICT = auto()         # Struggle during regency period
    LEGITIMACY_DISPUTE = auto()       # Questioning of rightful heir
    ASSASSINATION = auto()            # Key figures are eliminated
    COUP_DETAT = auto()               # Military or faction seizes power
    RELIGIOUS_SCHISM = auto()         # Religious differences cause conflict
    ECONOMIC_COLLAPSE = auto()        # Economic instability undermines process
    SUCCESSION_VACUUM = auto()        # No clear heir or candidate
    MINORITY_RULE_CRISIS = auto()     # Child ruler with disputed regency


class CrisisSeverity(Enum):
    """Severity levels for succession crises."""
    MINOR = auto()    # Easily resolved, minimal impact
    MODERATE = auto() # Significant but manageable disruption
    MAJOR = auto()    # Serious threat to stability
    CATASTROPHIC = auto() # Risk of complete collapse


class SuccessionOutcome(Enum):
    """Possible outcomes of a succession process."""
    STABLE_TRANSITION = auto()
    UNSTABLE_TRANSITION = auto()
    CIVIL_WAR = auto()
    FOREIGN_INTERVENTION = auto()
    DYNASTIC_CHANGE = auto()
    GOVERNMENT_CHANGE = auto()
    REALM_FRAGMENTATION = auto()


@dataclass
class SuccessionCandidate:
    """A candidate in a succession process with various attributes."""
    id: str
    name: str
    claim_strength: float = 0.0  # 0.0 to 1.0 (legal/hereditary claim)
    support: float = 0.0         # 0.0 to 1.0 (faction/popular support)
    qualifications: float = 0.0  # 0.0 to 1.0 (skills/experience)
    military_power: float = 0.0  # 0.0 to 1.0 (military strength)
    wealth: float = 0.0          # 0.0 to 1.0 (economic resources)
    traits: Set[str] = field(default_factory=set)
    alliances: Set[str] = field(default_factory=set)
    rivals: Set[str] = field(default_factory=set)
    age: int = 30
    health: float = 1.0          # 0.0 to 1.0
    legitimacy: float = 1.0      # 0.0 to 1.0 (perceived right to rule)
    faction_id: Optional[str] = None
    dynasty_id: Optional[str] = None
    religion: Optional[str] = None
    culture: Optional[str] = None


@dataclass
class Regency:
    """Information about a regency period for child rulers."""
    regent_id: str
    start_date: datetime
    expected_end_date: datetime
    council_members: List[str] = field(default_factory=list)
    power_struggle_level: float = 0.0  # 0.0 to 1.0
    is_contested: bool = False


@dataclass
class SuccessionProcess:
    """A complete process of transferring power between rulers."""
    id: str
    type: SuccessionType
    candidates: Dict[str, SuccessionCandidate]
    start_date: datetime
    duration_days: int
    resolved: bool = False
    winner: Optional[str] = None
    crisis: Optional[SuccessionCrisis] = None
    crisis_severity: CrisisSeverity = CrisisSeverity.MINOR
    resolution_method: Optional[str] = None
    stability_impact: float = 0.0  # -1.0 to 1.0 impact on realm stability
    historical_significance: float = 0.0  # 0.0 to 1.0
    notes: List[str] = field(default_factory=list)
    outcome: Optional[SuccessionOutcome] = None
    regency: Optional[Regency] = None
    faction_changes: Dict[str, float] = field(default_factory=dict)  # Faction power changes
    territory_changes: Dict[str, List[str]] = field(default_factory=dict)  # Territory gains/losses


@dataclass
class SuccessionHistory:
    """Historical record of succession events."""
    total_successions: int = 0
    successful_transitions: int = 0
    crises_occurred: int = 0
    average_duration_days: float = 0.0
    most_common_type: Optional[SuccessionType] = None
    last_succession: Optional[datetime] = None
    civil_wars: int = 0
    dynastic_changes: int = 0
    average_stability_impact: float = 0.0
    succession_outcomes: Dict[SuccessionOutcome, int] = field(default_factory=dict)


class SuccessionSystem:
    """
    Manages power succession processes for civilizations and factions.
    
    Handles the complex mechanics of transferring power between rulers,
    including crisis management, candidate evaluation, and historical tracking.
    """
    
    def __init__(self, realm_id: str, realm_name: str):
        self.realm_id = realm_id
        self.realm_name = realm_name
        self.active_successions: Dict[str, SuccessionProcess] = {}
        self.completed_successions: List[SuccessionProcess] = []
        self.succession_type = SuccessionType.HEREDITARY_MONARCHY
        self.base_crisis_risk: float = 0.15
        self.history = SuccessionHistory()
        self.crisis_handlers = self._initialize_crisis_handlers()
        self.realm_stability: float = 0.8  # Would integrate with realm state
        self.economic_health: float = 0.7  # Would integrate with economy system
        self.military_strength: float = 0.6  # Would integrate with military system
        
        logger.info(f"Succession system initialized for {realm_name}")
    
    def _initialize_crisis_handlers(self) -> Dict[SuccessionCrisis, Callable]:
        """Initialize handlers for different crisis types."""
        return {
            SuccessionCrisis.MULTIPLE_CLAIMANTS: self._handle_multiple_claimants,
            SuccessionCrisis.FOREIGN_INTERVENTION: self._handle_foreign_intervention,
            SuccessionCrisis.CIVIL_WAR: self._handle_civil_war,
            SuccessionCrisis.REGENCY_CONFLICT: self._handle_regency_conflict,
            SuccessionCrisis.LEGITIMACY_DISPUTE: self._handle_legitimacy_dispute,
            SuccessionCrisis.ASSASSINATION: self._handle_assassination,
            SuccessionCrisis.COUP_DETAT: self._handle_coup_detat,
            SuccessionCrisis.RELIGIOUS_SCHISM: self._handle_religious_schism,
            SuccessionCrisis.ECONOMIC_COLLAPSE: self._handle_economic_collapse,
            SuccessionCrisis.SUCCESSION_VACUUM: self._handle_succession_vacuum,
            SuccessionCrisis.MINORITY_RULE_CRISIS: self._handle_minority_rule_crisis,
        }
    
    def initialize_succession(self, succession_type: SuccessionType, 
                            candidates: Dict[str, SuccessionCandidate],
                            duration_days: int = 30,
                            previous_ruler_id: Optional[str] = None) -> SuccessionProcess:
        """
        Initialize a new succession process.
        
        Args:
            succession_type: The type of succession system to use
            candidates: Dictionary of candidates participating
            duration_days: Expected duration of the process
            previous_ruler_id: ID of the previous ruler (for hereditary systems)
            
        Returns:
            The created SuccessionProcess object
        """
        succession_id = f"{self.realm_id}_succession_{len(self.active_successions) + len(self.completed_successions) + 1}"
        
        succession = SuccessionProcess(
            id=succession_id,
            type=succession_type,
            candidates=candidates,
            start_date=datetime.now(),
            duration_days=duration_days
        )
        
        # Adjust candidate claims for hereditary systems
        if previous_ruler_id and succession_type in [SuccessionType.HEREDITARY_MONARCHY, 
                                                   SuccessionType.PRIMOGENITURE]:
            self._adjust_hereditary_claims(succession, previous_ruler_id)
        
        # Check for potential crisis
        crisis_risk = self._calculate_crisis_risk(succession)
        if random.random() < crisis_risk:
            succession.crisis = self._determine_crisis_type(succession)
            succession.crisis_severity = self._determine_crisis_severity(succession)
            logger.warning(f"Succession crisis detected: {succession.crisis.name}")
        
        # Check for potential regency (child ruler)
        if any(candidate.age < 16 for candidate in succession.candidates.values()):
            succession.notes.append("Minor candidate detected - regency may be required")
        
        self.active_successions[succession_id] = succession
        self.history.total_successions += 1
        
        logger.info(f"Succession process started: {succession_type.name} with {len(candidates)} candidates")
        return succession
    
    def _adjust_hereditary_claims(self, succession: SuccessionProcess, previous_ruler_id: str) -> None:
        """Adjust claim strengths based on relationship to previous ruler."""
        # This would integrate with character relationship system
        for candidate in succession.candidates.values():
            # Placeholder: candidates from same dynasty get claim boost
            if candidate.dynasty_id and f"dynasty_{previous_ruler_id}" == candidate.dynasty_id:
                candidate.claim_strength = min(1.0, candidate.claim_strength + 0.3)
                candidate.legitimacy = min(1.0, candidate.legitimacy + 0.2)
    
    def _calculate_crisis_risk(self, succession: SuccessionProcess) -> float:
        """Calculate the risk of a succession crisis occurring."""
        base_risk = self.base_crisis_risk
        
        # Factors increasing crisis risk
        if len(succession.candidates) > 3:
            base_risk += 0.2
        
        # Check for strong rival candidates
        strong_candidates = [c for c in succession.candidates.values() 
                           if c.support > 0.7 or c.military_power > 0.7]
        if len(strong_candidates) > 1:
            base_risk += 0.3
        
        # Realm stability factors
        base_risk += (1 - self.realm_stability) * 0.4
        
        # Economic factors
        base_risk += (1 - self.economic_health) * 0.3
        
        # Military factors - weak military increases crisis risk
        base_risk += (1 - self.military_strength) * 0.2
        
        # Succession type modifiers
        type_modifiers = {
            SuccessionType.HEREDITARY_MONARCHY: -0.1,
            SuccessionType.ELECTIVE_MONARCHY: 0.1,
            SuccessionType.MILITARY_STRENGTH: 0.25,
            SuccessionType.LOTTERY: 0.4,
            SuccessionType.DUEL: 0.35,
            SuccessionType.PRIMOGENITURE: -0.05,
            SuccessionType.SENIORITY: 0.05,
        }
        
        base_risk += type_modifiers.get(succession.type, 0.0)
        
        # Cultural/religious homogeneity (placeholder)
        cultures = set(c.culture for c in succession.candidates.values() if c.culture)
        religions = set(c.religion for c in succession.candidates.values() if c.religion)
        
        if len(cultures) > 1:
            base_risk += 0.15
        if len(religions) > 1:
            base_risk += 0.2
        
        return min(0.95, max(0.05, base_risk))  # Cap between 5% and 95%
    
    def _determine_crisis_type(self, succession: SuccessionProcess) -> SuccessionCrisis:
        """Determine the type of crisis that occurs."""
        crisis_weights = {
            SuccessionCrisis.MULTIPLE_CLAIMANTS: 0.3,
            SuccessionCrisis.CIVIL_WAR: 0.2,
            SuccessionCrisis.LEGITIMACY_DISPUTE: 0.15,
            SuccessionCrisis.ASSASSINATION: 0.1,
            SuccessionCrisis.COUP_DETAT: 0.1,
            SuccessionCrisis.FOREIGN_INTERVENTION: 0.05,
            SuccessionCrisis.REGENCY_CONFLICT: 0.05,
            SuccessionCrisis.RELIGIOUS_SCHISM: 0.03,
            SuccessionCrisis.ECONOMIC_COLLAPSE: 0.01,
            SuccessionCrisis.SUCCESSION_VACUUM: 0.01,
        }
        
        # Adjust weights based on situation
        if len(succession.candidates) > 4:
            crisis_weights[SuccessionCrisis.MULTIPLE_CLAIMANTS] += 0.2
        
        # Check for child candidates
        if any(c.age < 16 for c in succession.candidates.values()):
            crisis_weights[SuccessionCrisis.MINORITY_RULE_CRISIS] = 0.25
            crisis_weights[SuccessionCrisis.REGENCY_CONFLICT] += 0.15
        
        # Economic factors
        if self.economic_health < 0.4:
            crisis_weights[SuccessionCrisis.ECONOMIC_COLLAPSE] += 0.2
        
        crises = list(crisis_weights.keys())
        weights = list(crisis_weights.values())
        
        return random.choices(crises, weights=weights, k=1)[0]
    
    def _determine_crisis_severity(self, succession: SuccessionProcess) -> CrisisSeverity:
        """Determine the severity of a crisis."""
        severity_weights = {
            CrisisSeverity.MINOR: 0.4,
            CrisisSeverity.MODERATE: 0.3,
            CrisisSeverity.MAJOR: 0.2,
            CrisisSeverity.CATASTROPHIC: 0.1,
        }
        
        # More candidates → more severe crisis
        candidate_factor = min(1.0, len(succession.candidates) / 8)
        severity_weights[CrisisSeverity.MINOR] -= candidate_factor * 0.2
        severity_weights[CrisisSeverity.CATASTROPHIC] += candidate_factor * 0.1
        
        # Realm stability affects severity
        stability_factor = 1 - self.realm_stability
        severity_weights[CrisisSeverity.MINOR] -= stability_factor * 0.3
        severity_weights[CrisisSeverity.MAJOR] += stability_factor * 0.2
        severity_weights[CrisisSeverity.CATASTROPHIC] += stability_factor * 0.1
        
        severities = list(severity_weights.keys())
        weights = list(severity_weights.values())
        
        return random.choices(severities, weights=weights, k=1)[0]
    
    def resolve_succession(self, succession_id: str, winner_id: Optional[str] = None, 
                         resolution_method: Optional[str] = None) -> bool:
        """
        Resolve a succession process, either with a winner or through crisis resolution.
        
        Args:
            succession_id: ID of the succession process to resolve
            winner_id: Optional winner ID (if None, will be determined)
            resolution_method: How the succession was resolved
            
        Returns:
            True if successful, False otherwise
        """
        if succession_id not in self.active_successions:
            return False
        
        succession = self.active_successions[succession_id]
        
        # Handle crisis if present
        if succession.crisis:
            success = self._handle_crisis(succession)
            if not success:
                logger.error(f"Failed to resolve crisis in succession {succession_id}")
                return False
        
        # Determine winner if not specified
        if winner_id is None:
            winner_id = self.simulate_succession_outcome(succession_id)
            if winner_id is None:
                logger.error(f"Could not determine winner for succession {succession_id}")
                return False
        
        if winner_id not in succession.candidates:
            return False
        
        # Check if winner is a minor and establish regency if needed
        winner = succession.candidates[winner_id]
        if winner.age < 16:
            succession.regency = self._establish_regency(winner_id, succession)
            succession.notes.append(f"Regency established for minor ruler {winner.name}")
        
        # Determine succession outcome
        succession.outcome = self._determine_succession_outcome(succession, winner_id)
        
        # Finalize the succession
        succession.resolved = True
        succession.winner = winner_id
        succession.resolution_method = resolution_method or "simulated_outcome"
        succession.duration_days = (datetime.now() - succession.start_date).days
        
        # Calculate impact on realm stability and make faction/territory changes
        succession.stability_impact = self._calculate_stability_impact(succession)
        self._apply_succession_consequences(succession)
        
        self.completed_successions.append(succession)
        del self.active_successions[succession_id]
        
        # Update historical records
        self.history.successful_transitions += 1
        self.history.last_succession = datetime.now()
        if succession.crisis:
            self.history.crises_occurred += 1
        
        if succession.outcome == SuccessionOutcome.CIVIL_WAR:
            self.history.civil_wars += 1
        if any(c.dynasty_id != winner.dynasty_id for c in succession.candidates.values() 
               if c.id != winner_id and hasattr(c, 'dynasty_id')):
            self.history.dynastic_changes += 1
        
        # Update outcome statistics
        if succession.outcome not in self.history.succession_outcomes:
            self.history.succession_outcomes[succession.outcome] = 0
        self.history.succession_outcomes[succession.outcome] += 1
        
        logger.info(f"Succession resolved: {winner.name} becomes new ruler")
        return True
    
    def _establish_regency(self, ruler_id: str, succession: SuccessionProcess) -> Regency:
        """Establish a regency for a minor ruler."""
        # Select regent from among powerful candidates who didn't win
        potential_regents = [c for c in succession.candidates.values() 
                           if c.id != ruler_id and c.age >= 25 and c.qualifications > 0.5]
        
        if potential_regents:
            # Choose most qualified regent
            regent = max(potential_regents, key=lambda x: x.qualifications)
        else:
            # Create a default regent (royal relative or official)
            regent = SuccessionCandidate(
                id=f"regent_{random.randint(1000, 9999)}",
                name="Royal Regent",
                qualifications=0.7,
                support=0.5,
                claim_strength=0.3
            )
        
        # Calculate regency duration (until ruler comes of age)
        ruler_age = succession.candidates[ruler_id].age
        years_until_majority = 16 - ruler_age
        end_date = datetime.now() + timedelta(days=365 * years_until_majority)
        
        # Determine if regency will be contested
        power_struggle = 0.0
        if len([c for c in succession.candidates.values() if c.military_power > 0.6]) > 1:
            power_struggle = 0.7
        
        return Regency(
            regent_id=regent.id,
            start_date=datetime.now(),
            expected_end_date=end_date,
            power_struggle_level=power_struggle,
            is_contested=power_struggle > 0.5
        )
    
    def _determine_succession_outcome(self, succession: SuccessionProcess, winner_id: str) -> SuccessionOutcome:
        """Determine the overall outcome of the succession process."""
        winner = succession.candidates[winner_id]
        
        if succession.crisis_severity == CrisisSeverity.CATASTROPHIC:
            return SuccessionOutcome.REALM_FRAGMENTATION
        elif succession.crisis_severity == CrisisSeverity.MAJOR:
            if succession.crisis == SuccessionCrisis.CIVIL_WAR:
                return SuccessionOutcome.CIVIL_WAR
            elif succession.crisis == SuccessionCrisis.FOREIGN_INTERVENTION:
                return SuccessionOutcome.FOREIGN_INTERVENTION
        
        # Check for dynastic change
        all_dynasties = set(c.dynasty_id for c in succession.candidates.values() if c.dynasty_id)
        if len(all_dynasties) > 1 and random.random() < 0.4:
            return SuccessionOutcome.DYNASTIC_CHANGE
        
        # Stable outcomes
        if succession.stability_impact > 0.3:
            return SuccessionOutcome.STABLE_TRANSITION
        else:
            return SuccessionOutcome.UNSTABLE_TRANSITION
    
    def _apply_succession_consequences(self, succession: SuccessionProcess) -> None:
        """Apply consequences of succession to realm factions and territories."""
        winner = succession.candidates[succession.winner]
        
        # Simulate faction power changes based on winner's support
        for candidate in succession.candidates.values():
            if candidate.faction_id and candidate.id != succession.winner:
                # Factions that supported losers lose influence
                support_diff = winner.support - candidate.support
                power_change = -0.2 * support_diff
                
                if candidate.faction_id not in succession.faction_changes:
                    succession.faction_changes[candidate.faction_id] = 0
                succession.faction_changes[candidate.faction_id] += power_change
        
        # Winner's faction gains power
        if winner.faction_id:
            if winner.faction_id not in succession.faction_changes:
                succession.faction_changes[winner.faction_id] = 0
            succession.faction_changes[winner.faction_id] += 0.3
        
        # Simulate potential territory changes in major crises
        if succession.crisis_severity in [CrisisSeverity.MAJOR, CrisisSeverity.CATASTROPHIC]:
            lost_territories = random.sample([f"territory_{i}" for i in range(5)], 
                                           k=random.randint(1, 3))
            succession.territory_changes["lost"] = lost_territories
            succession.notes.append(f"Lost territories during crisis: {', '.join(lost_territories)}")
    
    def _handle_crisis(self, succession: SuccessionProcess) -> bool:
        """Handle a succession crisis using the appropriate handler."""
        if succession.crisis not in self.crisis_handlers:
            logger.error(f"No handler for crisis type: {succession.crisis}")
            return False
        
        handler = self.crisis_handlers[succession.crisis]
        return handler(succession)
    
    def _handle_multiple_claimants(self, succession: SuccessionProcess) -> bool:
        """Handle multiple claimants crisis through negotiation or conflict."""
        # Strongest candidates negotiate or fight
        strong_candidates = sorted(
            succession.candidates.values(),
            key=lambda x: (x.support + x.military_power) / 2,
            reverse=True
        )[:3]
        
        # 60% chance of peaceful resolution through deal-making
        if random.random() < 0.6:
            # Top candidate makes concessions to others
            succession.resolution_method = "negotiated_settlement"
            succession.notes.append("Crisis resolved through political negotiations")
            
            # Losers get compensation (titles, wealth, etc.)
            for candidate in strong_candidates[1:]:
                if random.random() < 0.7:
                    succession.notes.append(f"Candidate {candidate.name} received compensation")
            
            return True
        else:
            # Leads to civil war or other conflict
            succession.crisis_severity = CrisisSeverity(max(succession.crisis_severity.value, CrisisSeverity.MAJOR.value))
            succession.notes.append("Multiple claimants led to armed conflict")
            return self._handle_civil_war(succession)
    
    def _handle_civil_war(self, succession: SuccessionProcess) -> bool:
        """Handle civil war crisis - military resolution."""
        # Candidates with military power fight it out
        military_candidates = [c for c in succession.candidates.values() if c.military_power > 0.4]
        
        if not military_candidates:
            return False
        
        # Determine winner by military strength with some randomness
        weights = [c.military_power ** 2 + 0.1 for c in military_candidates]
        winner = random.choices(military_candidates, weights=weights, k=1)[0]
        
        # Find winner ID
        for cand_id, candidate in succession.candidates.items():
            if candidate.id == winner.id:
                succession.winner = cand_id
                break
        
        succession.resolution_method = "military_victory"
        succession.notes.append("Civil war resolved by military victory")
        succession.stability_impact -= 0.5  # Major stability hit
        
        # Some candidates may be killed or exiled
        casualties = random.sample([c for c in military_candidates if c.id != winner.id], 
                                 k=random.randint(1, len(military_candidates) - 1))
        for casualty in casualties:
            succession.notes.append(f"Candidate {casualty.name} defeated in civil war")
            if random.random() < 0.6:  # 60% chance of death
                del succession.candidates[casualty.id]
                succession.notes.append(f"Candidate {casualty.name} killed in battle")
        
        return True
    
    def _handle_foreign_intervention(self, succession: SuccessionProcess) -> bool:
        """Handle foreign intervention in succession."""
        # Foreign powers support candidates who align with their interests
        foreign_supported = [c for c in succession.candidates.values() 
                           if random.random() < 0.4]  # 40% chance of foreign support
        
        if foreign_supported:
            # Foreign-supported candidates get boost
            for candidate in foreign_supported:
                candidate.support += 0.2
                candidate.military_power += 0.1
                succession.notes.append(f"Candidate {candidate.name} received foreign support")
        
        succession.resolution_method = "foreign_mediated"
        succession.notes.append("Foreign powers influenced the succession")
        succession.stability_impact -= 0.3
        
        # Pick candidate with most support (may be foreign-supported)
        winner_id = max(succession.candidates.items(), 
                       key=lambda x: x[1].support)[0]
        succession.winner = winner_id
        
        return True
    
    def _handle_regency_conflict(self, succession: SuccessionProcess) -> bool:
        """Handle regency conflict crisis."""
        # Find minor candidates
        minor_candidates = [c for c in succession.candidates.values() if c.age < 16]
        
        if not minor_candidates:
            return False
        
        # Establish contested regency
        minor_candidate = random.choice(minor_candidates)
        succession.winner = minor_candidate.id
        
        succession.regency = Regency(
            regent_id=f"regent_{random.randint(1000, 9999)}",
            start_date=datetime.now(),
            expected_end_date=datetime.now() + timedelta(days=365 * (16 - minor_candidate.age)),
            power_struggle_level=0.8,
            is_contested=True
        )
        
        succession.resolution_method = "contested_regency"
        succession.notes.append("Regency conflict established with high tension")
        succession.stability_impact -= 0.4
        
        return True
    
    def _handle_legitimacy_dispute(self, succession: SuccessionProcess) -> bool:
        """Handle legitimacy dispute crisis."""
        # Resolve through legal or religious means
        if random.random() < 0.7:
            succession.resolution_method = "legal_settlement"
            succession.notes.append("Legitimacy dispute resolved legally")
            
            # Pick candidate with strongest claim
            winner_id = max(succession.candidates.items(), 
                           key=lambda x: x[1].claim_strength)[0]
            succession.winner = winner_id
        else:
            succession.resolution_method = "religious_decree"
            succession.notes.append("Legitimacy dispute resolved by religious authority")
            
            # Pick candidate with highest legitimacy (divine favor)
            winner_id = max(succession.candidates.items(), 
                           key=lambda x: x[1].legitimacy)[0]
            succession.winner = winner_id
        
        succession.stability_impact -= 0.2
        return True
    
    def _handle_assassination(self, succession: SuccessionProcess) -> bool:
        """Handle assassination crisis."""
        # Remove some candidates through assassination
        candidates_to_remove = random.randint(1, min(3, len(succession.candidates) - 1))
        removed_candidates = []
        
        for _ in range(candidates_to_remove):
            if len(succession.candidates) <= 1:
                break
            
            # Weight assassination probability by candidate strength
            candidates = list(succession.candidates.values())
            weights = [c.support + 0.1 for c in candidates]
            victim = random.choices(candidates, weights=weights, k=1)[0]
            
            # Remove candidate
            for cand_id, candidate in list(succession.candidates.items()):
                if candidate.id == victim.id:
                    del succession.candidates[cand_id]
                    removed_candidates.append(candidate.name)
                    break
        
        succession.resolution_method = "assassination_elimination"
        succession.notes.append(f"Candidates assassinated: {', '.join(removed_candidates)}")
        succession.stability_impact -= 0.4
        
        return len(succession.candidates) > 0
    
    def _handle_coup_detat(self, succession: SuccessionProcess) -> bool:
        """Handle coup d'etat crisis."""
        # Military candidate takes power by force
        military_candidates = [c for c in succession.candidates.values() if c.military_power > 0.6]
        
        if military_candidates:
            winner = max(military_candidates, key=lambda x: x.military_power)
            for cand_id, candidate in succession.candidates.items():
                if candidate.id == winner.id:
                    succession.winner = cand_id
                    break
        else:
            # Create a new military candidate (general seizing power)
            new_candidate = SuccessionCandidate(
                id=f"coup_leader_{random.randint(1000, 9999)}",
                name="Military Leader",
                military_power=0.9,
                support=0.4,
                qualifications=0.6,
                claim_strength=0.1
            )
            succession.candidates[new_candidate.id] = new_candidate
            succession.winner = new_candidate.id
        
        succession.resolution_method = "coup_detat"
        succession.notes.append("Military coup resolved the succession")
        succession.stability_impact -= 0.6
        
        # Remove political opponents
        for candidate in list(succession.candidates.values()):
            if candidate.id != succession.winner and random.random() < 0.5:
                del succession.candidates[candidate.id]
                succession.notes.append(f"Opponent {candidate.name} eliminated after coup")
        
        return True
    
    def _handle_religious_schism(self, succession: SuccessionProcess) -> bool:
        """Handle religious schism crisis."""
        # Group candidates by religion
        religious_groups = defaultdict(list)
        for candidate in succession.candidates.values():
            if candidate.religion:
                religious_groups[candidate.religion].append(candidate)
        
        if len(religious_groups) > 1:
            # Religious conflict - strongest religious group wins
            largest_group = max(religious_groups.values(), key=len)
            winner = random.choice(largest_group)
            
            for cand_id, candidate in succession.candidates.items():
                if candidate.id == winner.id:
                    succession.winner = cand_id
                    break
            
            succession.resolution_method = "religious_domination"
            succession.notes.append("Religious schism resolved by dominance of largest faith")
        else:
            # Internal religious dispute
            winner_id = max(succession.candidates.items(), 
                           key=lambda x: x[1].legitimacy)[0]
            succession.winner = winner_id
            succession.resolution_method = "theological_resolution"
            succession.notes.append("Religious dispute resolved through theological debate")
        
        succession.stability_impact -= 0.3
        return True
    
    def _handle_economic_collapse(self, succession: SuccessionProcess) -> bool:
        """Handle economic collapse crisis."""
        # Wealthy candidate buys support or mercenaries
        wealthy_candidates = [c for c in succession.candidates.values() if c.wealth > 0.7]
        
        if wealthy_candidates:
            winner = max(wealthy_candidates, key=lambda x: x.wealth)
            for cand_id, candidate in succession.candidates.items():
                if candidate.id == winner.id:
                    succession.winner = cand_id
                    break
        else:
            # Random selection due to chaos
            winner_id = random.choice(list(succession.candidates.keys()))
            succession.winner = winner_id
        
        succession.resolution_method = "economic_resolution"
        succession.notes.append("Economic collapse influenced succession outcome")
        succession.stability_impact -= 0.7
        
        return True
    
    def _handle_succession_vacuum(self, succession: SuccessionProcess) -> bool:
        """Handle succession vacuum crisis."""
        # No clear candidates - may need to find new ones
        if len(succession.candidates) == 0:
            # Create emergency candidate from nobility or military
            candidate_type = random.choice(["noble", "military", "religious"])
            if candidate_type == "noble":
                new_candidate = SuccessionCandidate(
                    id=f"emergency_noble_{random.randint(1000, 9999)}",
                    name="Emergency Noble",
                qualifications=0.6,
                support=0.5,
                claim_strength=0.4,
                wealth=0.7,
                age=random.randint(30, 60),
                dynasty_id=f"dynasty_{random.randint(100,999)}"
                )
            elif candidate_type == "military":
                new_candidate = SuccessionCandidate(
                    id=f"emergency_general_{random.randint(1000, 9999)}",
                    name="Emergency General",
                    military_power=0.8,
                    support=0.4,
                    qualifications=0.5,
                    claim_strength=0.2,
                    age=random.randint(35, 65)
                )
            else:
                new_candidate = SuccessionCandidate(
                    id=f"emergency_cleric_{random.randint(1000, 9999)}",
                    name="Emergency Cleric",
                    legitimacy=0.8,
                    support=0.3,
                    qualifications=0.7,
                    claim_strength=0.1,
                    religion="Faith of the Realm",
                    age=random.randint(40, 70)
                )
            succession.candidates[new_candidate.id] = new_candidate
            succession.notes.append(f"Emergency candidate {new_candidate.name} created due to vacuum")
            succession.winner = new_candidate.id
        else:
            # Pick candidate with highest legitimacy/support
            winner_id = max(succession.candidates.items(), key=lambda x: (x[1].legitimacy + x[1].support))[0]
            succession.winner = winner_id
        succession.resolution_method = "emergency_selection"
        succession.notes.append("Succession vacuum resolved by emergency appointment")
        succession.stability_impact -= 0.8
        return True

    def _handle_minority_rule_crisis(self, succession: SuccessionProcess) -> bool:
        """Handle crisis when a minor becomes ruler and regency is disputed."""
        minor_candidates = [c for c in succession.candidates.values() if c.age < 16]
        if not minor_candidates:
            return False
        minor = random.choice(minor_candidates)
        succession.winner = minor.id
        succession.regency = self._establish_regency(minor.id, succession)
        succession.regency.is_contested = True
        succession.regency.power_struggle_level = 0.9
        succession.notes.append(f"Minority rule crisis: regency for {minor.name} is highly contested")
        succession.resolution_method = "contested_regency"
        succession.stability_impact -= 0.5
        return True
    
    def _calculate_stability_impact(self, succession: SuccessionProcess) -> float:
        """Calculate the impact of the succession on realm stability."""
        base_impact = 0.0
        
        # Crisis severity impact
        crisis_impact = {
            CrisisSeverity.MINOR: -0.1,
            CrisisSeverity.MODERATE: -0.3,
            CrisisSeverity.MAJOR: -0.6,
            CrisisSeverity.CATASTROPHIC: -0.9,
        }
        
        if succession.crisis:
            base_impact += crisis_impact.get(succession.crisis_severity, 0)
        
        # Duration impact - longer successions are more destabilizing
        duration_factor = min(1.0, succession.duration_days / 365)  # Cap at 1 year
        base_impact -= duration_factor * 0.2
        
        # Winner legitimacy impact
        if succession.winner:
            winner = succession.candidates[succession.winner]
            base_impact += winner.legitimacy * 0.3 - 0.15
        
        # Regency impact
        if succession.regency:
            base_impact -= succession.regency.power_struggle_level * 0.4
        
        return max(-1.0, min(1.0, base_impact))  # Clamp between -1 and 1

    def simulate_succession_outcome(self, succession_id: str) -> Optional[str]:
        """Simulate the outcome of a succession and pick a winner."""
        if succession_id not in self.active_successions:
            return None
        succession = self.active_successions[succession_id]
        candidates = list(succession.candidates.values())
        if not candidates:
            return None

        def score(candidate: SuccessionCandidate) -> float:
            """Calculate a weighted score for a candidate based on succession type."""
            if succession.type == SuccessionType.HEREDITARY_MONARCHY:
                return self._simulate_hereditary_succession(succession, candidate)
            elif succession.type == SuccessionType.ELECTIVE_MONARCHY:
                return self._simulate_elective_succession(succession, candidate)
            elif succession.type == SuccessionType.PRIMOGENITURE:
                return self._simulate_primogeniture(succession, candidate)
            elif succession.type == SuccessionType.TANISTRY:
                return self._simulate_tanistry(succession, candidate)
            elif succession.type == SuccessionType.MERITOCRATIC:
                return self._simulate_meritocratic_succession(succession, candidate)
            elif succession.type == SuccessionType.MILITARY_STRENGTH:
                return self._simulate_military_succession(succession, candidate)
            elif succession.type == SuccessionType.POPULAR_ACCLAIM:
                return self._simulate_popular_succession(succession, candidate)
            elif succession.type == SuccessionType.DIVINE_SELECTION:
                return self._simulate_divine_succession(succession, candidate)
            elif succession.type == SuccessionType.COUNCIL_APPOINTMENT:
                return self._simulate_council_succession(succession, candidate)
            elif succession.type == SuccessionType.LOTTERY:
                return self._simulate_lottery_succession(succession, candidate)
            elif succession.type == SuccessionType.DUEL:
                return self._simulate_duel_succession(succession, candidate)
            elif succession.type == SuccessionType.SENIORITY:
                return self._simulate_seniority_succession(succession, candidate)
            elif succession.type == SuccessionType.APPOINTED_SUCCESSOR:
                return self._simulate_appointed_succession(succession, candidate)
            else:
                return self._simulate_general_succession(succession, candidate)

        # Compute noisy scores
        scores = [score(c) + random.uniform(-0.05, 0.05) for c in candidates]

        # Pick the candidate with the highest score
        winner = max(zip(candidates, scores), key=lambda x: x[1])[0]
        return winner.id


    def get_active_succession(self) -> Optional[SuccessionProcess]:
        """Return the currently active succession process, if any."""
        if self.active_successions:
            return next(iter(self.active_successions.values()))
        return None

    def get_history_summary(self) -> Dict[str, Any]:
        """Return a summary of succession history for the realm."""
        summary = {
            "total_successions": self.history.total_successions,
            "successful_transitions": self.history.successful_transitions,
            "crises_occurred": self.history.crises_occurred,
            "civil_wars": self.history.civil_wars,
            "dynastic_changes": self.history.dynastic_changes,
            "last_succession": self.history.last_succession,
            "succession_outcomes": {k.name: v for k, v in self.history.succession_outcomes.items()},
        }
        if self.completed_successions:
            durations = [s.duration_days for s in self.completed_successions]
            summary["average_duration_days"] = sum(durations) / len(durations)
            summary["average_stability_impact"] = sum(s.stability_impact for s in self.completed_successions) / len(self.completed_successions)
            types = [s.type for s in self.completed_successions]
            summary["most_common_type"] = max(set(types), key=types.count).name
        else:
            summary["average_duration_days"] = 0.0
            summary["average_stability_impact"] = 0.0
            summary["most_common_type"] = None
        return summary
    
    def _simulate_hereditary_succession(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate hereditary succession (strongest claim wins)."""
        best_candidate = None
        best_claim = -1.0
        
        for candidate_id, candidate in succession.candidates.items():
            # Consider both claim strength and legitimacy
            claim_score = candidate.claim_strength * 0.7 + candidate.legitimacy * 0.3
            if claim_score > best_claim:
                best_claim = claim_score
                best_candidate = candidate_id
        
        return best_candidate
    
    def _simulate_elective_succession(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate elective monarchy (weighted by support and qualifications)."""
        if not succession.candidates:
            return None
        
        candidates = list(succession.candidates.values())
        # Electors consider support, qualifications, and wealth
        weights = [
            (candidate.support * 0.5 + candidate.qualifications * 0.3 + candidate.wealth * 0.2) 
            for candidate in candidates
        ]
        
        # Add some randomness to simulate political uncertainty
        weights = [w * random.uniform(0.8, 1.2) for w in weights]
        
        winner = random.choices(candidates, weights=weights, k=1)[0]
        
        # Find the winner's ID
        for cand_id, candidate in succession.candidates.items():
            if candidate.id == winner.id:
                return cand_id
        
        return None
    
    def _simulate_primogeniture(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate primogeniture succession (eldest eligible candidate)."""
        # For simplicity, we'll use claim strength as proxy for birth order
        return self._simulate_hereditary_succession(succession)
    
    def _simulate_tanistry(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate tanistry (elected from extended family based on merit)."""
        if not succession.candidates:
            return None
        
        # Tanistry balances claim (family relation) with qualifications
        candidates = list(succession.candidates.values())
        weights = [
            (candidate.claim_strength * 0.4 + candidate.qualifications * 0.6)
            for candidate in candidates
        ]
        
        winner = random.choices(candidates, weights=weights, k=1)[0]
        
        for cand_id, candidate in succession.candidates.items():
            if candidate.id == winner.id:
                return cand_id
        
        return None
    
    def _simulate_meritocratic_succession(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate meritocratic succession (most qualified candidate wins)."""
        best_candidate = None
        best_qualifications = -1.0
        
        for candidate_id, candidate in succession.candidates.items():
            if candidate.qualifications > best_qualifications:
                best_qualifications = candidate.qualifications
                best_candidate = candidate_id
        
        return best_candidate
    
    def _simulate_military_succession(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate military-based succession (strongest military leader wins)."""
        best_candidate = None
        best_military = -1.0
        
        for candidate_id, candidate in succession.candidates.items():
            military_score = candidate.military_power * 0.8 + candidate.support * 0.2
            if military_score > best_military:
                best_military = military_score
                best_candidate = candidate_id
        
        return best_candidate
    
    def _simulate_popular_succession(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate popular acclaim succession (most popular candidate wins)."""
        best_candidate = None
        best_support = -1.0
        
        for candidate_id, candidate in succession.candidates.items():
            if candidate.support > best_support:
                best_support = candidate.support
                best_candidate = candidate_id
        
        return best_candidate
    
    def _simulate_divine_succession(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate divine selection (random with legitimacy bias)."""
        if not succession.candidates:
            return None
        
        candidates = list(succession.candidates.values())
        weights = [candidate.legitimacy + 0.1 for candidate in candidates]  # Add small base chance
        
        winner = random.choices(candidates, weights=weights, k=1)[0]
        
        for cand_id, candidate in succession.candidates.items():
            if candidate.id == winner.id:
                return cand_id
        
        return None
    
    def _simulate_council_succession(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate council appointment (balanced consideration of factors)."""
        if not succession.candidates:
            return None
        
        candidates = list(succession.candidates.values())
        # Council considers multiple factors more evenly
        weights = [
            (candidate.qualifications * 0.3 + candidate.support * 0.2 + 
             candidate.claim_strength * 0.2 + candidate.wealth * 0.2 + candidate.legitimacy * 0.1)
            for candidate in candidates
        ]
        
        winner = random.choices(candidates, weights=weights, k=1)[0]
        
        for cand_id, candidate in succession.candidates.items():
            if candidate.id == winner.id:
                return cand_id
        
        return None
    
    def _simulate_lottery_succession(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate lottery succession (completely random selection)."""
        if not succession.candidates:
            return None
        
        return random.choice(list(succession.candidates.keys()))
    
    def _simulate_duel_succession(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate duel succession (based on military power with randomness)."""
        if not succession.candidates:
            return None
        
        candidates = list(succession.candidates.values())
        # Military power is main factor but with significant randomness
        weights = [candidate.military_power * 0.6 + random.random() * 0.4 for candidate in candidates]
        
        winner = random.choices(candidates, weights=weights, k=1)[0]
        
        for cand_id, candidate in succession.candidates.items():
            if candidate.id == winner.id:
                return cand_id
        
        return None
    
    def _simulate_seniority_succession(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate seniority succession (eldest candidate wins)."""
        best_candidate = None
        oldest_age = -1
        
        for candidate_id, candidate in succession.candidates.items():
            if candidate.age > oldest_age:
                oldest_age = candidate.age
                best_candidate = candidate_id
        
        return best_candidate
    
    def _simulate_appointed_succession(self, succession: SuccessionProcess) -> Optional[str]:
        """Simulate appointed successor (previous ruler's choice)."""
        # This would normally use data about the previous ruler's appointed successor
        # For now, pick candidate with highest qualifications and support
        best_candidate = None
        best_score = -1.0
        
        for candidate_id, candidate in succession.candidates.items():
            score = candidate.qualifications * 0.6 + candidate.support * 0.4
            if score > best_score:
                best_score = score
                best_candidate = candidate_id
        
        return best_candidate
    
    def _simulate_general_succession(self, succession: SuccessionProcess) -> Optional[str]:
        """Fallback simulation for unspecified succession types."""
        if not succession.candidates:
            return None
        
        # Use a balanced approach considering multiple factors
        candidates = list(succession.candidates.values())
        weights = [
            (candidate.claim_strength * 0.3 + candidate.support * 0.3 + 
             candidate.qualifications * 0.2 + candidate.military_power * 0.1 + candidate.legitimacy * 0.1)
            for candidate in candidates
        ]
        
        winner = random.choices(candidates, weights=weights, k=1)[0]
        
        for cand_id, candidate in succession.candidates.items():
            if candidate.id == winner.id:
                return cand_id
        
        return None
    
    def get_succession_statistics(self) -> Dict[str, Any]:
        """Get statistics about past succession processes."""
        if not self.completed_successions:
            return {}
        
        # Calculate various statistics
        total_durations = [s.duration_days for s in self.completed_successions]
        crisis_count = sum(1 for s in self.completed_successions if s.crisis)
        stability_impacts = [s.stability_impact for s in self.completed_successions]
        
        # Count succession types
        type_counts = defaultdict(int)
        for succession in self.completed_successions:
            type_counts[succession.type] += 1
        
        most_common_type = max(type_counts.items(), key=lambda x: x[1])[0] if type_counts else None
        
        return {
            "total_successions": len(self.completed_successions),
            "crisis_rate": crisis_count / len(self.completed_successions),
            "average_duration": sum(total_durations) / len(total_durations),
            "average_stability_impact": sum(stability_impacts) / len(stability_impacts),
            "most_common_type": most_common_type.name if most_common_type else None,
            "last_succession": self.completed_successions[-1].start_date if self.completed_successions else None,
            "civil_war_count": self.history.civil_wars,
            "dynastic_changes": self.history.dynastic_changes,
            "outcome_distribution": {k.name: v for k, v in self.history.succession_outcomes.items()},
        }

    def get_candidate_statistics(self, succession_id: str) -> Optional[List[Dict[str, Any]]]:
        """Return statistics for candidates in a given succession."""
        succession = self.active_successions.get(succession_id) or next((s for s in self.completed_successions if s.id == succession_id), None)
        if not succession:
            return None
        stats = []
        for c in succession.candidates.values():
            stats.append({
                "id": c.id,
                "name": c.name,
                "claim_strength": c.claim_strength,
                "support": c.support,
                "qualifications": c.qualifications,
                "military_power": c.military_power,
                "wealth": c.wealth,
                "age": c.age,
                "health": c.health,
                "legitimacy": c.legitimacy,
                "faction_id": c.faction_id,
                "dynasty_id": c.dynasty_id,
                "religion": c.religion,
                "culture": c.culture,
            })
        return stats

    def advance_time(self, days: int = 1) -> None:
        """Advance the simulation time for all active successions."""
        for succession in self.active_successions.values():
            succession.duration_days += days
            # Check for regency expiration
            if succession.regency and succession.regency.expected_end_date <= datetime.now():
                succession.notes.append(f"Regency for {succession.winner} ended")
                succession.regency = None

    def log_succession_event(self, succession: SuccessionProcess) -> None:
        """Log details of a succession event for auditing or display."""
        logger.info(f"Succession Event: {succession.id} | Type: {succession.type.name} | Winner: {succession.winner}")
        logger.info(f"Notes: {'; '.join(succession.notes)}")
        if succession.crisis:
            logger.info(f"Crisis: {succession.crisis.name} | Severity: {succession.crisis_severity.name}")
        if succession.regency:
            logger.info(f"Regency: Regent {succession.regency.regent_id} until {succession.regency.expected_end_date}")

    def reset_history(self) -> None:
        """Reset all historical records for the succession system."""
        self.history = SuccessionHistory()
        self.completed_successions.clear()
        logger.info("Succession history reset for realm: %s", self.realm_name)

    def get_candidate_analysis(self, succession_id: str) -> Optional[Dict[str, Any]]:
        """
        Analyze candidates in a succession process.
        
        Args:
            succession_id: ID of the succession process to analyze
            
        Returns:
            Analysis data including strengths and weaknesses of each candidate
        """
        if succession_id not in self.active_successions:
            return None
        
        succession = self.active_successions[succession_id]
        analysis = {}
        
        for candidate_id, candidate in succession.candidates.items():
            # Calculate overall score based on succession type
            if succession.type == SuccessionType.HEREDITARY_MONARCHY:
                score = candidate.claim_strength * 0.8 + candidate.legitimacy * 0.2
                strengths = ["Strong claim", "High legitimacy"] if candidate.claim_strength > 0.7 else []
                weaknesses = ["Weak claim"] if candidate.claim_strength < 0.3 else []
            elif succession.type == SuccessionType.ELECTIVE_MONARCHY:
                score = candidate.support * 0.5 + candidate.qualifications * 0.3 + candidate.wealth * 0.2
                strengths = []
                if candidate.support > 0.7: strengths.append("Popular support")
                if candidate.qualifications > 0.7: strengths.append("High qualifications")
                if candidate.wealth > 0.7: strengths.append("Wealthy")
                weaknesses = []
                if candidate.support < 0.3: weaknesses.append("Unpopular")
                if candidate.qualifications < 0.3: weaknesses.append("Unqualified")
            elif succession.type == SuccessionType.MERITOCRATIC:
                score = candidate.qualifications * 0.9 + candidate.support * 0.1
                strengths = ["Highly qualified"] if candidate.qualifications > 0.7 else []
                weaknesses = ["Unqualified"] if candidate.qualifications < 0.3 else []
            elif succession.type == SuccessionType.MILITARY_STRENGTH:
                score = candidate.military_power * 0.8 + candidate.support * 0.2
                strengths = ["Strong military"] if candidate.military_power > 0.7 else []
                weaknesses = ["Weak military"] if candidate.military_power < 0.3 else []
            else:
                score = (candidate.claim_strength + candidate.support + candidate.qualifications) / 3
                strengths = []
                weaknesses = []
            
            # Add trait-based analysis
            if "charismatic" in candidate.traits:
                score += 0.1
                strengths.append("Charismatic")
            if "ruthless" in candidate.traits:
                score += 0.05
                strengths.append("Ruthless")
            if "indecisive" in candidate.traits:
                score -= 0.1
                weaknesses.append("Indecisive")
            if "ill" in candidate.traits:
                score -= 0.15
                weaknesses.append("Poor health")
            
            analysis[candidate_id] = {
                "name": candidate.name,
                "overall_score": min(1.0, max(0.0, score)),
                "strengths": strengths,
                "weaknesses": weaknesses,
                "claim_strength": candidate.claim_strength,
                "support": candidate.support,
                "qualifications": candidate.qualifications,
                "military_power": candidate.military_power,
                "wealth": candidate.wealth,
                "legitimacy": candidate.legitimacy,
                "age": candidate.age,
                "health": candidate.health,
            }
        
        return analysis
    
    def get_regency_info(self, succession_id: str) -> Optional[Dict[str, Any]]:
        """
        Get information about a regency period.
        
        Args:
            succession_id: ID of the succession process
            
        Returns:
            Information about the regency, or None if no regency
        """
        if succession_id not in self.active_successions:
            return None
        
        succession = self.active_successions[succession_id]
        if not succession.regency:
            return None
        
        regency = succession.regency
        return {
            "regent_id": regency.regent_id,
            "start_date": regency.start_date,
            "expected_end_date": regency.expected_end_date,
            "power_struggle_level": regency.power_struggle_level,
            "is_contested": regency.is_contested,
            "council_members": regency.council_members,
            "estimated_duration_days": (regency.expected_end_date - datetime.now()).days
        }
    
    def end_regency(self, succession_id: str) -> bool:
        """
        End a regency period (ruler comes of age).
        
        Args:
            succession_id: ID of the succession process
            
        Returns:
            True if successful, False otherwise
        """
        if succession_id not in self.active_successions:
            return False
        
        succession = self.active_successions[succession_id]
        if not succession.regency:
            return False
        
        # Check if ruler is actually of age
        winner = succession.candidates[succession.winner]
        if winner.age < 16:
            return False  # Ruler still too young
        
        # End the regency
        succession.regency = None
        succession.notes.append(f"Regency ended as {winner.name} came of age")
        
        # Small stability boost from stable transition to personal rule
        succession.stability_impact += 0.2
        
        return True
    
    def simulate_regency_struggle(self, succession_id: str) -> Optional[str]:
        """
        Simulate a power struggle during regency that may change the ruler.
        
        Args:
            succession_id: ID of the succession process
            
        Returns:
            ID of new ruler if coup successful, None otherwise
        """
        if succession_id not in self.active_successions:
            return None
        
        succession = self.active_successions[succession_id]
        if not succession.regency or not succession.regency.is_contested:
            return None
        
        # Chance of successful coup based on power struggle level
        coup_chance = succession.regency.power_struggle_level * 0.3
        
        if random.random() < coup_chance:
            # Regent or other powerful figure takes over
            potential_usurpers = [c for c in succession.candidates.values() 
                                if c.id != succession.winner and c.military_power > 0.5]
            
            if potential_usurpers:
                usurper = max(potential_usurpers, key=lambda x: x.military_power)
                old_ruler = succession.winner
                succession.winner = usurper.id
                succession.regency = None
                succession.notes.append(f"Regency coup: {usurper.name} overthrew {old_ruler}")
                succession.stability_impact -= 0.4
                return usurper.id
        
        return None

