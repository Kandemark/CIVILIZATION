"""Manages land claims and expansion"""

# Module implementation

"""
Territory Manager for Land Control and Administration

Handles territory claims, administration, development, and regional management.
"""

from enum import Enum, auto
from typing import Dict, List, Optional, Set, Tuple, Any
from dataclasses import dataclass, field
from datetime import datetime, timedelta
import logging
import random
from collections import defaultdict
import math

logger = logging.getLogger(__name__)


class DevelopmentLevel(Enum):
    """Levels of territory development."""
    WILDERNESS = auto()     # Uninhabited wilderness
    FRONTIER = auto()       # Sparsely populated frontier
    RURAL = auto()          # Agricultural countryside
    DEVELOPED = auto()      # Developed territory
    URBAN = auto()          # Urbanized area
    METROPOLITAN = auto()   # Major metropolitan region


@dataclass
class Territory:
    """A controlled territory with administrative information."""
    id: str
    name: str
    boundaries: List[Tuple[float, float]]  # Polygon points
    owner: str  # Controlling civilization
    capital: Optional[str] = None  # Capital settlement ID
    development_level: DevelopmentLevel = DevelopmentLevel.WILDERNESS
    population: int = 0
    population_density: float = 0.0  # people per square km
    economic_output: float = 0.0
    tax_rate: float = 0.1  # 0-1.0
    loyalty: float = 100.0  # 0-100%
    infrastructure: float = 0.0  # 0-1.0
    resources: Dict[str, float] = field(default_factory=dict)
    settlements: List[str] = field(default_factory=list)
    military_presence: int = 0
    claimed_date: datetime = field(default_factory=datetime.now)
    last_census: Optional[datetime] = None


@dataclass
class AdministrativeRegion:
    """An administrative region containing multiple territories."""
    id: str
    name: str
    territories: List[str]
    governor: Optional[str] = None  # Character ID
    administrative_efficiency: float = 0.7  # 0-1.0
    capital_territory: Optional[str] = None
    tax_collection: float = 0.0
    maintenance_cost: float = 0.0
    development_projects: List[str] = field(default_factory=list)


@dataclass
class TerritorialDispute:
    """A dispute over territory ownership."""
    id: str
    territory_id: str
    claimants: Set[str] = field(default_factory=set)
    start_date: datetime = field(default_factory=datetime.now)
    severity: float = 0.0  # 0-1.0
    resolution_progress: float = 0.0  # 0-100%
    conflict_level: float = 0.0  # 0-1.0


