"""Visualizes system relationships"""

# Module implementation

"""
Dependency graph system for visualizing and analyzing relationships between game systems.
"""

from typing import Dict, List, Set, Tuple, Optional
from enum import Enum
import networkx as nx
import matplotlib.pyplot as plt
from dataclasses import dataclass

class RelationshipType(Enum):
    """Types of relationships between systems."""
    DIRECT_INFLUENCE = "directly_influences"
    FEEDBACK_LOOP = "feedback_loop"
    DEPENDENCY = "depends_on"
    CORRELATION = "correlates_with"
    CAUSALITY = "causes"

@dataclass
class SystemRelationship:
    """Represents a relationship between two systems."""
    source: str
    target: str
    relationship_type: RelationshipType
    strength: float  # 0.0 to 1.0
    description: str
    bidirectional: bool = False

class DependencyGraph:
    """Graph-based system for tracking dependencies between game systems."""
    
    def __init__(self):
        self.graph = nx.DiGraph()
        self.relationships: List[SystemRelationship] = []
        self.system_categories: Dict[str, str] = {}
        
    def add_system(self, system_name: str, category: str = "uncategorized"):
        """Add a system to the dependency graph."""
        self.graph.add_node(system_name, category=category)
        self.system_categories[system_name] = category
        
    def add_relationship(self, relationship: SystemRelationship):
        """Add a relationship between systems."""
        self.relationships.append(relationship)
        
        # Add to networkx graph
        self.graph.add_edge(
            relationship.source, 
            relationship.target,
            relationship_type=relationship.relationship_type.value,
            strength=relationship.strength,
            description=relationship.description
        )
        
        if relationship.bidirectional:
            self.graph.add_edge(
                relationship.target,
                relationship.source,
                relationship_type=relationship.relationship_type.value,
                strength=relationship.strength,
                description=relationship.description
            )
    
    def find_dependencies(self, system_name: str) -> List[Tuple[str, float]]:
        """Find all systems that the given system depends on."""
        dependencies = []
        for predecessor in self.graph.predecessors(system_name):
            edge_data = self.graph[predecessor][system_name]
            dependencies.append((predecessor, edge_data['strength']))
        
        return sorted(dependencies, key=lambda x: x[1], reverse=True)
    
    def find_dependents(self, system_name: str) -> List[Tuple[str, float]]:
        """Find all systems that depend on the given system."""
        dependents = []
        for successor in self.graph.successors(system_name):
            edge_data = self.graph[system_name][successor]
            dependents.append((successor, edge_data['strength']))
        
        return sorted(dependents, key=lambda x: x[1], reverse=True)
    
    def calculate_system_centrality(self) -> Dict[str, float]:
        """Calculate centrality measures for all systems."""
        centrality = nx.betweenness_centrality(self.graph, weight='strength')
        return centrality
    
    def find_critical_paths(self) -> List[List[str]]:
        """Find critical paths in the dependency graph."""
        # Find nodes with no incoming edges (root nodes)
        root_nodes = [node for node in self.graph.nodes() if self.graph.in_degree(node) == 0]
        
        critical_paths = []
        for root in root_nodes:
            # Find all paths from root to leaf nodes
            leaf_nodes = [node for node in self.graph.nodes() if self.graph.out_degree(node) == 0]
            
            for leaf in leaf_nodes:
                if nx.has_path(self.graph, root, leaf):
                    all_paths = list(nx.all_simple_paths(self.graph, root, leaf))
                    if all_paths:
                        # Find the longest path (most dependencies)
                        critical_path = max(all_paths, key=len)
                        critical_paths.append(critical_path)
        
        return critical_paths
    
    def visualize(self, filename: Optional[str] = None):
        """Visualize the dependency graph."""
        plt.figure(figsize=(16, 12))
        
        # Create color map based on categories
        categories = set(self.system_categories.values())
        color_map = plt.cm.tab10(range(len(categories)))
        category_colors = {cat: color_map[i] for i, cat in enumerate(categories)}
        
        node_colors = [category_colors[self.system_categories[node]] for node in self.graph.nodes()]
        
        # Layout and draw
        pos = nx.spring_layout(self.graph, k=3, iterations=50)
        
        # Draw nodes
        nx.draw_networkx_nodes(self.graph, pos, node_color=node_colors, node_size=2000, alpha=0.8)
        
        # Draw edges with weights
        edge_widths = [self.graph[u][v]['strength'] * 5 for u, v in self.graph.edges()]
        nx.draw_networkx_edges(self.graph, pos, width=edge_widths, alpha=0.5, edge_color='gray')
        
        # Draw labels
        nx.draw_networkx_labels(self.graph, pos, font_size=8)
        
        # Draw edge labels
        edge_labels = {(u, v): f"{self.graph[u][v]['relationship_type']}\n{self.graph[u][v]['strength']:.2f}" 
                      for u, v in self.graph.edges()}
        nx.draw_networkx_edge_labels(self.graph, pos, edge_labels=edge_labels, font_size=6)
        
        # Create legend
        from matplotlib.patches import Patch
        legend_elements = [Patch(facecolor=category_colors[cat], label=cat) for cat in categories]
        plt.legend(handles=legend_elements, loc='upper left')
        
        plt.title("System Dependency Graph")
        plt.axis('off')
        plt.tight_layout()
        
        if filename:
            plt.savefig(filename, dpi=300, bbox_inches='tight')
        else:
            plt.show()
    
    def analyze_feedback_loops(self) -> List[List[str]]:
        """Find feedback loops in the system."""
        try:
            # Find cycles in the graph (feedback loops)
            cycles = list(nx.simple_cycles(self.graph))
            return cycles
        except nx.NetworkXNoCycle:
            return []
    
    def simulate_impact(self, source_system: str, change_magnitude: float) -> Dict[str, float]:
        """
        Simulate the impact of a change in one system on all other systems.
        Returns a dictionary of system -> impact magnitude.
        """
        impacts = {source_system: change_magnitude}
        
        # Use BFS to propagate impacts
        visited = set([source_system])
        queue = [(source_system, change_magnitude)]
        
        while queue:
            current_system, current_impact = queue.pop(0)
            
            for neighbor in self.graph.successors(current_system):
                if neighbor not in visited:
                    edge_data = self.graph[current_system][neighbor]
                    # Calculate propagated impact (diminishing returns)
                    propagation_factor = edge_data['strength'] * 0.7  # Dampening factor
                    neighbor_impact = current_impact * propagation_factor
                    
                    impacts[neighbor] = impacts.get(neighbor, 0) + neighbor_impact
                    visited.add(neighbor)
                    queue.append((neighbor, neighbor_impact))
        
        return impacts

