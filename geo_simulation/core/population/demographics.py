"""Population composition"""

# Module implementation

"""
Advanced demographic simulation with age groups, ethnicity, education, and social classes.
"""

from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass
from enum import Enum
import random
from datetime import datetime

class EducationLevel(Enum):
    """Education levels of population."""
    ILLITERATE = 0
    BASIC = 1
    SECONDARY = 2
    HIGHER = 3
    ADVANCED = 4

class SocialClass(Enum):
    """Social classes in the society."""
    LOWER = 0
    WORKING = 1
    MIDDLE = 2
    UPPER_MIDDLE = 3
    ELITE = 4

@dataclass
class AgeGroup:
    """Represents an age group in the population."""
    min_age: int
    max_age: int
    population: int
    birth_rate: float
    death_rate: float
    workforce_participation: float

@dataclass
class EthnicGroup:
    """Represents an ethnic group in the population."""
    name: str
    population: int
    growth_rate: float
    assimilation_rate: float
    cultural_influence: float

class PopulationManager:
    """Manages population dynamics across the civilization."""
    
    def __init__(self):
        self.total_population = 1000
        self.age_groups = self._create_default_age_groups()
        self.ethnic_groups = self._create_default_ethnic_groups()
        self.education_distribution = self._create_education_distribution()
        self.social_class_distribution = self._create_social_class_distribution()
        self.urbanization_rate = 0.3
        self.literacy_rate = 0.5
        self.average_age = 25.0
        
    def _create_default_age_groups(self) -> List[AgeGroup]:
        return [
            AgeGroup(0, 14, 250, 0.0, 0.005, 0.0),
            AgeGroup(15, 24, 180, 0.0, 0.001, 0.6),
            AgeGroup(25, 44, 300, 0.0, 0.002, 0.8),
            AgeGroup(45, 64, 200, 0.0, 0.005, 0.7),
            AgeGroup(65, 100, 70, 0.0, 0.02, 0.1)
        ]
    
    def _create_default_ethnic_groups(self) -> List[EthnicGroup]:
        return [
            EthnicGroup("Native", 700, 0.01, 0.0),
            EthnicGroup("Minority1", 200, 0.015, 0.05),
            EthnicGroup("Minority2", 100, 0.012, 0.03)
        ]
    
    def _create_education_distribution(self) -> Dict[EducationLevel, float]:
        return {
            EducationLevel.ILLITERATE: 0.3,
            EducationLevel.BASIC: 0.4,
            EducationLevel.SECONDARY: 0.2,
            EducationLevel.HIGHER: 0.08,
            EducationLevel.ADVANCED: 0.02
        }
    
    def _create_social_class_distribution(self) -> Dict[SocialClass, float]:
        return {
            SocialClass.LOWER: 0.4,
            SocialClass.WORKING: 0.3,
            SocialClass.MIDDLE: 0.2,
            SocialClass.UPPER_MIDDLE: 0.08,
            SocialClass.ELITE: 0.02
        }
    
    def update_population(self, time_delta: float, economic_conditions: Dict) -> Dict:
        total_births = self._calculate_births(time_delta, economic_conditions)
        total_deaths = self._calculate_deaths(time_delta, economic_conditions)
        migration = self._calculate_migration(time_delta, economic_conditions)
        
        self.total_population = max(0, self.total_population + total_births - total_deaths + migration)
        self._update_age_groups(total_births, total_deaths)
        
        return {
            'births': total_births,
            'deaths': total_deaths,
            'migration': migration,
            'total_population': self.total_population
        }
    
    def _calculate_births(self, time_delta: float, economic_conditions: Dict) -> int:
        base_birth_rate = 0.02
        economic_factor = 1.0 - (economic_conditions.get('unemployment', 0.5) * 0.5)
        return int(self.total_population * base_birth_rate * economic_factor * time_delta / 365)
    
    def _calculate_deaths(self, time_delta: float, economic_conditions: Dict) -> int:
        base_death_rate = 0.008
        healthcare_factor = 1.0 - (economic_conditions.get('healthcare_quality', 0.5) * 0.3)
        return int(self.total_population * base_death_rate * healthcare_factor * time_delta / 365)
    
    def _calculate_migration(self, time_delta: float, economic_conditions: Dict) -> int:
        migration_attractiveness = economic_conditions.get('gdp_per_capita', 0.5) * 0.4
        base_migration_rate = (migration_attractiveness - 0.5) * 0.01
        return int(self.total_population * base_migration_rate * time_delta / 365)
    
    def _update_age_groups(self, births: int, deaths: int):
        # Simplified age group update
        self.age_groups[0].population += births
        for group in self.age_groups:
            group_deaths = min(group.population, int(deaths * (group.population / self.total_population)))
            group.population -= group_deaths
    
    def get_workforce_size(self) -> int:
        workforce = 0
        for group in self.age_groups:
            if group.min_age >= 15 and group.max_age <= 64:
                workforce += int(group.population * group.workforce_participation)
        return workforce
    
    def to_dict(self) -> Dict:
        return {
            'total_population': self.total_population,
            'workforce_size': self.get_workforce_size(),
            'urbanization_rate': self.urbanization_rate,
            'literacy_rate': self.literacy_rate,
            'average_age': self.average_age
        }

