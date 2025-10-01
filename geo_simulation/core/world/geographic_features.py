"""Mountains, rivers, oceans as barriers/enablers"""

# Module implementation

"""
Geographic Features System for Natural Environments

Handles natural features, resources, climate, and environmental factors.
"""

from enum import Enum, auto
from typing import Dict, List, Optional, Set, Tuple, Any
from dataclasses import dataclass, field
from datetime import datetime, timedelta
import logging
import random
from collections import defaultdict
import math

logger = logging.getLogger(__name__)


class FeatureType(Enum):
    """Types of geographic features."""
    MOUNTAIN = auto()
    RIVER = auto()
    FOREST = auto()
    DESERT = auto()
    OCEAN = auto()
    LAKE = auto()
    VOLCANO = auto()
    CAVE = auto()
    CANYON = auto()
    GLACIER = auto()
    SWAMP = auto()
    REEF = auto()


class ResourceType(Enum):
    """Types of natural resources."""
    IRON = auto()
    GOLD = auto()
    SILVER = auto()
    COPPER = auto()
    COAL = auto()
    OIL = auto()
    URANIUM = auto()
    WOOD = auto()
    FOOD = auto()
    WATER = auto()
    FISH = auto()
    GEMS = auto()
    SPICES = auto()
    SILK = auto()


class ClimateZone(Enum):
    """Climate zones and biomes."""
    POLAR = auto()
    TUNDRA = auto()
    BOREAL = auto()
    TEMPERATE = auto()
    MEDITERRANEAN = auto()
    STEPPE = auto()
    DESERT = auto()
    SAVANNA = auto()
    TROPICAL = auto()
    RAINFOREST = auto()
    ALPINE = auto()
    MARINE = auto()


@dataclass
class GeographicFeature:
    """A natural geographic feature."""
    id: str
    name: str
    feature_type: FeatureType
    location: Tuple[float, float]
    size: float  # Relative size/importance
    elevation: float = 0.0
    climate: ClimateZone = ClimateZone.TEMPERATE
    resources: Dict[ResourceType, float] = field(default_factory=dict)
    accessibility: float = 1.0  # 0-1.0 (1 = easily accessible)
    discovery_date: Optional[datetime] = None
    discovered_by: Optional[str] = None
    settlements_nearby: List[str] = field(default_factory=list)
    environmental_value: float = 1.0  # Ecological importance


@dataclass
class ResourceDeposit:
    """A deposit of natural resources."""
    id: str
    resource_type: ResourceType
    location: Tuple[float, float]
    quantity: float  # Total available amount
    quality: float = 1.0  # 0-1.0 purity/quality
    extraction_cost: float = 1.0  # Difficulty to extract
    discovery_date: Optional[datetime] = None
    currently_exploited: bool = False
    exploitation_rate: float = 0.0  # Amount extracted per time unit
    owner: Optional[str] = None


@dataclass
class ClimateRegion:
    """A region with specific climate characteristics."""
    id: str
    climate_zone: ClimateZone
    boundaries: List[Tuple[float, float]]  # Polygon points
    average_temperature: float  # Celsius
    precipitation: float  # mm/year
    growing_season: int  # Days per year
    fertility: float = 0.5  # 0-1.0 agricultural potential
    hazards: List[str] = field(default_factory=list)  # Natural hazards


