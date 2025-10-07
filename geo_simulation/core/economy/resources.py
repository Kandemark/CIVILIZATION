"""Economic resources"""

# Module implementation

from typing import Dict, List, Set, Optional
from enum import Enum
import random
from base_economy import ResourceType, DevelopmentLevel

class ResourceTier(Enum):
    BASIC = "basic"           # Food, wood, stone
    STRATEGIC = "strategic"   # Iron, coal, oil
    LUXURY = "luxury"         # Silk, jewels, wine
    ADVANCED = "advanced"     # Uranium, electronics, advanced materials
    DIGITAL = "digital"       # Data, processing power, bandwidth

class Resource:
    def __init__(self, resource_type: ResourceType, base_value: float, abundance: float = 1.0,
                 tier: ResourceTier = ResourceTier.BASIC, required_tech: Optional[str] = None):
        self.type = resource_type
        self.base_value = base_value
        self.abundance = abundance  # 0.0 to 1.0
        self.tier = tier
        self.required_tech = required_tech
        self.demand = 1.0
        self.supply = 1.0
        self.price_volatility = 0.1
        self.global_reserves = 1000000  # Total available in world
        self.extracted = 0
        self.depletion_rate = 0.001  # Rate at which abundance decreases
        
    @property
    def scarcity(self) -> float:
        """Calculate resource scarcity (0-1, higher = more scarce)"""
        return 1.0 - (self.extracted / self.global_reserves) if self.global_reserves > 0 else 1.0
    
    @property
    def current_value(self) -> float:
        """Calculate current market value with volatility and scarcity"""
        base_price = self.base_value * (self.demand / max(self.supply, 0.1))
        scarcity_multiplier = 1.0 + (self.scarcity * 2.0)  # Double price at full scarcity
        volatility_factor = 1.0 + random.uniform(-self.price_volatility, self.price_volatility)
        
        return base_price * scarcity_multiplier * volatility_factor * (1.0 / self.abundance)
    
    def extract(self, amount: float) -> float:
        """Extract resources, returns actual amount extracted considering abundance"""
        if self.global_reserves <= 0:
            return 0.0
            
        actual_amount = amount * self.abundance
        actual_amount = min(actual_amount, self.global_reserves)
        
        self.extracted += actual_amount
        self.global_reserves -= actual_amount
        
        # Deplete abundance over time
        if actual_amount > 0:
            depletion = (actual_amount / 10000) * self.depletion_rate
            self.abundance = max(0.01, self.abundance - depletion)
            
        return actual_amount