class DemographicProfile:
    """Comprehensive demographic profile of a population."""
    
    def __init__(self, total_population: int = 1000):
        self.total_population = total_population
        self.age_groups = self._create_default_age_groups()
        self.ethnic_groups = self._create_default_ethnic_groups()
        self.education_distribution = self._create_education_distribution()
        self.social_class_distribution = self._create_social_class_distribution()
        self.urbanization_rate = 0.3
        self.literacy_rate = 0.5
        self.average_age = 25.0
        
    def _create_default_age_groups(self) -> List[AgeGroup]:
        """Create default age group distribution."""
        return [
            AgeGroup(0, 14, int(self.total_population * 0.25), 0.0, 0.005, 0.0),   # Children
            AgeGroup(15, 24, int(self.total_population * 0.18), 0.0, 0.001, 0.6),   # Young adults
            AgeGroup(25, 44, int(self.total_population * 0.30), 0.0, 0.002, 0.8),   # Adults
            AgeGroup(45, 64, int(self.total_population * 0.20), 0.0, 0.005, 0.7),   # Middle-aged
            AgeGroup(65, 100, int(self.total_population * 0.07), 0.0, 0.02, 0.1)    # Elderly
        ]
    
    def _create_default_ethnic_groups(self) -> List[EthnicGroup]:
        """Create default ethnic group distribution."""
        return [
            EthnicGroup("Native", int(self.total_population * 0.7), 0.01, 0.0, 0.8),
            EthnicGroup("Minority1", int(self.total_population * 0.2), 0.015, 0.05, 0.3),
            EthnicGroup("Minority2", int(self.total_population * 0.1), 0.012, 0.03, 0.2)
        ]
    
    def _create_education_distribution(self) -> Dict[EducationLevel, float]:
        """Create default education distribution."""
        return {
            EducationLevel.ILLITERATE: 0.3,
            EducationLevel.BASIC: 0.4,
            EducationLevel.SECONDARY: 0.2,
            EducationLevel.HIGHER: 0.08,
            EducationLevel.ADVANCED: 0.02
        }
    
    def _create_social_class_distribution(self) -> Dict[SocialClass, float]:
        """Create default social class distribution."""
        return {
            SocialClass.LOWER: 0.4,
            SocialClass.WORKING: 0.3,
            SocialClass.MIDDLE: 0.2,
            SocialClass.UPPER_MIDDLE: 0.08,
            SocialClass.ELITE: 0.02
        }
    
    def update_population(self, time_delta: float, economic_conditions: Dict):
        """Update population based on time and economic conditions."""
        # Calculate births
        total_births = self._calculate_births(time_delta, economic_conditions)
        
        # Calculate deaths
        total_deaths = self._calculate_deaths(time_delta, economic_conditions)
        
        # Calculate migration
        migration = self._calculate_migration(time_delta, economic_conditions)
        
        # Update total population
        self.total_population = max(0, self.total_population + total_births - total_deaths + migration)
        
        # Update age groups
        self._update_age_groups(total_births, total_deaths, time_delta)
        
        # Update ethnic groups
        self._update_ethnic_groups(time_delta)
        
        # Update education and social mobility
        self._update_social_mobility(time_delta, economic_conditions)
        
        return {
            'births': total_births,
            'deaths': total_deaths,
            'migration': migration,
            'growth_rate': (total_births - total_deaths + migration) / max(1, self.total_population)
        }
    
    def _calculate_births(self, time_delta: float, economic_conditions: Dict) -> int:
        """Calculate number of births based on economic conditions."""
        base_birth_rate = 0.02  # 2% annual birth rate
        economic_factor = 1.0 - (economic_conditions.get('unemployment', 0.5) * 0.5)
        education_factor = 1.0 - (self.education_distribution[EducationLevel.HIGHER] * 0.3)
        
        effective_birth_rate = base_birth_rate * economic_factor * education_factor
        return int(self.total_population * effective_birth_rate * time_delta / 365)
    
    def _calculate_deaths(self, time_delta: float, economic_conditions: Dict) -> int:
        """Calculate number of deaths based on economic conditions."""
        base_death_rate = 0.008  # 0.8% annual death rate
        healthcare_factor = 1.0 - (economic_conditions.get('healthcare_quality', 0.5) * 0.3)
        nutrition_factor = 1.0 - (economic_conditions.get('food_supply', 0.8) * 0.2)
        
        effective_death_rate = base_death_rate * healthcare_factor * nutrition_factor
        return int(self.total_population * effective_death_rate * time_delta / 365)
    
    def _calculate_migration(self, time_delta: float, economic_conditions: Dict) -> int:
        """Calculate net migration based on economic conditions."""
        migration_attractiveness = (
            economic_conditions.get('gdp_per_capita', 0.5) * 0.4 +
            economic_conditions.get('safety', 0.7) * 0.3 +
            economic_conditions.get('freedom', 0.6) * 0.3
        )
        
        base_migration_rate = (migration_attractiveness - 0.5) * 0.01
        return int(self.total_population * base_migration_rate * time_delta / 365)
    
    def get_workforce_size(self) -> int:
        """Calculate total workforce size."""
        workforce = 0
        for age_group in self.age_groups:
            if age_group.min_age >= 15 and age_group.max_age <= 64:
                workforce += int(age_group.population * age_group.workforce_participation)
        return workforce
    
    def get_dependency_ratio(self) -> float:
        """Calculate dependency ratio (non-workers / workers)."""
        workers = self.get_workforce_size()
        non_workers = self.total_population - workers
        return non_workers / max(1, workers)
    
    def to_dict(self) -> Dict:
        """Convert demographic profile to dictionary."""
        return {
            'total_population': self.total_population,
            'workforce_size': self.get_workforce_size(),
            'dependency_ratio': self.get_dependency_ratio(),
            'urbanization_rate': self.urbanization_rate,
            'literacy_rate': self.literacy_rate,
            'average_age': self.average_age,
            'ethnic_diversity': len(self.ethnic_groups)
        }

