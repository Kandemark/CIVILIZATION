"""Demographic calculations"""

# Module implementation

# utils/demographic_calculations.py
from typing import Dict, List, Optional, Tuple, Any
from dataclasses import dataclass, field
import numpy as np
from datetime import datetime, timedelta
import logging
import math
from enum import Enum, auto

logger = logging.getLogger(__name__)


class PopulationDistribution(Enum):
    """Types of population distributions."""
    STABLE = auto()
    GROWING = auto()
    DECLINING = auto()
    YOUNG = auto()
    AGING = auto()
    PYRAMIDAL = auto()
    COLUMNAR = auto()


class MigrationType(Enum):
    """Types of migration patterns."""
    INTERNAL = auto()
    EXTERNAL = auto()
    RURAL_URBAN = auto()
    REFUGEE = auto()
    ECONOMIC = auto()
    FORCED = auto()


@dataclass
class AgeGroup:
    """Represents an age group in population pyramid."""
    min_age: int
    max_age: int
    population: int
    male_ratio: float = 0.5
    birth_rate: float = 0.0  # Age-specific fertility rate
    death_rate: float = 0.0  # Age-specific mortality rate
    
    @property
    def female_population(self) -> int:
        """Calculate female population in this age group."""
        return int(self.population * (1 - self.male_ratio))
    
    @property
    def male_population(self) -> int:
        """Calculate male population in this age group."""
        return int(self.population * self.male_ratio)
    
    def get_fertile_women(self) -> int:
        """Get number of women in childbearing ages (15-49)."""
        if 15 <= self.min_age <= 49:
            return self.female_population
        return 0


@dataclass
class PopulationPyramid:
    """Population pyramid with age groups."""
    age_groups: List[AgeGroup]
    total_population: int = 0
    region_id: str = "unknown"
    timestamp: datetime = field(default_factory=datetime.now)
    
    def __post_init__(self):
        self.total_population = sum(group.population for group in self.age_groups)
    
    def get_dependency_ratio(self) -> float:
        """Calculate dependency ratio (non-working age / working age)."""
        young_pop = sum(g.population for g in self.age_groups if g.max_age < 15)
        old_pop = sum(g.population for g in self.age_groups if g.min_age >= 65)
        working_pop = self.total_population - young_pop - old_pop
        
        return (young_pop + old_pop) / working_pop if working_pop > 0 else 0.0
    
    def get_median_age(self) -> float:
        """Calculate median age of the population."""
        if not self.age_groups:
            return 0.0
        
        cumulative_pop = 0
        target_pop = self.total_population / 2
        
        for group in self.age_groups:
            cumulative_pop += group.population
            if cumulative_pop >= target_pop:
                # Linear interpolation within the age group
                group_start_pop = cumulative_pop - group.population
                pop_in_group = target_pop - group_start_pop
                fraction = pop_in_group / group.population
                age_range = group.max_age - group.min_age
                return group.min_age + fraction * age_range
        
        return self.age_groups[-1].max_age
    
    def get_fertility_rate(self) -> float:
        """Calculate total fertility rate."""
        fertile_women = sum(group.get_fertile_women() for group in self.age_groups)
        if fertile_women == 0:
            return 0.0
        
        total_births = sum(group.population * group.birth_rate for group in self.age_groups)
        return total_births / fertile_women * 35  # Approximate TFR
    
    def get_life_expectancy(self) -> float:
        """Calculate approximate life expectancy."""
        if not self.age_groups:
            return 0.0
        
        total_person_years = 0
        survivors = self.total_population
        
        for group in self.age_groups:
            # Average age in group
            avg_age = (group.min_age + group.max_age) / 2
            # People surviving to this age group
            total_person_years += survivors * avg_age
            # Apply mortality to next group
            survivors -= group.population * group.death_rate
        
        return total_person_years / self.total_population if self.total_population > 0 else 0.0
    
    def project_population(self, years: int = 1) -> 'PopulationPyramid':
        """Project population forward by specified years."""
        new_groups = []
        
        for group in self.age_groups:
            # Calculate survivors moving to next age group
            survivors = int(group.population * (1 - group.death_rate))
            
            # Calculate new age range
            new_min_age = group.min_age + years
            new_max_age = group.max_age + years
            
            # Only include if not beyond maximum reasonable age
            if new_min_age <= 100:
                new_group = AgeGroup(
                    min_age=new_min_age,
                    max_age=new_max_age,
                    population=survivors,
                    male_ratio=group.male_ratio,
                    birth_rate=group.birth_rate,
                    death_rate=min(1.0, group.death_rate * 1.02)  # Slight increase with age
                )
                new_groups.append(new_group)
        
        return PopulationPyramid(
            age_groups=new_groups,
            region_id=self.region_id,
            timestamp=self.timestamp + timedelta(days=365 * years)
        )