class GeographicFeatures:
    """
    Manages geographic features, resources, and climate systems.
    """
    
    def __init__(self, world_size: Tuple[int, int] = (1000, 1000)):
        self.world_size = world_size
        self.features: Dict[str, GeographicFeature] = {}
        self.resource_deposits: Dict[str, ResourceDeposit] = {}
        self.climate_regions: Dict[str, ClimateRegion] = {}
        self.feature_locations: Dict[Tuple[int, int], List[str]] = defaultdict(list)
        
        self._generate_natural_features()
        self._generate_resource_deposits()
        self._generate_climate_zones()
        
        logger.info("Geographic features system initialized")
    
    def _generate_natural_features(self) -> None:
        """Generate natural features across the world."""
        # Generate mountains
        for i in range(50):
            feature_id = f"mountain_{i+1}"
            location = (random.randint(0, self.world_size[0]), 
                       random.randint(0, self.world_size[1]))
            
            feature = GeographicFeature(
                id=feature_id,
                name=f"Mountain Range {i+1}",
                feature_type=FeatureType.MOUNTAIN,
                location=location,
                size=random.uniform(0.5, 2.0),
                elevation=random.uniform(1000, 5000),
                climate=random.choice([ClimateZone.ALPINE, ClimateZone.BOREAL])
            )
            
            self.features[feature_id] = feature
            self.feature_locations[location].append(feature_id)
        
        # Generate rivers
        for i in range(30):
            feature_id = f"river_{i+1}"
            location = (random.randint(0, self.world_size[0]), 
                       random.randint(0, self.world_size[1]))
            
            feature = GeographicFeature(
                id=feature_id,
                name=f"River {i+1}",
                feature_type=FeatureType.RIVER,
                location=location,
                size=random.uniform(0.3, 1.0),
                accessibility=0.9,
                environmental_value=1.0
            )
            
            self.features[feature_id] = feature
            self.feature_locations[location].append(feature_id)
        
        # Add other feature types...
    
    def _generate_resource_deposits(self) -> None:
        """Generate resource deposits across the world."""
        resource_distribution = {
            ResourceType.IRON: (0.05, 0.8),  # (rarity, average_quality)
            ResourceType.GOLD: (0.01, 0.9),
            ResourceType.COAL: (0.08, 0.7),
            ResourceType.OIL: (0.03, 0.6),
            ResourceType.WOOD: (0.15, 0.9),
            ResourceType.FOOD: (0.20, 0.8),
        }
        
        for resource_type, (rarity, avg_quality) in resource_distribution.items():
            deposit_count = int(self.world_size[0] * self.world_size[1] * rarity / 1000)
            
            for i in range(deposit_count):
                deposit_id = f"{resource_type.name.lower()}_{i+1}"
                location = (random.randint(0, self.world_size[0]), 
                           random.randint(0, self.world_size[1]))
                
                deposit = ResourceDeposit(
                    id=deposit_id,
                    resource_type=resource_type,
                    location=location,
                    quantity=random.uniform(1000, 100000),
                    quality=random.uniform(avg_quality * 0.7, avg_quality * 1.3),
                    extraction_cost=random.uniform(0.5, 2.0)
                )
                
                self.resource_deposits[deposit_id] = deposit
    
    def _generate_climate_zones(self) -> None:
        """Generate climate zones across the world."""
        # Simplified climate zone generation based on latitude
        zones_by_latitude = [
            (0.0, 0.1, ClimateZone.RAINFOREST),
            (0.1, 0.2, ClimateZone.TROPICAL),
            (0.2, 0.3, ClimateZone.DESERT),
            (0.3, 0.4, ClimateZone.STEPPE),
            (0.4, 0.5, ClimateZone.TEMPERATE),
            (0.5, 0.6, ClimateZone.BOREAL),
            (0.6, 0.7, ClimateZone.TUNDRA),
            (0.7, 1.0, ClimateZone.POLAR),
        ]
        
        for i, (min_lat, max_lat, climate_zone) in enumerate(zones_by_latitude):
            region_id = f"climate_{climate_zone.name.lower()}_{i+1}"
            
            region = ClimateRegion(
                id=region_id,
                climate_zone=climate_zone,
                boundaries=[(0, min_lat * self.world_size[1]), 
                           (self.world_size[0], min_lat * self.world_size[1]),
                           (self.world_size[0], max_lat * self.world_size[1]),
                           (0, max_lat * self.world_size[1])],
                average_temperature=20 * (1 - max_lat) - 10,  # Rough approximation
                precipitation=random.uniform(500, 2000),
                growing_season=int(365 * (1 - max_lat * 0.8)),
                fertility=random.uniform(0.3, 0.8)
            )
            
            self.climate_regions[region_id] = region
    
    def discover_feature(self, feature_id: str, discoverer_id: str) -> bool:
        """Mark a feature as discovered by a civilization."""
        if feature_id not in self.features:
            return False
        
        feature = self.features[feature_id]
        feature.discovery_date = datetime.now()
        feature.discovered_by = discoverer_id
        
        logger.info(f"Feature {feature.name} discovered by {discoverer_id}")
        return True
    
    def discover_resource(self, deposit_id: str, discoverer_id: str) -> bool:
        """Mark a resource deposit as discovered."""
        if deposit_id not in self.resource_deposits:
            return False
        
        deposit = self.resource_deposits[deposit_id]
        deposit.discovery_date = datetime.now()
        deposit.owner = discoverer_id
        
        logger.info(f"Resource deposit {deposit_id} discovered by {discoverer_id}")
        return True
    
    def exploit_resource(self, deposit_id: str, extraction_rate: float, owner_id: str) -> bool:
        """Begin exploiting a resource deposit."""
        if deposit_id not in self.resource_deposits:
            return False
        
        deposit = self.resource_deposits[deposit_id]
        
        if deposit.owner and deposit.owner != owner_id:
            return False  # Already owned by someone else
        
        deposit.currently_exploited = True
        deposit.exploitation_rate = extraction_rate
        deposit.owner = owner_id
        
        logger.info(f"Resource deposit {deposit_id} being exploited by {owner_id}")
        return True
    
    def update_resource_extraction(self, delta_time: float) -> None:
        """Update resource extraction and depletion."""
        for deposit in self.resource_deposits.values():
            if deposit.currently_exploited:
                extracted = deposit.exploitation_rate * delta_time
                deposit.quantity = max(0, deposit.quantity - extracted)
                
                if deposit.quantity <= 0:
                    deposit.currently_exploited = False
                    logger.warning(f"Resource deposit {deposit.id} depleted")
    
    def get_features_at_location(self, location: Tuple[float, float], 
                               radius: float = 10.0) -> List[GeographicFeature]:
        """Get features near a specific location."""
        nearby_features = []
        
        for feature in self.features.values():
            distance = math.sqrt((feature.location[0] - location[0])**2 + 
                               (feature.location[1] - location[1])**2)
            if distance <= radius:
                nearby_features.append(feature)
        
        return nearby_features
    
    def get_resources_at_location(self, location: Tuple[float, float],
                                radius: float = 5.0) -> List[ResourceDeposit]:
        """Get resource deposits near a specific location."""
        nearby_resources = []
        
        for deposit in self.resource_deposits.values():
            distance = math.sqrt((deposit.location[0] - location[0])**2 + 
                               (deposit.location[1] - location[1])**2)
            if distance <= radius:
                nearby_resources.append(deposit)
        
        return nearby_resources
    
    def get_climate_at_location(self, location: Tuple[float, float]) -> Optional[ClimateZone]:
        """Get climate zone at a specific location."""
        latitude = location[1] / self.world_size[1]  # Normalized latitude
        
        for region in self.climate_regions.values():
            # Simple latitude-based climate determination
            min_y = min(point[1] for point in region.boundaries)
            max_y = max(point[1] for point in region.boundaries)
            
            if min_y <= location[1] <= max_y:
                return region.climate_zone
        
        return None
    
    def assess_settlement_suitability(self, location: Tuple[float, float]) -> Dict[str, float]:
        """Assess suitability of a location for settlement."""
        suitability = {
            "overall": 0.5,
            "water_access": 0.0,
            "fertility": 0.0,
            "defensibility": 0.0,
            "resource_availability": 0.0,
            "climate": 0.0
        }
        
        # Check for water features
        water_features = [f for f in self.get_features_at_location(location, 20.0) 
                         if f.feature_type in [FeatureType.RIVER, FeatureType.LAKE]]
        suitability["water_access"] = min(1.0, len(water_features) * 0.3)
        
        # Check climate
        climate = self.get_climate_at_location(location)
        if climate in [ClimateZone.TEMPERATE, ClimateZone.MEDITERRANEAN]:
            suitability["climate"] = 0.8
        elif climate in [ClimateZone.TROPICAL, ClimateZone.RAINFOREST]:
            suitability["climate"] = 0.6
        else:
            suitability["climate"] = 0.3
        
        # Check resources
        resources = self.get_resources_at_location(location, 15.0)
        resource_score = min(1.0, len(resources) * 0.2)
        suitability["resource_availability"] = resource_score
        
        # Check defensibility (elevation, natural barriers)
        features = self.get_features_at_location(location, 10.0)
        defensibility = 0.0
        for feature in features:
            if feature.feature_type == FeatureType.MOUNTAIN:
                defensibility += 0.2
            elif feature.feature_type == FeatureType.RIVER:
                defensibility += 0.1
        
        suitability["defensibility"] = min(1.0, defensibility)
        
        # Calculate overall suitability
        weights = {
            "water_access": 0.25,
            "climate": 0.20,
            "resource_availability": 0.20,
            "defensibility": 0.15,
            "fertility": 0.20
        }
        
        overall = sum(suitability[factor] * weight for factor, weight in weights.items())
        suitability["overall"] = overall
        
        return suitability