class PopulationManager:
    """Manages population dynamics across the civilization."""
    
    def __init__(self):
        self.demographics = {}
        self.migration_network = {}
        self.population_policies = {}
        self.health_system = HealthSystem()
        self.populations = []
        
    def initialize_region(self, region_id: str, initial_population: int = 1000):
        """Initialize population for a region."""
        self.demographics[region_id] = DemographicProfile(initial_population)
        
    def update_all_regions(self, time_delta: float, economic_conditions: Dict):
        """Update all regional populations."""
        results = {}
        
        for region_id, demographics in self.demographics.items():
            region_economics = economic_conditions.get(region_id, {})
            results[region_id] = demographics.update_population(time_delta, region_economics)
        
        # Handle migration between regions
        self._handle_migration(time_delta)
        
        return results
    
    def _handle_migration(self, time_delta: float):
        """Handle migration between regions based on attractiveness."""
        # This would implement complex migration patterns
        pass

    def to_dict(self):
        return {
            "total": sum(p.size for p in self.populations),
            "groups": [p.to_dict() for p in self.populations]
        }

class HealthSystem:
    """Manages health and disease dynamics."""
    
    def __init__(self):
        self.diseases = {}
        self.healthcare_quality = 0.5
        self.sanitation_level = 0.6
        self.nutrition_level = 0.7
        
    def update_diseases(self, time_delta: float, population: int):
        """Update disease spread and impact."""
        # Simulate disease outbreaks and spread
        pass
    
    def calculate_health_metrics(self) -> Dict:
        """Calculate health-related metrics."""
        return {
            'life_expectancy': 70.0 + (self.healthcare_quality * 15),
            'infant_mortality': 0.05 - (self.healthcare_quality * 0.03),
            'disease_prevalence': 0.1 - (self.sanitation_level * 0.08)
        }