"""Optional city development"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple, Any
from dataclasses import dataclass
from enum import Enum
import random
from base_economy import ResourceType, EconomicEntity, DevelopmentLevel

class CitySize(Enum):
    HAMLET = "hamlet"      # < 1,000
    VILLAGE = "village"    # 1,000 - 5,000
    TOWN = "town"          # 5,000 - 20,000
    CITY = "city"          # 20,000 - 100,000
    METROPOLIS = "metropolis" # 100,000 - 1,000,000
    MEGALOPOLIS = "megalopolis" # > 1,000,000

class UrbanizationLevel(Enum):
    RURAL = "rural"           # < 20% urban
    TRANSITIONAL = "transitional" # 20-50% urban
    URBAN = "urban"           # 50-80% urban
    HIGHLY_URBAN = "highly_urban" # > 80% urban

@dataclass
class DemographicSegment:
    segment: str
    population: int
    birth_rate: float
    death_rate: float
    migration_tendency: float  # -1 to 1, negative = emigrate, positive = immigrate
    economic_activity: float   # 0-1, economic participation

class City(EconomicEntity):
    """Represents a city with complex economic and demographic dynamics"""
    
    def __init__(self, city_id: str, name: str, region_id: str, base_population: int = 1000,
                 development_level: DevelopmentLevel = DevelopmentLevel.AGRARIAN):
        super().__init__(city_id, name, development_level)
        self.region_id = region_id
        self.population = base_population
        self.demographics: Dict[str, DemographicSegment] = self._initialize_demographics()
        self.city_size = self._calculate_city_size()
        self.urbanization_level = UrbanizationLevel.RURAL
        
        # Economic characteristics
        self.employment_rate = 0.95
        self.average_income = 1000
        self.cost_of_living = 1.0
        self.housing_availability = 1.0
        self.public_services_quality = 0.7
        
        # Growth factors
        self.natural_growth_rate = 0.01
        self.migration_rate = 0.0
        self.carrying_capacity = base_population * 2
        
        # Specializations
        self.industrial_sectors: Dict[str, float] = {}  # sector -> strength
        self.commercial_activity = 0.5
        self.tourism_potential = 0.0
        
        # Urban infrastructure
        self.land_usage: Dict[str, float] = {  # percentage allocation
            'residential': 0.4,
            'commercial': 0.2,
            'industrial': 0.2,
            'agricultural': 0.1,
            'public': 0.1
        }
    
    def _initialize_demographics(self) -> Dict[str, DemographicSegment]:
        """Initialize demographic segments"""
        return {
            'children': DemographicSegment('children', int(self.population * 0.2), 0.02, 0.005, -0.1, 0.0),
            'young_adults': DemographicSegment('young_adults', int(self.population * 0.3), 0.03, 0.002, 0.3, 0.8),
            'adults': DemographicSegment('adults', int(self.population * 0.35), 0.015, 0.005, 0.1, 0.9),
            'seniors': DemographicSegment('seniors', int(self.population * 0.15), 0.0, 0.02, -0.2, 0.3)
        }
    
    def _calculate_city_size(self) -> CitySize:
        """Calculate city size based on population"""
        if self.population < 1000:
            return CitySize.HAMLET
        elif self.population < 5000:
            return CitySize.VILLAGE
        elif self.population < 20000:
            return CitySize.TOWN
        elif self.population < 100000:
            return CitySize.CITY
        elif self.population < 1000000:
            return CitySize.METROPOLIS
        else:
            return CitySize.MEGALOPOLIS
    
    def calculate_growth_potential(self, regional_economy) -> Dict[str, float]:
        """Calculate city growth potential based on various factors"""
        factors = {}
        
        # Economic opportunities
        job_opportunities = regional_economy.get('job_growth', 0.02)
        factors['economic'] = job_opportunities * 2
        
        # Quality of life
        housing_factor = min(1.0, self.housing_availability)
        services_factor = self.public_services_quality
        factors['quality_of_life'] = (housing_factor + services_factor) / 2
        
        # Infrastructure capacity
        infrastructure_quality = regional_economy.get('infrastructure_quality', 0.5)
        factors['infrastructure'] = infrastructure_quality
        
        # Carrying capacity pressure
        capacity_pressure = 1.0 - (self.population / self.carrying_capacity)
        factors['capacity'] = max(0.0, capacity_pressure)
        
        return factors
    
    def simulate_demographic_changes(self, regional_economy: Dict[str, float]) -> Dict[str, int]:
        """Simulate population changes through birth, death, and migration"""
        changes = {}
        total_population_change = 0
        
        # Natural population changes
        for segment_name, segment in self.demographics.items():
            # Births and deaths
            births = segment.population * segment.birth_rate
            deaths = segment.population * segment.death_rate
            natural_change = births - deaths
            
            # Migration based on economic opportunities and quality of life
            migration_potential = segment.migration_tendency * regional_economy.get('migration_pull', 0.0)
            migration = segment.population * migration_potential * 0.1
            
            segment_change = natural_change + migration
            segment.population = max(0, int(segment.population + segment_change))
            total_population_change += segment_change
            
            changes[segment_name] = segment_change
        
        # Update total population
        old_population = self.population
        self.population = max(100, int(self.population + total_population_change))
        
        # Update city size
        self.city_size = self._calculate_city_size()
        
        changes['total'] = self.population - old_population
        return changes
    
    def calculate_economic_output(self) -> float:
        """Calculate city's economic output (GDP)"""
        base_output = 0
        
        # Calculate output from employed population
        employed_population = self.population * self.employment_rate
        for segment in self.demographics.values():
            segment_workers = segment.population * segment.economic_activity
            segment_output = segment_workers * self.average_income
            base_output += segment_output
        
        # Apply development level multiplier
        dev_multipliers = {
            DevelopmentLevel.PRIMITIVE: 0.5,
            DevelopmentLevel.AGRARIAN: 0.8,
            DevelopmentLevel.FEUDAL: 1.0,
            DevelopmentLevel.MERCANTILE: 1.3,
            DevelopmentLevel.INDUSTRIAL: 2.0,
            DevelopmentLevel.POST_INDUSTRIAL: 3.0,
            DevelopmentLevel.DIGITAL: 4.0,
            DevelopmentLevel.AUTOMATED: 5.0
        }
        
        multiplier = dev_multipliers.get(self.development_level, 1.0)
        
        # Apply industrial specialization bonuses
        specialization_bonus = 1.0 + sum(self.industrial_sectors.values()) * 0.1
        
        return base_output * multiplier * specialization_bonus
    
    def update_urbanization(self, regional_development: Dict[str, float]):
        """Update urbanization level based on regional development"""
        urban_population_share = regional_development.get('urban_population_share', 0.1)
        
        if urban_population_share < 0.2:
            self.urbanization_level = UrbanizationLevel.RURAL
        elif urban_population_share < 0.5:
            self.urbanization_level = UrbanizationLevel.TRANSITIONAL
        elif urban_population_share < 0.8:
            self.urbanization_level = UrbanizationLevel.URBAN
        else:
            self.urbanization_level = UrbanizationLevel.HIGHLY_URBAN
    
    def simulate_turn(self, global_economy) -> Dict[str, Any]:
        """Simulate one turn for the city"""
        regional_economy = global_economy.get_regional_data(self.region_id)
        
        # Demographic changes
        demographic_changes = self.simulate_demographic_changes(regional_economy)
        
        # Economic calculations
        self.gdp = self.calculate_economic_output()
        
        # Update urbanization
        self.update_urbanization(regional_economy)
        
        # Update employment based on economic conditions
        economic_health = regional_economy.get('economic_health', 0.7)
        self.employment_rate = 0.9 * economic_health + 0.05  # 5-95% range
        
        # Adjust cost of living based on population pressure
        population_pressure = self.population / self.carrying_capacity
        self.cost_of_living = 0.5 + (population_pressure * 0.5)
        
        return {
            'demographic_changes': demographic_changes,
            'gdp': self.gdp,
            'employment_rate': self.employment_rate,
            'city_size': self.city_size,
            'urbanization_level': self.urbanization_level
        }

