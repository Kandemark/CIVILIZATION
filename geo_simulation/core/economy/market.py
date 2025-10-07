"""Market system"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple, Any
from dataclasses import dataclass
from enum import Enum
import random
from base_economy import ResourceType, EconomicEntity

class MarketType(Enum):
    LOCAL = "local"           # Village/town level
    REGIONAL = "regional"     # City/regional level  
    NATIONAL = "national"     # Country level
    INTERNATIONAL = "international" # Global trade
    COMMODITY = "commodity"   # Specialized resource markets
    FINANCIAL = "financial"   # Capital markets

class OrderType(Enum):
    MARKET_ORDER = "market_order"     # Execute at current price
    LIMIT_ORDER = "limit_order"       # Execute at specified price or better
    STOP_ORDER = "stop_order"         # Execute when price reaches level

@dataclass
class TradeOrder:
    order_id: str
    entity: EconomicEntity
    resource: ResourceType
    order_type: OrderType
    quantity: float
    price: float  # For limit orders, max/min price
    is_buy: bool
    timestamp: int
    filled_quantity: float = 0.0
    active: bool = True
    
    @property
    def remaining_quantity(self) -> float:
        return self.quantity - self.filled_quantity

@dataclass
class PriceLevel:
    price: float
    total_quantity: float
    orders: List[TradeOrder]

class OrderBook:
    """Represents the order book for a single resource"""
    
    def __init__(self, resource: ResourceType):
        self.resource = resource
        self.buy_orders: List[PriceLevel] = []  # Sorted high to low
        self.sell_orders: List[PriceLevel] = [] # Sorted low to high
        self.last_trade_price: Optional[float] = None
        self.volume_traded = 0.0
        self.price_history: List[float] = []
        
    def add_order(self, order: TradeOrder):
        """Add an order to the appropriate side of the book"""
        target_orders = self.buy_orders if order.is_buy else self.sell_orders
        
        # Find or create price level
        price_level = None
        for level in target_orders:
            if level.price == order.price:
                price_level = level
                break
        
        if price_level is None:
            price_level = PriceLevel(order.price, 0.0, [])
            target_orders.append(price_level)
            # Re-sort orders
            if order.is_buy:
                self.buy_orders.sort(key=lambda x: x.price, reverse=True)
            else:
                self.sell_orders.sort(key=lambda x: x.price)
        
        price_level.orders.append(order)
        price_level.total_quantity += order.remaining_quantity
    
    def match_orders(self) -> List[Tuple[TradeOrder, TradeOrder, float, float]]:
        """Match buy and sell orders, return executed trades"""
        executed_trades = []
        
        while self.buy_orders and self.sell_orders:
            best_buy = self.buy_orders[0]   # Highest buy price
            best_sell = self.sell_orders[0] # Lowest sell price
            
            if best_buy.price >= best_sell.price:
                # Trade can occur
                buy_order = best_buy.orders[0]
                sell_order = best_sell.orders[0]
                
                trade_quantity = min(buy_order.remaining_quantity, sell_order.remaining_quantity)
                trade_price = (best_buy.price + best_sell.price) / 2  # Mid-price execution
                
                # Execute trade
                self._execute_trade(buy_order, sell_order, trade_quantity, trade_price)
                executed_trades.append((buy_order, sell_order, trade_quantity, trade_price))
                
                # Update price levels
                best_buy.total_quantity -= trade_quantity
                best_sell.total_quantity -= trade_quantity
                
                # Remove filled orders
                if buy_order.remaining_quantity <= 0:
                    best_buy.orders.pop(0)
                if sell_order.remaining_quantity <= 0:
                    best_sell.orders.pop(0)
                
                # Remove empty price levels
                if not best_buy.orders:
                    self.buy_orders.pop(0)
                if not best_sell.orders:
                    self.sell_orders.pop(0)
                
                # Update market data
                self.last_trade_price = trade_price
                self.volume_traded += trade_quantity
                self.price_history.append(trade_price)
                
            else:
                break  # No more matches possible
        
        return executed_trades
    
    def _execute_trade(self, buy_order: TradeOrder, sell_order: TradeOrder, 
                      quantity: float, price: float):
        """Execute a single trade between buyer and seller"""
        # Update order quantities
        buy_order.filled_quantity += quantity
        sell_order.filled_quantity += quantity
        
        # Calculate total value
        total_value = quantity * price
        
        # Transfer resources and wealth (simplified - in reality would interface with entity systems)
        if hasattr(buy_order.entity, 'inventory'):
            buy_order.entity.inventory[buy_order.resource] = \
                buy_order.entity.inventory.get(buy_order.resource, 0) + quantity
        
        if hasattr(sell_order.entity, 'inventory'):
            current_amount = sell_order.entity.inventory.get(sell_order.resource, 0)
            sell_order.entity.inventory[sell_order.resource] = max(0, current_amount - quantity)
        
        # Transfer wealth
        buy_order.entity.wealth -= total_value
        sell_order.entity.wealth += total_value
        
        # Check if orders are completely filled
        if buy_order.remaining_quantity <= 0:
            buy_order.active = False
        if sell_order.remaining_quantity <= 0:
            sell_order.active = False
    
    def get_market_depth(self) -> Tuple[List[Tuple[float, float]], List[Tuple[float, float]]]:
        """Get market depth for buy and sell sides"""
        buy_depth = [(level.price, level.total_quantity) for level in self.buy_orders[:10]]  # Top 10 levels
        sell_depth = [(level.price, level.total_quantity) for level in self.sell_orders[:10]]
        return buy_depth, sell_depth
    
    def calculate_vwap(self, period: int = 10) -> Optional[float]:
        """Calculate Volume Weighted Average Price for recent trades"""
        if not self.price_history or period <= 0:
            return self.last_trade_price
        
        recent_trades = self.price_history[-period:]
        if not recent_trades:
            return self.last_trade_price
        
        # Simplified VWAP (in reality would use actual trade volumes)
        return sum(recent_trades) / len(recent_trades)

class Market:
    """Represents a complete market for multiple resources"""
    
    def __init__(self, market_id: str, name: str, market_type: MarketType, location: str):
        self.market_id = market_id
        self.name = name
        self.market_type = market_type
        self.location = location
        self.order_books: Dict[ResourceType, OrderBook] = {}
        self.participants: Set[EconomicEntity] = set()
        self.transaction_fee = 0.01  # 1% transaction fee
        self.market_hours = True  # Whether market is open
        self.regulations: Dict[str, Any] = {}
        
        # Market statistics
        self.daily_volume = 0.0
        self.price_volatility: Dict[ResourceType, float] = {}
        self.liquidity: Dict[ResourceType, float] = {}
        
    def add_resource_market(self, resource: ResourceType):
        """Add a new resource to be traded on this market"""
        if resource not in self.order_books:
            self.order_books[resource] = OrderBook(resource)
    
    def place_order(self, order: TradeOrder) -> bool:
        """Place a new trade order in the market"""
        if not self.market_hours:
            return False
        
        if order.resource not in self.order_books:
            self.add_resource_market(order.resource)
        
        order_book = self.order_books[order.resource]
        order_book.add_order(order)
        
        # Add participant if not already registered
        self.participants.add(order.entity)
        
        return True
    
    def execute_market_cycle(self) -> Dict[ResourceType, List[Tuple]]:
        """Execute one market cycle (matching orders)"""
        if not self.market_hours:
            return {}
        
        all_executed_trades = {}
        self.daily_volume = 0.0
        
        for resource, order_book in self.order_books.items():
            executed_trades = order_book.match_orders()
            if executed_trades:
                all_executed_trades[resource] = executed_trades
                self.daily_volume += sum(trade[2] for trade in executed_trades)  # Sum quantities
        
        # Update market statistics
        self._update_market_statistics()
        
        return all_executed_trades
    
    def _update_market_statistics(self):
        """Update market statistics after trading"""
        for resource, order_book in self.order_books.items():
            # Calculate price volatility (standard deviation of recent prices)
            recent_prices = order_book.price_history[-20:]  # Last 20 trades
            if len(recent_prices) >= 2:
                avg_price = sum(recent_prices) / len(recent_prices)
                variance = sum((p - avg_price) ** 2 for p in recent_prices) / len(recent_prices)
                self.price_volatility[resource] = variance ** 0.5 / avg_price if avg_price > 0 else 0.0
            else:
                self.price_volatility[resource] = 0.1  # Default volatility
            
            # Calculate liquidity (total quantity at best bid/ask)
            buy_depth, sell_depth = order_book.get_market_depth()
            best_bid_quantity = buy_depth[0][1] if buy_depth else 0
            best_ask_quantity = sell_depth[0][1] if sell_depth else 0
            self.liquidity[resource] = (best_bid_quantity + best_ask_quantity) / 2
    
    def get_market_data(self, resource: ResourceType) -> Dict[str, Any]:
        """Get comprehensive market data for a resource"""
        if resource not in self.order_books:
            return {}
        
        order_book = self.order_books[resource]
        buy_depth, sell_depth = order_book.get_market_depth()
        
        return {
            'resource': resource,
            'last_price': order_book.last_trade_price,
            'vwap': order_book.calculate_vwap(20),
            'daily_volume': order_book.volume_traded,
            'buy_depth': buy_depth,
            'sell_depth': sell_depth,
            'volatility': self.price_volatility.get(resource, 0.1),
            'liquidity': self.liquidity.get(resource, 0.0),
            'price_history': order_book.price_history[-50:]  # Last 50 trades
        }
    
    def calculate_transaction_cost(self, resource: ResourceType, quantity: float, is_buy: bool) -> float:
        """Calculate total transaction costs for a trade"""
        if resource not in self.order_books:
            return 0.0
        
        order_book = self.order_books[resource]
        current_price = order_book.last_trade_price or 100  # Default price
        
        # Base transaction fee
        base_fee = quantity * current_price * self.transaction_fee
        
        # Market impact cost (larger orders move prices more)
        liquidity = self.liquidity.get(resource, 1.0)
        impact_cost = (quantity / max(liquidity, 1.0)) * current_price * 0.1
        
        # Spread cost (difference between bid and ask)
        buy_depth, sell_depth = order_book.get_market_depth()
        best_bid = buy_depth[0][0] if buy_depth else current_price * 0.99
        best_ask = sell_depth[0][0] if sell_depth else current_price * 1.01
        spread = best_ask - best_bid
        spread_cost = (spread / current_price) * quantity * current_price * 0.5
        
        return base_fee + impact_cost + spread_cost
    
    def set_market_hours(self, open: bool):
        """Set whether market is open or closed"""
        self.market_hours = open
        if not open:
            print(f"🏛️ Market {self.name} is now closed")
        else:
            print(f"🏛️ Market {self.name} is now open")
    
    def implement_regulation(self, regulation_type: str, parameters: Dict[str, Any]):
        """Implement market regulations"""
        self.regulations[regulation_type] = parameters
        
        if regulation_type == "circuit_breaker":
            print(f"⚡ Circuit breaker implemented: {parameters}")
        elif regulation_type == "position_limits":
            print(f"📊 Position limits implemented: {parameters}")
        elif regulation_type == "short_selling_restrictions":
            print(f"📉 Short selling restrictions implemented: {parameters}")

class MarketManager:
    """Manages all markets in the economic system"""
    
    def __init__(self):
        self.markets: Dict[str, Market] = {}
        self.global_price_index: Dict[ResourceType, float] = {}
        self.arbitrage_opportunities: List[Tuple[str, str, ResourceType, float]] = []
        self.initialize_core_markets()
    
    def initialize_core_markets(self):
        """Initialize essential markets"""
        core_markets = [
            ("local_goods", "Local Goods Market", MarketType.LOCAL, "Central Square"),
            ("regional_trade", "Regional Trade Hub", MarketType.REGIONAL, "Trade District"),
            ("national_exchange", "National Commodity Exchange", MarketType.NATIONAL, "Capital City"),
            ("global_trade", "Global Trade Network", MarketType.INTERNATIONAL, "International Waters")
        ]
        
        for market_id, name, market_type, location in core_markets:
            self.add_market(market_id, name, market_type, location)
    
    def add_market(self, market_id: str, name: str, market_type: MarketType, location: str) -> Market:
        """Add a new market to the system"""
        market = Market(market_id, name, market_type, location)
        self.markets[market_id] = market
        return market
    
    def simulate_global_markets(self) -> Dict[str, Any]:
        """Simulate all markets for one turn"""
        global_results = {
            'market_activity': {},
            'price_changes': {},
            'arbitrage_opportunities': [],
            'regulatory_actions': []
        }
        
        # Execute trading in all markets
        for market_id, market in self.markets.items():
            if market.market_hours:
                market_results = market.execute_market_cycle()
                global_results['market_activity'][market_id] = {
                    'volume': market.daily_volume,
                    'trades_executed': sum(len(trades) for trades in market_results.values())
                }
        
        # Update global price index
        self._update_global_price_index()
        global_results['price_changes'] = self._calculate_price_changes()
        
        # Find arbitrage opportunities
        global_results['arbitrage_opportunities'] = self._find_arbitrage_opportunities()
        
        return global_results
    
    def _update_global_price_index(self):
        """Update global price index based on all market prices"""
        for resource in ResourceType:
            prices = []
            weights = []
            
            for market in self.markets.values():
                if resource in market.order_books:
                    market_data = market.get_market_data(resource)
                    if market_data['last_price']:
                        prices.append(market_data['last_price'])
                        # Weight by market liquidity and volume
                        weight = market_data['liquidity'] * market.daily_volume
                        weights.append(weight)
            
            if prices and weights:
                # Calculate weighted average price
                total_weight = sum(weights)
                weighted_price = sum(p * w for p, w in zip(prices, weights)) / total_weight
                self.global_price_index[resource] = weighted_price
    
    def _calculate_price_changes(self) -> Dict[ResourceType, float]:
        """Calculate price changes from previous turn"""
        # This would compare with stored previous prices
        # For now, return random changes for demonstration
        price_changes = {}
        for resource in self.global_price_index:
            change = random.uniform(-0.05, 0.05)  # ±5% change
            price_changes[resource] = change
        return price_changes
    
    def _find_arbitrage_opportunities(self) -> List[Tuple[str, str, ResourceType, float]]:
        """Find arbitrage opportunities between markets"""
        opportunities = []
        
        for resource in ResourceType:
            market_prices = {}
            
            # Collect prices from all markets
            for market_id, market in self.markets.items():
                if resource in market.order_books:
                    market_data = market.get_market_data(resource)
                    if market_data['last_price']:
                        market_prices[market_id] = market_data['last_price']
            
            # Find price differences
            if len(market_prices) >= 2:
                min_market = min(market_prices, key=market_prices.get)
                max_market = max(market_prices, key=market_prices.get)
                price_diff = market_prices[max_market] - market_prices[min_market]
                price_diff_percent = price_diff / market_prices[min_market]
                
                # Arbitrage opportunity if difference exceeds transaction costs
                if price_diff_percent > 0.05:  # 5% threshold
                    opportunities.append((min_market, max_market, resource, price_diff_percent))
        
        return opportunities
    
    def get_best_market_for_trade(self, resource: ResourceType, quantity: float, 
                                is_buy: bool) -> Tuple[Optional[str], float]:
        """Find the best market for a given trade"""
        best_market = None
        best_total_cost = float('inf')
        
        for market_id, market in self.markets.items():
            if resource in market.order_books:
                market_data = market.get_market_data(resource)
                if market_data['last_price']:
                    # Calculate total cost including transaction costs
                    price = market_data['last_price']
                    transaction_cost = market.calculate_transaction_cost(resource, quantity, is_buy)
                    total_cost = (price * quantity) + (transaction_cost if is_buy else -transaction_cost)
                    
                    if (is_buy and total_cost < best_total_cost) or \
                       (not is_buy and total_cost > best_total_cost):
                        best_total_cost = total_cost
                        best_market = market_id
        
        return best_market, best_total_cost
    
    def place_global_order(self, entity: EconomicEntity, resource: ResourceType,
                          quantity: float, is_buy: bool, order_type: OrderType = OrderType.MARKET_ORDER,
                          price: Optional[float] = None) -> bool:
        """Place an order in the best available market"""
        best_market, best_cost = self.get_best_market_for_trade(resource, quantity, is_buy)
        
        if not best_market:
            return False
        
        market = self.markets[best_market]
        
        # Determine order price
        if order_type == OrderType.MARKET_ORDER:
            market_data = market.get_market_data(resource)
            order_price = market_data['last_price'] or 100
        else:  # LIMIT_ORDER or STOP_ORDER
            if price is None:
                return False
            order_price = price
        
        # Create and place order
        order = TradeOrder(
            order_id=f"order_{len(market.participants)}_{entity.id}",
            entity=entity,
            resource=resource,
            order_type=order_type,
            quantity=quantity,
            price=order_price,
            is_buy=is_buy,
            timestamp=self._get_current_turn()
        )
        
        return market.place_order(order)
    
    def _get_current_turn(self) -> int:
        """Get current game turn (would interface with game engine)"""
        return 0  # Placeholder