"""Pathfinding algorithms"""

# Module implementation

# utils/pathfinding.py
from typing import Dict, List, Optional, Tuple, Set
import heapq
import numpy as np
from .geometry import Point
from .math.graph_theory import Graph, AStar, Dijkstra
import logging

logger = logging.getLogger(__name__)

class TerrainWeighting:
    """Terrain-based weighting for pathfinding"""
    
    def __init__(self):
        self.terrain_weights = {
            'plains': 1.0,
            'forest': 1.5,
            'mountains': 3.0,
            'desert': 2.0,
            'water': 10.0,
            'urban': 0.8,
            'swamp': 2.5,
            'road': 0.5
        }
    
    def get_terrain_cost(self, terrain_type: str) -> float:
        """Get movement cost for terrain type"""
        return self.terrain_weights.get(terrain_type, 1.0)
    
    def set_custom_weight(self, terrain_type: str, weight: float):
        """Set custom weight for terrain type"""
        self.terrain_weights[terrain_type] = weight

class Pathfinder:
    """Advanced pathfinding system with multiple algorithms"""
    
    def __init__(self):
        self.graph = Graph()
        self.terrain_weighting = TerrainWeighting()
        self.algorithm_cache: Dict[Tuple[str, str], List[str]] = {}
    
    def add_terrain_node(self, node_id: str, position: Point, terrain_type: str):
        """Add a node with terrain information"""
        self.graph.add_node(node_id, position.x, position.y)
        # Store terrain type as node property
        if not hasattr(self.graph, 'node_properties'):
            self.graph.node_properties = {}
        self.graph.node_properties[node_id] = {'terrain': terrain_type}
    
    def find_path(self, start: str, end: str, algorithm: str = 'astar',
                 consider_terrain: bool = True) -> Tuple[List[str], float]:
        """Find path between nodes using specified algorithm"""
        cache_key = (start, end, algorithm, consider_terrain)
        if cache_key in self.algorithm_cache:
            path = self.algorithm_cache[cache_key]
            cost = self._calculate_path_cost(path, consider_terrain)
            return path, cost
        
        if algorithm == 'astar':
            path, _ = AStar.find_path(self.graph, start, end)
        elif algorithm == 'dijkstra':
            path, _ = Dijkstra.find_shortest_path(self.graph, start, end)
        else:
            raise ValueError(f"Unknown algorithm: {algorithm}")
        
        if consider_terrain:
            # Adjust path cost based on terrain
            path = self._optimize_path_terrain(path)
        
        self.algorithm_cache[cache_key] = path
        cost = self._calculate_path_cost(path, consider_terrain)
        
        return path, cost
    
    def _calculate_path_cost(self, path: List[str], consider_terrain: bool) -> float:
        """Calculate total cost of path"""
        if len(path) < 2:
            return 0.0
        
        total_cost = 0.0
        for i in range(len(path) - 1):
            base_cost = self.graph.edges[path[i]][path[i + 1]]
            
            if consider_terrain:
                # Adjust cost based on terrain
                terrain_cost = 1.0
                if hasattr(self.graph, 'node_properties'):
                    node_props = self.graph.node_properties.get(path[i], {})
                    terrain_type = node_props.get('terrain', 'plains')
                    terrain_cost = self.terrain_weighting.get_terrain_cost(terrain_type)
                
                total_cost += base_cost * terrain_cost
            else:
                total_cost += base_cost
        
        return total_cost
    
    def _optimize_path_terrain(self, path: List[str]) -> List[str]:
        """Optimize path considering terrain costs"""
        if len(path) <= 2:
            return path
        
        # Simple optimization: avoid high-cost terrain when possible
        optimized_path = [path[0]]
        
        for i in range(1, len(path) - 1):
            current_node = path[i]
            
            if hasattr(self.graph, 'node_properties'):
                node_props = self.graph.node_properties.get(current_node, {})
                terrain_type = node_props.get('terrain', 'plains')
                terrain_cost = self.terrain_weighting.get_terrain_cost(terrain_type)
                
                # Skip node if terrain cost is too high and there are alternatives
                if terrain_cost > 2.0 and len(self.graph.get_neighbors(current_node)) > 1:
                    continue
            
            optimized_path.append(current_node)
        
        optimized_path.append(path[-1])
        return optimized_path
    
    def find_multiple_paths(self, start: str, ends: List[str], 
                           algorithm: str = 'astar') -> Dict[str, Tuple[List[str], float]]:
        """Find paths to multiple destinations"""
        results = {}
        
        for end in ends:
            try:
                path, cost = self.find_path(start, end, algorithm)
                results[end] = (path, cost)
            except Exception as e:
                logger.warning(f"Could not find path from {start} to {end}: {e}")
        
        return results

class RouteOptimizer:
    """Route optimization for multiple points"""
    
    @staticmethod
    def traveling_salesman(nodes: List[str], graph: Graph) -> List[str]:
        """Solve traveling salesman problem (approximate)"""
        if len(nodes) <= 1:
            return nodes
        
        # Nearest neighbor heuristic
        unvisited = set(nodes[1:])
        route = [nodes[0]]
        current = nodes[0]
        
        while unvisited:
            nearest = min(unvisited, 
                         key=lambda node: graph.edges[current].get(node, float('inf')))
            route.append(nearest)
            unvisited.remove(nearest)
            current = nearest
        
        return route
    
    @staticmethod
    def optimize_delivery_route(stops: List[str], graph: Graph,
                               capacity_constraints: Dict[str, int] = None) -> List[str]:
        """Optimize delivery route with capacity constraints"""
        # Simplified implementation
        if capacity_constraints:
            # Group stops by capacity requirements
            heavy_stops = [stop for stop in stops if capacity_constraints.get(stop, 0) > 5]
            light_stops = [stop for stop in stops if stop not in heavy_stops]
            
            # Optimize heavy stops first, then light stops
            heavy_route = RouteOptimizer.traveling_salesman(heavy_stops, graph)
            light_route = RouteOptimizer.traveling_salesman(light_stops, graph)
            
            return heavy_route + light_route
        else:
            return RouteOptimizer.traveling_salesman(stops, graph)