class RegionalEconomy:
    """Represents a regional economy containing multiple cities"""
    
    def __init__(self, region_id: str, name: str, development_level: DevelopmentLevel):
        self.region_id = region_id
        self.name = name
        self.development_level = development_level
        self.cities: Dict[str, City] = {}
        self.rural_population = 0
        self.regional_gdp = 0
        self.urbanization_rate = 0.1  # 10% urban initially
        
        # Regional characteristics
        self.geography = "plains"  # plains, mountains, coastal, etc.
        self.climate = "temperate"
        self.natural_resources: Dict[ResourceType, float] = {}
        self.transportation_network = 0.5
        
        # Economic integration
        self.trade_connectivity = 0.3
        self.labor_mobility = 0.4
        self.capital_mobility = 0.5
    
    def add_city(self, city: City):
        """Add a city to the region"""
        self.cities[city.id] = city
    
    def calculate_regional_economy(self) -> Dict[str, float]:
        """Calculate regional economic indicators"""
        total_population = self.rural_population
        urban_population = 0
        total_gdp = 0
        
        for city in self.cities.values():
            total_population += city.population
            urban_population += city.population
            total_gdp += city.gdp
        
        # Rural economic contribution
        rural_gdp = self.rural_population * 500  # Base rural productivity
        total_gdp += rural_gdp
        
        # Update urbanization rate
        if total_population > 0:
            self.urbanization_rate = urban_population / total_population
        
        self.regional_gdp = total_gdp
        
        return {
            'total_population': total_population,
            'urban_population': urban_population,
            'rural_population': self.rural_population,
            'urbanization_rate': self.urbanization_rate,
            'regional_gdp': total_gdp,
            'gdp_per_capita': total_gdp / total_population if total_population > 0 else 0,
            'economic_health': self._calculate_economic_health(),
            'migration_pull': self._calculate_migration_pull(),
            'job_growth': self._calculate_job_growth()
        }
    
    def _calculate_economic_health(self) -> float:
        """Calculate regional economic health (0-1)"""
        health_indicators = []
        
        # GDP growth indicator (simplified)
        health_indicators.append(0.7)  # Placeholder
        
        # Employment indicator
        avg_employment = sum(city.employment_rate for city in self.cities.values()) / max(len(self.cities), 1)
        health_indicators.append(avg_employment)
        
        # Urbanization efficiency
        urbanization_efficiency = min(1.0, self.urbanization_rate * 1.2)
        health_indicators.append(urbanization_efficiency)
        
        return sum(health_indicators) / len(health_indicators)
    
    def _calculate_migration_pull(self) -> float:
        """Calculate migration attractiveness of the region"""
        pull_factors = []
        
        # Economic opportunities
        avg_income = sum(city.average_income for city in self.cities.values()) / max(len(self.cities), 1)
        pull_factors.append(min(1.0, avg_income / 2000))
        
        # Employment opportunities
        avg_employment = sum(city.employment_rate for city in self.cities.values()) / max(len(self.cities), 1)
        pull_factors.append(avg_employment)
        
        # Quality of life (simplified)
        avg_cost_of_living = sum(city.cost_of_living for city in self.cities.values()) / max(len(self.cities), 1)
        quality_of_life = 1.0 - (avg_cost_of_living - 1.0)  # Lower cost = higher quality
        pull_factors.append(max(0.0, quality_of_life))
        
        return sum(pull_factors) / len(pull_factors)
    
    def _calculate_job_growth(self) -> float:
        """Calculate job growth rate in the region"""
        # Base growth based on development level
        base_growth = {
            DevelopmentLevel.PRIMITIVE: 0.005,
            DevelopmentLevel.AGRARIAN: 0.01,
            DevelopmentLevel.FEUDAL: 0.015,
            DevelopmentLevel.MERCANTILE: 0.02,
            DevelopmentLevel.INDUSTRIAL: 0.03,
            DevelopmentLevel.POST_INDUSTRIAL: 0.04,
            DevelopmentLevel.DIGITAL: 0.05,
            DevelopmentLevel.AUTOMATED: 0.05,

        }
        return base_growth.get(self.development_level, 0.01)
    
    def simulate_turn(self, global_economy) -> Dict[str, Any]:
        """Simulate one turn for the regional economy"""
        regional_data = self.calculate_regional_economy()
        city_reports = {}
        for city in self.cities.values():
            report = city.simulate_turn(global_economy)
            city_reports[city.id] = report
            return report
        return {
            'regional_data': regional_data,
            'city_reports': city_reports,
            'regional_gdp': self.regional_gdp,
            'urbanization_rate': self.urbanization_rate,
            'total_population': regional_data['total_population'],
            'rural_population': self.rural_population,
            'economic_health': regional_data['economic_health'],
            'migration_pull': regional_data['migration_pull'],
            'job_growth': regional_data['job_growth'],
            'gdp_per_capita': regional_data['gdp_per_capita'],
            'average_income': sum(city.average_income for city in self.cities.values()) / max(len(self.cities), 1)
        }