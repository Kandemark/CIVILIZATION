# Package initialization

"""
World Module for Civilization Simulation

This module handles the geographic, territorial, and colonial aspects of the game world.
It includes comprehensive systems for dynamic borders, geographic features, territory 
management, and colonialism with exploration and cartography mechanics.

Components:
- CartographySystem: Map generation, exploration, and discovery systems
- ColonialSystem: Colonial expansion, settlement management, and development
- DynamicBorders: Border dynamics, conflicts, and territorial disputes  
- GeographicFeatures: Natural features, resources, and environmental factors
- TerritoryManager: Territory control, administration, and development

Enhanced Features:
- Exploration parties with supplies and morale
- Fog of war and discovery systems
- Settlement establishment and growth
- Border tension and conflict simulation
- Resource deposit management
- Climate zones and environmental factors
- Population growth and economic simulation
- Loyalty and rebellion mechanics
"""

__version__ = "1.0.0"
__author__ = "World Simulation Team"

# Import core systems
from .cartography_system import CartographySystem, MapType, ExplorationLevel, MapLayer, MapProjection
from .colonial_system import ColonialSystem, ColonialStatus, ColonialPolicy, Colony
from .dynamic_borders import DynamicBorders, BorderType, BorderConflict, BorderEvolution
from .geographic_features import GeographicFeatures, FeatureType, ResourceType, ClimateZone, TerrainType, ResourceNode
from .territory_manager import TerritoryManager, Territory, DevelopmentLevel, TerritorialClaim

__all__ = [
    # Cartography System
    'CartographySystem', 'MapType', 'ExplorationLevel', 'MapLayer', 'MapProjection',
    
    # Colonial System
    'ColonialSystem', 'ColonialStatus', 'ColonialPolicy', 'Colony',
    
    # Dynamic Borders
    'DynamicBorders', 'BorderType', 'BorderConflict', 'BorderEvolution',
    
    # Geographic Features
    'GeographicFeatures', 'FeatureType', 'ResourceType', 'ClimateZone', 
    'TerrainType', 'ResourceNode',
    
    # Territory Management
    'TerritoryManager', 'Territory', 'DevelopmentLevel', 'TerritorialClaim'
]

# System Integration Notes
"""
System Integration:

The world module systems are designed to work together seamlessly:

1. Cartography System provides the foundation with map generation and exploration
2. Geographic Features define the environmental context and resources
3. Territory Manager handles control and administration of discovered lands
4. Colonial System manages settlement and expansion into new territories
5. Dynamic Borders simulate the evolving boundaries between controlled areas

Key Integration Points:
- Exploration reveals geographic features which become territories
- Territories can be developed into colonies with proper infrastructure
- Colonial expansion triggers border dynamics with neighboring regions
- Geographic features influence settlement suitability and development
- Resource distribution affects territorial value and colonial priorities
"""