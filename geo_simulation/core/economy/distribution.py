"""Resource allocation"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
import random
from base_economy import ResourceType, EconomicEntity

class DistributionMethod(Enum):
    MARKET = "market"              # Price-based distribution
    CENTRAL_PLANNING = "central_planning"  # Government allocation
    RATIONING = "rationing"        # Equal distribution
    WEALTH_BASED = "wealth_based"  # Distribution by wealth
    HYBRID = "hybrid"              # Mixed system

class DistributionNetwork:
    """Manages distribution of resources within an economic entity"""
    
    def __init__(self, entity_id: str, distribution_method: DistributionMethod = DistributionMethod.MARKET):
        self.entity_id = entity_id
        self.distribution_method = distribution_method
        self.transportation_efficiency = 1.0
        self.storage_capacity: Dict[ResourceType, float] = {}
        self.current_storage: Dict[ResourceType, float] = {}
        self.distribution_costs: Dict[ResourceType, float] = {}
        self.regional_allocations: Dict[str, Dict[ResourceType, float]] = {}  # region -> allocations
        self.price_controls: Dict[ResourceType, Tuple[Optional[float], Optional[float]]] = {}  # (min, max) prices
        
    def distribute_resources(self, available_resources: Dict[ResourceType, float],
                           population: Dict[str, int], wealth_distribution: Dict[str, float],
                           regional_needs: Dict[str, Dict[ResourceType, float]]) -> Dict[str, Dict[ResourceType, float]]:
        """Distribute resources according to the chosen method"""
        
        if self.distribution_method == DistributionMethod.MARKET:
            return self._market_distribution(available_resources, population, wealth_distribution)
        elif self.distribution_method == DistributionMethod.CENTRAL_PLANNING:
            return self._central_planning_distribution(available_resources, regional_needs)
        elif self.distribution_method == DistributionMethod.RATIONING:
            return self._rationing_distribution(available_resources, population)
        elif self.distribution_method == DistributionMethod.WEALTH_BASED:
            return self._wealth_based_distribution(available_resources, wealth_distribution)
        else:  # HYBRID
            return self._hybrid_distribution(available_resources, population, wealth_distribution, regional_needs)
    
    def _market_distribution(self, available_resources: Dict[ResourceType, float],
                           population: Dict[str, int], wealth_distribution: Dict[str, float]) -> Dict[str, Dict[ResourceType, float]]:
        """Price-based distribution - wealthier regions get more resources"""
        distribution = {}
        total_wealth = sum(wealth_distribution.values())
        
        if total_wealth == 0:
            return self._rationing_distribution(available_resources, population)
        
        for region, wealth in wealth_distribution.items():
            distribution[region] = {}
            wealth_share = wealth / total_wealth
            
            for resource, amount in available_resources.items():
                distribution[region][resource] = amount * wealth_share
        
        return distribution
    
    def _central_planning_distribution(self, available_resources: Dict[ResourceType, float],
                                    regional_needs: Dict[str, Dict[ResourceType, float]]) -> Dict[str, Dict[ResourceType, float]]:
        """Government-planned distribution based on assessed needs"""
        distribution = {}
        total_needs: Dict[ResourceType, float] = {}
        
        # Calculate total needs
        for region_needs in regional_needs.values():
            for resource, need in region_needs.items():
                total_needs[resource] = total_needs.get(resource, 0) + need
        
        # Allocate based on needs and availability
        for region, region_needs in regional_needs.items():
            distribution[region] = {}
            for resource, need in region_needs.items():
                if resource in available_resources and total_needs[resource] > 0:
                    need_share = need / total_needs[resource]
                    distribution[region][resource] = available_resources[resource] * need_share
                else:
                    distribution[region][resource] = 0
        
        return distribution
    
    def _rationing_distribution(self, available_resources: Dict[ResourceType, float],
                              population: Dict[str, int]) -> Dict[str, Dict[ResourceType, float]]:
        """Equal per-capita distribution"""
        distribution = {}
        total_population = sum(population.values())
        
        if total_population == 0:
            return {region: {res: 0 for res in available_resources} for region in population}
        
        for region, pop in population.items():
            distribution[region] = {}
            population_share = pop / total_population
            
            for resource, amount in available_resources.items():
                distribution[region][resource] = amount * population_share
        
        return distribution
    
    def _wealth_based_distribution(self, available_resources: Dict[ResourceType, float],
                                 wealth_distribution: Dict[str, float]) -> Dict[str, Dict[ResourceType, float]]:
        """Distribution purely based on wealth (extreme capitalism)"""
        return self._market_distribution(available_resources, {}, wealth_distribution)
    
    def _hybrid_distribution(self, available_resources: Dict[ResourceType, float],
                           population: Dict[str, int], wealth_distribution: Dict[str, float],
                           regional_needs: Dict[str, Dict[ResourceType, float]]) -> Dict[str, Dict[ResourceType, float]]:
        """Mixed distribution system"""
        # 50% market-based, 30% needs-based, 20% rationing
        market_dist = self._market_distribution(available_resources, population, wealth_distribution)
        needs_dist = self._central_planning_distribution(available_resources, regional_needs)
        ration_dist = self._rationing_distribution(available_resources, population)
        
        distribution = {}
        for region in population:
            distribution[region] = {}
            for resource in available_resources:
                market_amt = market_dist.get(region, {}).get(resource, 0)
                needs_amt = needs_dist.get(region, {}).get(resource, 0)
                ration_amt = ration_dist.get(region, {}).get(resource, 0)
                
                distribution[region][resource] = (market_amt * 0.5 + needs_amt * 0.3 + ration_amt * 0.2)
        
        return distribution
    
    def apply_price_controls(self, resource: ResourceType, market_price: float) -> float:
        """Apply price controls if they exist"""
        if resource in self.price_controls:
            min_price, max_price = self.price_controls[resource]
            if min_price is not None and market_price < min_price:
                return min_price
            if max_price is not None and market_price > max_price:
                return max_price
        return market_price
    
    def set_price_control(self, resource: ResourceType, min_price: Optional[float] = None, 
                         max_price: Optional[float] = None):
        """Set price controls for a resource"""
        self.price_controls[resource] = (min_price, max_price)
    
    def calculate_storage_needs(self, production: Dict[ResourceType, float], 
                              consumption: Dict[ResourceType, float]) -> Dict[ResourceType, float]:
        """Calculate optimal storage levels to smooth supply/demand"""
        storage_needs = {}
        
        for resource in set(production.keys()) | set(consumption.keys()):
            net_flow = production.get(resource, 0) - consumption.get(resource, 0)
            # Store 10% of annual consumption as buffer
            annual_consumption = consumption.get(resource, 0) * 12  # Assuming monthly turns
            storage_needs[resource] = annual_consumption * 0.1
            
            # Adjust for production surplus/deficit
            if net_flow > 0:
                storage_needs[resource] += net_flow * 3  # 3 turns of surplus
        
        return storage_needs
    
    def simulate_storage_decay(self, decay_rates: Dict[ResourceType, float]):
        """Simulate natural decay of stored resources"""
        for resource, amount in self.current_storage.items():
            decay_rate = decay_rates.get(resource, 0.01)  # 1% default decay
            decay_amount = amount * decay_rate
            self.current_storage[resource] = max(0, amount - decay_amount)

class LogisticsNetwork:
    """Manages physical transportation of goods"""
    
    def __init__(self):
        self.transport_routes: Dict[Tuple[str, str], float] = {}  # (origin, dest) -> capacity
        self.transport_costs: Dict[Tuple[str, str], float] = {}   # cost per unit distance
        self.infrastructure_level: Dict[str, float] = {}          # region -> infrastructure quality
        self.vehicle_fleets: Dict[str, Dict[str, int]] = {}       # region -> {vehicle_type: count}
        
    def calculate_transport_cost(self, origin: str, destination: str, 
                               resource: ResourceType, amount: float) -> float:
        """Calculate cost to transport goods"""
        if (origin, destination) not in self.transport_costs:
            return amount * 0.1  # Default cost
            
        base_cost = self.transport_costs[(origin, destination)]
        infrastructure_penalty = 1.0 - self.infrastructure_level.get(origin, 0.5)
        
        # Different resources have different transport costs
        resource_multiplier = self._get_resource_transport_multiplier(resource)
        
        return base_cost * amount * resource_multiplier * (1.0 + infrastructure_penalty)
    
    def _get_resource_transport_multiplier(self, resource: ResourceType) -> float:
        """Get transport cost multiplier for different resource types"""
        multipliers = {
            ResourceType.FOOD: 1.2,      # Perishable
            ResourceType.WOOD: 1.0,      # Bulk
            ResourceType.STONE: 1.5,     # Heavy
            ResourceType.IRON: 1.3,      # Heavy
            ResourceType.GOLD: 0.8,      # High value, small volume
            ResourceType.OIL: 1.4,       # Requires special handling
            ResourceType.ELECTRONICS: 0.9, # High value, fragile
        }
        return multipliers.get(resource, 1.0)
    
    def can_transport(self, origin: str, destination: str, amount: float) -> bool:
        """Check if transport route has sufficient capacity"""
        route_capacity = self.transport_routes.get((origin, destination), 0)
        return route_capacity >= amount
    
    def add_transport_route(self, origin: str, destination: str, capacity: float, cost: float):
        """Add a new transport route"""
        self.transport_routes[(origin, destination)] = capacity
        self.transport_costs[(origin, destination)] = cost
    
    def improve_infrastructure(self, region: str, improvement: float = 0.1):
        """Improve infrastructure in a region"""
        current = self.infrastructure_level.get(region, 0.5)
        self.infrastructure_level[region] = min(1.0, current + improvement)