"""Handles colonization of unclaimed lands"""

# Module implementation

"""
Colonial System for Expansion and Settlement

Handles colonial expansion, settlement establishment, and colonial management.
"""

from enum import Enum, auto
from typing import Dict, List, Optional, Set, Tuple, Any
from dataclasses import dataclass, field
from datetime import datetime, timedelta
import logging
import random
import math
from collections import defaultdict

logger = logging.getLogger(__name__)


class ColonialStatus(Enum):
    """Status of colonial settlements."""
    EXPLORATION = auto()    # Area being explored
    OUTPOST = auto()        # Small military or trading outpost
    SETTLEMENT = auto()     # Established settlement
    COLONY = auto()         # Developed colony
    INTEGRATED = auto()     # Fully integrated territory
    REBELLING = auto()      # In rebellion
    ABANDONED = auto()      # Abandoned settlement


class ColonialPolicy(Enum):
    """Types of colonial policies."""
    EXPLOITATION = auto()   # Resource extraction focus
    SETTLEMENT = auto()     # Population settlement focus
    TRADE = auto()          # Trade and commerce focus
    ASSIMILATION = auto()   # Cultural assimilation focus
    PROTECTORATE = auto()   # Indirect rule through locals
    AUTONOMY = auto()       # Significant local autonomy


@dataclass
class ColonialSettlement:
    """A colonial settlement or outpost."""
    id: str
    name: str
    location: Tuple[int, int]
    founding_civ: str
    founding_date: datetime
    status: ColonialStatus = ColonialStatus.OUTPOST
    population: int = 0
    growth_rate: float = 0.02  # per year
    loyalty: float = 100.0  # 0-100%
    infrastructure: float = 0.0  # 0-1.0
    resources: Dict[str, float] = field(default_factory=dict)
    policy: ColonialPolicy = ColonialPolicy.EXPLOITATION
    garrison_size: int = 0
    trade_routes: Set[str] = field(default_factory=set)
    dependencies: List[str] = field(default_factory=list)  # Dependent settlements


@dataclass
class ColonialProject:
    """An ongoing colonial development project."""
    id: str
    settlement_id: str
    project_type: str  # "infrastructure", "defense", "expansion", etc.
    progress: float = 0.0  # 0-100%
    cost: float = 0.0
    required_resources: Dict[str, float] = field(default_factory=dict)
    completion_date: Optional[datetime] = None
    workers: int = 0