class TerritoryManager:
    """
    Manages territory control, administration, and development.
    """
    
    def __init__(self):
        self.territories: Dict[str, Territory] = {}
        self.regions: Dict[str, AdministrativeRegion] = {}
        self.territorial_disputes: Dict[str, TerritorialDispute] = {}
        self.ownership_history: Dict[str, List[Dict[str, Any]]] = defaultdict(list)
        
        logger.info("Territory manager initialized")
    
    def claim_territory(self, territory_id: str, civ_id: str, 
                       boundaries: List[Tuple[float, float]], name: str) -> Optional[Territory]:
        """Claim a new territory for a civilization."""
        if territory_id in self.territories:
            return None  # Territory already claimed
        
        territory = Territory(
            id=territory_id,
            name=name,
            boundaries=boundaries,
            owner=civ_id,
            development_level=DevelopmentLevel.WILDERNESS,
            claimed_date=datetime.now()
        )
        
        self.territories[territory_id] = territory
        
        # Record ownership claim
        self.ownership_history[territory_id].append({
            "owner": civ_id,
            "date": datetime.now(),
            "type": "initial_claim",
            "notes": "Territory initially claimed"
        })
        
        logger.info(f"Territory {name} claimed by civilization {civ_id}")
        return territory
    
    def transfer_territory(self, territory_id: str, new_owner: str, 
                          transfer_type: str = "diplomatic") -> bool:
        """Transfer territory ownership."""
        if territory_id not in self.territories:
            return False
        
        territory = self.territories[territory_id]
        old_owner = territory.owner
        
        territory.owner = new_owner
        territory.loyalty = max(50.0, territory.loyalty - 20.0)  # Loyalty drop
        
        # Record transfer
        self.ownership_history[territory_id].append({
            "old_owner": old_owner,
            "new_owner": new_owner,
            "date": datetime.now(),
            "type": transfer_type,
            "notes": f"Territory transferred via {transfer_type}"
        })
        
        logger.info(f"Territory {territory_id} transferred from {old_owner} to {new_owner}")
        return True
    
    def update_territory_development(self, delta_time: float) -> None:
        """Update development and population for all territories."""
        for territory in self.territories.values():
            self._update_population(territory, delta_time)
            self._update_development(territory, delta_time)
            self._update_economy(territory, delta_time)
            self._update_loyalty(territory, delta_time)
    
    def _update_population(self, territory: Territory, delta_time: float) -> None:
        """Update territory population."""
        growth_rate = 0.02  # Base annual growth rate
        
        # Modify based on development and factors
        if territory.development_level == DevelopmentLevel.URBAN:
            growth_rate *= 1.3
        elif territory.development_level == DevelopmentLevel.WILDERNESS:
            growth_rate *= 0.5
        
        if territory.infrastructure > 0.7:
            growth_rate *= 1.2
        if territory.loyalty < 50.0:
            growth_rate *= 0.7
        
        # Calculate population change
        population_change = territory.population * growth_rate * delta_time / 365
        territory.population = max(0, int(territory.population + population_change))
        
        # Update population density (simplified)
        area = self._calculate_territory_area(territory)
        if area > 0:
            territory.population_density = territory.population / area
    
    def _update_development(self, territory: Territory, delta_time: float) -> None:
        """Update territory development level."""
        development_rate = 0.01 * delta_time / 365  # Annual rate
        
        # Factors affecting development
        if territory.population > 10000:
            development_rate *= 1.5
        if territory.economic_output > 5000:
            development_rate *= 1.3
        if territory.infrastructure > 0.6:
            development_rate *= 1.2
        
        # Check for development level upgrades
        current_level = territory.development_level.value
        development_score = territory.infrastructure + (territory.population / 50000)
        
        if development_score > 1.5 and current_level < DevelopmentLevel.METROPOLITAN.value:
            territory.development_level = DevelopmentLevel.METROPOLITAN
        elif development_score > 1.2 and current_level < DevelopmentLevel.URBAN.value:
            territory.development_level = DevelopmentLevel.URBAN
        elif development_score > 0.8 and current_level < DevelopmentLevel.DEVELOPED.value:
            territory.development_level = DevelopmentLevel.DEVELOPED
        elif development_score > 0.5 and current_level < DevelopmentLevel.RURAL.value:
            territory.development_level = DevelopmentLevel.RURAL
        elif development_score > 0.2 and current_level < DevelopmentLevel.FRONTIER.value:
            territory.development_level = DevelopmentLevel.FRONTIER
    
    def _update_economy(self, territory: Territory, delta_time: float) -> None:
        """Update territory economic output."""
        base_output = territory.population * 0.1  # Base per capita output
        
        # Modify based on development
        development_multiplier = {
            DevelopmentLevel.WILDERNESS: 0.3,
            DevelopmentLevel.FRONTIER: 0.6,
            DevelopmentLevel.RURAL: 0.8,
            DevelopmentLevel.DEVELOPED: 1.2,
            DevelopmentLevel.URBAN: 1.5,
            DevelopmentLevel.METROPOLITAN: 2.0
        }
        
        multiplier = development_multiplier.get(territory.development_level, 1.0)
        
        # Infrastructure effect
        infrastructure_bonus = territory.infrastructure * 0.5
        
        territory.economic_output = base_output * multiplier * (1 + infrastructure_bonus)
    
    def _update_loyalty(self, territory: Territory, delta_time: float) -> None:
        """Update territory loyalty to the ruling civilization."""
        loyalty_change = 0.0
        
        # Base loyalty drift toward 50%
        if territory.loyalty > 50.0:
            loyalty_change -= 0.01 * delta_time
        else:
            loyalty_change += 0.01 * delta_time
        
        # Development effects
        if territory.development_level.value >= DevelopmentLevel.DEVELOPED.value:
            loyalty_change += 0.02 * delta_time
        
        # Tax effects
        if territory.tax_rate > 0.2:
            loyalty_change -= (territory.tax_rate - 0.2) * 0.1 * delta_time
        
        # Military presence effects
        if territory.military_presence > territory.population * 0.01:  # Over-militarized
            loyalty_change -= 0.01 * delta_time
        elif territory.military_presence < territory.population * 0.001:  # Under-protected
            loyalty_change -= 0.005 * delta_time
        
        territory.loyalty = max(0.0, min(100.0, territory.loyalty + loyalty_change))
    
    def _calculate_territory_area(self, territory: Territory) -> float:
        """Calculate the area of a territory polygon."""
        # Simple area calculation using shoelace formula
        points = territory.boundaries
        if len(points) < 3:
            return 0.0
        
        area = 0.0
        n = len(points)
        for i in range(n):
            j = (i + 1) % n
            area += points[i][0] * points[j][1]
            area -= points[j][0] * points[i][1]
        
        return abs(area) / 2.0
    
    def create_administrative_region(self, name: str, territory_ids: List[str], 
                                   capital_territory: Optional[str] = None) -> AdministrativeRegion:
        """Create a new administrative region."""
        region_id = f"region_{len(self.regions) + 1}"
        
        region = AdministrativeRegion(
            id=region_id,
            name=name,
            territories=territory_ids,
            capital_territory=capital_territory or territory_ids[0] if territory_ids else None
        )
        
        self.regions[region_id] = region
        logger.info(f"Administrative region {name} created with {len(territory_ids)} territories")
        
        return region
    
    def get_territory_stats(self, territory_id: str) -> Optional[Dict[str, Any]]:
        """Get comprehensive statistics for a territory."""
        if territory_id not in self.territories:
            return None
        
        territory = self.territories[territory_id]
        area = self._calculate_territory_area(territory)
        
        return {
            "name": territory.name,
            "owner": territory.owner,
            "development_level": territory.development_level.name,
            "population": territory.population,
            "population_density": territory.population_density,
            "area": area,
            "economic_output": territory.economic_output,
            "tax_revenue": territory.economic_output * territory.tax_rate,
            "loyalty": territory.loyalty,
            "infrastructure": territory.infrastructure,
            "military_presence": territory.military_presence,
            "settlement_count": len(territory.settlements),
            "age_days": (datetime.now() - territory.claimed_date).days
        }
    
    def get_civilization_territories(self, civ_id: str) -> List[Territory]:
        """Get all territories owned by a civilization."""
        return [t for t in self.territories.values() if t.owner == civ_id]
    
    def calculate_civilization_stats(self, civ_id: str) -> Dict[str, Any]:
        """Calculate comprehensive statistics for a civilization's territories."""
        territories = self.get_civilization_territories(civ_id)
        
        if not territories:
            return {}
        
        total_population = sum(t.population for t in territories)
        total_economic_output = sum(t.economic_output for t in territories)
        total_area = sum(self._calculate_territory_area(t) for t in territories)
        average_loyalty = sum(t.loyalty for t in territories) / len(territories)
        
        development_distribution = defaultdict(int)
        for t in territories:
            development_distribution[t.development_level.name] += 1
        
        return {
            "territory_count": len(territories),
            "total_population": total_population,
            "total_economic_output": total_economic_output,
            "total_area": total_area,
            "average_loyalty": average_loyalty,
            "development_distribution": dict(development_distribution),
            "average_population_density": total_population / total_area if total_area > 0 else 0,
            "total_tax_revenue": sum(t.economic_output * t.tax_rate for t in territories)
        }
    
    def invest_in_infrastructure(self, territory_id: str, investment: float) -> bool:
        """Invest in territory infrastructure development."""
        if territory_id not in self.territories:
            return False
        
        territory = self.territories[territory_id]
        
        # Infrastructure improvement based on investment
        improvement = investment * 0.001  # Scale factor
        territory.infrastructure = min(1.0, territory.infrastructure + improvement)
        
        logger.info(f"Infrastructure investment in {territory_id}: +{improvement:.3f}")
        return True
    
    def adjust_tax_rate(self, territory_id: str, new_tax_rate: float) -> bool:
        """Adjust the tax rate for a territory."""
        if territory_id not in self.territories:
            return False
        
        territory = self.territories[territory_id]
        territory.tax_rate = max(0.0, min(1.0, new_tax_rate))
        
        # Tax changes affect loyalty
        if new_tax_rate > territory.tax_rate:
            territory.loyalty -= 5.0  # Loyalty penalty for tax increases
        else:
            territory.loyalty += 2.0  # Small loyalty boost for tax decreases
        
        logger.info(f"Tax rate adjusted in {territory_id}: {new_tax_rate:.2f}")
        return True
    
