"""Supply, demand, and prices"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
import random
from base_economy import ResourceType, EconomicEntity

class MarketSentiment(Enum):
    BULLISH = "bullish"      # Optimistic, rising prices
    BEARISH = "bearish"      # Pessimistic, falling prices
    VOLATILE = "volatile"    # Highly fluctuating
    STABLE = "stable"        # Little movement
    PANIC = "panic"          # Crash conditions

@dataclass
class PriceHistory:
    resource: ResourceType
    prices: List[float]
    moving_averages: Dict[int, float]  # days: average
    volatility: float
    trend: float  # -1 to 1, negative = downward

class MarketPsychology:
    """Models investor and consumer psychology"""
    
    def __init__(self):
        self.confidence = 0.5  # 0-1, overall market confidence
        self.risk_aversion = 0.3  # 0-1, higher = more risk averse
        self.speculation_level = 0.2  # 0-1, speculative activity
        self.consumer_sentiment = 0.6  # 0-1, willingness to spend
        self.investor_sentiment = 0.5  # 0-1, willingness to invest
        
    def update_sentiment(self, economic_indicators: Dict[str, float], recent_events: List[str]):
        """Update market psychology based on economic conditions and events"""
        # Economic factors
        gdp_growth = economic_indicators.get('gdp_growth', 0)
        inflation = economic_indicators.get('inflation', 0.02)
        unemployment = economic_indicators.get('unemployment', 0.05)
        
        # Adjust confidence based on growth
        if gdp_growth > 0.03:
            self.confidence = min(1.0, self.confidence + 0.1)
        elif gdp_growth < -0.02:
            self.confidence = max(0.0, self.confidence - 0.15)
            
        # High inflation reduces confidence
        if inflation > 0.05:
            self.confidence *= 0.8
            
        # High unemployment increases risk aversion
        self.risk_aversion = min(1.0, unemployment * 2)
        
        # Events impact
        for event in recent_events:
            if any(keyword in event.lower() for keyword in ['crisis', 'war', 'disaster']):
                self.confidence *= 0.7
                self.risk_aversion = min(1.0, self.risk_aversion + 0.3)
            elif any(keyword in event.lower() for keyword in ['boom', 'discovery', 'peace']):
                self.confidence = min(1.0, self.confidence + 0.2)
                self.risk_aversion = max(0.0, self.risk_aversion - 0.1)

class MarketDynamics:
    """Handles complex market behaviors including speculation, bubbles, and crashes"""
    
    def __init__(self):
        self.price_histories: Dict[ResourceType, PriceHistory] = {}
        self.market_psychology = MarketPsychology()
        self.speculative_bubbles: Dict[ResourceType, float] = {}  # resource -> bubble size (0-1)
        self.market_manipulation: Dict[str, List[Tuple]] = {}  # entity -> manipulation attempts
        self.information_asymmetry: float = 0.3  # 0-1, how uneven is information distribution
        
    def calculate_price_impact(self, resource: ResourceType, trade_volume: float, 
                             total_volume: float, is_buy: bool) -> float:
        """Calculate price impact of a trade based on market depth"""
        if total_volume == 0:
            return 1.0
            
        # Market depth effect - large trades move prices more
        volume_ratio = trade_volume / total_volume
        base_impact = volume_ratio * 2.0  # Linear impact
        
        # Sentiment effects
        if is_buy:
            # Bullish sentiment amplifies buy pressure
            base_impact *= (1.0 + self.market_psychology.confidence)
        else:
            # Bearish sentiment amplifies sell pressure
            base_impact *= (1.0 + self.market_psychology.risk_aversion)
            
        return min(5.0, base_impact)  # Cap at 5x impact
    
    def detect_bubbles(self, current_prices: Dict[ResourceType, float], 
                      fundamental_values: Dict[ResourceType, float]) -> Dict[ResourceType, float]:
        """Detect speculative bubbles in resource markets"""
        bubbles = {}
        
        for resource, current_price in current_prices.items():
            fundamental_value = fundamental_values.get(resource, current_price)
            
            if fundamental_value > 0:
                price_ratio = current_price / fundamental_value
                if price_ratio > 1.5:  # 50% over fundamental value
                    bubble_size = min(1.0, (price_ratio - 1.5) / 3.0)  # Normalize to 0-1
                    bubbles[resource] = bubble_size
                    
                    # Update psychology
                    if bubble_size > 0.3:
                        self.market_psychology.speculation_level = min(1.0, 
                            self.market_psychology.speculation_level + 0.1)
        
        return bubbles
    
    def simulate_bubble_pop(self, resource: ResourceType, bubble_size: float):
        """Simulate a bubble popping"""
        pop_severity = bubble_size * random.uniform(0.5, 1.5)
        
        # Sharp price decline
        price_drop = pop_severity * 0.8  # 80% of bubble size
        
        # Contagion effect to other markets
        for other_resource in self.speculative_bubbles:
            if other_resource != resource and self.speculative_bubbles[other_resource] > 0.2:
                contagion = pop_severity * 0.3  # 30% contagion
                self.speculative_bubbles[other_resource] = max(0.0, 
                    self.speculative_bubbles[other_resource] - contagion)
        
        # Update psychology
        self.market_psychology.confidence *= (1.0 - pop_severity * 0.5)
        self.market_psychology.risk_aversion = min(1.0, 
            self.market_psychology.risk_aversion + pop_severity * 0.3)
            
        return price_drop
    
    def update_price_histories(self, current_prices: Dict[ResourceType, float]):
        """Update price history and calculate trends"""
        for resource, price in current_prices.items():
            if resource not in self.price_histories:
                self.price_histories[resource] = PriceHistory(
                    resource=resource,
                    prices=[],
                    moving_averages={},
                    volatility=0.1,
                    trend=0.0
                )
            
            history = self.price_histories[resource]
            history.prices.append(price)
            
            # Keep only last 100 prices
            if len(history.prices) > 100:
                history.prices.pop(0)
            
            # Calculate moving averages
            for days in [5, 20, 50]:
                if len(history.prices) >= days:
                    history.moving_averages[days] = sum(history.prices[-days:]) / days
            
            # Calculate volatility (standard deviation of recent prices)
            if len(history.prices) >= 10:
                recent_prices = history.prices[-10:]
                avg_price = sum(recent_prices) / len(recent_prices)
                variance = sum((p - avg_price) ** 2 for p in recent_prices) / len(recent_prices)
                history.volatility = variance ** 0.5 / avg_price  # Relative volatility
            
            # Calculate trend
            if len(history.prices) >= 5:
                recent_trend = sum(history.prices[-5:]) / 5
                older_trend = sum(history.prices[-10:-5]) / 5 if len(history.prices) >= 10 else recent_trend
                if older_trend > 0:
                    history.trend = (recent_trend - older_trend) / older_trend
                else:
                    history.trend = 0.0
    
    def get_market_sentiment(self) -> MarketSentiment:
        """Determine overall market sentiment"""
        confidence = self.market_psychology.confidence
        volatility = self._get_average_volatility()
        
        if confidence > 0.7 and volatility < 0.1:
            return MarketSentiment.BULLISH
        elif confidence < 0.3:
            return MarketSentiment.BEARISH
        elif volatility > 0.2:
            return MarketSentiment.VOLATILE
        elif confidence < 0.2 and volatility > 0.3:
            return MarketSentiment.PANIC
        else:
            return MarketSentiment.STABLE
    
    def _get_average_volatility(self) -> float:
        """Calculate average market volatility"""
        if not self.price_histories:
            return 0.1
            
        volatilities = [h.volatility for h in self.price_histories.values()]
        return sum(volatilities) / len(volatilities)