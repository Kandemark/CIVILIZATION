"""Earthquakes, volcanoes, storms"""

# Module implementation

"""
Natural disaster modeling, prediction, and impact assessment.
Includes earthquakes, floods, hurricanes, wildfires, etc.
"""

from dataclasses import dataclass
from typing import List, Dict, Optional, Tuple
from enum import Enum
import numpy as np
from datetime import datetime, timedelta

class DisasterType(Enum):
    EARTHQUAKE = "earthquake"
    FLOOD = "flood"
    HURRICANE = "hurricane"
    WILDFIRE = "wildfire"
    DROUGHT = "drought"
    TSUNAMI = "tsunami"
    VOLCANO = "volcanic_eruption"

class AlertLevel(Enum):
    LOW = "low"
    MODERATE = "moderate"
    HIGH = "high"
    SEVERE = "severe"
    EXTREME = "extreme"

@dataclass
class DisasterEvent:
    """Record of a natural disaster event"""
    disaster_type: DisasterType
    location: str
    coordinates: Tuple[float, float]  # lat, lon
    magnitude: float  # type-specific scale
    start_time: datetime
    duration: timedelta
    affected_area: float  # km²
    casualties: int
    economic_damage: float  # monetary units
    
    def calculate_severity(self) -> float:
        """Calculate overall severity score (0-1)"""
        # Type-specific severity calculations
        if self.disaster_type == DisasterType.EARTHQUAKE:
            base_severity = min(1.0, self.magnitude / 10.0)
        elif self.disaster_type == DisasterType.FLOOD:
            base_severity = min(1.0, self.affected_area / 10000)  # Normalized
        elif self.disaster_type == DisasterType.HURRICANE:
            base_severity = min(1.0, self.magnitude / 5.0)  # Saffir-Simpson scale
        else:
            base_severity = self.magnitude / 10.0
        
        # Adjust for human impact
        casualty_factor = min(1.0, self.casualties / 1000)
        damage_factor = min(1.0, self.economic_damage / 1e9)  # Billion units
        
        return (base_severity * 0.5 + casualty_factor * 0.25 + damage_factor * 0.25)

@dataclass
class EarlyWarning:
    """Early warning for potential natural disasters"""
    disaster_type: DisasterType
    location: str
    alert_level: AlertLevel
    probability: float  # 0-1
    expected_intensity: float
    expected_time: datetime
    recommended_actions: List[str]

class NaturalDisaster:
    """
    Base class for natural disaster modeling and analysis
    """
    
    def __init__(self, disaster_type: DisasterType):
        self.disaster_type = disaster_type
        self.historical_events: List[DisasterEvent] = []
        self.risk_factors: Dict[str, float] = {}
        
    def add_historical_event(self, event: DisasterEvent) -> None:
        """Add historical disaster event to records"""
        self.historical_events.append(event)
    
    def calculate_risk_index(self, location: str, environmental_data: Dict) -> float:
        """Calculate disaster risk index for a location"""
        base_risk = self._get_base_risk(location)
        
        # Adjust based on environmental factors
        environmental_risk = self._assess_environmental_risk(environmental_data)
        
        # Historical frequency adjustment
        historical_frequency = self._calculate_historical_frequency(location)
        
        return min(1.0, base_risk * environmental_risk * (1 + historical_frequency))
    
    def _get_base_risk(self, location: str) -> float:
        """Get base risk level based on geographical location"""
        # Simplified base risk - in reality this would use detailed geographical data
        risk_zones = {
            "california": 0.7,  # Earthquakes
            "florida": 0.8,     # Hurricanes
            "midwest": 0.6,     # Tornadoes
            "coastal": 0.5,     # Tsunamis
            "default": 0.3
        }
        
        location_lower = location.lower()
        for zone, risk in risk_zones.items():
            if zone in location_lower:
                return risk
        return risk_zones["default"]
    
    def _assess_environmental_risk(self, environmental_data: Dict) -> float:
        """Assess risk based on current environmental conditions"""
        risk_factors = []
        
        if 'temperature' in environmental_data:
            # Higher temperatures can increase wildfire risk
            temp_risk = max(0, (environmental_data['temperature'] - 25) / 20)
            risk_factors.append(temp_risk)
        
        if 'precipitation' in environmental_data:
            # Extreme precipitation increases flood risk
            precip_risk = min(1.0, environmental_data['precipitation'] / 500)
            risk_factors.append(precip_risk)
        
        if 'soil_moisture' in environmental_data:
            # Dry conditions increase wildfire risk
            dryness_risk = 1 - environmental_data['soil_moisture']
            risk_factors.append(dryness_risk)
        
        return np.mean(risk_factors) if risk_factors else 0.5
    
    def _calculate_historical_frequency(self, location: str) -> float:
        """Calculate historical frequency of disasters in location"""
        location_events = [event for event in self.historical_events 
                          if location.lower() in event.location.lower()]
        
        if not self.historical_events:
            return 0
        
        frequency = len(location_events) / len(self.historical_events)
        return min(1.0, frequency * 10)  # Normalized

