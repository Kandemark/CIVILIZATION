"""Domestic and international trade"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
import random
from base_economy import ResourceType, EconomicEntity

class RouteType(Enum):
    LAND = "land"
    SEA = "sea"
    RIVER = "river"
    AIR = "air"
    DIGITAL = "digital"

class RouteEfficiency(Enum):
    VERY_LOW = "very_low"    # 0.2
    LOW = "low"              # 0.4
    MEDIUM = "medium"        # 0.6
    HIGH = "high"            # 0.8
    VERY_HIGH = "very_high"  # 1.0

@dataclass
class TradeRoute:
    route_id: str
    origin: str
    destination: str
    route_type: RouteType
    capacity: float
    base_efficiency: float
    distance: float
    resources_traded: Dict[ResourceType, float]
    maintenance_cost: Dict[ResourceType, float]
    risks: Dict[str, float]  # risk_type -> probability
    
    @property
    def effective_capacity(self) -> float:
        """Calculate effective capacity considering efficiency and risks"""
        risk_factor = 1.0 - sum(self.risks.values()) / 10.0  # Average risk impact
        return self.capacity * self.base_efficiency * risk_factor
    
    def calculate_transport_cost(self, resource: ResourceType, amount: float) -> float:
        """Calculate cost to transport goods via this route"""
        base_cost_per_unit = self.distance * 0.01
        
        # Resource-specific multipliers
        resource_multipliers = {
            ResourceType.FOOD: 1.2,      # Perishable
            ResourceType.WOOD: 1.0,      # Bulk
            ResourceType.STONE: 1.5,     # Heavy
            ResourceType.IRON: 1.3,      # Heavy
            ResourceType.GOLD: 0.8,      # High value, small volume
            ResourceType.ELECTRONICS: 0.9, # High value, fragile
            ResourceType.OIL: 1.4,       # Special handling
        }
        
        multiplier = resource_multipliers.get(resource, 1.0)
        route_type_multiplier = self._get_route_type_multiplier()
        
        return base_cost_per_unit * amount * multiplier * route_type_multiplier
    
    def _get_route_type_multiplier(self) -> float:
        """Get cost multiplier based on route type"""
        multipliers = {
            RouteType.LAND: 1.0,
            RouteType.SEA: 0.7,
            RouteType.RIVER: 0.8,
            RouteType.AIR: 2.0,
            RouteType.DIGITAL: 0.3
        }
        return multipliers.get(self.route_type, 1.0)
    
    def simulate_route_operation(self, trade_volume: Dict[ResourceType, float]) -> Tuple[Dict[ResourceType, float], float]:
        """Simulate one turn of route operation"""
        successful_trade = {}
        total_cost = 0.0
        
        for resource, amount in trade_volume.items():
            if amount <= self.effective_capacity:
                # Route can handle this volume
                cost = self.calculate_transport_cost(resource, amount)
                successful_trade[resource] = amount
                total_cost += cost
            else:
                # Partial capacity
                partial_amount = self.effective_capacity
                cost = self.calculate_transport_cost(resource, partial_amount)
                successful_trade[resource] = partial_amount
                total_cost += cost
        
        # Apply random events based on risks
        for risk_type, probability in self.risks.items():
            if random.random() < probability:
                self._apply_risk_effect(risk_type, successful_trade)
        
        return successful_trade, total_cost
    
    def _apply_risk_effect(self, risk_type: str, trade_volume: Dict[ResourceType, float]):
        """Apply effects of realized risks"""
        if risk_type == "piracy" and self.route_type == RouteType.SEA:
            # Lose 10-30% of goods
            loss_factor = random.uniform(0.1, 0.3)
            for resource in trade_volume:
                trade_volume[resource] *= (1.0 - loss_factor)
        
        elif risk_type == "banditry" and self.route_type == RouteType.LAND:
            # Lose 5-20% of goods
            loss_factor = random.uniform(0.05, 0.2)
            for resource in trade_volume:
                trade_volume[resource] *= (1.0 - loss_factor)
        
        elif risk_type == "weather" and self.route_type in [RouteType.SEA, RouteType.AIR]:
            # Delay and damage
            delay_factor = random.uniform(0.1, 0.4)
            for resource in trade_volume:
                trade_volume[resource] *= (1.0 - delay_factor * 0.5)

class TradeNetwork:
    """Manages the complete network of trade routes between economic entities"""
    
    def __init__(self):
        self.routes: Dict[str, TradeRoute] = {}
        self.network_nodes: Set[str] = set()
        self.network_connectivity: Dict[str, Set[str]] = {}  # node -> connected nodes
        self.trade_flows: Dict[Tuple[str, str], Dict[ResourceType, float]] = {}
        self.network_efficiency = 1.0
    
    def add_route(self, route: TradeRoute):
        """Add a trade route to the network"""
        self.routes[route.route_id] = route
        self.network_nodes.add(route.origin)
        self.network_nodes.add(route.destination)
        
        # Update connectivity
        if route.origin not in self.network_connectivity:
            self.network_connectivity[route.origin] = set()
        if route.destination not in self.network_connectivity:
            self.network_connectivity[route.destination] = set()
        
        self.network_connectivity[route.origin].add(route.destination)
        self.network_connectivity[route.destination].add(route.origin)
        
        self._calculate_network_efficiency()
    
    def _calculate_network_efficiency(self):
        """Calculate overall network efficiency"""
        if not self.routes:
            self.network_efficiency = 0.0
            return
        
        total_efficiency = sum(route.base_efficiency for route in self.routes.values())
        self.network_efficiency = total_efficiency / len(self.routes)
    
    def find_optimal_route(self, origin: str, destination: str, 
                          resource: ResourceType, amount: float) -> Optional[Tuple[List[str], float]]:
        """Find the most efficient route between two nodes"""
        if origin == destination:
            return None
        
        # Use Dijkstra's algorithm to find shortest path
        distances = {node: float('inf') for node in self.network_nodes}
        previous = {node: None for node in self.network_nodes}
        distances[origin] = 0
        
        unvisited = set(self.network_nodes)
        
        while unvisited:
            # Get node with smallest distance
            current = min(unvisited, key=lambda node: distances[node])
            
            if distances[current] == float('inf'):
                break
            
            unvisited.remove(current)
            
            # Update distances to neighbors
            for neighbor in self.network_connectivity.get(current, set()):
                route = self._find_direct_route(current, neighbor)
                if route:
                    alternative = distances[current] + route.distance
                    if alternative < distances[neighbor]:
                        distances[neighbor] = alternative
                        previous[neighbor] = current
        
        # Reconstruct path
        if distances[destination] == float('inf'):
            return None
        
        path = []
        current = destination
        while current is not None:
            path.append(current)
            current = previous[current]
        
        path.reverse()
        
        # Calculate total cost
        total_cost = 0.0
        for i in range(len(path) - 1):
            route = self._find_direct_route(path[i], path[i + 1])
            if route:
                total_cost += route.calculate_transport_cost(resource, amount)
        
        return path, total_cost
    
    def _find_direct_route(self, origin: str, destination: str) -> Optional[TradeRoute]:
        """Find a direct route between two nodes"""
        for route in self.routes.values():
            if (route.origin == origin and route.destination == destination) or \
               (route.origin == destination and route.destination == origin):
                return route
        return None
    
    def simulate_network_turn(self, trade_demands: Dict[Tuple[str, str], Dict[ResourceType, float]]) -> Dict[str, Dict[ResourceType, float]]:
        """Simulate one turn of trade across the entire network"""
        net_trade_flows = {node: {} for node in self.network_nodes}
        
        for (origin, destination), resources in trade_demands.items():
            for resource, amount in resources.items():
                # Find optimal route
                route_info = self.find_optimal_route(origin, destination, resource, amount)
                if route_info:
                    path, cost = route_info
                    
                    # Execute trade along the route
                    successful_trade = amount  # Simplified - assume full success for now
                    
                    # Update trade flows
                    if origin not in net_trade_flows:
                        net_trade_flows[origin] = {}
                    if destination not in net_trade_flows:
                        net_trade_flows[destination] = {}
                    
                    net_trade_flows[origin][resource] = net_trade_flows[origin].get(resource, 0) - successful_trade
                    net_trade_flows[destination][resource] = net_trade_flows[destination].get(resource, 0) + successful_trade
        
        return net_trade_flows
    
    def calculate_network_resilience(self) -> float:
        """Calculate how resilient the trade network is to disruptions"""
        if len(self.network_nodes) < 2:
            return 0.0
        
        # Calculate average node connectivity
        total_connectivity = sum(len(connections) for connections in self.network_connectivity.values())
        average_connectivity = total_connectivity / len(self.network_nodes)
        
        # Calculate route redundancy
        direct_routes = 0
        for node in self.network_nodes:
            for other_node in self.network_nodes:
                if node != other_node:
                    paths = self._find_all_routes(node, other_node, max_paths=3)
                    direct_routes += len(paths)
        
        max_possible_routes = len(self.network_nodes) * (len(self.network_nodes) - 1)
        route_redundancy = direct_routes / max_possible_routes if max_possible_routes > 0 else 0
        
        # Calculate risk diversification
        total_risk = 0.0
        for route in self.routes.values():
            total_risk += sum(route.risks.values())
        average_risk = total_risk / len(self.routes) if self.routes else 0.0
        risk_diversification = 1.0 - min(1.0, average_risk)
        
        resilience = (average_connectivity * 0.4 + 
                     route_redundancy * 0.4 + 
                     risk_diversification * 0.2)
        
        return resilience
    
    def _find_all_routes(self, start: str, end: str, path: List[str] = None, max_paths: int = 5) -> List[List[str]]:
        """Find all possible routes between two nodes (up to max_paths)"""
        if path is None:
            path = []
        
        path = path + [start]
        
        if start == end:
            return [path]
        
        if start not in self.network_connectivity:
            return []
        
        paths = []
        for node in self.network_connectivity[start]:
            if node not in path:
                new_paths = self._find_all_routes(node, end, path, max_paths)
                for new_path in new_paths:
                    paths.append(new_path)
                    if len(paths) >= max_paths:
                        return paths
        
        return paths
    
    def identify_critical_nodes(self) -> List[Tuple[str, float]]:
        """Identify the most critical nodes in the trade network"""
        node_criticality = {}
        
        for node in self.network_nodes:
            # Calculate betweenness centrality (simplified)
            centrality = 0
            for start in self.network_nodes:
                for end in self.network_nodes:
                    if start != end and start != node and end != node:
                        route = self.find_optimal_route(start, end, ResourceType.FOOD, 1.0)
                        if route and node in route[0]:
                            centrality += 1
            
            # Calculate connectivity importance
            connectivity = len(self.network_connectivity.get(node, set()))
            
            criticality = (centrality * 0.6 + connectivity * 0.4)
            node_criticality[node] = criticality
        
        return sorted(node_criticality.items(), key=lambda x: x[1], reverse=True)