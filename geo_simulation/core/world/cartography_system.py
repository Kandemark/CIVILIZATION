"""Map representation and fog of war"""

# Module implementation

"""
Cartography System for World Mapping and Exploration

Handles map generation, exploration mechanics, fog of war, and geographic discovery.
"""

from enum import Enum, auto
from typing import Dict, List, Optional, Set, Tuple, Any
from dataclasses import dataclass, field
import numpy as np
from datetime import datetime, timedelta
import logging
import random
from collections import defaultdict
import math

logger = logging.getLogger(__name__)


class MapType(Enum):
    """Types of maps available in the game."""
    POLITICAL = auto()      # Shows political boundaries and territories
    PHYSICAL = auto()       # Shows physical geography and terrain
    RESOURCE = auto()       # Shows resource distribution
    CLIMATE = auto()        # Shows climate zones and weather patterns
    TRADE = auto()          # Shows trade routes and economic zones
    STRATEGIC = auto()      # Shows military and strategic information


class ExplorationLevel(Enum):
    """Levels of exploration and knowledge about regions."""
    UNKNOWN = auto()        # Completely unexplored
    RUMORED = auto()        # Hearsay and legends
    PARTIAL = auto()        # Some exploration, incomplete knowledge
    EXPLORED = auto()       # Fully explored but not mapped in detail
    MAPPED = auto()         # Detailed mapping completed
    SETTLED = auto()        # Settled and fully integrated


@dataclass
class MapTile:
    """Represents a single tile on the game map."""
    x: int
    y: int
    terrain_type: str
    exploration_level: ExplorationLevel = ExplorationLevel.UNKNOWN
    discovered_by: Optional[str] = None  # Civilization that discovered it
    discovery_date: Optional[datetime] = None
    resources: Dict[str, float] = field(default_factory=dict)  # Resource type -> quantity
    features: List[str] = field(default_factory=list)
    elevation: float = 0.0  # Height above sea level
    fertility: float = 0.0  # Agricultural potential


@dataclass
class ExplorationParty:
    """A group sent to explore unknown territories."""
    id: str
    leader_id: str
    civilization_id: str
    size: int  # Number of members
    speed: float  # Movement speed
    range: int  # Maximum exploration range
    current_position: Tuple[int, int]
    supplies: float = 100.0  # 0-100%
    morale: float = 100.0  # 0-100%
    discovered_tiles: Set[Tuple[int, int]] = field(default_factory=set)
    objectives: List[Tuple[int, int]] = field(default_factory=list)