@dataclass
class TerritorialClaim:
    """A claim on territory, which may or may not be currently controlled."""
    id: str
    claimant: str  # Civilization making the claim
    territory_id: str
    claim_date: datetime
    claim_type: str  # "historical", "cultural", "geographical", "conquest", "discovery"
    
    # Claim strength factors
    historical_evidence: float = 0.0  # 0-1.0
    cultural_connections: float = 0.0  # 0-1.0
    geographical_continuity: float = 0.0  # 0-1.0
    current_presence: float = 0.0  # 0-1.0
    international_recognition: float = 0.0  # 0-1.0
    
    # Claim status
    is_active: bool = True
    is_recognized: bool = False
    challenge_level: float = 0.0  # 0-1.0
    
    # Supporting evidence
    supporting_documents: List[str] = field(default_factory=list)
    witness_statements: List[str] = field(default_factory=list)
    historical_artifacts: List[str] = field(default_factory=list)
    
    def calculate_claim_strength(self) -> float:
        """Calculate the overall strength of this territorial claim."""
        weights = {
            "historical": 0.25,
            "cultural": 0.20,
            "geographical": 0.15,
            "presence": 0.25,
            "recognition": 0.15
        }
        
        strength = (
            self.historical_evidence * weights["historical"] +
            self.cultural_connections * weights["cultural"] +
            self.geographical_continuity * weights["geographical"] +
            self.current_presence * weights["presence"] +
            self.international_recognition * weights["recognition"]
        )
        
        # Active claims are stronger
        if self.is_active:
            strength *= 1.2
        
        # Recognized claims are much stronger
        if self.is_recognized:
            strength *= 1.5
        
        # Challenges weaken the claim
        strength *= (1.0 - self.challenge_level * 0.3)
        
        return min(1.0, max(0.0, strength))
    
    def add_supporting_evidence(self, evidence_type: str, evidence_id: str, strength: float) -> None:
        """Add supporting evidence to strengthen the claim."""
        if evidence_type == "document":
            self.supporting_documents.append(evidence_id)
            self.historical_evidence = min(1.0, self.historical_evidence + strength * 0.1)
        elif evidence_type == "witness":
            self.witness_statements.append(evidence_id)
            self.current_presence = min(1.0, self.current_presence + strength * 0.15)
        elif evidence_type == "artifact":
            self.historical_artifacts.append(evidence_id)
            self.historical_evidence = min(1.0, self.historical_evidence + strength * 0.2)
    
    def challenge_claim(self, challenger: str, challenge_strength: float, 
                       challenge_reason: str) -> bool:
        """Challenge this territorial claim."""
        self.challenge_level = min(1.0, self.challenge_level + challenge_strength)
        
        # Strong challenges might invalidate recognition
        if challenge_strength > 0.7 and self.is_recognized:
            self.is_recognized = False
        
        logger.info(f"Territorial claim {self.id} challenged by {challenger}: {challenge_reason}")
        return True
    
    def seek_recognition(self, recognizing_party: str, influence: float) -> bool:
        """Attempt to get international recognition for this claim."""
        recognition_chance = self.calculate_claim_strength() * influence
        
        if random.random() < recognition_chance:
            self.is_recognized = True
            self.international_recognition = min(1.0, self.international_recognition + 0.2)
            logger.info(f"Territorial claim {self.id} recognized by {recognizing_party}")
            return True
        else:
            logger.info(f"Recognition attempt failed for claim {self.id}")
            return False
    
    def compare_claims(self, other_claim: 'TerritorialClaim') -> Dict[str, Any]:
        """Compare this claim with another claim on the same territory."""
        if self.territory_id != other_claim.territory_id:
            return {"error": "Claims are for different territories"}
        
        self_strength = self.calculate_claim_strength()
        other_strength = other_claim.calculate_claim_strength()
        
        return {
            "territory_id": self.territory_id,
            "claimant_a": self.claimant,
            "claimant_b": other_claim.claimant,
            "strength_a": self_strength,
            "strength_b": other_strength,
            "stronger_claimant": self.claimant if self_strength > other_strength else other_claim.claimant,
            "strength_difference": abs(self_strength - other_strength),
            "dispute_level": max(self.challenge_level, other_claim.challenge_level)
        }

