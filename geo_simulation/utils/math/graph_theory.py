"""Graph theory utilities"""

# Module implementation

"""
Graph Theory Utilities for Network Analysis

Implements graph algorithms for trade routes, diplomacy networks, and pathfinding.
"""

from typing import Dict, List, Optional, Tuple, Set, Any, Union
from dataclasses import dataclass, field
import heapq
import logging
from collections import defaultdict, deque
import random

logger = logging.getLogger(__name__)


@dataclass
class Node:
    """A node in a graph."""
    id: str
    data: Dict[str, Any] = field(default_factory=dict)
    position: Optional[Tuple[float, float]] = None


@dataclass
class Edge:
    """An edge in a graph with weight and direction."""
    source: str
    target: str
    weight: float = 1.0
    directed: bool = False
    data: Dict[str, Any] = field(default_factory=dict)


class Graph:
    """A graph data structure with nodes and edges."""
    
    def __init__(self, directed: bool = False):
        self.directed = directed
        self.nodes: Dict[str, Node] = {}
        self.edges: Dict[Tuple[str, str], Edge] = {}
        self.adjacency: Dict[str, Dict[str, float]] = defaultdict(dict)
    
    def add_node(self, node_id: str, **kwargs) -> Node:
        """Add a node to the graph."""
        node = Node(id=node_id, data=kwargs)
        self.nodes[node_id] = node
        return node
    
    def add_edge(self, source: str, target: str, weight: float = 1.0, **kwargs) -> Edge:
        """Add an edge to the graph."""
        if source not in self.nodes:
            self.add_node(source)
        if target not in self.nodes:
            self.add_node(target)
        
        edge = Edge(source=source, target=target, weight=weight, data=kwargs)
        self.edges[(source, target)] = edge
        self.adjacency[source][target] = weight
        
        if not self.directed:
            self.adjacency[target][source] = weight
            self.edges[(target, source)] = edge
        
        return edge
    
    def remove_node(self, node_id: str) -> bool:
        """Remove a node and its edges from the graph."""
        if node_id not in self.nodes:
            return False
        
        # Remove edges connected to this node
        edges_to_remove = []
        for edge_key in self.edges:
            if node_id in edge_key:
                edges_to_remove.append(edge_key)
        
        for edge_key in edges_to_remove:
            del self.edges[edge_key]
        
        # Remove from adjacency lists
        if node_id in self.adjacency:
            del self.adjacency[node_id]
        
        for adj in self.adjacency.values():
            if node_id in adj:
                del adj[node_id]
        
        # Remove node
        del self.nodes[node_id]
        return True
    
    def get_neighbors(self, node_id: str) -> List[str]:
        """Get neighbors of a node."""
        return list(self.adjacency.get(node_id, {}).keys())
    
    def get_edge_weight(self, source: str, target: str) -> Optional[float]:
        """Get weight of an edge."""
        return self.adjacency.get(source, {}).get(target)
    
    def degree(self, node_id: str) -> int:
        """Get degree of a node."""
        return len(self.adjacency.get(node_id, {}))
    
    def bfs(self, start: str, goal: str) -> Optional[List[str]]:
        """Breadth-first search for path finding."""
        if start not in self.nodes or goal not in self.nodes:
            return None
        
        queue = deque([(start, [start])])
        visited = set([start])
        
        while queue:
            current, path = queue.popleft()
            
            if current == goal:
                return path
            
            for neighbor in self.get_neighbors(current):
                if neighbor not in visited:
                    visited.add(neighbor)
                    queue.append((neighbor, path + [neighbor]))
        
        return None
    
    def dfs(self, start: str, goal: str, max_depth: int = 100) -> Optional[List[str]]:
        """Depth-first search for path finding."""
        if start not in self.nodes or goal not in self.nodes:
            return None
        
        stack = [(start, [start], 0)]
        visited = set()
        
        while stack:
            current, path, depth = stack.pop()
            
            if current == goal:
                return path
            
            if depth < max_depth and current not in visited:
                visited.add(current)
                for neighbor in self.get_neighbors(current):
                    if neighbor not in visited:
                        stack.append((neighbor, path + [neighbor], depth + 1))
        
        return None