@dataclass
class MigrationFlow:
    """Represents population migration."""
    source: str
    destination: str
    rate: float  # people per time unit
    migration_type: MigrationType
    reasons: List[str]
    age_distribution: List[float] = field(default_factory=list)  # Distribution by age groups
    distance: float = 0.0  # Distance between source and destination
    barriers: List[str] = field(default_factory=list)  # Migration barriers
    
    def get_effective_rate(self, time_delta: float, conditions: Dict[str, float]) -> float:
        """Calculate effective migration rate considering conditions."""
        base_migrants = self.rate * time_delta
        
        # Apply condition modifiers
        push_factor = conditions.get('push_strength', 1.0)
        pull_factor = conditions.get('pull_strength', 1.0)
        barrier_factor = 1.0 / (1.0 + len(self.barriers) * 0.2)
        distance_factor = 1.0 / (1.0 + self.distance * 0.01)
        
        return base_migrants * push_factor * pull_factor * barrier_factor * distance_factor


class DemographicModel:
    """Advanced demographic modeling system."""
    
    def __init__(self):
        self.birth_rates: Dict[str, float] = {}  # Region -> birth rate
        self.death_rates: Dict[str, float] = {}  # Region -> death rate
        self.migration_flows: List[MigrationFlow] = []
        self.population_history: Dict[str, List[Tuple[datetime, int]]] = {}
        self.age_distributions: Dict[str, PopulationPyramid] = {}
        
        # Demographic transition parameters
        self.demographic_transition_phase: Dict[str, int] = {}  # 1-4 phases
        self.urbanization_rates: Dict[str, float] = {}
    
    def calculate_growth(self, current_population: int, time_delta: float = 1.0, 
                        birth_rate: float = 0.02, death_rate: float = 0.01,
                        migration_net: int = 0) -> int:
        """Calculate population growth using logistic model with migration."""
        growth_rate = birth_rate - death_rate
        
        # Use logistic growth to account for carrying capacity
        # Simple exponential for small populations, logistic for larger
        if current_population < 100000:
            new_population = int(current_population * math.exp(growth_rate * time_delta))
        else:
            # Logistic growth approximation
            carrying_capacity = self._estimate_carrying_capacity(current_population)
            if carrying_capacity > current_population:
                growth_factor = 1 + growth_rate * (1 - current_population / carrying_capacity)
                new_population = int(current_population * growth_factor * time_delta)
            else:
                new_population = int(current_population * math.exp(growth_rate * time_delta))
        
        # Apply migration
        new_population += migration_net
        
        return max(0, new_population)
    
    def _estimate_carrying_capacity(self, current_population: int) -> int:
        """Estimate carrying capacity based on current population."""
        # Simple heuristic - carrying capacity is roughly 10x current population for growing societies
        return current_population * 10
    
    def create_population_pyramid(self, total_population: int, distribution_type: PopulationDistribution = PopulationDistribution.STABLE,
                                region_id: str = "unknown") -> PopulationPyramid:
        """Create a population pyramid based on distribution type."""
        age_groups = []
        
        # Define age groups (5-year intervals)
        age_ranges = [
            (0, 4), (5, 9), (10, 14), (15, 19), (20, 24),
            (25, 29), (30, 34), (35, 39), (40, 44), (45, 49),
            (50, 54), (55, 59), (60, 64), (65, 69), (70, 74), (75, 79), (80, 100)
        ]
        
        if distribution_type == PopulationDistribution.STABLE:
            # Stable population distribution (stationary pyramid)
            weights = [0.065, 0.065, 0.065, 0.065, 0.065,
                      0.065, 0.065, 0.065, 0.065, 0.065,
                      0.06, 0.055, 0.05, 0.04, 0.03, 0.02, 0.01]
            
        elif distribution_type == PopulationDistribution.GROWING:
            # Young population (growing pyramid)
            weights = [0.12, 0.11, 0.10, 0.095, 0.09,
                      0.085, 0.08, 0.075, 0.07, 0.065,
                      0.05, 0.035, 0.025, 0.015, 0.01, 0.005, 0.002]
            
        elif distribution_type == PopulationDistribution.DECLINING:
            # Aging population (declining pyramid)
            weights = [0.04, 0.045, 0.05, 0.055, 0.06,
                      0.065, 0.07, 0.075, 0.08, 0.085,
                      0.09, 0.08, 0.07, 0.05, 0.035, 0.02, 0.01]
            
        elif distribution_type == PopulationDistribution.YOUNG:
            # Very young population (high growth)
            weights = [0.15, 0.14, 0.13, 0.12, 0.11,
                      0.09, 0.07, 0.06, 0.05, 0.04,
                      0.03, 0.02, 0.015, 0.01, 0.005, 0.002, 0.001]
        
        else:
            # Default to stable
            weights = [0.065] * 10 + [0.06, 0.055, 0.05, 0.04, 0.03, 0.02, 0.01]
        
        # Normalize weights
        total_weight = sum(weights)
        weights = [w / total_weight for w in weights]
        
        # Age-specific rates (simplified)
        fertility_rates = [0.0, 0.0, 0.0, 0.05, 0.08,
                          0.10, 0.09, 0.07, 0.04, 0.02,
                          0.01, 0.005, 0.0, 0.0, 0.0, 0.0, 0.0]
        
        mortality_rates = [0.02, 0.005, 0.003, 0.004, 0.005,
                          0.006, 0.007, 0.008, 0.01, 0.012,
                          0.015, 0.02, 0.03, 0.05, 0.08, 0.12, 0.20]
        
        for (min_age, max_age), weight, fertility, mortality in zip(age_ranges, weights, fertility_rates, mortality_rates):
            population = int(total_population * weight)
            male_ratio = 0.51 if min_age < 15 else 0.49  # Slightly more males at birth
            
            age_group = AgeGroup(
                min_age=min_age,
                max_age=max_age,
                population=population,
                male_ratio=male_ratio,
                birth_rate=fertility,
                death_rate=mortality
            )
            age_groups.append(age_group)
        
        return PopulationPyramid(
            age_groups=age_groups,
            total_population=total_population,
            region_id=region_id
        )
    
    def simulate_migration(self, regions: List[str], time_delta: float, conditions: Dict[str, Dict[str, float]]) -> Dict[str, int]:
        """Simulate migration between regions."""
        migration_changes = {region: 0 for region in regions}
        
        for flow in self.migration_flows:
            if flow.source in regions and flow.destination in regions:
                source_conditions = conditions.get(flow.source, {})
                dest_conditions = conditions.get(flow.destination, {})
                
                # Combine conditions for migration calculation
                combined_conditions = {
                    'push_strength': source_conditions.get('push_factor', 1.0),
                    'pull_strength': dest_conditions.get('pull_factor', 1.0)
                }
                
                migrants = int(flow.get_effective_rate(time_delta, combined_conditions))
                
                # Ensure we don't migrate more than source population
                source_pop = self._get_current_population(flow.source)
                migrants = min(migrants, source_pop // 10)  # Max 10% of population
                
                migration_changes[flow.source] -= migrants
                migration_changes[flow.destination] += migrants
        
        return migration_changes
    
    def _get_current_population(self, region: str) -> int:
        """Get current population for a region."""
        if region in self.population_history and self.population_history[region]:
            return self.population_history[region][-1][1]
        return 1000  # Default population
    
    def calculate_carrying_capacity(self, area: float, development: float, resources: float, technology: float = 1.0) -> int:
        """Calculate carrying capacity of a region."""
        base_capacity = area * 50  # Base capacity per square km
        
        # Development increases capacity (infrastructure, organization)
        development_multiplier = 1.0 + (development * 3.0)
        
        # Resource availability affects capacity
        resource_multiplier = min(3.0, 1.0 + resources * 0.5)
        
        # Technology level significantly impacts capacity
        tech_multiplier = 1.0 + (technology * 2.0)
        
        capacity = int(base_capacity * development_multiplier * resource_multiplier * tech_multiplier)
        
        # Urban areas have higher density capacity
        if development > 0.7:  # Highly developed = urban
            capacity *= 5
        
        return capacity
    
    def predict_population_trend(self, current_population: int, time_periods: int, birth_rate: float, death_rate: float, migration_trend: Optional[List[int]] = None) -> List[int]:
        """Predict population trend over multiple time periods."""
        trend = [current_population]
        migration_trend = migration_trend or [0] * time_periods
        
        for period in range(time_periods):
            migration_net = migration_trend[period] if period < len(migration_trend) else 0
            next_pop = self.calculate_growth(
                trend[-1], 
                1.0, 
                birth_rate, 
                death_rate, 
                migration_net
            )
            trend.append(next_pop)
        
        return trend
    
    def calculate_demographic_transition_phase(self, birth_rate: float, death_rate: float, growth_rate: float, urbanization: float) -> int:
        """Determine demographic transition phase (1-4)."""
        if birth_rate > 0.03 and death_rate > 0.025:
            return 1  # Pre-transition: high birth and death rates
        elif birth_rate > 0.03 and death_rate < 0.02:
            return 2  # Early transition: death rate falls, birth rate high
        elif birth_rate < 0.025 and death_rate < 0.015:
            return 3  # Late transition: birth rate falls
        elif birth_rate < 0.015 and death_rate < 0.01:
            return 4  # Post-transition: low birth and death rates
        else:
            return 2  # Default to early transition


class MigrationCalculator:
    """Specialized calculator for migration patterns."""
    
    def __init__(self):
        self.push_factors = ['poverty', 'conflict', 'oppression', 'disaster', 'unemployment']
        self.pull_factors = ['opportunity', 'safety', 'freedom', 'family', 'education']
        self.barrier_factors = ['distance', 'borders', 'cost', 'language', 'culture']
    
    def calculate_migration_potential(self, source_conditions: Dict[str, float], destination_conditions: Dict[str, float], distance: float, barriers: List[str] = None) -> float:
        """Calculate migration potential between two regions."""
        barriers = barriers or []
        
        # Calculate push factors (reasons to leave source)
        push_strength = sum(source_conditions.get(factor, 0.0) for factor in self.push_factors)
        push_strength /= len(self.push_factors)  # Average
        
        # Calculate pull factors (reasons to go to destination)
        pull_strength = sum(destination_conditions.get(factor, 0.0) for factor in self.pull_factors)
        pull_strength /= len(self.pull_factors)  # Average
        
        # Distance acts as a barrier
        distance_factor = 1.0 / (1.0 + distance * 0.01)  # 1% reduction per unit distance
        
        # Additional barriers reduce migration
        barrier_factor = 1.0 / (1.0 + len(barriers) * 0.3)
        
        # Combined migration potential
        migration_potential = (push_strength + pull_strength) * distance_factor * barrier_factor
        
        return min(1.0, max(0.0, migration_potential))
    
    def estimate_migration_flow(self, source_population: int, destination_population: int, migration_potential: float, existing_connections: float = 0.5) -> float:
        """Estimate the actual migration flow rate."""
        # Gravity model: migration proportional to product of populations
        gravity_factor = (source_population * destination_population) / 1000000
        
        # Existing connections (diaspora, trade) increase migration
        connection_multiplier = 1.0 + existing_connections * 2.0
        
        base_flow = gravity_factor * migration_potential * connection_multiplier
        
        # Convert to people per year
        return base_flow * 100  # Scaling factor
    
    def calculate_network_effects(self, existing_migrants: int, total_population: int) -> float:
        """Calculate network effects on migration (friends and relatives effect)."""
        if total_population == 0:
            return 1.0
        
        migrant_ratio = existing_migrants / total_population
        # Network effect increases with existing migrant population
        return 1.0 + migrant_ratio * 5.0


class UrbanizationModel:
    """Model for urbanization trends."""
    
    def __init__(self):
        self.urban_growth_rates: Dict[str, float] = {}
        self.rural_decline_rates: Dict[str, float] = {}
    
    def calculate_urbanization_rate(self, gdp_per_capita: float, industrialization: float, infrastructure: float) -> float:
        """Calculate urbanization rate based on economic factors."""
        # S-curve model for urbanization
        base_rate = 0.2  # Base urbanization rate
        
        # GDP effect (logistic)
        gdp_effect = 1.0 / (1.0 + math.exp(-0.001 * (gdp_per_capita - 3000)))
        
        # Industrialization effect
        industry_effect = industrialization * 0.8
        
        # Infrastructure effect
        infra_effect = infrastructure * 0.5
        
        urbanization_rate = base_rate + gdp_effect + industry_effect + infra_effect
        
        return min(0.95, max(0.05, urbanization_rate))
    
    def project_urban_population(self, total_population: int, urbanization_rate: float, time_periods: int) -> Tuple[List[int], List[int]]:
        """Project urban and rural population trends."""
        urban_pop = [int(total_population * urbanization_rate)]
        rural_pop = [total_population - urban_pop[0]]
        
        for _ in range(time_periods):
            # Urban growth (typically faster)
            urban_growth = urban_pop[-1] * 0.025  # 2.5% urban growth
            # Rural decline or slower growth
            rural_change = rural_pop[-1] * 0.005  # 0.5% rural growth
            
            new_urban = int(urban_pop[-1] + urban_growth)
            new_rural = int(rural_pop[-1] + rural_change)
            
            # Ensure total consistency
            total = new_urban + new_rural
            if total != urban_pop[-1] + rural_pop[-1] + int(urban_growth + rural_change):
                # Adjust for rounding errors
                new_rural = total_population - new_urban
            
            urban_pop.append(new_urban)
            rural_pop.append(new_rural)
        
        return urban_pop, rural_pop