class ResourceManager:
    def __init__(self):
        self.resources: Dict[ResourceType, Resource] = {}
        self.regional_variations: Dict[str, Dict[ResourceType, float]] = {}  # Region -> resource abundance multipliers
        self.initialize_comprehensive_resources()
    
    def initialize_comprehensive_resources(self):
        """Initialize all game resources with realistic values"""
        resources_data = {
            # Basic Resources
            ResourceType.FOOD: (2.0, 0.8, ResourceTier.BASIC, None),
            ResourceType.WOOD: (3.0, 0.7, ResourceTier.BASIC, None),
            ResourceType.STONE: (4.0, 0.6, ResourceTier.BASIC, None),
            ResourceType.WATER: (1.0, 0.9, ResourceTier.BASIC, None),
            
            # Strategic Resources
            ResourceType.IRON: (10.0, 0.4, ResourceTier.STRATEGIC, "iron_working"),
            ResourceType.COAL: (8.0, 0.5, ResourceTier.STRATEGIC, "industrialization"),
            ResourceType.OIL: (15.0, 0.3, ResourceTier.STRATEGIC, "oil_drilling"),
            ResourceType.URANIUM: (100.0, 0.1, ResourceTier.STRATEGIC, "nuclear_physics"),
            ResourceType.ALUMINUM: (12.0, 0.4, ResourceTier.STRATEGIC, "electrolysis"),
            ResourceType.COPPER: (7.0, 0.5, ResourceTier.STRATEGIC, "metalworking"),
            
            # Luxury Resources
            ResourceType.SILK: (25.0, 0.3, ResourceTier.LUXURY, "textile_production"),
            ResourceType.SPICES: (30.0, 0.2, ResourceTier.LUXURY, "trade_routes"),
            ResourceType.WINE: (20.0, 0.4, ResourceTier.LUXURY, "viticulture"),
            ResourceType.JEWELS: (80.0, 0.1, ResourceTier.LUXURY, "gem_cutting"),
            ResourceType.GOLD: (50.0, 0.2, ResourceTier.LUXURY, "mining"),
            ResourceType.SILVER: (40.0, 0.3, ResourceTier.LUXURY, "mining"),
            ResourceType.IVORY: (60.0, 0.15, ResourceTier.LUXURY, "hunting"),
            ResourceType.FURS: (35.0, 0.25, ResourceTier.LUXURY, "trapping"),
            
            # Advanced Resources
            ResourceType.ELECTRONICS: (45.0, 0.3, ResourceTier.ADVANCED, "electronics"),
            ResourceType.MACHINERY: (35.0, 0.4, ResourceTier.ADVANCED, "advanced_engineering"),
            ResourceType.CHEMICALS: (25.0, 0.5, ResourceTier.ADVANCED, "chemistry"),
            ResourceType.PHARMACEUTICALS: (70.0, 0.2, ResourceTier.ADVANCED, "biochemistry"),
            
            # Digital Resources
            ResourceType.DATA: (20.0, 0.6, ResourceTier.DIGITAL, "information_technology"),
            ResourceType.COMPUTING_POWER: (15.0, 0.7, ResourceTier.DIGITAL, "computing"),
            ResourceType.BANDWIDTH: (10.0, 0.8, ResourceTier.DIGITAL, "telecommunications"),
        }
        
        for res_type, (value, abundance, tier, tech) in resources_data.items():
            self.resources[res_type] = Resource(res_type, value, abundance, tier, tech)
    
    def get_resources_by_tier(self, tier: ResourceTier) -> List[Resource]:
        """Get all resources of a specific tier"""
        return [res for res in self.resources.values() if res.tier == tier]
    
    def get_available_resources(self, development_level: DevelopmentLevel, known_techs: Set[str]) -> List[ResourceType]:
        """Get resources available based on development level and technology"""
        available = []
        
        for resource in self.resources.values():
            # Check if required technology is known
            if resource.required_tech and resource.required_tech not in known_techs:
                continue
                
            # Check development level requirements
            if (resource.tier == ResourceTier.ADVANCED and 
                development_level.value < DevelopmentLevel.INDUSTRIAL.value):
                continue
            elif (resource.tier == ResourceTier.DIGITAL and 
                  development_level.value < DevelopmentLevel.DIGITAL.value):
                continue
                  
            available.append(resource.type)
            
        return available
    
    def update_global_supply_demand(self, global_demand: Dict[ResourceType, float], 
                                  global_supply: Dict[ResourceType, float]):
        """Update global supply and demand dynamics"""
        for res_type, resource in self.resources.items():
            resource.demand = global_demand.get(res_type, 1.0)
            resource.supply = global_supply.get(res_type, 1.0)
            
            # Adjust volatility based on supply/demand imbalance
            imbalance = abs(resource.demand - resource.supply) / max(resource.demand, resource.supply)
            resource.price_volatility = 0.1 + (imbalance * 0.3)  # More volatility with imbalance
    
    def add_regional_variation(self, region_id: str, resource_abundance: Dict[ResourceType, float]):
        """Add regional variations in resource abundance"""
        self.regional_variations[region_id] = resource_abundance
    
    def get_regional_abundance(self, region_id: str, resource_type: ResourceType) -> float:
        """Get abundance multiplier for a resource in a specific region"""
        regional = self.regional_variations.get(region_id, {})
        return regional.get(resource_type, 1.0)