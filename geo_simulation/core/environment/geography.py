"""Terrain and physical features"""

# Module implementation

"""
Geographical and spatial analysis for environmental studies.
Handles terrain, land use, and spatial patterns.
"""

from dataclasses import dataclass
from typing import List, Dict, Tuple, Optional
from enum import Enum
import numpy as np

class LandUseType(Enum):
    FOREST = "forest"
    AGRICULTURE = "agriculture"
    URBAN = "urban"
    WETLAND = "wetland"
    GRASSLAND = "grassland"
    DESERT = "desert"
    WATER = "water"
    TUNDRA = "tundra"

class TerrainType(Enum):
    MOUNTAIN = "mountain"
    HILL = "hill"
    PLAIN = "plain"
    VALLEY = "valley"
    PLATEAU = "plateau"
    COASTAL = "coastal"

@dataclass
class Coordinate:
    """Geographic coordinate with latitude and longitude"""
    latitude: float
    longitude: float
    
    def distance_to(self, other: 'Coordinate') -> float:
        """Calculate great-circle distance between coordinates in kilometers"""
        # Haversine formula
        R = 6371  # Earth's radius in km
        
        lat1, lon1 = np.radians(self.latitude), np.radians(self.longitude)
        lat2, lon2 = np.radians(other.latitude), np.radians(other.longitude)
        
        dlat = lat2 - lat1
        dlon = lon2 - lon1
        
        a = np.sin(dlat/2)**2 + np.cos(lat1) * np.cos(lat2) * np.sin(dlon/2)**2
        c = 2 * np.arctan2(np.sqrt(a), np.sqrt(1-a))
        
        return R * c

@dataclass
class LandPatch:
    """Represents a unit of land with specific characteristics"""
    coordinate: Coordinate
    area: float  # km²
    elevation: float  # meters
    land_use: LandUseType
    terrain: TerrainType
    soil_quality: float  # 0-1 scale
    vegetation_cover: float  # 0-1 scale