class CartographySystem:
    """
    Manages world mapping, exploration, and geographic discovery.
    """
    
    def __init__(self, world_size: Tuple[int, int] = (1000, 1000)):
        self.world_size = world_size
        self.world_map: Dict[Tuple[int, int], MapTile] = {}
        self.exploration_parties: Dict[str, ExplorationParty] = {}
        self.civilization_knowledge: Dict[str, Set[Tuple[int, int]]] = defaultdict(set)
        self._initialize_world_map()
        
        logger.info(f"Cartography system initialized with world size {world_size}")
    
    def _initialize_world_map(self) -> None:
        """Initialize the world map with basic terrain."""
        # This would be replaced with proper terrain generation
        for x in range(self.world_size[0]):
            for y in range(self.world_size[1]):
                # Simple terrain generation for demonstration
                terrain = self._generate_terrain(x, y)
                self.world_map[(x, y)] = MapTile(x, y, terrain)
    
    def _generate_terrain(self, x: int, y: int) -> str:
        """Generate terrain type based on position (simplified)."""
        # Simple noise-based terrain generation
        noise = random.random()
        
        if noise < 0.3:
            return "ocean"
        elif noise < 0.4:
            return "coast"
        elif noise < 0.5:
            return "plains"
        elif noise < 0.6:
            return "hills"
        elif noise < 0.7:
            return "mountains"
        elif noise < 0.8:
            return "forest"
        elif noise < 0.9:
            return "desert"
        else:
            return "jungle"
    
    def create_exploration_party(self, leader_id: str, civilization_id: str, start_position: Tuple[int, int], size: int = 10) -> ExplorationParty:
        """Create a new exploration party."""
        party_id = f"exp_party_{len(self.exploration_parties) + 1}"
        
        party = ExplorationParty(
            id=party_id,
            leader_id=leader_id,
            civilization_id=civilization_id,
            size=size,
            speed=5.0,  # tiles per day
            range=50,   # tiles from start
            current_position=start_position
        )
        
        self.exploration_parties[party_id] = party
        logger.info(f"Exploration party {party_id} created for civilization {civilization_id}")
        
        return party
    
    def update_exploration(self, delta_time: float) -> None:
        """Update exploration parties and discover new tiles."""
        for party_id, party in list(self.exploration_parties.items()):
            # Update party position and supplies
            self._update_party_movement(party, delta_time)
            party.supplies -= 0.1 * delta_time  # Consume supplies
            
            # Discover surrounding tiles
            self._discover_tiles(party)
            
            # Check if party should return (low supplies or morale)
            if party.supplies < 20.0 or party.morale < 30.0:
                self._return_to_base(party)
            
            # Remove destroyed parties
            if party.supplies <= 0 or party.morale <= 0:
                self._handle_party_loss(party)
                del self.exploration_parties[party_id]
    
    def _update_party_movement(self, party: ExplorationParty, delta_time: float) -> None:
        """Update party movement towards objectives."""
        if not party.objectives:
            return
        
        current_obj = party.objectives[0]
        dx = current_obj[0] - party.current_position[0]
        dy = current_obj[1] - party.current_position[1]
        distance = math.sqrt(dx**2 + dy**2)
        
        if distance < 1.0:  # Reached objective
            party.objectives.pop(0)
            return
        
        # Move towards objective
        move_distance = party.speed * delta_time
        if move_distance > distance:
            party.current_position = current_obj
        else:
            direction = (dx/distance, dy/distance)
            party.current_position = (
                party.current_position[0] + direction[0] * move_distance,
                party.current_position[1] + direction[1] * move_distance
            )
    
    def _discover_tiles(self, party: ExplorationParty) -> None:
        """Discover tiles around the exploration party."""
        center_x, center_y = party.current_position
        discovery_radius = 3  # tiles
        
        for dx in range(-discovery_radius, discovery_radius + 1):
            for dy in range(-discovery_radius, discovery_radius + 1):
                tile_x, tile_y = center_x + dx, center_y + dy
                tile_pos = (tile_x, tile_y)
                
                if tile_pos in self.world_map:
                    tile = self.world_map[tile_pos]
                    if tile.exploration_level.value < ExplorationLevel.EXPLORED.value:
                        tile.exploration_level = ExplorationLevel.EXPLORED
                        tile.discovered_by = party.civilization_id
                        tile.discovery_date = datetime.now()
                        party.discovered_tiles.add(tile_pos)
                        self.civilization_knowledge[party.civilization_id].add(tile_pos)
    
    def _return_to_base(self, party: ExplorationParty) -> None:
        """Make party return to its starting point."""
        # For simplicity, just remove objectives to make it return
        if not party.objectives:
            party.objectives = [party.current_position]  # Stay put for now
    
    def _handle_party_loss(self, party: ExplorationParty) -> None:
        """Handle the loss of an exploration party."""
        logger.warning(f"Exploration party {party.id} was lost!")
        # Could trigger events or notifications
    
    def get_known_world(self, civilization_id: str, map_type: MapType = MapType.POLITICAL) -> Dict[Tuple[int, int], MapTile]:
        """Get the known world for a specific civilization."""
        known_tiles = {}
        for tile_pos in self.civilization_knowledge[civilization_id]:
            known_tiles[tile_pos] = self.world_map[tile_pos]
        return known_tiles
    
    def generate_map(self, civilization_id: str, map_type: MapType, region: Optional[Tuple[Tuple[int, int], Tuple[int, int]]] = None) -> np.ndarray:
        """Generate a map array for visualization."""
        known_tiles = self.get_known_world(civilization_id)
        
        if region:
            x_min, y_min = region[0]
            x_max, y_max = region[1]
        else:
            # Use entire known area
            if not known_tiles:
                return np.zeros((100, 100))
            xs = [pos[0] for pos in known_tiles.keys()]
            ys = [pos[1] for pos in known_tiles.keys()]
            x_min, x_max = min(xs), max(xs)
            y_min, y_max = min(ys), max(ys)
        
        width = x_max - x_min + 1
        height = y_max - y_min + 1
        map_array = np.zeros((height, width))
        
        for (x, y), tile in known_tiles.items():
            if x_min <= x <= x_max and y_min <= y <= y_max:
                # Convert tile information to map value based on map type
                if map_type == MapType.PHYSICAL:
                    # Use elevation or terrain type
                    map_array[y - y_min, x - x_min] = tile.elevation
                elif map_type == MapType.RESOURCE:
                    # Use resource abundance
                    resource_value = sum(tile.resources.values()) if tile.resources else 0
                    map_array[y - y_min, x - x_min] = resource_value
                # Add other map type handling...
        
        return map_array
    
    def add_exploration_objective(self, party_id: str, target_position: Tuple[int, int]) -> bool:
        """Add an exploration objective for a party."""
        if party_id not in self.exploration_parties:
            return False
        
        party = self.exploration_parties[party_id]
        
        # Check if objective is within range
        start_x, start_y = party.current_position
        distance = math.sqrt((target_position[0] - start_x)**2 + (target_position[1] - start_y)**2)
        
        if distance > party.range:
            return False
        
        party.objectives.append(target_position)
        return True