class DisasterManager:
    """
    Comprehensive natural disaster management and prediction system
    """
    
    def __init__(self):
        self.disaster_models: Dict[DisasterType, NaturalDisaster] = {}
        self.active_warnings: List[EarlyWarning] = []
        self.mitigation_measures: Dict[DisasterType, List[str]] = {}
        self._initialize_disaster_models()
        self._initialize_mitigation_measures()
    
    def _initialize_disaster_models(self) -> None:
        """Initialize models for different disaster types"""
        for disaster_type in DisasterType:
            self.disaster_models[disaster_type] = NaturalDisaster(disaster_type)
    
    def _initialize_mitigation_measures(self) -> None:
        """Initialize recommended mitigation measures"""
        self.mitigation_measures[DisasterType.EARTHQUAKE] = [
            "Retrofit buildings",
            "Secure heavy furniture", 
            "Prepare emergency kits",
            "Establish evacuation routes"
        ]
        self.mitigation_measures[DisasterType.FLOOD] = [
            "Build flood barriers",
            "Elevate structures",
            "Improve drainage systems",
            "Create flood warning systems"
        ]
        self.mitigation_measures[DisasterType.HURRICANE] = [
            "Reinforce buildings",
            "Trim trees near structures",
            "Install storm shutters",
            "Prepare emergency supplies"
        ]
        self.mitigation_measures[DisasterType.WILDFIRE] = [
            "Create defensible space",
            "Use fire-resistant materials",
            "Clear vegetation around structures",
            "Develop evacuation plans"
        ]
    
    def assess_regional_risk(self, region: str, environmental_data: Dict) -> Dict[DisasterType, float]:
        """Assess risk for all disaster types in a region"""
        risk_assessment = {}
        
        for disaster_type, model in self.disaster_models.items():
            risk = model.calculate_risk_index(region, environmental_data)
            risk_assessment[disaster_type] = risk
        
        return risk_assessment
    
    def generate_early_warning(self, disaster_type: DisasterType, location: str, 
                             probability: float, intensity: float) -> EarlyWarning:
        """Generate early warning for potential disaster"""
        
        # Determine alert level based on probability and intensity
        if probability > 0.8 and intensity > 0.7:
            alert_level = AlertLevel.EXTREME
        elif probability > 0.6 and intensity > 0.5:
            alert_level = AlertLevel.SEVERE
        elif probability > 0.4:
            alert_level = AlertLevel.HIGH
        elif probability > 0.2:
            alert_level = AlertLevel.MODERATE
        else:
            alert_level = AlertLevel.LOW
        
        # Get recommended actions
        recommended_actions = self.mitigation_measures.get(disaster_type, [])
        
        warning = EarlyWarning(
            disaster_type=disaster_type,
            location=location,
            alert_level=alert_level,
            probability=probability,
            expected_intensity=intensity,
            expected_time=datetime.now() + timedelta(hours=24),  # Default 24h warning
            recommended_actions=recommended_actions
        )
        
        self.active_warnings.append(warning)
        return warning
    
    def calculate_disaster_impact(self, event: DisasterEvent, population_density: float, 
                                infrastructure_value: float) -> Dict[str, float]:
        """Calculate comprehensive impact of a disaster event"""
        
        # Casualty estimation
        expected_casualties = self._estimate_casualties(event, population_density)
        
        # Economic damage estimation
        economic_damage = self._estimate_economic_damage(event, infrastructure_value)
        
        # Environmental impact
        environmental_impact = self._estimate_environmental_impact(event)
        
        # Recovery time estimation
        recovery_time = self._estimate_recovery_time(event, economic_damage)
        
        return {
            'expected_casualties': expected_casualties,
            'economic_damage_usd': economic_damage,
            'environmental_impact_score': environmental_impact,
            'recovery_time_days': recovery_time,
            'overall_severity': event.calculate_severity()
        }
    
    def _estimate_casualties(self, event: DisasterEvent, population_density: float) -> int:
        """Estimate potential casualties"""
        base_casualties = event.magnitude * population_density * event.affected_area
        
        # Type-specific adjustments
        if event.disaster_type == DisasterType.EARTHQUAKE:
            base_casualties *= 10  # Earthquakes have higher casualty rates
        elif event.disaster_type == DisasterType.FLOOD:
            base_casualties *= 2
        
        return int(base_casualties)
    
    def _estimate_economic_damage(self, event: DisasterEvent, infrastructure_value: float) -> float:
        """Estimate economic damage in monetary units"""
        damage_ratio = event.magnitude * event.affected_area / 1000
        return infrastructure_value * damage_ratio
    
    def _estimate_environmental_impact(self, event: DisasterEvent) -> float:
        """Estimate environmental impact score (0-1)"""
        base_impact = event.affected_area / 10000  # Normalized
        
        # Type-specific environmental impacts
        if event.disaster_type == DisasterType.WILDFIRE:
            base_impact *= 1.5  # Wildfires have high environmental impact
        elif event.disaster_type == DisasterType.FLOOD:
            base_impact *= 1.2  # Floods affect ecosystems
        
        return min(1.0, base_impact)
    
    def _estimate_recovery_time(self, event: DisasterEvent, economic_damage: float) -> int:
        """Estimate recovery time in days"""
        base_time = event.magnitude * 30  # Base recovery time
        
        # Adjust based on damage and disaster type
        damage_factor = economic_damage / 1e6  # Millions
        recovery_time = base_time * (1 + damage_factor / 10)
        
        return int(recovery_time)
    
    def get_mitigation_recommendations(self, disaster_type: DisasterType, risk_level: float) -> List[str]:
        """Get mitigation recommendations based on risk level"""
        base_measures = self.mitigation_measures.get(disaster_type, [])
        
        if risk_level > 0.7:
            # High risk - return all measures
            return base_measures
        elif risk_level > 0.4:
            # Medium risk - return core measures
            return base_measures[:len(base_measures)//2]
        else:
            # Low risk - return basic preparedness
            return base_measures[:2] if base_measures else []