class Geography:
    """
    Geographical analysis and spatial modeling for environmental applications
    """
    
    def __init__(self, region_name: str, bounds: Tuple[Coordinate, Coordinate]):
        self.region_name = region_name
        self.bounds = bounds  # (southwest, northeast)
        self.land_patches: List[LandPatch] = []
        self.elevation_data: Optional[np.ndarray] = None
        self.land_use_map: Optional[np.ndarray] = None
        
    def add_land_patch(self, patch: LandPatch) -> None:
        """Add a land patch to the geographical model"""
        if self._is_within_bounds(patch.coordinate):
            self.land_patches.append(patch)
    
    def _is_within_bounds(self, coord: Coordinate) -> bool:
        """Check if coordinate is within region bounds"""
        sw, ne = self.bounds
        return (sw.latitude <= coord.latitude <= ne.latitude and 
                sw.longitude <= coord.longitude <= ne.longitude)
    
    def calculate_land_use_statistics(self) -> Dict[LandUseType, float]:
        """Calculate statistics for different land use types"""
        stats = {}
        total_area = sum(patch.area for patch in self.land_patches)
        
        for land_use in LandUseType:
            land_use_area = sum(patch.area for patch in self.land_patches 
                              if patch.land_use == land_use)
            stats[land_use] = {
                'area_km2': land_use_area,
                'percentage': (land_use_area / total_area * 100) if total_area > 0 else 0
            }
        
        return stats
    
    def analyze_topography(self) -> Dict[str, float]:
        """Analyze topographic characteristics of the region"""
        if not self.land_patches:
            return {}
        
        elevations = [patch.elevation for patch in self.land_patches]
        areas = [patch.area for patch in self.land_patches]
        
        # Weighted statistics (by area)
        weighted_elevations = [e * a for e, a in zip(elevations, areas)]
        total_area = sum(areas)
        
        return {
            'mean_elevation': sum(weighted_elevations) / total_area,
            'max_elevation': max(elevations),
            'min_elevation': min(elevations),
            'elevation_range': max(elevations) - min(elevations),
            'ruggedness_index': self._calculate_ruggedness_index(elevations, areas)
        }
    
    def _calculate_ruggedness_index(self, elevations: List[float], areas: List[float]) -> float:
        """Calculate terrain ruggedness index"""
        if len(elevations) < 2:
            return 0
        
        # Simplified ruggedness calculation
        elevation_variance = np.var(elevations)
        area_weighted_variance = elevation_variance * np.mean(areas)
        return min(1.0, area_weighted_variance / 1000)  # Normalized
    
    def identify_vulnerable_areas(self, climate_data: Dict) -> List[LandPatch]:
        """Identify areas vulnerable to climate impacts"""
        vulnerable_patches = []
        
        for patch in self.land_patches:
            vulnerability_score = self._calculate_vulnerability_score(patch, climate_data)
            if vulnerability_score > 0.7:  # High vulnerability threshold
                vulnerable_patches.append((patch, vulnerability_score))
        
        # Sort by vulnerability score
        vulnerable_patches.sort(key=lambda x: x[1], reverse=True)
        return [patch for patch, score in vulnerable_patches]
    
    def _calculate_vulnerability_score(self, patch: LandPatch, climate_data: Dict) -> float:
        """Calculate climate vulnerability score for a land patch"""
        score_components = []
        
        # Elevation-based vulnerability (sea level rise)
        if patch.elevation < 10:  # Coastal/low-lying areas
            score_components.append(0.8)
        elif patch.elevation < 50:
            score_components.append(0.5)
        
        # Land use vulnerability
        land_use_vulnerability = {
            LandUseType.COASTAL: 0.9,
            LandUseType.WETLAND: 0.7,
            LandUseType.AGRICULTURE: 0.6,
            LandUseType.URBAN: 0.4,
            LandUseType.FOREST: 0.3
        }
        score_components.append(land_use_vulnerability.get(patch.land_use, 0.5))
        
        # Soil quality vulnerability
        score_components.append(1 - patch.soil_quality)
        
        return sum(score_components) / len(score_components)
    
    def calculate_connectivity_index(self) -> float:
        """Calculate landscape connectivity index for wildlife"""
        if not self.land_patches:
            return 0
        
        # Simplified connectivity measure based on habitat patch distribution
        suitable_habitats = [patch for patch in self.land_patches 
                           if patch.land_use in [LandUseType.FOREST, LandUseType.GRASSLAND, LandUseType.WETLAND]]
        
        if not suitable_habitats:
            return 0
        
        total_habitat_area = sum(patch.area for patch in suitable_habitats)
        total_area = sum(patch.area for patch in self.land_patches)
        
        # Connectivity considers both area and fragmentation
        habitat_proportion = total_habitat_area / total_area
        
        # Fragmentation measure (inverse of average patch size)
        avg_patch_size = total_habitat_area / len(suitable_habitats)
        fragmentation = 1 / (1 + avg_patch_size)  # Normalized
        
        connectivity = habitat_proportion * (1 - fragmentation)
        return connectivity
    
    def generate_land_suitability_map(self, purpose: str) -> Dict[Coordinate, float]:
        """Generate land suitability map for specific purposes"""
        suitability_map = {}
        
        for patch in self.land_patches:
            if purpose == "conservation":
                suitability = self._calculate_conservation_suitability(patch)
            elif purpose == "agriculture":
                suitability = self._calculate_agricultural_suitability(patch)
            elif purpose == "urban_development":
                suitability = self._calculate_urban_suitability(patch)
            else:
                suitability = 0.5  # Neutral
            
            suitability_map[patch.coordinate] = suitability
        
        return suitability_map
    
    def _calculate_conservation_suitability(self, patch: LandPatch) -> float:
        """Calculate suitability for conservation"""
        score = 0.0
        
        # High value for existing natural areas
        if patch.land_use in [LandUseType.FOREST, LandUseType.WETLAND]:
            score += 0.8
        
        # High vegetation cover is good
        score += patch.vegetation_cover * 0.2
        
        # Remote areas might be better for conservation
        # This would normally use distance from development, but simplified here
        if patch.terrain in [TerrainType.MOUNTAIN, TerrainType.HILL]:
            score += 0.1
        
        return min(1.0, score)
    
    def _calculate_agricultural_suitability(self, patch: LandPatch) -> float:
        """Calculate suitability for agriculture"""
        score = 0.0
        
        # Flat terrain is better
        if patch.terrain in [TerrainType.PLAIN, TerrainType.VALLEY]:
            score += 0.4
        
        # Good soil quality is essential
        score += patch.soil_quality * 0.4
        
        # Existing agricultural land is suitable
        if patch.land_use == LandUseType.AGRICULTURE:
            score += 0.2
        
        return min(1.0, score)
    
    def _calculate_urban_suitability(self, patch: LandPatch) -> float:
        """Calculate suitability for urban development"""
        score = 0.0
        
        # Flat terrain is better for construction
        if patch.terrain in [TerrainType.PLAIN, TerrainType.PLATEAU]:
            score += 0.3
        
        # Not too steep
        if patch.elevation < 500:
            score += 0.2
        
        # Existing urban land is suitable for further development
        if patch.land_use == LandUseType.URBAN:
            score += 0.3
        
        # Good soil for foundation (moderate soil quality)
        score += (0.5 - abs(0.5 - patch.soil_quality)) * 0.2
        
        return min(1.0, score)