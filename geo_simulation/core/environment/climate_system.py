"""Weather patterns and changes"""

# Module implementation

"""
Earth climate system modeling including atmosphere, oceans, cryosphere, and biosphere interactions.
"""

from dataclasses import dataclass
from typing import Dict, List, Tuple
import numpy as np
from enum import Enum

class SystemComponent(Enum):
    ATMOSPHERE = "atmosphere"
    OCEAN = "ocean" 
    CRYOSPHERE = "cryosphere"
    BIOSPHERE = "biosphere"
    LAND = "land"

@dataclass
class EnergyBalance:
    """Earth's energy balance components"""
    solar_input: float  # W/m²
    albedo: float  # reflectivity
    greenhouse_effect: float  # W/m²
    outgoing_longwave: float  # W/m²
    heat_absorption: float  # W/m²

@dataclass
class SystemFeedback:
    """Climate system feedback mechanisms"""
    name: str
    component: SystemComponent
    feedback_strength: float  # W/m²/°C
    is_positive: bool

class ClimateSystem:
    """
    Models the Earth's climate system and its components
    """
    
    def __init__(self):
        self.components: Dict[SystemComponent, Dict] = {}
        self.feedbacks: List[SystemFeedback] = []
        self.energy_balance = EnergyBalance(
            solar_input=340,
            albedo=0.3,
            greenhouse_effect=150,
            outgoing_longwave=240,
            heat_absorption=0.9
        )
        self._initialize_components()
        self._initialize_feedbacks()
    
    def _initialize_components(self) -> None:
        """Initialize climate system components"""
        self.components[SystemComponent.ATMOSPHERE] = {
            'temperature': 15.0,
            'co2_concentration': 410,
            'water_vapor': 4.0
        }
        self.components[SystemComponent.OCEAN] = {
            'temperature': 4.0,
            'heat_content': 1.0,
            'ph': 8.1
        }
        self.components[SystemComponent.CRYOSPHERE] = {
            'ice_extent': 25.0,  # million km²
            'albedo': 0.8
        }
    
    def _initialize_feedbacks(self) -> None:
        """Initialize key climate feedback mechanisms"""
        self.feedbacks.extend([
            SystemFeedback("Ice-Albedo", SystemComponent.CRYOSPHERE, 0.3, True),
            SystemFeedback("Water Vapor", SystemComponent.ATMOSPHERE, 1.8, True),
            SystemFeedback("Cloud", SystemComponent.ATMOSPHERE, 0.5, False),  # Negative feedback
            SystemFeedback("Carbon Cycle", SystemComponent.BIOSPHERE, 0.2, True)
        ])
    
    def calculate_equilibrium_temperature(self) -> float:
        """Calculate Earth's equilibrium temperature using energy balance"""
        absorbed_solar = self.energy_balance.solar_input * (1 - self.energy_balance.albedo)
        net_radiation = absorbed_solar - self.energy_balance.outgoing_longwave
        climate_sensitivity = 0.8  # °C per W/m²
        
        return self.components[SystemComponent.ATMOSPHERE]['temperature'] + (net_radiation * climate_sensitivity)
    
    def simulate_feedback_effects(self, temperature_change: float) -> Dict[str, float]:
        """Simulate the effects of climate feedbacks on temperature"""
        total_feedback = sum(
            fb.feedback_strength * (1 if fb.is_positive else -1) 
            for fb in self.feedbacks
        )
        
        feedback_amplification = 1 + (total_feedback * temperature_change)
        final_temperature_change = temperature_change * feedback_amplification
        
        return {
            'initial_change': temperature_change,
            'feedback_amplification': feedback_amplification,
            'final_change': final_temperature_change,
            'total_feedback_strength': total_feedback
        }
    
    def update_system_state(self, co2_increase: float, time_step: int = 1) -> None:
        """Update climate system state based on CO2 increase and time step"""
        # Update atmospheric CO2
        current_co2 = self.components[SystemComponent.ATMOSPHERE]['co2_concentration']
        self.components[SystemComponent.ATMOSPHERE]['co2_concentration'] = current_co2 + co2_increase
        
        # Calculate temperature change from CO2 (simplified)
        co2_ratio = current_co2 / 280  # Pre-industrial baseline
        temperature_change = 3.0 * np.log2(co2_ratio)  # 3°C doubling sensitivity
        
        # Apply feedbacks
        feedback_result = self.simulate_feedback_effects(temperature_change)
        
        # Update component temperatures
        for component in [SystemComponent.ATMOSPHERE, SystemComponent.OCEAN]:
            self.components[component]['temperature'] += feedback_result['final_change'] * 0.5
        
        # Update cryosphere (melting)
        ice_melt = feedback_result['final_change'] * 0.1  # million km² per °C
        self.components[SystemComponent.CRYOSPHERE]['ice_extent'] = max(
            0, self.components[SystemComponent.CRYOSPHERE]['ice_extent'] - ice_melt
        )
        
        # Update albedo based on ice extent
        self._update_albedo()
    
    def _update_albedo(self) -> None:
        """Update planetary albedo based on ice cover"""
        ice_fraction = self.components[SystemComponent.CRYOSPHERE]['ice_extent'] / 25.0
        ocean_albedo = 0.1
        ice_albedo = 0.8
        land_albedo = 0.3
        
        # Simplified albedo calculation
        new_albedo = (ice_fraction * ice_albedo + 
                     (0.7 - ice_fraction) * ocean_albedo + 
                     0.3 * land_albedo)
        
        self.energy_balance.albedo = new_albedo
    
    def get_system_health_index(self) -> float:
        """Calculate overall climate system health index (0-1)"""
        indicators = []
        
        # Temperature stability
        temp = self.components[SystemComponent.ATMOSPHERE]['temperature']
        indicators.append(max(0, 1 - abs(temp - 15) / 10))
        
        # Ice extent health
        ice = self.components[SystemComponent.CRYOSPHERE]['ice_extent']
        indicators.append(ice / 25.0)
        
        # CO2 concentration health
        co2 = self.components[SystemComponent.ATMOSPHERE]['co2_concentration']
        indicators.append(max(0, 1 - (co2 - 280) / 200))
        
        return np.mean(indicators)