# Example usage and predefined relationships
def create_default_dependency_graph() -> DependencyGraph:
    """Create a default dependency graph with common system relationships."""
    graph = DependencyGraph()
    
    # Add systems with categories
    systems = [
        ("economy", "Economic"),
        ("population", "Social"),
        ("technology", "Technological"),
        ("military", "Military"),
        ("diplomacy", "Political"),
        ("environment", "Environmental"),
        ("culture", "Cultural"),
        ("government", "Political")
    ]
    
    for system_name, category in systems:
        graph.add_system(system_name, category)
    
    # Define relationships
    relationships = [
        SystemRelationship("economy", "technology", RelationshipType.DIRECT_INFLUENCE, 0.8,
                          "Economic strength enables technological research", True),
        SystemRelationship("technology", "military", RelationshipType.DIRECT_INFLUENCE, 0.7,
                          "Technology improves military capabilities"),
        SystemRelationship("military", "diplomacy", RelationshipType.DIRECT_INFLUENCE, 0.6,
                          "Military strength influences diplomatic relations"),
        SystemRelationship("diplomacy", "economy", RelationshipType.DIRECT_INFLUENCE, 0.5,
                          "Diplomatic relations affect trade and economy", True),
        SystemRelationship("population", "economy", RelationshipType.DIRECT_INFLUENCE, 0.9,
                          "Population provides workforce for economy", True),
        SystemRelationship("economy", "population", RelationshipType.DIRECT_INFLUENCE, 0.7,
                          "Economy affects population happiness and growth"),
        SystemRelationship("environment", "population", RelationshipType.DIRECT_INFLUENCE, 0.6,
                          "Environment affects population health"),
        SystemRelationship("technology", "environment", RelationshipType.DIRECT_INFLUENCE, 0.4,
                          "Technology can improve or harm environment"),
        SystemRelationship("government", "economy", RelationshipType.DIRECT_INFLUENCE, 0.7,
                          "Government policies affect economy"),
        SystemRelationship("culture", "population", RelationshipType.DIRECT_INFLUENCE, 0.8,
                          "Culture influences population behavior", True),
        SystemRelationship("culture", "diplomacy", RelationshipType.DIRECT_INFLUENCE, 0.5,
                          "Cultural influence affects diplomacy"),
    ]
    
    for relationship in relationships:
        graph.add_relationship(relationship)
    
    return graph