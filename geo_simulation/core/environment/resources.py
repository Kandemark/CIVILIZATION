"""Natural resource distribution"""

# Module implementation

"""
Comprehensive natural resource management system.
Tracks availability, usage, and sustainability of various resources.
"""

from dataclasses import dataclass
from typing import List, Dict, Optional
from enum import Enum
import numpy as np
from datetime import datetime

class ResourceCategory(Enum):
    ENERGY = "energy"
    WATER = "water"
    MINERAL = "mineral"
    BIOLOGICAL = "biological"
    LAND = "land"

@dataclass
class ResourceMetrics:
    """Comprehensive metrics for resource assessment"""
    availability: float  # 0-1 scale
    accessibility: float  # 0-1 scale
    quality: float  # 0-1 scale
    sustainability: float  # 0-1 scale
    economic_viability: float  # 0-1 scale

@dataclass
class ResourceUsage:
    """Record of resource usage patterns"""
    resource_name: str
    amount_used: float
    timestamp: datetime
    sector: str  # industrial, agricultural, residential, etc.
    efficiency: float  # 0-1 scale

class NaturalResource:
    """
    Represents a specific natural resource with comprehensive tracking
    """
    
    def __init__(self, name: str, category: ResourceCategory, total_reserves: float,
                 unit: str, renewal_rate: float = 0.0):
        self.name = name
        self.category = category
        self.total_reserves = total_reserves
        self.current_reserves = total_reserves
        self.unit = unit
        self.renewal_rate = renewal_rate  # annual renewal for renewable resources
        self.usage_history: List[ResourceUsage] = []
        self.metrics = ResourceMetrics(
            availability=1.0,
            accessibility=1.0,
            quality=1.0,
            sustainability=1.0,
            economic_viability=1.0
        )
        
    def record_usage(self, usage: ResourceUsage) -> None:
        """Record resource usage and update reserves"""
        self.usage_history.append(usage)
        
        # Update current reserves
        if self.category in [ResourceCategory.MINERAL, ResourceCategory.ENERGY]:
            # Non-renewable or slowly renewable
            self.current_reserves = max(0, self.current_reserves - usage.amount_used)
        elif self.category in [ResourceCategory.WATER, ResourceCategory.BIOLOGICAL]:
            # Renewable resources
            net_usage = usage.amount_used - (self.renewal_rate / 365)  # Daily renewal
            self.current_reserves = max(0, self.current_reserves - net_usage)
    
    def calculate_usage_trend(self, period_days: int = 30) -> Dict[str, float]:
        """Calculate usage trends over specified period"""
        if not self.usage_history:
            return {}
        
        recent_usage = [u for u in self.usage_history 
                       if (datetime.now() - u.timestamp).days <= period_days]
        
        if not recent_usage:
            return {}
        
        amounts = [u.amount_used for u in recent_usage]
        efficiencies = [u.efficiency for u in recent_usage]
        
        return {
            'total_usage': sum(amounts),
            'average_daily_usage': np.mean(amounts),
            'usage_trend': self._calculate_trend(amounts),
            'average_efficiency': np.mean(efficiencies),
            'peak_usage': max(amounts)
        }
    
    def update_metrics(self) -> None:
        """Update comprehensive resource metrics"""
        # Availability based on remaining reserves
        reserve_ratio = self.current_reserves / self.total_reserves
        self.metrics.availability = reserve_ratio
        
        # Accessibility (simplified - could incorporate geographical, political factors)
        self.metrics.accessibility = max(0.1, 1 - (1 - reserve_ratio) * 0.5)
        
        # Quality (could incorporate pollution, contamination data)
        # Simplified: decreases with heavy usage
        usage_intensity = len(self.usage_history) / 1000  # Normalized
        self.metrics.quality = max(0.3, 1 - usage_intensity)
        
        # Sustainability based on renewal vs usage
        if self.renewal_rate > 0:
            recent_usage = self.calculate_usage_trend(30)
            avg_daily_usage = recent_usage.get('average_daily_usage', 0)
            sustainable_level = self.renewal_rate / 365  # Daily renewal
            self.metrics.sustainability = min(1.0, sustainable_level / avg_daily_usage) if avg_daily_usage > 0 else 1.0
        else:
            # Non-renewable: sustainability decreases with usage
            self.metrics.sustainability = reserve_ratio
        
        # Economic viability (simplified)
        self.metrics.economic_viability = (self.metrics.availability + 
                                         self.metrics.accessibility + 
                                         self.metrics.quality) / 3
    
    def predict_lifespan(self, usage_scenario: str = "current") -> Optional[float]:
        """Predict resource lifespan under different usage scenarios"""
        if self.current_reserves <= 0:
            return 0
        
        trend = self.calculate_usage_trend()
        current_usage = trend.get('average_daily_usage', 0)
        
        if current_usage <= 0:
            return None
        
        scenarios = {
            "current": current_usage,
            "increasing": current_usage * 1.05,  # 5% annual growth
            "decreasing": current_usage * 0.95,  # 5% annual reduction
            "sustainable": self.renewal_rate / 365 if self.renewal_rate > 0 else current_usage
        }
        
        usage_rate = scenarios.get(usage_scenario, current_usage)
        
        if usage_rate <= 0:
            return None
        
        # Account for renewal for renewable resources
        if self.renewal_rate > 0 and usage_scenario == "sustainable":
            return float('inf')  # Sustainable usage can continue indefinitely
        elif self.renewal_rate > 0:
            net_depletion = usage_rate - (self.renewal_rate / 365)
            if net_depletion <= 0:
                return float('inf')
            return self.current_reserves / net_depletion
        else:
            return self.current_reserves / usage_rate
    
    def _calculate_trend(self, values: List[float]) -> float:
        """Calculate trend slope from time series data"""
        if len(values) < 2:
            return 0
        
        x = np.arange(len(values))
        slope = np.polyfit(x, values, 1)[0]
        return slope / np.mean(values) if np.mean(values) > 0 else 0

