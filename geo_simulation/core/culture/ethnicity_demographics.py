"""Racial and ethnic composition"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
import random

class EthnicityStability(Enum):
    HOMOGENEOUS = "homogeneous"    # 90%+ one ethnicity
    DOMINANT = "dominant"          # 70-90% one ethnicity
    MIXED = "mixed"               # 30-70% majority
    PLURALISTIC = "pluralistic"   # No majority, multiple significant groups
    DIVERSE = "diverse"           # Many small groups

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
import random
import math

@dataclass
class InfiniteEthnicGroup:
    """Ethnic group that can evolve without genetic limits"""
    group_id: str
    name: str
    physical_tendencies: Dict[str, float]  # Not fixed traits, but tendencies
    cultural_affinity: str
    population: int
    adaptive_capacity: float  # Ability to evolve new characteristics
    
    # Infinite evolution metrics
    genetic_diversity: float = 1.0
    phenotypic_plasticity: float = 0.5
    evolutionary_potential: float = 0.3
    
    def evolve_characteristics(self, environmental_pressures: Dict[str, float]):
        """Evolve physical characteristics based on environment"""
        for pressure, intensity in environmental_pressures.items():
            if intensity > 0.3:
                # Pressure can drive evolution of new characteristics
                evolution_chance = intensity * self.adaptive_capacity * 0.1
                
                if random.random() < evolution_chance:
                    self._develop_new_trait(pressure, intensity)
    
    def _develop_new_trait(self, pressure: str, intensity: float):
        """Develop new physical trait in response to pressure"""
        trait_domains = {
            "cold": ["cold_resistance", "insulation", "metabolic_efficiency"],
            "heat": ["heat_tolerance", "sweat_efficiency", "vascular_adaptation"],
            "altitude": ["oxygen_efficiency", "lung_capacity", "hemoglobin_optimization"],
            "radiation": ["dna_repair", "melanin_variation", "cellular_protection"],
            "disease": ["immune_adaptation", "pathogen_resistance", "microbiome_specialization"]
        }
        
        relevant_traits = trait_domains.get(pressure, [f"adaptation_{pressure}"])
        new_trait = random.choice(relevant_traits)
        
        # Add or enhance trait
        current = self.physical_tendencies.get(new_trait, 0.0)
        enhancement = intensity * 0.2
        self.physical_tendencies[new_trait] = min(1.0, current + enhancement)
        
        # Increase diversity
        self.genetic_diversity += 0.1

class InfiniteDemographicSystem:
    """Demographic system that scales infinitely"""
    
    def __init__(self, region_id: str):
        self.region_id = region_id
        self.ethnic_groups: Dict[str, InfiniteEthnicGroup] = {}
        self.total_population = 0
        self.carrying_capacity = 1000000  # Can be increased infinitely
        self.demographic_complexity = 1.0
        
        # Infinite scaling metrics
        self.social_integration = 1.0
        self.cultural_synthesis = 0.0
        self.genetic_amalgamation = 0.0
    
    def add_ethnic_group(self, group: InfiniteEthnicGroup):
        """Add ethnic group to the system"""
        self.ethnic_groups[group.group_id] = group
        self._update_population()
        self._update_demographic_metrics()
    
    def _update_population(self):
        """Update total population"""
        self.total_population = sum(group.population for group in self.ethnic_groups.values())
    
    def _update_demographic_metrics(self):
        """Update infinite scaling metrics"""
        group_count = len(self.ethnic_groups)
        
        # Complexity increases with diversity but has diminishing returns
        self.demographic_complexity = 1.0 + math.log(1.0 + group_count) * 2.0
        
        # Social integration potential
        self.social_integration = 1.0 / (1.0 + group_count * 0.1)
        
        # Cultural synthesis increases over time
        self.cultural_synthesis += 0.01
    
    def simulate_demographic_evolution(self, environmental_conditions: Dict[str, float]):
        """Simulate infinite demographic evolution"""
        # Population changes
        for group in self.ethnic_groups.values():
            # Population growth with carrying capacity limits
            growth_factor = (self.carrying_capacity - self.total_population) / self.carrying_capacity
            base_growth = random.uniform(-0.01, 0.02)
            actual_growth = base_growth * max(0.1, growth_factor)
            
            group.population = max(1, int(group.population * (1.0 + actual_growth)))
            
            # Evolutionary pressures
            group.evolve_characteristics(environmental_conditions)
        
        self._update_population()
        
        # Inter-group dynamics
        self._simulate_intergroup_dynamics()
        
        # Carrying capacity can increase with technology
        self._expand_carrying_capacity()
    
    def _simulate_intergroup_dynamics(self):
        """Simulate infinite inter-group relationships"""
        if len(self.ethnic_groups) < 2:
            return
        
        groups = list(self.ethnic_groups.values())
        
        for i, group1 in enumerate(groups):
            for group2 in groups[i+1:]:
                # Contact between groups
                contact_intensity = min(group1.population, group2.population) / self.total_population
                
                if contact_intensity > 0.1 and random.random() < contact_intensity:
                    # Cultural exchange
                    self.cultural_synthesis += contact_intensity * 0.01
                    
                    # Genetic mixing at higher contact levels
                    if contact_intensity > 0.3 and random.random() < 0.05:
                        self._genetic_mixing(group1, group2)
    
    def _genetic_mixing(self, group1: InfiniteEthnicGroup, group2: InfiniteEthnicGroup):
        """Genetic mixing creating new diversity"""
        # Create synthetic traits from both groups
        for trait, value in group1.physical_tendencies.items():
            if trait in group2.physical_tendencies:
                # Blend traits
                blended_value = (value + group2.physical_tendencies[trait]) / 2
                group1.physical_tendencies[trait] = blended_value
                group2.physical_tendencies[trait] = blended_value
        
        # Increase both groups' diversity
        group1.genetic_diversity += 0.05
        group2.genetic_diversity += 0.05
        
        self.genetic_amalgamation += 0.01
    
    def _expand_carrying_capacity(self):
        """Expand carrying capacity infinitely with development"""
        # Carrying capacity grows with demographic complexity
        capacity_growth = self.demographic_complexity * 1000
        self.carrying_capacity += capacity_growth
        
        # No upper limit - can support infinite population in theory

@dataclass
class EthnicGroup:
    group_id: str
    name: str
    physical_traits: Dict[str, str]  # skin_tone, hair_color, etc.
    cultural_affinity: str  # Primary culture ID
    population: int
    birth_rate: float
    assimilation_rate: float  # 0-1, tendency to assimilate into dominant culture
    segregation_level: float  # 0-1, how segregated from other groups
    
    @property
    def growth_potential(self) -> float:
        """Calculate population growth potential"""
        base_growth = self.birth_rate - 0.02  # Base death rate
        # Higher segregation can lead to higher growth in some cases
        segregation_effect = self.segregation_level * 0.1 if self.segregation_level > 0.7 else 0.0
        return max(-0.1, base_growth + segregation_effect)

class DemographicProfile:
    """Tracks the ethnic and demographic composition of a population"""
    
    def __init__(self, region_id: str):
        self.region_id = region_id
        self.ethnic_groups: Dict[str, EthnicGroup] = {}
        self.total_population = 0
        self.demographic_trends: Dict[str, float] = {}  # trend -> strength
        self.migration_flows: Dict[Tuple[str, str], int] = {}  # (from, to) -> amount
        self.urbanization_rate = 0.0
        self.age_distribution: Dict[str, float] = {  # age_group -> proportion
            'children': 0.25,
            'young_adults': 0.30,
            'adults': 0.30,
            'seniors': 0.15
        }
    
    def add_ethnic_group(self, group: EthnicGroup):
        """Add an ethnic group to the demographic profile"""
        self.ethnic_groups[group.group_id] = group
        self._update_total_population()
    
    def _update_total_population(self):
        """Update total population count"""
        self.total_population = sum(group.population for group in self.ethnic_groups.values())
    
    def calculate_ethnic_stability(self) -> EthnicityStability:
        """Calculate the ethnic stability of the region"""
        if not self.ethnic_groups:
            return EthnicityStability.HOMOGENEOUS
        
        populations = [group.population for group in self.ethnic_groups.values()]
        total = sum(populations)
        
        if total == 0:
            return EthnicityStability.HOMOGENEOUS
        
        largest_group = max(populations)
        proportion = largest_group / total
        
        if proportion >= 0.9:
            return EthnicityStability.HOMOGENEOUS
        elif proportion >= 0.7:
            return EthnicityStability.DOMINANT
        elif proportion >= 0.3:
            return EthnicityStability.MIXED
        elif len(self.ethnic_groups) <= 3:
            return EthnicityStability.PLURALISTIC
        else:
            return EthnicityStability.DIVERSE
    
    def simulate_demographic_changes(self) -> Dict[str, int]:
        """Simulate population changes for one turn"""
        changes = {}
        
        for group_id, group in self.ethnic_groups.items():
            # Natural population change
            growth = group.population * group.growth_potential
            new_population = max(0, int(group.population + growth))
            changes[group_id] = new_population - group.population
            group.population = new_population
            
            # Assimilation effects
            if len(self.ethnic_groups) > 1:
                self._simulate_assimilation(group_id)
        
        self._update_total_population()
        self._update_demographic_trends(changes)
        
        return changes
    
    def _simulate_assimilation(self, group_id: str):
        """Simulate assimilation of ethnic groups"""
        group = self.ethnic_groups[group_id]
        
        # Higher assimilation rate and lower segregation increase assimilation
        assimilation_chance = group.assimilation_rate * (1.0 - group.segregation_level) * 0.1
        
        if random.random() < assimilation_chance and len(self.ethnic_groups) > 1:
            # Find dominant group to assimilate into
            dominant_group = max(self.ethnic_groups.values(), key=lambda g: g.population)
            if dominant_group.group_id != group_id:
                # Transfer some population
                assimilation_amount = int(group.population * 0.01)  # 1% assimilate
                if assimilation_amount > 0:
                    group.population -= assimilation_amount
                    dominant_group.population += assimilation_amount
                    
                    # Cultural traits may transfer slightly
                    print(f"📊 {assimilation_amount} {group.name} assimilated into {dominant_group.name}")
    
    def _update_demographic_trends(self, changes: Dict[str, int]):
        """Update long-term demographic trends"""
        for group_id, change in changes.items():
            if self.total_population > 0:
                change_proportion = change / self.total_population
                
                trend_key = f"{group_id}_growth"
                if trend_key not in self.demographic_trends:
                    self.demographic_trends[trend_key] = 0.0
                
                # Update trend with smoothing
                self.demographic_trends[trend_key] = (
                    self.demographic_trends[trend_key] * 0.8 + change_proportion * 0.2
                )
    
    def calculate_social_tension(self) -> float:
        """Calculate social tension based on ethnic composition"""
        stability = self.calculate_ethnic_stability()
        
        # More diverse populations can have higher tension
        tension_base = {
            EthnicityStability.HOMOGENEOUS: 0.1,
            EthnicityStability.DOMINANT: 0.3,
            EthnicityStability.MIXED: 0.5,
            EthnicityStability.PLURALISTIC: 0.7,
            EthnicityStability.DIVERSE: 0.6
        }
        
        tension = tension_base.get(stability, 0.5)
        
        # High segregation increases tension
        avg_segregation = sum(g.segregation_level for g in self.ethnic_groups.values()) / len(self.ethnic_groups)
        tension += avg_segregation * 0.3
        
        # Large population disparities increase tension
        if len(self.ethnic_groups) > 1:
            populations = [g.population for g in self.ethnic_groups.values()]
            disparity = max(populations) / min(populations)
            tension += min(0.3, disparity * 0.1)
        
        return min(1.0, tension)
    
    def implement_integration_policy(self, policy_strength: float) -> Dict[str, float]:
        """Implement policies to promote ethnic integration"""
        results = {}
        
        for group_id, group in self.ethnic_groups.items():
            old_segregation = group.segregation_level
            group.segregation_level = max(0.0, group.segregation_level - policy_strength * 0.2)
            results[group_id] = old_segregation - group.segregation_level
        
        return results

class EthnicityManager:
    """Manages ethnic demographics across civilizations"""
    
    def __init__(self):
        self.regional_demographics: Dict[str, DemographicProfile] = {}
        self.global_migration_trends: Dict[str, float] = {}  # region -> migration attractiveness
        self.historical_migrations: List[Dict] = []
    
    def add_region(self, region_id: str) -> DemographicProfile:
        """Add a new region for demographic tracking"""
        profile = DemographicProfile(region_id)
        self.regional_demographics[region_id] = profile
        return profile
    
    def simulate_global_demographics(self):
        """Simulate global demographic changes"""
        migration_flows = {}
        
        # Calculate migration attractiveness for each region
        attractiveness = {}
        for region_id, profile in self.regional_demographics.items():
            # Factors: low tension, economic opportunity, stability
            tension = profile.calculate_social_tension()
            stability_score = 1.0 - tension
            economic_opportunity = random.uniform(0.3, 0.8)  # Would come from economy module
            
            attractiveness[region_id] = stability_score * 0.6 + economic_opportunity * 0.4
        
        # Simulate migration between regions
        for from_region, from_profile in self.regional_demographics.items():
            from_attractiveness = attractiveness.get(from_region, 0.5)
            
            for to_region, to_profile in self.regional_demographics.items():
                if from_region != to_region:
                    to_attractiveness = attractiveness.get(to_region, 0.5)
                    
                    # Migration occurs from less attractive to more attractive regions
                    if to_attractiveness > from_attractiveness:
                        migration_drive = to_attractiveness - from_attractiveness
                        
                        # Some ethnic groups migrate more readily
                        for group_id, group in from_profile.ethnic_groups.items():
                            migration_chance = group.assimilation_rate * migration_drive * 0.1
                            
                            if random.random() < migration_chance and group.population > 100:
                                migrants = int(group.population * 0.01)  # 1% migrate
                                if migrants > 0:
                                    # Remove from source
                                    group.population -= migrants
                                    
                                    # Add to destination (or create new group)
                                    if group_id in to_profile.ethnic_groups:
                                        to_profile.ethnic_groups[group_id].population += migrants
                                    else:
                                        new_group = EthnicGroup(
                                            group_id=group_id,
                                            name=group.name,
                                            physical_traits=group.physical_traits.copy(),
                                            cultural_affinity=group.cultural_affinity,
                                            population=migrants,
                                            birth_rate=group.birth_rate,
                                            assimilation_rate=group.assimilation_rate,
                                            segregation_level=group.segregation_level
                                        )
                                        to_profile.add_ethnic_group(new_group)
                                    
                                    # Record migration
                                    key = (from_region, to_region)
                                    migration_flows[key] = migration_flows.get(key, 0) + migrants
                                    
                                    self.historical_migrations.append({
                                        'from_region': from_region,
                                        'to_region': to_region,
                                        'ethnic_group': group_id,
                                        'population': migrants,
                                        'turn': len(self.historical_migrations)
                                    })
        
        # Update all regional populations
        for profile in self.regional_demographics.values():
            profile._update_total_population()
        
        return migration_flows