class MapProjection(Enum):
    """Types of map projections for different purposes."""
    FLAT = auto()           # Simple flat projection
    MERCATOR = auto()       # Mercator projection for navigation
    POLAR = auto()         # Polar projection for polar regions
    CONIC = auto()         # Conic projection for mid-latitudes
    CYLINDRICAL = auto()   # Cylindrical equal-area
    ORTHOGRAPHIC = auto()  # Globe-like view

@dataclass
class MapLayer:
    """A layer in the map system that can be toggled on/off."""
    id: str
    name: str
    layer_type: str  # "terrain", "political", "resource", "climate", etc.
    data: np.ndarray
    opacity: float = 1.0
    visible: bool = True
    order: int = 0  # Rendering order
    source: str = "generated"  # "explored", "satellite", "theoretical"
    accuracy: float = 1.0  # 0-1.0 how accurate the data is
    last_updated: datetime = field(default_factory=datetime.now)

    def blend_with(self, other_layer: 'MapLayer', blend_mode: str = "overlay") -> 'MapLayer':
        """Blend this layer with another layer."""
        blended_data = self.data.copy()
        
        if blend_mode == "overlay":
            # Simple overlay blending
            mask = other_layer.data > 0
            blended_data[mask] = other_layer.data[mask]
        elif blend_mode == "alpha":
            # Alpha blending
            alpha = other_layer.opacity
            blended_data = (1 - alpha) * self.data + alpha * other_layer.data
        
        return MapLayer(
            id=f"blended_{self.id}_{other_layer.id}",
            name=f"Blended: {self.name} + {other_layer.name}",
            layer_type="composite",
            data=blended_data,
            opacity=(self.opacity + other_layer.opacity) / 2
        )


class MapLayerManager:
    """Manages multiple map layers and their composition."""
    
    def __init__(self, world_size: Tuple[int, int]):
        self.world_size = world_size
        self.layers: Dict[str, MapLayer] = {}
        self.layer_groups: Dict[str, List[str]] = defaultdict(list)
        
    def create_layer(self, name: str, layer_type: str, data: Optional[np.ndarray] = None) -> MapLayer:
        """Create a new map layer."""
        layer_id = f"layer_{len(self.layers) + 1}"
        
        if data is None:
            data = np.zeros(self.world_size)
        
        layer = MapLayer(
            id=layer_id,
            name=name,
            layer_type=layer_type,
            data=data
        )
        
        self.layers[layer_id] = layer
        return layer
    
    def add_to_group(self, layer_id: str, group_name: str) -> bool:
        """Add a layer to a group."""
        if layer_id not in self.layers:
            return False
        
        self.layer_groups[group_name].append(layer_id)
        return True
    
    def get_composite_layer(self, layer_ids: List[str], blend_mode: str = "overlay") -> Optional[MapLayer]:
        """Create a composite layer from multiple layers."""
        if not layer_ids:
            return None
        
        composite = self.layers[layer_ids[0]]
        
        for layer_id in layer_ids[1:]:
            if layer_id in self.layers:
                composite = composite.blend_with(self.layers[layer_id], blend_mode)
        
        return composite
    
    def update_layer_data(self, layer_id: str, new_data: np.ndarray, accuracy: float = 1.0) -> bool:
        """Update a layer's data."""
        if layer_id not in self.layers:
            return False
        
        layer = self.layers[layer_id]
        layer.data = new_data
        layer.accuracy = accuracy
        layer.last_updated = datetime.now()
        return True