def shortest_path(graph: Graph, start: str, goal: str, heuristic: Optional[Callable[[str, str], float]] = None) -> Optional[Tuple[List[str], float]]:
    """Find shortest path using A* algorithm."""
    if start not in graph.nodes or goal not in graph.nodes:
        return None
    
    # Default heuristic (constant 0 for Dijkstra)
    if heuristic is None:
        heuristic = lambda u, v: 0.0
    
    # Priority queue: (f_score, node, path, g_score)
    open_set = []
    heapq.heappush(open_set, (heuristic(start, goal), start, [start], 0.0))
    
    g_scores = {start: 0.0}
    closed_set = set()
    
    while open_set:
        current_f, current, path, current_g = heapq.heappop(open_set)
        
        if current in closed_set:
            continue
        
        if current == goal:
            return path, current_g
        
        closed_set.add(current)
        
        for neighbor in graph.get_neighbors(current):
            if neighbor in closed_set:
                continue
            
            edge_weight = graph.get_edge_weight(current, neighbor)
            if edge_weight is None:
                continue
            
            tentative_g = current_g + edge_weight
            
            if neighbor not in g_scores or tentative_g < g_scores[neighbor]:
                g_scores[neighbor] = tentative_g
                f_score = tentative_g + heuristic(neighbor, goal)
                heapq.heappush(open_set, (f_score, neighbor, path + [neighbor], tentative_g))
    
    return None


def minimum_spanning_tree(graph: Graph) -> Graph:
    """Find minimum spanning tree using Prim's algorithm."""
    if not graph.nodes:
        return Graph(directed=graph.directed)
    
    mst = Graph(directed=graph.directed)
    start_node = next(iter(graph.nodes.keys()))
    mst.add_node(start_node)
    
    # Priority queue: (weight, source, target)
    edges_queue = []
    
    while len(mst.nodes) < len(graph.nodes):
        # Add edges from current MST to outside nodes
        for node in list(mst.nodes.keys()):
            for neighbor, weight in graph.adjacency.get(node, {}).items():
                if neighbor not in mst.nodes:
                    heapq.heappush(edges_queue, (weight, node, neighbor))
        
        if not edges_queue:
            break
        
        # Get minimum weight edge
        weight, source, target = heapq.heappop(edges_queue)
        
        if target not in mst.nodes:
            mst.add_node(target)
            mst.add_edge(source, target, weight=weight)
    
    return mst


def betweenness_centrality(graph: Graph, normalized: bool = True) -> Dict[str, float]:
    """Calculate betweenness centrality for all nodes."""
    centrality = defaultdict(float)
    nodes = list(graph.nodes.keys())
    
    for i, s in enumerate(nodes):
        # Shortest paths from s to all other nodes
        stack = []
        pred = defaultdict(list)
        dist = {s: 0}
        sigma = defaultdict(float)
        sigma[s] = 1
        
        queue = deque([s])
        while queue:
            v = queue.popleft()
            stack.append(v)
            for w in graph.get_neighbors(v):
                # Path discovery
                if w not in dist:
                    dist[w] = dist[v] + 1
                    queue.append(w)
                
                # Path counting
                if dist[w] == dist[v] + 1:
                    sigma[w] += sigma[v]
                    pred[w].append(v)
        
        # Accumulation
        delta = defaultdict(float)
        while stack:
            w = stack.pop()
            for v in pred[w]:
                delta[v] += (sigma[v] / sigma[w]) * (1 + delta[w])
            if w != s:
                centrality[w] += delta[w]
    
    # Normalization
    if normalized:
        n = len(nodes)
        if n <= 2:
            scale = 1.0
        else:
            scale = 1.0 / ((n - 1) * (n - 2))
        
        for node in centrality:
            centrality[node] *= scale
    
    return dict(centrality)


def community_detection(graph: Graph, resolution: float = 1.0) -> Dict[str, int]:
    """Detect communities using label propagation."""
    if not graph.nodes:
        return {}
    
    # Initialize each node with its own community
    communities = {node: i for i, node in enumerate(graph.nodes)}
    changed = True
    
    while changed:
        changed = False
        nodes = list(graph.nodes.keys())
        random.shuffle(nodes)  # Random order for asynchronous update
        
        for node in nodes:
            # Count neighbors by community
            neighbor_communities = defaultdict(int)
            for neighbor in graph.get_neighbors(node):
                neighbor_communities[communities[neighbor]] += 1
            
            if not neighbor_communities:
                continue
            
            # Find most frequent community
            max_community = max(neighbor_communities.items(), key=lambda x: x[1])[0]
            
            if communities[node] != max_community:
                communities[node] = max_community
                changed = True
    
    return communities