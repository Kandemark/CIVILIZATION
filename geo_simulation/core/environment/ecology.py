"""Environmental health and impact"""

# Module implementation

"""
Ecological systems modeling including species, populations, and ecosystem dynamics.
"""

from dataclasses import dataclass
from typing import List, Dict, Optional, Set
from enum import Enum
import numpy as np

class TrophicLevel(Enum):
    PRODUCER = "producer"
    PRIMARY_CONSUMER = "primary_consumer"
    SECONDARY_CONSUMER = "secondary_consumer"
    TERTIARY_CONSUMER = "tertiary_consumer"
    DECOMPOSER = "decomposer"

class ConservationStatus(Enum):
    LEAST_CONCERN = "LC"
    NEAR_THREATENED = "NT"
    VULNERABLE = "VU"
    ENDANGERED = "EN"
    CRITICALLY_ENDANGERED = "CR"
    EXTINCT_IN_WILD = "EW"
    EXTINCT = "EX"

@dataclass
class Species:
    """Represents a biological species"""
    name: str
    scientific_name: str
    trophic_level: TrophicLevel
    population: int
    growth_rate: float
    carrying_capacity: int
    conservation_status: ConservationStatus
    habitat_requirements: Dict[str, float]  # e.g., {'temperature_range': [10, 30], 'precipitation': 500}
    
    def calculate_population_growth(self, resources_available: float = 1.0) -> float:
        """Calculate population growth using logistic growth model"""
        if self.carrying_capacity <= 0:
            return 0
            
        # Logistic growth: dN/dt = rN(1-N/K)
        growth = self.growth_rate * self.population * (
            1 - (self.population / (self.carrying_capacity * resources_available))
        )
        return growth
    
    def update_population(self, time_step: float = 1.0, resources: float = 1.0) -> None:
        """Update population based on growth model"""
        growth = self.calculate_population_growth(resources)
        new_population = max(0, self.population + growth * time_step)
        self.population = int(new_population)
        
        # Update conservation status based on population
        self._update_conservation_status()
    
    def _update_conservation_status(self) -> None:
        """Update conservation status based on population thresholds"""
        pop_ratio = self.population / self.carrying_capacity
        
        if pop_ratio <= 0.01:
            self.conservation_status = ConservationStatus.EXTINCT
        elif pop_ratio <= 0.05:
            self.conservation_status = ConservationStatus.CRITICALLY_ENDANGERED
        elif pop_ratio <= 0.1:
            self.conservation_status = ConservationStatus.ENDANGERED
        elif pop_ratio <= 0.2:
            self.conservation_status = ConservationStatus.VULNERABLE
        elif pop_ratio <= 0.5:
            self.conservation_status = ConservationStatus.NEAR_THREATENED
        else:
            self.conservation_status = ConservationStatus.LEAST_CONCERN

@dataclass
class TrophicInteraction:
    """Represents predator-prey or competitive relationships"""
    predator: Species
    prey: Species
    interaction_strength: float  # 0-1 scale
    type: str  # 'predation', 'competition', 'mutualism'

