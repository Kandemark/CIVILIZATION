"""Goods production"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
from base_economy import ResourceType, DevelopmentLevel, EconomicEntity
import random

@dataclass
class ProductionRecipe:
    """Defines inputs and outputs for production"""
    recipe_id: str
    name: str
    inputs: Dict[ResourceType, float]  # Required resources
    outputs: Dict[ResourceType, float]  # Produced resources
    base_time: int  # Turns to complete
    required_tech: Optional[str] = None
    workforce_required: int = 10
    development_level: DevelopmentLevel = DevelopmentLevel.PRIMITIVE
    efficiency_improvement: float = 0.0  # Learning curve effect

class ProductionFacility:
    def __init__(self, facility_id: str, name: str, recipe: ProductionRecipe,
                 location: EconomicEntity, base_efficiency: float = 1.0):
        self.id = facility_id
        self.name = name
        self.recipe = recipe
        self.location = location
        self.base_efficiency = base_efficiency
        self.current_efficiency = base_efficiency
        self.workforce = recipe.workforce_required
        self.actual_workforce = 0
        self.active = True
        self.production_queue: List[Dict] = []
        self.experience = 0.0  # Learning curve
        self.condition = 1.0  # Maintenance state
        self.last_output: Dict[ResourceType, float] = {}
        
    @property
    def can_operate(self) -> bool:
        """Check if facility can operate"""
        return (self.active and 
                self.actual_workforce >= self.workforce * 0.5 and  # At least 50% staffed
                self.condition > 0.3)
    
    def calculate_efficiency(self, available_inputs: Dict[ResourceType, float]) -> float:
        """Calculate current production efficiency"""
        if not self.can_operate:
            return 0.0
            
        # Workforce efficiency
        workforce_efficiency = min(1.0, self.actual_workforce / self.workforce)
        
        # Input availability
        input_efficiency = 1.0
        for resource, amount in self.recipe.inputs.items():
            available = available_inputs.get(resource, 0)
            required = amount * workforce_efficiency
            if required > 0:
                input_efficiency = min(input_efficiency, available / required)
        
        # Experience bonus (learning curve)
        experience_bonus = min(0.5, self.experience * 0.01)  # Up to 50% bonus
        
        # Condition effect
        condition_effect = 0.3 + (self.condition * 0.7)  # 30-100% based on condition
        
        return (self.current_efficiency * input_efficiency * workforce_efficiency * 
                condition_effect * (1.0 + experience_bonus))
    
    def produce(self, available_inputs: Dict[ResourceType, float]) -> Dict[ResourceType, float]:
        """Execute production cycle"""
        efficiency = self.calculate_efficiency(available_inputs)
        
        if efficiency <= 0:
            self.last_output = {}
            return {}
        
        # Consume inputs
        consumed_inputs = {}
        for resource, amount in self.recipe.inputs.items():
            consumed = amount * efficiency
            if consumed > 0 and available_inputs.get(resource, 0) >= consumed:
                available_inputs[resource] -= consumed
                consumed_inputs[resource] = consumed
        
        # Generate outputs
        output = {}
        for resource, amount in self.recipe.outputs.items():
            output[resource] = amount * efficiency
        
        # Gain experience
        if efficiency > 0.5:  # Only gain experience when operating reasonably well
            self.experience += efficiency * 0.1
            
        # Improve efficiency through learning
        if self.experience > 100:
            self.current_efficiency = min(2.0, self.base_efficiency * 
                                        (1.0 + self.recipe.efficiency_improvement * 
                                         (self.experience / 1000)))
        
        self.last_output = output
        return output
    
    def degrade(self, degradation_rate: float = 0.01):
        """Degrade facility condition"""
        self.condition = max(0.0, self.condition - degradation_rate)
        if self.condition < 0.3:
            self.active = False

class IndustrySector:
    """Represents an entire industrial sector (agriculture, manufacturing, etc.)"""
    
    def __init__(self, sector_id: str, name: str, development_requirements: List[DevelopmentLevel]):
        self.id = sector_id
        self.name = name
        self.development_requirements = development_requirements
        self.facilities: List[ProductionFacility] = []
        self.employment = 0
        self.total_output: Dict[ResourceType, float] = {}
        self.productivity = 1.0
        
    def add_facility(self, facility: ProductionFacility):
        self.facilities.append(facility)
        self.employment += facility.workforce
    
    def simulate_sector_turn(self, available_inputs: Dict[ResourceType, float]) -> Dict[ResourceType, float]:
        """Simulate production for entire sector"""
        sector_output = {}
        
        for facility in self.facilities:
            if facility.can_operate:
                output = facility.produce(available_inputs.copy())  # Copy to avoid modifying original
                for resource, amount in output.items():
                    sector_output[resource] = sector_output.get(resource, 0) + amount
                
                # Degrade facility
                facility.degrade()
        
        self.total_output = sector_output
        return sector_output

class ProductionManager:
    def __init__(self):
        self.sectors: Dict[str, IndustrySector] = {}
        self.recipes: Dict[str, ProductionRecipe] = {}
        self.initialize_core_recipes()
        self.initialize_industry_sectors()
    
    def initialize_industry_sectors(self):
        """Initialize major industry sectors"""
        sectors_data = [
            ("agriculture", "Agriculture", [DevelopmentLevel.PRIMITIVE, DevelopmentLevel.AGRARIAN]),
            ("mining", "Mining", [DevelopmentLevel.AGRARIAN, DevelopmentLevel.FEUDAL]),
            ("craftsmanship", "Craftsmanship", [DevelopmentLevel.FEUDAL, DevelopmentLevel.MERCANTILE]),
            ("manufacturing", "Manufacturing", [DevelopmentLevel.INDUSTRIAL]),
            ("technology", "Technology", [DevelopmentLevel.POST_INDUSTRIAL, DevelopmentLevel.DIGITAL]),
            ("services", "Services", [DevelopmentLevel.POST_INDUSTRIAL, DevelopmentLevel.DIGITAL]),
        ]
        
        for sector_id, name, dev_levels in sectors_data:
            self.sectors[sector_id] = IndustrySector(sector_id, name, dev_levels)
    
    def initialize_core_recipes(self):
        """Initialize production recipes for all development levels"""
        
        # Primitive recipes
        self.recipes["hunting"] = ProductionRecipe(
            "hunting", "Hunting",
            inputs={},  # No inputs needed for basic hunting
            outputs={ResourceType.FOOD: 10},
            base_time=1,
            development_level=DevelopmentLevel.PRIMITIVE,
            efficiency_improvement=0.1
        )
        
        self.recipes["farming"] = ProductionRecipe(
            "farming", "Farming",
            inputs={ResourceType.FOOD: 1},  # Seeds
            outputs={ResourceType.FOOD: 20},
            base_time=2,
            development_level=DevelopmentLevel.AGRARIAN,
            efficiency_improvement=0.2
        )
        
        self.recipes["woodcutting"] = ProductionRecipe(
            "woodcutting", "Wood Cutting",
            inputs={},
            outputs={ResourceType.WOOD: 15},
            base_time=1,
            development_level=DevelopmentLevel.PRIMITIVE
        )
        
        # Industrial recipes
        self.recipes["steel_production"] = ProductionRecipe(
            "steel_production", "Steel Production",
            inputs={ResourceType.IRON: 10, ResourceType.COAL: 5},
            outputs={ResourceType.MACHINERY: 8},
            base_time=3,
            required_tech="steel_production",
            development_level=DevelopmentLevel.INDUSTRIAL,
            workforce_required=50
        )
        
        self.recipes["electronics"] = ProductionRecipe(
            "electronics", "Electronics Manufacturing",
            inputs={ResourceType.COPPER: 8, ResourceType.ALUMINUM: 5, ResourceType.CHEMICALS: 3},
            outputs={ResourceType.ELECTRONICS: 6},
            base_time=4,
            required_tech="electronics",
            development_level=DevelopmentLevel.DIGITAL,
            workforce_required=30
        )
    
    def get_available_recipes(self, development_level: DevelopmentLevel, 
                            known_techs: Set[str]) -> List[ProductionRecipe]:
        """Get recipes available based on development and technology"""
        available = []
        
        for recipe in self.recipes.values():
            # Check development level
            if recipe.development_level.value > development_level.value:
                continue
                
            # Check technology requirements
            if recipe.required_tech and recipe.required_tech not in known_techs:
                continue
                
            available.append(recipe)
            
        return available
    
    def create_facility(self, recipe_id: str, location: EconomicEntity, 
                       sector_id: str) -> Optional[ProductionFacility]:
        """Create a new production facility"""
        recipe = self.recipes.get(recipe_id)
        if not recipe:
            return None
            
        facility = ProductionFacility(
            f"facility_{len(self.sectors[sector_id].facilities)}",
            f"{recipe.name} Facility",
            recipe,
            location
        )
        
        self.sectors[sector_id].add_facility(facility)
        return facility
    
    def simulate_production_cycle(self, available_resources: Dict[ResourceType, float],
                                development_level: DevelopmentLevel) -> Dict[ResourceType, float]:
        """Run production for all sectors"""
        total_output = {}
        resources = available_resources.copy()
        
        # Process sectors in order of complexity
        sector_order = ["agriculture", "mining", "craftsmanship", "manufacturing", "technology", "services"]
        
        for sector_id in sector_order:
            sector = self.sectors.get(sector_id)
            if sector and any(dev.value <= development_level.value for dev in sector.development_requirements):
                sector_output = sector.simulate_sector_turn(resources)
                
                # Add outputs to available resources for downstream sectors
                for resource, amount in sector_output.items():
                    total_output[resource] = total_output.get(resource, 0) + amount
                    resources[resource] = resources.get(resource, 0) + amount
        
        return total_output