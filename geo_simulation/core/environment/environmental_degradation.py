"""Pollution and ecological damage"""

# Module implementation

"""
Environmental degradation monitoring and impact assessment.
Analyzes pollution, habitat loss, and other forms of environmental damage.
"""

from dataclasses import dataclass
from typing import List, Dict, Tuple
from enum import Enum
import numpy as np

class DegradationType(Enum):
    AIR_POLLUTION = "air_pollution"
    WATER_POLLUTION = "water_pollution"
    SOIL_CONTAMINATION = "soil_contamination"
    DEFORESTATION = "deforestation"
    HABITAT_FRAGMENTATION = "habitat_fragmentation"
    DESERTIFICATION = "desertification"
    URBAN_SPRAWL = "urban_sprawl"

@dataclass
class PollutionMeasurement:
    """Measurement of pollutant concentrations"""
    pollutant: str
    concentration: float  # ppm, mg/L, etc.
    unit: str
    location: str
    timestamp: str

@dataclass
class DegradationImpact:
    """Assessment of degradation impacts"""
    type: DegradationType
    severity: float  # 0-1 scale
    area_affected: float  # km²
    recovery_time: int  # years
    economic_cost: float  # monetary units

class EnvironmentalDegradation:
    """
    Monitors and assesses environmental degradation across multiple dimensions
    """
    
    def __init__(self):
        self.pollution_data: List[PollutionMeasurement] = []
        self.land_use_changes: List[Dict] = []
        self.degradation_impacts: List[DegradationImpact] = []
        self.monitoring_stations: Dict[str, List[str]] = {}  # location: parameters
        
    def add_pollution_data(self, data: PollutionMeasurement) -> None:
        """Add pollution measurement data"""
        self.pollution_data.append(data)
    
    def assess_air_quality(self, location: str = None) -> Dict[str, float]:
        """Assess air quality based on pollution data"""
        relevant_data = [d for d in self.pollution_data if d.location == location] if location else self.pollution_data
        
        if not relevant_data:
            return {}
        
        # Common air quality indicators
        pollutants = {}
        for data in relevant_data:
            if data.pollutant not in pollutants:
                pollutants[data.pollutant] = []
            pollutants[data.pollutant].append(data.concentration)
        
        # Calculate air quality index components
        aqi_components = {}
        for pollutant, concentrations in pollutants.items():
            avg_concentration = np.mean(concentrations)
            aqi_components[pollutant] = self._calculate_pollutant_index(pollutant, avg_concentration)
        
        # Overall AQI (worst pollutant)
        overall_aqi = max(aqi_components.values()) if aqi_components else 0
        
        return {
            'overall_aqi': overall_aqi,
            'aqi_components': aqi_components,
            'health_risk': self._assess_health_risk(overall_aqi),
            'primary_pollutant': max(aqi_components, key=aqi_components.get) if aqi_components else None
        }
    
    def _calculate_pollutant_index(self, pollutant: str, concentration: float) -> float:
        """Calculate sub-index for specific pollutant"""
        # Simplified AQI calculation - in reality this uses breakpoint tables
        standards = {
            'PM2.5': [12, 35, 55, 150, 250],  # μg/m³
            'PM10': [54, 154, 254, 354, 424],
            'O3': [0.054, 0.070, 0.085, 0.105, 0.200],  # ppm
            'NO2': [0.053, 0.100, 0.360, 0.649, 1.249],  # ppm
            'SO2': [0.034, 0.144, 0.224, 0.304, 0.604]  # ppm
        }
        
        if pollutant not in standards:
            return concentration  # Fallback
            
        breakpoints = standards[pollutant]
        aqi_breakpoints = [0, 50, 100, 150, 200, 300]  # AQI categories
        
        for i in range(len(breakpoints) - 1):
            if concentration <= breakpoints[i + 1]:
                # Linear interpolation
                aqi = ((aqi_breakpoints[i + 1] - aqi_breakpoints[i]) / 
                       (breakpoints[i + 1] - breakpoints[i]) * 
                       (concentration - breakpoints[i])) + aqi_breakpoints[i]
                return min(500, aqi)  # Cap at 500
        
        return 500  # Hazardous
    
    def _assess_health_risk(self, aqi: float) -> str:
        """Assess health risk based on AQI"""
        if aqi <= 50:
            return "Good"
        elif aqi <= 100:
            return "Moderate"
        elif aqi <= 150:
            return "Unhealthy for Sensitive Groups"
        elif aqi <= 200:
            return "Unhealthy"
        elif aqi <= 300:
            return "Very Unhealthy"
        else:
            return "Hazardous"
    
    def calculate_ecological_footprint(self, population: int, consumption_data: Dict) -> Dict[str, float]:
        """Calculate ecological footprint for a population"""
        # Components of ecological footprint
        components = {
            'carbon': consumption_data.get('energy_use', 0) * 0.5,
            'cropland': consumption_data.get('food_consumption', 0) * 0.3,
            'grazing_land': consumption_data.get('meat_consumption', 0) * 0.8,
            'forest': consumption_data.get('wood_products', 0) * 0.4,
            'fishing_grounds': consumption_data.get('fish_consumption', 0) * 0.2,
            'built_up_land': consumption_data.get('urban_area', 0) * 1.0
        }
        
        total_footprint = sum(components.values())
        footprint_per_capita = total_footprint / population if population > 0 else 0
        
        # Earth's biocapacity per capita (global hectares)
        earth_biocapacity = 1.6
        
        return {
            'total_footprint': total_footprint,
            'footprint_per_capita': footprint_per_capita,
            'biocapacity_deficit': max(0, footprint_per_capita - earth_biocapacity),
            'earth_equivalents': footprint_per_capita / earth_biocapacity,
            'components': components
        }
    
    def assess_land_degradation(self, land_cover_data: Dict[str, float]) -> Dict[str, float]:
        """Assess land degradation based on land cover changes"""
        # Key indicators of land degradation
        indicators = {}
        
        # Forest loss
        if 'forest_area' in land_cover_data and 'original_forest' in land_cover_data:
            forest_loss = (land_cover_data['original_forest'] - land_cover_data['forest_area']) 
            indicators['forest_loss_ratio'] = forest_loss / land_cover_data['original_forest']
        
        # Soil erosion
        if 'soil_quality' in land_cover_data:
            indicators['soil_degradation'] = 1 - land_cover_data['soil_quality']
        
        # Biodiversity impact
        if 'species_richness' in land_cover_data and 'original_richness' in land_cover_data:
            biodiversity_loss = (land_cover_data['original_richness'] - land_cover_data['species_richness'])
            indicators['biodiversity_loss_ratio'] = biodiversity_loss / land_cover_data['original_richness']
        
        # Overall degradation index (0-1)
        if indicators:
            degradation_index = sum(indicators.values()) / len(indicators)
        else:
            degradation_index = 0
        
        return {
            'degradation_index': degradation_index,
            'degradation_level': self._classify_degradation_level(degradation_index),
            'indicators': indicators
        }
    
    def _classify_degradation_level(self, index: float) -> str:
        """Classify degradation severity"""
        if index < 0.1:
            return "None"
        elif index < 0.3:
            return "Light"
        elif index < 0.6:
            return "Moderate"
        elif index < 0.8:
            return "Severe"
        else:
            return "Very Severe"
    
    def predict_recovery_time(self, degradation_type: DegradationType, severity: float) -> int:
        """Predict natural recovery time for degradation type"""
        # Base recovery times in years for different degradation types
        base_recovery = {
            DegradationType.DEFORESTATION: 50,
            DegradationType.SOIL_CONTAMINATION: 100,
            DegradationType.WATER_POLLUTION: 20,
            DegradationType.AIR_POLLUTION: 5,
            DegradationType.DESERTIFICATION: 200
        }
        
        base_time = base_recovery.get(degradation_type, 50)
        return int(base_time * severity)