class ColonialSystem:
    """
    Manages colonial expansion, settlement, and administration.
    """
    
    def __init__(self):
        self.settlements: Dict[str, ColonialSettlement] = {}
        self.projects: Dict[str, ColonialProject] = {}
        self.colonial_ambitions: Dict[str, List[Tuple[int, int]]] = defaultdict(list)
        
        logger.info("Colonial system initialized")
    
    def establish_settlement(self, civ_id: str, location: Tuple[int, int], 
                           name: str, initial_population: int = 100) -> Optional[ColonialSettlement]:
        """Establish a new colonial settlement."""
        # Check if location is already settled
        for settlement in self.settlements.values():
            if settlement.location == location:
                return None
        
        settlement_id = f"colony_{len(self.settlements) + 1}"
        
        settlement = ColonialSettlement(
            id=settlement_id,
            name=name,
            location=location,
            founding_civ=civ_id,
            founding_date=datetime.now(),
            status=ColonialStatus.OUTPOST,
            population=initial_population,
            loyalty=80.0,  # Initial loyalty
            infrastructure=0.1  # Basic infrastructure
        )
        
        self.settlements[settlement_id] = settlement
        logger.info(f"New settlement established: {name} at {location}")
        
        return settlement
    
    def update_settlements(self, delta_time: float) -> None:
        """Update all colonial settlements."""
        for settlement in self.settlements.values():
            self._update_settlement_growth(settlement, delta_time)
            self._update_settlement_loyalty(settlement, delta_time)
            self._update_settlement_development(settlement, delta_time)
    
    def _update_settlement_growth(self, settlement: ColonialSettlement, delta_time: float) -> None:
        """Update settlement population growth."""
        growth_factor = settlement.growth_rate * delta_time / 365  # Annual growth rate
        
        # Modify growth based on factors
        if settlement.infrastructure > 0.5:
            growth_factor *= 1.2
        if settlement.loyalty < 50.0:
            growth_factor *= 0.5
        
        settlement.population = int(settlement.population * (1 + growth_factor))
        
        # Upgrade status based on population
        if settlement.population >= 10000 and settlement.status.value < ColonialStatus.INTEGRATED.value:
            settlement.status = ColonialStatus.INTEGRATED
        elif settlement.population >= 5000 and settlement.status.value < ColonialStatus.COLONY.value:
            settlement.status = ColonialStatus.COLONY
        elif settlement.population >= 1000 and settlement.status.value < ColonialStatus.SETTLEMENT.value:
            settlement.status = ColonialStatus.SETTLEMENT
    
    def _update_settlement_loyalty(self, settlement: ColonialSettlement, delta_time: float) -> None:
        """Update settlement loyalty."""
        # Base loyalty change
        loyalty_change = 0.0
        
        # Factors affecting loyalty
        if settlement.infrastructure < 0.3:
            loyalty_change -= 0.1 * delta_time
        if settlement.population > 5000 and settlement.status != ColonialStatus.INTEGRATED:
            loyalty_change -= 0.05 * delta_time  # Desire for integration
        
        # Policy effects
        if settlement.policy == ColonialPolicy.ASSIMILATION:
            loyalty_change += 0.02 * delta_time
        elif settlement.policy == ColonialPolicy.EXPLOITATION:
            loyalty_change -= 0.03 * delta_time
        
        settlement.loyalty = max(0.0, min(100.0, settlement.loyalty + loyalty_change))
        
        # Check for rebellion
        if settlement.loyalty < 30.0 and settlement.status != ColonialStatus.REBELLING:
            settlement.status = ColonialStatus.REBELLING
            logger.warning(f"Settlement {settlement.name} is rebelling!")
    
    def _update_settlement_development(self, settlement: ColonialSettlement, delta_time: float) -> None:
        """Update settlement infrastructure development."""
        if settlement.status == ColonialStatus.REBELLING:
            return  # No development during rebellion
        
        # Natural infrastructure growth
        development_rate = 0.01 * delta_time / 365  # Annual rate
        
        # Boost from population and resources
        if settlement.population > 2000:
            development_rate *= 1.5
        if any(qty > 100 for qty in settlement.resources.values()):
            development_rate *= 1.3
        
        settlement.infrastructure = min(1.0, settlement.infrastructure + development_rate)
    
    def start_colonial_project(self, settlement_id: str, project_type: str, cost: float, required_resources: Dict[str, float]) -> Optional[ColonialProject]:
        """Start a new colonial development project."""
        if settlement_id not in self.settlements:
            return None
        
        project_id = f"project_{len(self.projects) + 1}"
        
        project = ColonialProject(
            id=project_id,
            settlement_id=settlement_id,
            project_type=project_type,
            cost=cost,
            required_resources=required_resources,
            progress=0.0
        )
        
        self.projects[project_id] = project
        return project
    
    def update_projects(self, delta_time: float) -> None:
        """Update all colonial projects."""
        for project_id, project in list(self.projects.items()):
            settlement = self.settlements.get(project.settlement_id)
            if not settlement:
                del self.projects[project_id]
                continue
            
            # Update project progress
            progress_rate = self._calculate_project_progress_rate(project, settlement)
            project.progress = min(100.0, project.progress + progress_rate * delta_time)
            
            # Check for completion
            if project.progress >= 100.0:
                self._complete_project(project, settlement)
                del self.projects[project_id]
    
    def _calculate_project_progress_rate(self, project: ColonialProject, settlement: ColonialSettlement) -> float:
        """Calculate progress rate for a project."""
        base_rate = 0.1  # % per day
        
        # Modify based on settlement factors
        if settlement.infrastructure > 0.5:
            base_rate *= 1.2
        if settlement.population > 1000:
            base_rate *= 1.1
        
        # Check if required resources are available
        for resource, required in project.required_resources.items():
            available = settlement.resources.get(resource, 0.0)
            if available < required:
                base_rate *= 0.5  # Slow down if resources lacking
        
        return base_rate
    
    def _complete_project(self, project: ColonialProject, settlement: ColonialSettlement) -> None:
        """Complete a project and apply its effects."""
        if project.project_type == "infrastructure":
            settlement.infrastructure = min(1.0, settlement.infrastructure + 0.2)
            logger.info(f"Infrastructure project completed in {settlement.name}")
        elif project.project_type == "defense":
            settlement.garrison_size += 50
            logger.info(f"Defense project completed in {settlement.name}")
        elif project.project_type == "expansion":
            settlement.population += 200
            logger.info(f"Expansion project completed in {settlement.name}")
    
    def get_settlement_stats(self, settlement_id: str) -> Optional[Dict[str, Any]]:
        """Get statistics for a settlement."""
        if settlement_id not in self.settlements:
            return None
        
        settlement = self.settlements[settlement_id]
        
        return {
            "name": settlement.name,
            "status": settlement.status.name,
            "population": settlement.population,
            "loyalty": settlement.loyalty,
            "infrastructure": settlement.infrastructure,
            "age_days": (datetime.now() - settlement.founding_date).days,
            "garrison_size": settlement.garrison_size,
            "resource_production": settlement.resources
        }
    
    def can_establish_settlement(self, civ_id: str, location: Tuple[int, int]) -> bool:
        """Check if a settlement can be established at a location."""
        # Check distance from existing settlements
        for settlement in self.settlements.values():
            dist = math.sqrt((settlement.location[0] - location[0])**2 + 
                           (settlement.location[1] - location[1])**2)
            if dist < 50:  # Minimum distance between settlements
                return False
        
        # Additional checks would go here (terrain, resources, etc.)
        return True