class Ecosystem:
    """
    Models an ecological system with multiple species and their interactions
    """
    
    def __init__(self, name: str, area: float, location: str):
        self.name = name
        self.area = area  # km²
        self.location = location
        self.species: List[Species] = []
        self.interactions: List[TrophicInteraction] = []
        self.environmental_conditions: Dict[str, float] = {
            'temperature': 20.0,
            'precipitation': 1000.0,
            'soil_quality': 0.8
        }
        self.biodiversity_index: float = 0.0
        
    def add_species(self, species: Species) -> None:
        """Add a species to the ecosystem"""
        self.species.append(species)
        self._update_biodiversity_index()
    
    def add_interaction(self, interaction: TrophicInteraction) -> None:
        """Add a species interaction to the ecosystem"""
        self.interactions.append(interaction)
    
    def simulate_ecosystem_dynamics(self, time_steps: int = 1) -> None:
        """Simulate ecosystem dynamics over multiple time steps"""
        for _ in range(time_steps):
            # Update each species population
            for species in self.species:
                # Calculate resource availability based on environmental conditions
                resource_availability = self._calculate_resource_availability(species)
                species.update_population(resources=resource_availability)
            
            # Apply species interactions
            self._apply_species_interactions()
            
            # Update biodiversity index
            self._update_biodiversity_index()
    
    def _calculate_resource_availability(self, species: Species) -> float:
        """Calculate resource availability for a species based on environmental conditions"""
        # Simplified resource calculation
        temp_optimum = species.habitat_requirements.get('temperature_optimum', 20)
        temp_range = species.habitat_requirements.get('temperature_range', 10)
        precip_requirement = species.habitat_requirements.get('precipitation', 1000)
        
        # Temperature suitability (Gaussian response)
        temp_diff = abs(self.environmental_conditions['temperature'] - temp_optimum)
        temp_suitability = np.exp(-(temp_diff ** 2) / (2 * (temp_range / 3) ** 2))
        
        # Precipitation suitability (linear response within range)
        precip_diff = abs(self.environmental_conditions['precipitation'] - precip_requirement)
        precip_suitability = max(0, 1 - precip_diff / precip_requirement)
        
        # Overall resource availability
        return (temp_suitability + precip_suitability + self.environmental_conditions['soil_quality']) / 3
    
    def _apply_species_interactions(self) -> None:
        """Apply predator-prey and competitive interactions"""
        for interaction in self.interactions:
            if interaction.type == 'predation':
                # Predator-prey dynamics (simplified Lotka-Volterra)
                prey_consumption = interaction.interaction_strength * interaction.predator.population
                interaction.prey.population = max(0, interaction.prey.population - prey_consumption)
                interaction.predator.population += prey_consumption * 0.1  # Energy transfer efficiency
    
    def _update_biodiversity_index(self) -> None:
        """Calculate Shannon biodiversity index"""
        total_population = sum(species.population for species in self.species)
        if total_population == 0:
            self.biodiversity_index = 0
            return
            
        proportions = [species.population / total_population for species in self.species]
        self.biodiversity_index = -sum(p * np.log(p) for p in proportions if p > 0)
    
    def assess_ecosystem_health(self) -> Dict[str, float]:
        """Comprehensive ecosystem health assessment"""
        health_metrics = {
            'biodiversity_index': self.biodiversity_index,
            'total_biomass': sum(species.population for species in self.species),
            'species_richness': len(self.species),
            'endangered_species_count': sum(1 for s in self.species 
                                          if s.conservation_status.value in ['EN', 'CR', 'VU']),
            'average_population_health': np.mean([
                s.population / s.carrying_capacity for s in self.species 
                if s.carrying_capacity > 0
            ])
        }
        
        # Overall health score (0-1)
        health_metrics['overall_health'] = (
            health_metrics['biodiversity_index'] / 3 +  # Normalized
            min(1, health_metrics['average_population_health']) +
            (1 - health_metrics['endangered_species_count'] / len(self.species))
        ) / 3
        
        return health_metrics
    
    def predict_impact(self, environmental_change: Dict[str, float]) -> Dict[str, float]:
        """Predict impact of environmental changes on ecosystem"""
        original_health = self.assess_ecosystem_health()
        
        # Store original conditions
        original_conditions = self.environmental_conditions.copy()
        
        # Apply changes
        for key, change in environmental_change.items():
            if key in self.environmental_conditions:
                self.environmental_conditions[key] += change
        
        # Simulate short-term response
        self.simulate_ecosystem_dynamics(time_steps=5)
        new_health = self.assess_ecosystem_health()
        
        # Restore original conditions
        self.environmental_conditions = original_conditions
        
        # Calculate impact
        impact = {}
        for key in original_health:
            if key in new_health:
                impact[key] = new_health[key] - original_health[key]
        
        return impact