class TerritorialClaimManager:
    """Manages multiple territorial claims and disputes."""
    
    def __init__(self):
        self.claims: Dict[str, TerritorialClaim] = {}
        self.territory_claims: Dict[str, List[str]] = defaultdict(list)  # territory_id -> claim_ids
        
    def register_claim(self, claimant: str, territory_id: str, claim_type: str,
                      initial_evidence: Dict[str, float] = None) -> TerritorialClaim:
        """Register a new territorial claim."""
        claim_id = f"claim_{len(self.claims) + 1}"
        
        claim = TerritorialClaim(
            id=claim_id,
            claimant=claimant,
            territory_id=territory_id,
            claim_date=datetime.now(),
            claim_type=claim_type
        )
        
        # Apply initial evidence
        if initial_evidence:
            for evidence_type, strength in initial_evidence.items():
                if evidence_type == "historical":
                    claim.historical_evidence = strength
                elif evidence_type == "cultural":
                    claim.cultural_connections = strength
                elif evidence_type == "geographical":
                    claim.geographical_continuity = strength
                elif evidence_type == "presence":
                    claim.current_presence = strength
        
        self.claims[claim_id] = claim
        self.territory_claims[territory_id].append(claim_id)
        
        logger.info(f"New territorial claim registered: {claimant} claims {territory_id}")
        return claim
    
    def get_claims_on_territory(self, territory_id: str) -> List[TerritorialClaim]:
        """Get all claims on a specific territory."""
        claim_ids = self.territory_claims.get(territory_id, [])
        return [self.claims[claim_id] for claim_id in claim_ids if claim_id in self.claims]
    
    def resolve_territorial_dispute(self, territory_id: str, 
                                  resolution_method: str = "diplomatic") -> Optional[str]:
        """Attempt to resolve all claims on a territory."""
        claims = self.get_claims_on_territory(territory_id)
        if not claims:
            return None
        
        if len(claims) == 1:
            # Only one claim, straightforward
            winner = claims[0]
            winner.is_recognized = True
            return winner.claimant
        
        # Multiple claims - need resolution
        if resolution_method == "strength":
            # Strongest claim wins
            strongest_claim = max(claims, key=lambda c: c.calculate_claim_strength())
            strongest_claim.is_recognized = True
            
            # Deactivate other claims
            for claim in claims:
                if claim != strongest_claim:
                    claim.is_active = False
            
            return strongest_claim.claimant
        
        elif resolution_method == "diplomatic":
            # Diplomatic resolution - might involve compromises
            # For now, pick randomly weighted by claim strength
            strengths = [c.calculate_claim_strength() for c in claims]
            total_strength = sum(strengths)
            probabilities = [s / total_strength for s in strengths]
            
            winner = random.choices(claims, weights=probabilities, k=1)[0]
            winner.is_recognized = True
            
            logger.info(f"Diplomatic resolution: {winner.claimant} gains {territory_id}")
            return winner.claimant
        
        return None