@dataclass
class Colony:
    """A full colony with comprehensive management systems."""
    id: str
    name: str
    location: Tuple[float, float]
    founding_civ: str
    founding_date: datetime
    status: ColonialStatus = ColonialStatus.OUTPOST
    
    # Population & Demographics
    total_population: int = 0
    population_growth_rate: float = 0.02
    demographic_composition: Dict[str, float] = field(default_factory=dict)  # culture -> percentage
    migration_rate: float = 0.0
    
    # Economy & Production
    economic_focus: str = "mixed"  # "agriculture", "mining", "trade", "industry"
    production_output: Dict[str, float] = field(default_factory=dict)
    trade_balance: float = 0.0
    infrastructure_level: float = 0.1
    
    # Governance & Loyalty
    government_type: str = "colonial_administration"
    loyalty: float = 80.0
    autonomy_level: float = 0.1  # 0-1.0 (1 = full autonomy)
    corruption_level: float = 0.1  # 0-1.0
    
    # Military & Defense
    garrison_size: int = 0
    defense_structures: List[str] = field(default_factory=list)
    naval_presence: int = 0
    
    # Resources & Environment
    local_resources: Dict[str, float] = field(default_factory=dict)
    environmental_impact: float = 0.0
    sustainability_score: float = 0.5
    
    # Development
    research_focus: str = "practical"
    education_level: float = 0.1
    cultural_influence: float = 0.1
    
    # Relations
    relations_with_natives: float = 50.0  # 0-100
    relations_with_homeland: float = 80.0  # 0-100
    
    def calculate_viability_score(self) -> float:
        """Calculate overall colony viability score (0-100)."""
        scores = {
            "population": min(100, self.total_population / 100),  # Cap at 10k pop = 100
            "economy": self.trade_balance + 50,  # Center at 50
            "loyalty": self.loyalty,
            "infrastructure": self.infrastructure_level * 100,
            "defense": min(100, self.garrison_size * 2),  # 50 garrison = 100
            "sustainability": self.sustainability_score * 100
        }
        
        return sum(scores.values()) / len(scores)
    
    def update_economic_output(self) -> None:
        """Update colony economic output based on focus and resources."""
        base_output = self.total_population * 0.1
        
        if self.economic_focus == "agriculture":
            base_output *= 1.3
        elif self.economic_focus == "mining":
            base_output *= 1.5
        elif self.economic_focus == "trade":
            base_output *= 1.4
        
        # Infrastructure multiplier
        infrastructure_multiplier = 1.0 + (self.infrastructure_level * 0.5)
        
        self.production_output["total"] = base_output * infrastructure_multiplier
    
    def can_upgrade_status(self) -> bool:
        """Check if colony can upgrade to next status level."""
        current_status = self.status
        
        requirements = {
            ColonialStatus.OUTPOST: {
                "population": 500,
                "infrastructure": 0.3,
                "viability": 60.0
            },
            ColonialStatus.SETTLEMENT: {
                "population": 2000,
                "infrastructure": 0.5,
                "viability": 70.0
            },
            ColonialStatus.COLONY: {
                "population": 10000,
                "infrastructure": 0.7,
                "viability": 80.0
            },
            ColonialStatus.INTEGRATED: {
                "population": 50000,
                "infrastructure": 0.9,
                "viability": 90.0
            }
        }
        
        if current_status not in requirements:
            return False
        
        req = requirements[current_status]
        viability = self.calculate_viability_score()
        
        return (self.total_population >= req["population"] and
                self.infrastructure_level >= req["infrastructure"] and
                viability >= req["viability"])