"""Supply, demand, and prices"""

# Module implementation

"""
Sophisticated economic system with supply-demand dynamics, markets, and trade.
"""

from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass
from enum import Enum
import random
import numpy as np

class ResourceType(Enum):
    """Types of economic resources."""
    FOOD = "food"
    WOOD = "wood"
    STONE = "stone"
    IRON = "iron"
    GOLD = "gold"
    OIL = "oil"
    URANIUM = "uranium"
    CONSUMER_GOODS = "consumer_goods"
    LUXURY_GOODS = "luxury_goods"

class IndustryType(Enum):
    """Types of industries."""
    AGRICULTURE = "agriculture"
    MINING = "mining"
    MANUFACTURING = "manufacturing"
    CONSTRUCTION = "construction"
    SERVICES = "services"
    TECHNOLOGY = "technology"
    FINANCE = "finance"

@dataclass
class Resource:
    """Represents an economic resource."""
    type: ResourceType
    base_price: float
    supply: float
    demand: float
    volatility: float  # Price volatility factor

@dataclass
class Industry:
    """Represents an economic industry."""
    type: IndustryType
    size: float  # 0.0 to 1.0
    productivity: float
    employment: int
    capital_investment: float

class MarketEconomy:
    """Simulates market economy with supply-demand dynamics."""
    
    def __init__(self):
        self.resources = self._initialize_resources()
        self.industries = self._initialize_industries()
        self.gdp = 1000.0
        self.inflation_rate = 0.02
        self.unemployment_rate = 0.05
        self.tax_rate = 0.2
        self.gdp_per_capita = 0.0
        
    def _initialize_resources(self) -> Dict[ResourceType, Resource]:
        """Initialize economic resources."""
        return {
            ResourceType.FOOD: Resource(ResourceType.FOOD, 1.0, 1000, 800, 0.1),
            ResourceType.WOOD: Resource(ResourceType.WOOD, 2.0, 500, 400, 0.15),
            ResourceType.STONE: Resource(ResourceType.STONE, 3.0, 300, 250, 0.12),
            ResourceType.IRON: Resource(ResourceType.IRON, 5.0, 200, 180, 0.2),
            ResourceType.GOLD: Resource(ResourceType.GOLD, 50.0, 50, 45, 0.3),
            ResourceType.OIL: Resource(ResourceType.OIL, 8.0, 150, 140, 0.25),
            ResourceType.URANIUM: Resource(ResourceType.URANIUM, 100.0, 20, 15, 0.4),
            ResourceType.CONSUMER_GOODS: Resource(ResourceType.CONSUMER_GOODS, 4.0, 400, 380, 0.1),
            ResourceType.LUXURY_GOODS: Resource(ResourceType.LUXURY_GOODS, 20.0, 80, 75, 0.15)
        }
    
    def _initialize_industries(self) -> Dict[IndustryType, Industry]:
        """Initialize economic industries."""
        return {
            IndustryType.AGRICULTURE: Industry(IndustryType.AGRICULTURE, 0.3, 0.7, 300, 500),
            IndustryType.MINING: Industry(IndustryType.MINING, 0.2, 0.6, 200, 800),
            IndustryType.MANUFACTURING: Industry(IndustryType.MANUFACTURING, 0.25, 0.8, 250, 1200),
            IndustryType.CONSTRUCTION: Industry(IndustryType.CONSTRUCTION, 0.1, 0.65, 100, 600),
            IndustryType.SERVICES: Industry(IndustryType.SERVICES, 0.08, 0.75, 80, 400),
            IndustryType.TECHNOLOGY: Industry(IndustryType.TECHNOLOGY, 0.05, 0.9, 50, 1500),
            IndustryType.FINANCE: Industry(IndustryType.FINANCE, 0.02, 0.95, 20, 2000)
        }
    
    def update_market(self, time_delta: float, population_data: Dict, tech_level: float):
        """Update market conditions based on time and external factors."""
        # Update supply and demand based on population and technology
        self._update_supply_demand(population_data, tech_level)
        
        # Calculate new prices based on market dynamics
        self._calculate_prices()
        
        # Update industries
        self._update_industries(time_delta, tech_level)
        
        # Calculate economic indicators
        self._calculate_economic_indicators(population_data)
        
        return self.get_economic_report()
    
    def _update_supply_demand(self, population_data: Dict, tech_level: float):
        """Update supply and demand for all resources."""
        population_size = population_data.get('total_population', 1000)
        workforce_size = population_data.get('workforce_size', 500)
        
        for resource in self.resources.values():
            # Basic demand increases with population
            base_demand = population_size * 0.1
            
            # Adjust based on resource type
            if resource.type == ResourceType.FOOD:
                resource.demand = base_demand * 2.0
            elif resource.type == ResourceType.CONSUMER_GOODS:
                resource.demand = base_demand * 0.8 * (1 + tech_level * 0.5)
            elif resource.type == ResourceType.LUXURY_GOODS:
                resource.demand = base_demand * 0.1 * (1 + tech_level)
            else:
                resource.demand = base_demand * 0.5
            
            # Supply increases with technology and workforce
            resource.supply = workforce_size * 0.2 * (1 + tech_level * 0.3)
            
            # Add some randomness
            resource.demand *= random.uniform(0.9, 1.1)
            resource.supply *= random.uniform(0.95, 1.05)
    
    def _calculate_prices(self):
        """Calculate prices based on supply-demand balance."""
        for resource in self.resources.values():
            # Basic price calculation based on supply-demand ratio
            supply_demand_ratio = resource.supply / max(1, resource.demand)
            price_factor = 1.0 / max(0.1, supply_demand_ratio)
            
            # Apply volatility
            volatility_effect = 1.0 + random.uniform(-resource.volatility, resource.volatility)
            
            new_price = resource.base_price * price_factor * volatility_effect
            resource.base_price = max(0.1, new_price)  # Prevent negative prices
    
    def _update_industries(self, time_delta: float, tech_level: float):
        """Update industry performance."""
        total_employment = 0
        total_production = 0
        
        for industry in self.industries.values():
            # Productivity increases with technology
            industry.productivity = min(1.0, industry.productivity + tech_level * 0.01 * time_delta)
            
            # Calculate production
            production = industry.size * industry.productivity * industry.employment
            total_production += production
            total_employment += industry.employment
            
            # Industry growth based on profitability
            profitability = random.uniform(0.8, 1.2)
            industry.size = max(0.01, min(1.0, industry.size * profitability))
        
        self.unemployment_rate = max(0.01, 1.0 - (total_employment / 1000))
        self.gdp = total_production * (1 + tech_level)
    
    def _calculate_economic_indicators(self, population_data: Dict):
        """Calculate various economic indicators."""
        # Calculate per capita metrics
        population = max(1, population_data.get('total_population', 1000))
        self.gdp_per_capita = self.gdp / population
        
        # Calculate inflation based on price changes
        total_price_change = sum(
            (resource.base_price - 1.0) * weight 
            for resource, weight in zip(self.resources.values(), [0.3, 0.2, 0.15, 0.1, 0.05, 0.05, 0.05, 0.05, 0.05])
        )
        self.inflation_rate = max(-0.1, min(0.5, total_price_change))
    
    def get_economic_report(self) -> Dict:
        """Generate comprehensive economic report."""
        return {
            'gdp': self.gdp,
            'gdp_per_capita': self.gdp_per_capita,
            'inflation_rate': self.inflation_rate,
            'unemployment_rate': self.unemployment_rate,
            'resource_prices': {r.type.value: r.base_price for r in self.resources.values()},
            'industry_sizes': {i.type.value: i.size for i in self.industries.values()}
        }
    
    def apply_economic_policy(self, policy_type: str, magnitude: float):
        """Apply economic policy with specified magnitude."""
        if policy_type == 'tax_cut':
            self.tax_rate = max(0.1, self.tax_rate - magnitude * 0.1)
            # Stimulate consumption
            for resource in self.resources.values():
                resource.demand *= (1 + magnitude * 0.2)
                
        elif policy_type == 'infrastructure_investment':
            # Boost productivity across industries
            for industry in self.industries.values():
                industry.productivity = min(1.0, industry.productivity + magnitude * 0.1)
                
        elif policy_type == 'research_subsidy':
            # Specifically boost technology industry
            tech_industry = self.industries.get(IndustryType.TECHNOLOGY)
            if tech_industry:
                tech_industry.size = min(1.0, tech_industry.size + magnitude * 0.2)
                tech_industry.productivity = min(1.0, tech_industry.productivity + magnitude * 0.15)