class TerrainType(Enum):
    """Detailed terrain types with specific characteristics."""
    # Water
    DEEP_OCEAN = auto()
    SHALLOW_OCEAN = auto()
    COASTAL_WATERS = auto()
    FRESHWATER_LAKE = auto()
    RIVER = auto()
    SWAMP = auto()
    
    # Flatlands
    GRASSLAND = auto()
    PLAINS = auto()
    FLOODPLAIN = auto()
    TUNDRA = auto()
    DESERT = auto()
    SALT_FLAT = auto()
    
    # Hills
    ROLLING_HILLS = auto()
    FOOTHILLS = auto()
    BADLANDS = auto()
    
    # Mountains
    LOW_MOUNTAINS = auto()
    HIGH_MOUNTAINS = auto()
    VOLCANIC = auto()
    GLACIATED = auto()
    
    # Forests
    BOREAL_FOREST = auto()
    TEMPERATE_FOREST = auto()
    TROPICAL_FOREST = auto()
    JUNGLE = auto()
    
    # Special
    OASIS = auto()
    GEOTHERMAL = auto()
    CAVE_SYSTEM = auto()
    CANYON = auto()


@dataclass
class ResourceNode:
    """A specific node within a resource deposit with detailed characteristics."""
    id: str
    resource_type: ResourceType
    parent_deposit_id: str
    location: Tuple[float, float]
    
    # Resource characteristics
    estimated_reserves: float
    current_reserves: float
    purity: float  # 0-1.0 quality/purity
    accessibility: float  # 0-1.0 ease of extraction
    renewal_rate: float  # 0-1.0 for renewable resources
    
    # Extraction status
    is_being_extracted: bool = False
    extraction_rate: float = 0.0
    extraction_efficiency: float = 0.5
    infrastructure_level: float = 0.0
    
    # Economic factors
    extraction_cost: float = 1.0
    market_value: float = 1.0
    labor_required: int = 0
    
    # Environmental impact
    environmental_impact: float = 0.0
    sustainability_score: float = 0.5
    
    def calculate_production_rate(self) -> float:
        """Calculate current production rate."""
        if not self.is_being_extracted:
            return 0.0
        
        base_rate = self.extraction_rate
        efficiency_bonus = self.extraction_efficiency * 0.5
        infrastructure_bonus = self.infrastructure_level * 0.3
        
        return base_rate * (1.0 + efficiency_bonus + infrastructure_bonus)
    
    def extract_resources(self, time_delta: float) -> float:
        """Extract resources over time and return amount extracted."""
        if not self.is_being_extracted or self.current_reserves <= 0:
            return 0.0
        
        production_rate = self.calculate_production_rate()
        amount_extracted = production_rate * time_delta
        
        # Can't extract more than available
        amount_extracted = min(amount_extracted, self.current_reserves)
        self.current_reserves -= amount_extracted
        
        # Update environmental impact
        self.environmental_impact += amount_extracted * 0.001
        
        # For renewable resources, apply renewal
        if self.renewal_rate > 0:
            renewal_amount = self.renewal_rate * time_delta
            self.current_reserves = min(self.estimated_reserves, 
                                      self.current_reserves + renewal_amount)
        
        return amount_extracted
    
    def calculate_profitability(self) -> float:
        """Calculate profitability of this resource node."""
        production_rate = self.calculate_production_rate()
        revenue = production_rate * self.market_value
        costs = self.extraction_cost + (self.labor_required * 0.1)
        
        if costs == 0:
            return 0.0
        
        return (revenue - costs) / costs  # Return on investment ratio
    
    def upgrade_infrastructure(self, investment: float) -> bool:
        """Upgrade extraction infrastructure."""
        improvement = investment * 0.001
        self.infrastructure_level = min(1.0, self.infrastructure_level + improvement)
        
        # Infrastructure improvements reduce extraction costs
        self.extraction_cost *= (1.0 - improvement * 0.1)
        
        return True
    
    def get_extraction_metadata(self) -> Dict[str, Any]:
        """Get metadata about resource extraction."""
        return {
            "current_reserves": self.current_reserves,
            "depletion_percentage": (1 - self.current_reserves / self.estimated_reserves) * 100,
            "production_rate": self.calculate_production_rate(),
            "profitability": self.calculate_profitability(),
            "environmental_impact": self.environmental_impact,
            "remaining_lifespan": (self.current_reserves / self.calculate_production_rate() 
                                 if self.calculate_production_rate() > 0 else float('inf')),
            "sustainability": "renewable" if self.renewal_rate > 0 else "non-renewable"
        }