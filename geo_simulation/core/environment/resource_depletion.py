"""Exhaustion of natural resources"""

# Module implementation

"""
Natural resource depletion tracking and sustainable management.
Monitors renewable and non-renewable resource usage.
"""

from dataclasses import dataclass
from typing import List, Dict, Optional, Tuple
from enum import Enum
import numpy as np
from datetime import datetime

class ResourceType(Enum):
    RENEWABLE = "renewable"
    NON_RENEWABLE = "non_renewable"
    FLOW = "flow"  # e.g., solar, wind

class DepletionLevel(Enum):
    ABUNDANT = "abundant"
    ADEQUATE = "adequate"
    STRESSED = "stressed"
    SCARCE = "scarce"
    CRITICAL = "critical"

@dataclass
class ResourceStock:
    """Represents a stock of natural resource"""
    resource_type: ResourceType
    name: str
    current_reserves: float  # units specific to resource
    initial_reserves: float
    extraction_rate: float  # units per year
    regeneration_rate: float  # units per year (for renewables)
    unit: str
    
    def calculate_depletion_time(self) -> Optional[float]:
        """Calculate time until depletion in years"""
        if self.extraction_rate <= 0:
            return None
        
        if self.resource_type == ResourceType.NON_RENEWABLE:
            return self.current_reserves / self.extraction_rate
        elif self.resource_type == ResourceType.RENEWABLE:
            if self.extraction_rate > self.regeneration_rate:
                net_depletion = self.extraction_rate - self.regeneration_rate
                return self.current_reserves / net_depletion if net_depletion > 0 else None
            else:
                return None  # Sustainable usage
        return None
    
    def update_reserves(self, time_step: float = 1.0) -> None:
        """Update reserves based on extraction and regeneration"""
        if self.resource_type == ResourceType.NON_RENEWABLE:
            self.current_reserves = max(0, self.current_reserves - self.extraction_rate * time_step)
        elif self.resource_type == ResourceType.RENEWABLE:
            net_change = self.regeneration_rate - self.extraction_rate
            self.current_reserves = max(0, self.current_reserves + net_change * time_step)
        # Flow resources are not stored, so no update needed
    
    def get_depletion_level(self) -> DepletionLevel:
        """Get current depletion level"""
        if self.initial_reserves <= 0:
            return DepletionLevel.CRITICAL
        
        reserve_ratio = self.current_reserves / self.initial_reserves
        
        if reserve_ratio > 0.8:
            return DepletionLevel.ABUNDANT
        elif reserve_ratio > 0.5:
            return DepletionLevel.ADEQUATE
        elif reserve_ratio > 0.2:
            return DepletionLevel.STRESSED
        elif reserve_ratio > 0.05:
            return DepletionLevel.SCARCE
        else:
            return DepletionLevel.CRITICAL

@dataclass
class ExtractionRecord:
    """Record of resource extraction activity"""
    resource_name: str
    amount: float
    timestamp: datetime
    location: str
    efficiency: float  # 0-1 scale