class TradeNetwork:
    """Manages trade between different regions/nations."""
    
    def __init__(self):
        self.trade_routes = {}
        self.tariffs = {}
        self.trade_agreements = {}
        
    def establish_trade_route(self, region1: str, region2: str, resources: List[ResourceType]):
        """Establish a new trade route."""
        route_id = f"{region1}-{region2}"
        self.trade_routes[route_id] = {
            'regions': (region1, region2),
            'resources': resources,
            'volume': 0.0,
            'efficiency': 0.8
        }
    
    def calculate_trade_benefits(self, region1: str, region2: str) -> float:
        """Calculate mutual benefits from trade."""
        # This would use comparative advantage theory
        return random.uniform(0.1, 0.3)
    
    def update_trade_volumes(self, economic_conditions: Dict):
        """Update trade volumes based on economic conditions."""
        for route_id, route in self.trade_routes.items():
            region1, region2 = route['regions']
            econ1 = economic_conditions.get(region1, {})
            econ2 = economic_conditions.get(region2, {})
            
            # Trade volume based on economic size and complementarity
            base_volume = (econ1.get('gdp', 1000) + econ2.get('gdp', 1000)) * 0.01
            route['volume'] = base_volume * route['efficiency']

    def impose_tariff(self, region1: str, region2: str, rate: float):
        """Impose tariff on trade between two regions."""
        route_id = f"{region1}-{region2}"
        self.tariffs[route_id] = rate
        if route_id in self.trade_routes:
            self.trade_routes[route_id]['efficiency'] *= (1 - rate)

    @property
    def gdp_per_capita(self):
        population = 1
        if self.population_manager:
            total_pop = sum(p.size for p in self.population_manager.populations)
            population = max(total_pop, 1)  # avoid division by zero
        return self.gdp / population
    
    def get_economic_report(self):
        return {
            "gdp": self.gdp,
            "gdp_growth": self.gdp_growth,
            "gdp_per_capita": self.gdp_per_capita,
            "inflation": self.inflation,
            "unemployment": self.unemployment,
        }


