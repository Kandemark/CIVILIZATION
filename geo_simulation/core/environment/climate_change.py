"""Long-term environmental shifts"""

# Module implementation

"""
Climate change impact assessment and modeling.
Analyzes long-term climate patterns and anthropogenic influences.
"""

from dataclasses import dataclass
from typing import List, Dict, Tuple
import numpy as np
from datetime import datetime, timedelta
from .climate import Climate, ClimateData

@dataclass
class EmissionData:
    """Greenhouse gas emission measurements"""
    co2_ppm: float
    ch4_ppb: float
    n2o_ppb: float
    timestamp: datetime

@dataclass
class ClimateImpact:
    """Assessment of climate change impacts"""
    sea_level_rise: float  # meters
    temperature_anomaly: float  # degrees Celsius
    extreme_events_increase: float  # percentage
    biodiversity_loss: float  # percentage

class ClimateChange:
    """
    Analyzes climate change patterns, causes, and impacts
    """
    
    def __init__(self, baseline_year: int = 1850):
        self.baseline_year = baseline_year
        self.emission_records: List[EmissionData] = []
        self.temperature_records: List[Tuple[datetime, float]] = []
        self.impact_assessments: List[ClimateImpact] = []
        
    def add_emission_data(self, data: EmissionData) -> None:
        """Add greenhouse gas emission measurements"""
        self.emission_records.append(data)
        
    def add_temperature_record(self, date: datetime, anomaly: float) -> None:
        """Add temperature anomaly record"""
        self.temperature_records.append((date, anomaly))
        
    def calculate_warming_trend(self) -> Dict[str, float]:
        """Calculate global warming trend from temperature records"""
        if len(self.temperature_records) < 2:
            return {}
            
        dates, anomalies = zip(*self.temperature_records)
        years = [(date - datetime(self.baseline_year, 1, 1)).days / 365.25 for date in dates]
        
        # Linear regression for warming rate
        slope, intercept = np.polyfit(years, anomalies, 1)
        
        return {
            'warming_rate_per_decade': slope * 10,
            'current_anomaly': anomalies[-1],
            'r_squared': self._calculate_r_squared(years, anomalies, slope, intercept)
        }
    
    def project_future_temperatures(self, scenario: str = "medium") -> Dict[int, float]:
        """Project future temperatures based on different scenarios"""
        scenarios = {
            "low": 0.01,    # Degrees per year
            "medium": 0.02, # Degrees per year  
            "high": 0.03    # Degrees per year
        }
        
        current_anomaly = self.temperature_records[-1][1] if self.temperature_records else 0
        rate = scenarios.get(scenario, 0.02)
        
        projections = {}
        for years in [10, 20, 50, 100]:
            projections[years] = current_anomaly + (rate * years)
            
        return projections
    
    def assess_impacts(self, temperature_increase: float) -> ClimateImpact:
        """Assess impacts of temperature increase"""
        # Simplified impact model - in reality this would be much more complex
        return ClimateImpact(
            sea_level_rise=temperature_increase * 0.2,  # meters per degree
            temperature_anomaly=temperature_increase,
            extreme_events_increase=temperature_increase * 15,  # percentage
            biodiversity_loss=temperature_increase * 10  # percentage
        )
    
    def calculate_carbon_budget(self, target_increase: float = 1.5) -> Dict[str, float]:
        """Calculate remaining carbon budget to stay below target temperature increase"""
        current_co2 = self.emission_records[-1].co2_ppm if self.emission_records else 410
        budget_co2 = 450  # Approximate CO2 concentration for 1.5°C
        
        return {
            'remaining_budget_ppm': max(0, budget_co2 - current_co2),
            'current_concentration': current_co2,
            'target_concentration': budget_co2,
            'exceedance_probability': self._calculate_exceedance_probability()
        }
    
    def _calculate_r_squared(self, x, y, slope, intercept) -> float:
        """Calculate R-squared for trend line"""
        y_pred = [slope * xi + intercept for xi in x]
        ss_res = sum((yi - ypi) ** 2 for yi, ypi in zip(y, y_pred))
        ss_tot = sum((yi - np.mean(y)) ** 2 for yi in y)
        return 1 - (ss_res / ss_tot) if ss_tot != 0 else 0
    
    def _calculate_exceedance_probability(self) -> float:
        """Calculate probability of exceeding temperature targets"""
        # Simplified calculation
        trend = self.calculate_warming_trend()
        rate = trend.get('warming_rate_per_decade', 0.2)
        return min(1.0, rate / 0.15)  # Normalized probability