class ResourceDepletion:
    """
    Tracks and analyzes natural resource depletion patterns
    """
    
    def __init__(self):
        self.resource_stocks: Dict[str, ResourceStock] = {}
        self.extraction_history: List[ExtractionRecord] = []
        self.conservation_policies: Dict[str, float] = {}  # policy: effectiveness
        
    def add_resource_stock(self, stock: ResourceStock) -> None:
        """Add a resource stock to monitor"""
        self.resource_stocks[stock.name] = stock
    
    def record_extraction(self, record: ExtractionRecord) -> None:
        """Record resource extraction activity"""
        self.extraction_history.append(record)
        
        # Update the corresponding resource stock
        if record.resource_name in self.resource_stocks:
            stock = self.resource_stocks[record.resource_name]
            # Adjust extraction based on efficiency
            effective_extraction = record.amount * (1 / record.efficiency if record.efficiency > 0 else 1)
            stock.extraction_rate = effective_extraction  # Update current rate
    
    def simulate_future_scenario(self, years: int, consumption_growth: float = 0.02) -> Dict[str, Dict]:
        """Simulate resource availability under different scenarios"""
        scenario_results = {}
        
        for name, stock in self.resource_stocks.items():
            # Create a copy for simulation
            simulated_stock = ResourceStock(
                resource_type=stock.resource_type,
                name=stock.name,
                current_reserves=stock.current_reserves,
                initial_reserves=stock.initial_reserves,
                extraction_rate=stock.extraction_rate,
                regeneration_rate=stock.regeneration_rate,
                unit=stock.unit
            )
            
            # Simulate with growth in consumption
            for year in range(years):
                current_extraction = simulated_stock.extraction_rate
                simulated_stock.extraction_rate = current_extraction * (1 + consumption_growth)
                simulated_stock.update_reserves()
            
            scenario_results[name] = {
                'final_reserves': simulated_stock.current_reserves,
                'depletion_level': simulated_stock.get_depletion_level(),
                'reserve_ratio': simulated_stock.current_reserves / simulated_stock.initial_reserves,
                'years_until_depletion': simulated_stock.calculate_depletion_time()
            }
        
        return scenario_results
    
    def calculate_sustainable_yield(self, resource_name: str) -> Optional[float]:
        """Calculate sustainable yield for renewable resources"""
        if resource_name not in self.resource_stocks:
            return None
        
        stock = self.resource_stocks[resource_name]
        
        if stock.resource_type != ResourceType.RENEWABLE:
            return None
        
        # Sustainable yield is typically less than regeneration rate
        return stock.regeneration_rate * 0.8  # 80% of regeneration rate
    
    def assess_critical_resources(self, threshold: float = 0.1) -> List[Tuple[str, DepletionLevel]]:
        """Identify resources with critical depletion levels"""
        critical_resources = []
        
        for name, stock in self.resource_stocks.items():
            depletion_level = stock.get_depletion_level()
            if (depletion_level in [DepletionLevel.SCARCE, DepletionLevel.CRITICAL] or 
                stock.current_reserves / stock.initial_reserves < threshold):
                critical_resources.append((name, depletion_level))
        
        return sorted(critical_resources, key=lambda x: x[1].value)
    
    def optimize_extraction_strategy(self, resource_name: str, 
                                  time_horizon: int = 50) -> Dict[str, float]:
        """Calculate optimal extraction strategy to maximize long-term availability"""
        if resource_name not in self.resource_stocks:
            return {}
        
        stock = self.resource_stocks[resource_name]
        
        if stock.resource_type == ResourceType.NON_RENEWABLE:
            return self._optimize_non_renewable(stock, time_horizon)
        elif stock.resource_type == ResourceType.RENEWABLE:
            return self._optimize_renewable(stock, time_horizon)
        else:
            return {'optimal_rate': stock.extraction_rate}
    
    def _optimize_non_renewable(self, stock: ResourceStock, years: int) -> Dict[str, float]:
        """Optimize extraction of non-renewable resources"""
        # Hotelling's rule: extract at a rate that increases at the discount rate
        # Simplified version: constant extraction that depletes over desired period
        optimal_rate = stock.current_reserves / years
        
        return {
            'optimal_rate': optimal_rate,
            'current_vs_optimal': stock.extraction_rate - optimal_rate,
            'sustainability_improvement': max(0, (stock.extraction_rate - optimal_rate) / stock.extraction_rate),
            'depletion_time_at_optimal': years
        }
    
    def _optimize_renewable(self, stock: ResourceStock, years: int) -> Dict[str, float]:
        """Optimize extraction of renewable resources"""
        # Maximum sustainable yield
        sustainable_yield = self.calculate_sustainable_yield(stock.name) or stock.regeneration_rate * 0.8
        
        return {
            'optimal_rate': sustainable_yield,
            'current_vs_optimal': stock.extraction_rate - sustainable_yield,
            'sustainability_status': 'sustainable' if stock.extraction_rate <= sustainable_yield else 'unsustainable',
            'regeneration_capacity': stock.regeneration_rate
        }
    
    def calculate_ecological_footprint(self, consumption_patterns: Dict[str, float]) -> Dict[str, float]:
        """Calculate ecological footprint based on resource consumption"""
        total_footprint = 0
        component_footprints = {}
        
        for resource, consumption in consumption_patterns.items():
            if resource in self.resource_stocks:
                stock = self.resource_stocks[resource]
                
                # Footprint calculation based on resource type and scarcity
                if stock.resource_type == ResourceType.NON_RENEWABLE:
                    scarcity_factor = 1 / (stock.current_reserves / stock.initial_reserves) if stock.initial_reserves > 0 else 10
                    footprint = consumption * scarcity_factor
                else:
                    footprint = consumption * 0.5  # Lower impact for renewables
                
                component_footprints[resource] = footprint
                total_footprint += footprint
        
        return {
            'total_ecological_footprint': total_footprint,
            'component_footprints': component_footprints,
            'earth_equivalents': total_footprint / 2.1,  # Global hectares per capita
            'sustainability_rating': self._calculate_sustainability_rating(total_footprint)
        }
    
    def _calculate_sustainability_rating(self, footprint: float) -> str:
        """Calculate sustainability rating based on ecological footprint"""
        if footprint < 1.7:
            return "Highly Sustainable"
        elif footprint < 3.0:
            return "Moderately Sustainable"
        elif footprint < 5.0:
            return "Unsustainable"
        else:
            return "Highly Unsustainable"
    
    def recommend_conservation_measures(self, resource_name: str) -> List[str]:
        """Recommend conservation measures for specific resources"""
        if resource_name not in self.resource_stocks:
            return ["Resource not monitored"]
        
        stock = self.resource_stocks[resource_name]
        depletion_level = stock.get_depletion_level()
        
        base_measures = [
            "Improve extraction efficiency",
            "Invest in recycling technologies",
            "Develop substitutes",
            "Implement usage quotas"
        ]
        
        if depletion_level in [DepletionLevel.SCARCE, DepletionLevel.CRITICAL]:
            critical_measures = [
                "Immediate consumption reduction",
                "Emergency stockpile release",
                "International resource sharing",
                "Rationing implementation"
            ]
            return base_measures + critical_measures
        
        return base_measures