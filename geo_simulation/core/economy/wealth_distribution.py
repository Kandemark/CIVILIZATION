"""Income inequality metrics"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
import numpy as np
from base_economy import EconomicEntity, DevelopmentLevel

class WealthTier(Enum):
    DESTITUTE = "destitute"    # < 0.1x avg wealth
    POOR = "poor"              # 0.1-0.5x avg wealth
    LOWER_MIDDLE = "lower_middle" # 0.5-1x avg wealth
    UPPER_MIDDLE = "upper_middle" # 1-2x avg wealth
    WEALTHY = "wealthy"        # 2-10x avg wealth
    RICH = "rich"              # 10-100x avg wealth
    ULTRA_RICH = "ultra_rich"  # > 100x avg wealth

@dataclass
class WealthSegment:
    tier: WealthTier
    population_share: float
    wealth_share: float
    average_income: float
    economic_mobility: float  # 0-1, ability to move between tiers
    consumption_propensity: float  # 0-1, tendency to consume vs save
    tax_contribution: float

class WealthDistribution:
    """Models the distribution of wealth within an economic entity"""
    
    def __init__(self, entity_id: str):
        self.entity_id = entity_id
        self.segments: Dict[WealthTier, WealthSegment] = {}
        self.gini_coefficient = 0.0
        self.palma_ratio = 0.0
        self.social_mobility = 0.5
        self.initialize_default_distribution()
    
    def initialize_default_distribution(self):
        """Initialize with realistic wealth distribution based on development level"""
        # Default distribution for industrial economy
        self.segments = {
            WealthTier.DESTITUTE: WealthSegment(WealthTier.DESTITUTE, 0.10, 0.01, 500, 0.1, 0.95, 0.01),
            WealthTier.POOR: WealthSegment(WealthTier.POOR, 0.25, 0.04, 1500, 0.3, 0.85, 0.05),
            WealthTier.LOWER_MIDDLE: WealthSegment(WealthTier.LOWER_MIDDLE, 0.35, 0.15, 3000, 0.5, 0.75, 0.15),
            WealthTier.UPPER_MIDDLE: WealthSegment(WealthTier.UPPER_MIDDLE, 0.20, 0.30, 6000, 0.6, 0.65, 0.30),
            WealthTier.WEALTHY: WealthSegment(WealthTier.WEALTHY, 0.08, 0.25, 15000, 0.7, 0.50, 0.25),
            WealthTier.RICH: WealthSegment(WealthTier.RICH, 0.015, 0.20, 50000, 0.8, 0.35, 0.20),
            WealthTier.ULTRA_RICH: WealthSegment(WealthTier.ULTRA_RICH, 0.005, 0.05, 200000, 0.9, 0.20, 0.04)
        }
        self.calculate_inequality_metrics()
    
    def calculate_inequality_metrics(self):
        """Calculate Gini coefficient and other inequality measures"""
        # Sort segments by wealth per capita
        sorted_segments = sorted(
            [s for s in self.segments.values() if s.population_share > 0],
            key=lambda x: (x.wealth_share / x.population_share) if x.population_share > 0 else 0
        )
        
        # Calculate Lorenz curve and Gini coefficient
        cumulative_population = 0.0
        cumulative_wealth = 0.0
        gini_numerator = 0.0
        
        for segment in sorted_segments:
            pop_prev = cumulative_population
            wealth_prev = cumulative_wealth
            
            cumulative_population += segment.population_share
            cumulative_wealth += segment.wealth_share
            
            gini_numerator += (cumulative_population * wealth_prev - cumulative_wealth * pop_prev)
        
        self.gini_coefficient = gini_numerator
        
        # Calculate Palma ratio (top 10% / bottom 40%)
        top_10_wealth = 0.0
        bottom_40_wealth = 0.0
        
        for segment in self.segments.values():
            if segment.tier in [WealthTier.RICH, WealthTier.ULTRA_RICH]:
                top_10_wealth += segment.wealth_share
            elif segment.tier in [WealthTier.DESTITUTE, WealthTier.POOR]:
                bottom_40_wealth += segment.wealth_share
        
        self.palma_ratio = top_10_wealth / bottom_40_wealth if bottom_40_wealth > 0 else float('inf')
    
    def update_distribution(self, total_wealth: float, entities: List[EconomicEntity], 
                          economic_conditions: Dict[str, float]):
        """Update wealth distribution based on economic activity"""
        if not entities or total_wealth <= 0:
            return
        
        # Calculate new wealth distribution based on economic factors
        growth_impact = economic_conditions.get('gdp_growth', 0.0)
        inflation = economic_conditions.get('inflation', 0.02)
        unemployment = economic_conditions.get('unemployment', 0.05)
        
        # Adjust segments based on economic conditions
        for segment in self.segments.values():
            # Wealthier segments benefit more from growth
            growth_benefit = growth_impact * (0.5 + (segment.economic_mobility * 0.5))
            
            # Poorer segments hurt more by inflation
            inflation_impact = inflation * (1.0 - segment.economic_mobility)
            
            # Unemployment disproportionately affects lower tiers
            unemployment_impact = unemployment * (1.0 - segment.economic_mobility) * 2.0
            
            net_impact = growth_benefit - inflation_impact - unemployment_impact
            segment.wealth_share = max(0.01, segment.wealth_share * (1.0 + net_impact))
        
        # Normalize to ensure total wealth share = 1.0
        total_share = sum(segment.wealth_share for segment in self.segments.values())
        for segment in self.segments.values():
            segment.wealth_share /= total_share
        
        self.calculate_inequality_metrics()
    
    def calculate_tax_revenue(self, tax_rate: float) -> float:
        """Calculate tax revenue based on current distribution"""
        total_revenue = 0.0
        
        for segment in self.segments.values():
            # Progressive taxation: higher rates for wealthier segments
            effective_tax_rate = tax_rate * (1.0 + (segment.economic_mobility * 0.5))
            segment.tax_contribution = segment.wealth_share * effective_tax_rate
            total_revenue += segment.tax_contribution
        
        return total_revenue
    
    def simulate_wealth_mobility(self):
        """Simulate movement between wealth tiers"""
        for from_tier, from_segment in self.segments.items():
            if from_segment.economic_mobility <= 0:
                continue
                
            # Calculate probability of moving up or down
            move_up_prob = from_segment.economic_mobility * 0.1
            move_down_prob = (1.0 - from_segment.economic_mobility) * 0.1
            
            # Implement mobility (simplified)
            population_moved_up = from_segment.population_share * move_up_prob
            population_moved_down = from_segment.population_share * move_down_prob
            
            # Find adjacent tiers
            tiers = list(WealthTier)
            current_index = tiers.index(from_tier)
            
            if current_index > 0 and population_moved_down > 0:
                lower_tier = tiers[current_index - 1]
                self._transfer_population(from_tier, lower_tier, population_moved_down)
            
            if current_index < len(tiers) - 1 and population_moved_up > 0:
                higher_tier = tiers[current_index + 1]
                self._transfer_population(from_tier, higher_tier, population_moved_up)
    
    def _transfer_population(self, from_tier: WealthTier, to_tier: WealthTier, amount: float):
        """Transfer population between wealth tiers"""
        if from_tier in self.segments and to_tier in self.segments:
            from_segment = self.segments[from_tier]
            to_segment = self.segments[to_tier]
            
            # Transfer population
            from_segment.population_share -= amount
            to_segment.population_share += amount
            
            # Transfer proportional wealth
            wealth_transfer = from_segment.wealth_share * (amount / from_segment.population_share)
            from_segment.wealth_share -= wealth_transfer
            to_segment.wealth_share += wealth_transfer
    
    def get_economic_impact(self) -> Dict[str, float]:
        """Calculate economic impact of current wealth distribution"""
        # Calculate aggregate consumption
        total_consumption = 0.0
        for segment in self.segments.values():
            consumption = segment.wealth_share * segment.consumption_propensity
            total_consumption += consumption
        
        # Calculate economic stability
        stability = 1.0 - (self.gini_coefficient * 0.5)  # More equal = more stable
        
        # Calculate innovation potential (wealthier segments invest more)
        innovation_potential = 0.0
        for segment in self.segments.values():
            if segment.tier in [WealthTier.WEALTHY, WealthTier.RICH, WealthTier.ULTRA_RICH]:
                investment_rate = 1.0 - segment.consumption_propensity
                innovation_potential += segment.wealth_share * investment_rate
        
        return {
            'aggregate_consumption': total_consumption,
            'economic_stability': stability,
            'innovation_potential': innovation_potential,
            'social_cohesion': 1.0 - min(1.0, self.gini_coefficient * 1.5),
            'tax_base_quality': self.palma_ratio  # Lower is generally better
        }