class ResourceManager:
    """
    Comprehensive management system for multiple natural resources
    """
    
    def __init__(self):
        self.resources: Dict[str, NaturalResource] = {}
        self.conservation_targets: Dict[str, float] = {}
        
    def add_resource(self, resource: NaturalResource) -> None:
        """Add a resource to management system"""
        self.resources[resource.name] = resource
    
    def record_usage_across_resources(self, usages: List[ResourceUsage]) -> None:
        """Record usage for multiple resources simultaneously"""
        for usage in usages:
            if usage.resource_name in self.resources:
                self.resources[usage.resource_name].record_usage(usage)
    
    def assess_overall_sustainability(self) -> Dict[str, float]:
        """Assess overall sustainability of managed resources"""
        if not self.resources:
            return {}
        
        category_sustainability = {}
        overall_metrics = {
            'availability': 0,
            'accessibility': 0,
            'quality': 0,
            'sustainability': 0,
            'economic_viability': 0
        }
        
        for resource in self.resources.values():
            resource.update_metrics()
            
            # Aggregate by category
            category = resource.category.value
            if category not in category_sustainability:
                category_sustainability[category] = []
            category_sustainability[category].append(resource.metrics.sustainability)
            
            # Overall averages
            for metric in overall_metrics:
                overall_metrics[metric] += getattr(resource.metrics, metric)
        
        # Calculate averages
        num_resources = len(self.resources)
        for metric in overall_metrics:
            overall_metrics[metric] /= num_resources
        
        # Category sustainability
        category_results = {}
        for category, values in category_sustainability.items():
            category_results[category] = np.mean(values)
        
        return {
            'overall_sustainability_index': overall_metrics['sustainability'],
            'category_sustainability': category_results,
            'detailed_metrics': overall_metrics,
            'critical_resources': self.identify_critical_resources()
        }
    
    def identify_critical_resources(self, threshold: float = 0.3) -> List[str]:
        """Identify resources with critical sustainability levels"""
        critical = []
        
        for name, resource in self.resources.items():
            resource.update_metrics()
            if resource.metrics.sustainability < threshold:
                critical.append(name)
        
        return critical
    
    def optimize_resource_allocation(self, demands: Dict[str, float]) -> Dict[str, float]:
        """Optimize resource allocation based on availability and sustainability"""
        allocation = {}
        
        for resource_name, demand in demands.items():
            if resource_name not in self.resources:
                allocation[resource_name] = 0
                continue
            
            resource = self.resources[resource_name]
            resource.update_metrics()
            
            # Allocation based on sustainability and availability
            sustainability_factor = resource.metrics.sustainability
            availability_factor = resource.metrics.availability
            
            # Combined priority score
            priority_score = (sustainability_factor + availability_factor) / 2
            
            # Allocate proportionally to priority score
            allocation[resource_name] = demand * priority_score
        
        return allocation
    
    def generate_conservation_plan(self, target_sustainability: float = 0.7) -> Dict[str, List[str]]:
        """Generate conservation plan to improve sustainability"""
        conservation_plan = {}
        
        for name, resource in self.resources.items():
            resource.update_metrics()
            current_sustainability = resource.metrics.sustainability
            
            if current_sustainability >= target_sustainability:
                conservation_plan[name] = ["Maintain current practices"]
                continue
            
            # Generate specific recommendations based on resource type and issues
            recommendations = []
            
            if resource.metrics.availability < 0.5:
                recommendations.append("Reduce consumption by 20%")
                recommendations.append("Implement recycling program")
            
            if resource.metrics.quality < 0.6:
                recommendations.append("Improve extraction/processing methods")
                recommendations.append("Monitor contamination levels")
            
            if resource.metrics.accessibility < 0.5:
                recommendations.append("Develop alternative sources")
                recommendations.append("Improve distribution infrastructure")
            
            # Type-specific recommendations
            if resource.category == ResourceCategory.WATER:
                recommendations.extend([
                    "Fix leakage in distribution systems",
                    "Promote water-efficient technologies",
                    "Implement rainwater harvesting"
                ])
            elif resource.category == ResourceCategory.ENERGY:
                recommendations.extend([
                    "Increase renewable energy investment",
                    "Improve energy efficiency standards",
                    "Promote energy conservation"
                ])
            
            conservation_plan[name] = recommendations
        
        return conservation_plan
    
    def simulate_climate_impact(self, climate_scenario: Dict[str, float]) -> Dict[str, float]:
        """Simulate impact of climate change on resources"""
        impacts = {}
        
        for name, resource in self.resources.items():
            base_sustainability = resource.metrics.sustainability
            
            # Climate impact factors (simplified)
            if resource.category == ResourceCategory.WATER:
                # Water resources affected by precipitation changes
                precip_change = climate_scenario.get('precipitation_change', 0)
                impact = max(0, base_sustainability * (1 - abs(precip_change) / 100))
            elif resource.category == ResourceCategory.BIOLOGICAL:
                # Biological resources affected by temperature changes
                temp_change = climate_scenario.get('temperature_change', 0)
                impact = max(0, base_sustainability * (1 - abs(temp_change) / 5))
            else:
                # Other resources less directly affected
                impact = base_sustainability * 0.9  # 10% reduction
            
            impacts[name] = {
                'current_sustainability': base_sustainability,
                'projected_sustainability': impact,
                'impact_percentage': (impact - base_sustainability) / base_sustainability * 100
            }
        
        return impacts