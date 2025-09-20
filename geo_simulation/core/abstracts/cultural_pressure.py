"""Cultural influence diffusion"""

# Module implementation


"""
Cultural Influence Diffusion Simulation

This module models how cultural traits spread through populations using
an agent-based approach with network diffusion mechanics. It includes
multiple diffusion models, visualization capabilities, and analysis tools.
"""

import numpy as np
import networkx as nx
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.colors import ListedColormap
import seaborn as sns
from typing import Dict, List, Tuple, Optional, Callable, Any
from enum import Enum
import pandas as pd
from dataclasses import dataclass
from scipy import stats
import warnings
warnings.filterwarnings('ignore')


class DiffusionModel(Enum):
    """Enumeration of available diffusion models."""
    THRESHOLD = "threshold"
    CONTAGION = "contagion"
    SOCIAL_LEARNING = "social_learning"
    COMPETITIVE = "competitive"
    HYBRID = "hybrid"


@dataclass
class SimulationParameters:
    """Container for simulation parameters."""
    model_type: DiffusionModel = DiffusionModel.THRESHOLD
    adoption_threshold: float = 0.3
    infection_rate: float = 0.1
    recovery_rate: float = 0.05
    learning_rate: float = 0.2
    innovation_rate: float = 0.01
    decay_rate: float = 0.02
    num_cultural_traits: int = 5
    initial_adoption_rate: float = 0.1
    max_influence: float = 1.0
    min_influence: float = 0.1


class CulturalAgent:
    """Represents an agent in the cultural diffusion simulation."""
    
    __slots__ = ('id', 'traits', 'influenced', 'resistance', 'connections')
    
    def __init__(self, agent_id: int, num_traits: int):
        self.id = agent_id
        self.traits = np.zeros(num_traits, dtype=np.float32)
        self.influenced = np.zeros(num_traits, dtype=bool)
        self.resistance = np.random.random(num_traits) * 0.5
        self.connections = []
    
    def adopt_trait(self, trait_idx: int, strength: float = 1.0):
        """Adopt a cultural trait with given strength."""
        adoption_prob = strength * (1 - self.resistance[trait_idx])
        if np.random.random() < adoption_prob:
            self.traits[trait_idx] += strength
            self.traits[trait_idx] = min(1.0, self.traits[trait_idx])
            self.influenced[trait_idx] = True
            return True
        return False
    
    def innovate(self, trait_idx: int, rate: float):
        """Generate new cultural innovations."""
        if np.random.random() < rate and not self.influenced[trait_idx]:
            self.traits[trait_idx] += np.random.random() * 0.1
            self.traits[trait_idx] = min(1.0, self.traits[trait_idx])
    
    def decay_traits(self, rate: float):
        """Gradual decay of cultural traits over time."""
        decay_mask = np.random.random(len(self.traits)) < rate
        self.traits[decay_mask] *= 0.95
        self.influenced[decay_mask] = False


class CulturalNetwork:
    """Represents the social network through which culture diffuses."""
    
    def __init__(self, num_agents: int, network_type: str = "scale_free"):
        self.num_agents = num_agents
        self.agents = [CulturalAgent(i, 0) for i in range(num_agents)]
        self.graph = self._create_network(network_type)
        self._setup_connections()
    
    def _create_network(self, network_type: str) -> nx.Graph:
        """Create a network of specified type."""
        if network_type == "scale_free":
            return nx.barabasi_albert_graph(self.num_agents, 2)
        elif network_type == "small_world":
            return nx.watts_strogatz_graph(self.num_agents, 4, 0.1)
        elif network_type == "random":
            return nx.erdos_renyi_graph(self.num_agents, 0.1)
        elif network_type == "complete":
            return nx.complete_graph(self.num_agents)
        else:
            raise ValueError(f"Unknown network type: {network_type}")
    
    def _setup_connections(self):
        """Set up connections between agents based on the network."""
        for i, agent in enumerate(self.agents):
            agent.connections = list(self.graph.neighbors(i))
    
    def add_agent_connections(self, agent_id: int, connections: List[int]):
        """Add custom connections for an agent."""
        self.agents[agent_id].connections.extend(connections)
        for conn in connections:
            self.graph.add_edge(agent_id, conn)
    
    def get_influential_agents(self, top_n: int = 10) -> List[int]:
        """Get the most influential agents based on network centrality."""
        centrality = nx.degree_centrality(self.graph)
        return sorted(centrality.keys(), key=lambda x: centrality[x], reverse=True)[:top_n]


class CulturalDiffusion:
    """
    Main class for simulating cultural diffusion across a network.
    
    Attributes:
        network (CulturalNetwork): The social network
        params (SimulationParameters): Simulation parameters
        history (List[np.ndarray]): History of trait adoption
        time_elapsed (int): Number of simulation steps
    """
    
    def __init__(self, num_agents: int = 100, params: Optional[SimulationParameters] = None):
        self.params = params or SimulationParameters()
        self.network = CulturalNetwork(num_agents)
        self.num_agents = num_agents
        self.history = []
        self.time_elapsed = 0
        
        # Initialize cultural traits for all agents
        for agent in self.network.agents:
            agent.traits = np.zeros(self.params.num_cultural_traits, dtype=np.float32)
            agent.influenced = np.zeros(self.params.num_cultural_traits, dtype=bool)
            agent.resistance = np.random.random(self.params.num_cultural_traits) * 0.5
        
        # Set initial adopters
        initial_adopters = np.random.choice(
            num_agents, 
            size=int(num_agents * self.params.initial_adoption_rate), 
            replace=False
        )
        
        for agent_id in initial_adopters:
            trait_idx = np.random.randint(self.params.num_cultural_traits)
            self.network.agents[agent_id].traits[trait_idx] = 1.0
            self.network.agents[agent_id].influenced[trait_idx] = True
    
    def step(self):
        """Advance the simulation by one time step."""
        new_trait_strengths = np.zeros((self.num_agents, self.params.num_cultural_traits))
        
        for i, agent in enumerate(self.network.agents):
            # Apply the selected diffusion model
            if self.params.model_type == DiffusionModel.THRESHOLD:
                self._apply_threshold_model(i, agent, new_trait_strengths)
            elif self.params.model_type == DiffusionModel.CONTAGION:
                self._apply_contagion_model(i, agent, new_trait_strengths)
            elif self.params.model_type == DiffusionModel.SOCIAL_LEARNING:
                self._apply_social_learning_model(i, agent, new_trait_strengths)
            elif self.params.model_type == DiffusionModel.COMPETITIVE:
                self._apply_competitive_model(i, agent, new_trait_strengths)
            elif self.params.model_type == DiffusionModel.HYBRID:
                self._apply_hybrid_model(i, agent, new_trait_strengths)
            
            # Innovation and decay
            for trait_idx in range(self.params.num_cultural_traits):
                agent.innovate(trait_idx, self.params.innovation_rate)
                agent.decay_traits(self.params.decay_rate)
        
        # Update history
        current_state = np.array([agent.traits for agent in self.network.agents])
        self.history.append(current_state)
        self.time_elapsed += 1
    
    def _apply_threshold_model(self, agent_id: int, agent: CulturalAgent, new_trait_strengths: np.ndarray):
        """Apply threshold model of diffusion."""
        for trait_idx in range(self.params.num_cultural_traits):
            if agent.influenced[trait_idx]:
                continue
                
            neighbor_influence = 0
            for neighbor_id in agent.connections:
                neighbor = self.network.agents[neighbor_id]
                neighbor_influence += neighbor.traits[trait_idx]
            
            avg_influence = neighbor_influence / max(1, len(agent.connections))
            if avg_influence > self.params.adoption_threshold:
                agent.adopt_trait(trait_idx, avg_influence)
    
    def _apply_contagion_model(self, agent_id: int, agent: CulturalAgent, new_trait_strengths: np.ndarray):
        """Apply contagion model of diffusion."""
        for trait_idx in range(self.params.num_cultural_traits):
            for neighbor_id in agent.connections:
                neighbor = self.network.agents[neighbor_id]
                if neighbor.traits[trait_idx] > 0 and np.random.random() < self.params.infection_rate:
                    if agent.adopt_trait(trait_idx, neighbor.traits[trait_idx]):
                        break  # Only adopt one trait per step in contagion model
    
    def _apply_social_learning_model(self, agent_id: int, agent: CulturalAgent, new_trait_strengths: np.ndarray):
        """Apply social learning model of diffusion."""
        for trait_idx in range(self.params.num_cultural_traits):
            if agent.influenced[trait_idx]:
                continue
                
            # Learn from successful neighbors
            successful_neighbors = [
                n for n in agent.connections 
                if self.network.agents[n].traits[trait_idx] > 0.7
            ]
            
            if successful_neighbors:
                exemplar = np.random.choice(successful_neighbors)
                exemplar_trait = self.network.agents[exemplar].traits[trait_idx]
                agent.adopt_trait(trait_idx, exemplar_trait * self.params.learning_rate)
    
    def _apply_competitive_model(self, agent_id: int, agent: CulturalAgent, new_trait_strengths: np.ndarray):
        """Apply competitive model where traits compete for adoption."""
        for trait_idx in range(self.params.num_cultural_traits):
            trait_strength = 0
            
            for neighbor_id in agent.connections:
                neighbor = self.network.agents[neighbor_id]
                trait_strength += neighbor.traits[trait_idx]
            
            new_trait_strengths[agent_id, trait_idx] = trait_strength
        
        # Adopt the trait with highest influence
        if np.max(new_trait_strengths[agent_id]) > 0:
            dominant_trait = np.argmax(new_trait_strengths[agent_id])
            agent.adopt_trait(dominant_trait, np.max(new_trait_strengths[agent_id]))
    
    def _apply_hybrid_model(self, agent_id: int, agent: CulturalAgent, new_trait_strengths: np.ndarray):
        """Apply a hybrid model combining multiple diffusion mechanisms."""
        # Threshold mechanism
        self._apply_threshold_model(agent_id, agent, new_trait_strengths)
        
        # With some probability, also use contagion
        if np.random.random() < 0.3:
            self._apply_contagion_model(agent_id, agent, new_trait_strengths)
    
    def simulate(self, steps: int = 50):
        """Run the simulation for a specified number of steps."""
        for _ in range(steps):
            self.step()
    
    def get_adoption_rate(self, trait_idx: Optional[int] = None) -> np.ndarray:
        """Get the adoption rate for each trait over time."""
        if trait_idx is not None:
            return np.array([np.mean(step[:, trait_idx] > 0.5) for step in self.history])
        else:
            return np.array([np.mean(step > 0.5) for step in self.history])
    
    def get_diversity_index(self) -> np.ndarray:
        """Calculate cultural diversity index over time."""
        diversity = []
        for state in self.history:
            # Calculate entropy-based diversity
            trait_presence = (state > 0.5).astype(int)
            trait_proportions = np.mean(trait_presence, axis=0)
            trait_proportions = trait_proportions[trait_proportions > 0]
            if len(trait_proportions) > 0:
                diversity.append(-np.sum(trait_proportions * np.log(trait_proportions)))
            else:
                diversity.append(0)
        return np.array(diversity)
    
    def get_cultural_distance_matrix(self) -> np.ndarray:
        """Calculate cultural distance matrix between agents."""
        current_state = self.history[-1]
        distances = np.zeros((self.num_agents, self.num_agents))
        for i in range(self.num_agents):
            for j in range(i + 1, self.num_agents):
                distances[i, j] = np.sqrt(np.sum((current_state[i] - current_state[j]) ** 2))
                distances[j, i] = distances[i, j]
        return distances
    
    def visualize_network(self, trait_idx: int = 0, step: int = -1, ax: Optional[plt.Axes] = None):
        """Visualize the network with nodes colored by trait adoption."""
        if ax is None:
            fig, ax = plt.subplots(figsize=(10, 8))
        
        state = self.history[step]
        node_colors = state[:, trait_idx]
        
        pos = nx.spring_layout(self.network.graph)
        nodes = nx.draw_networkx_nodes(
            self.network.graph, pos, 
            node_color=node_colors,
            cmap='viridis',
            ax=ax,
            node_size=100
        )
        nx.draw_networkx_edges(self.network.graph, pos, alpha=0.2, ax=ax)
        
        ax.set_title(f"Cultural Trait {trait_idx} Diffusion at Step {step if step >= 0 else self.time_elapsed}")
        plt.colorbar(nodes, ax=ax, label='Trait Strength')
        
        if ax is None:
            plt.show()
    
    def visualize_adoption_curve(self, trait_idx: Optional[int] = None, ax: Optional[plt.Axes] = None):
        """Plot adoption curve for a trait or overall."""
        if ax is None:
            fig, ax = plt.subplots(figsize=(10, 6))
        
        adoption_rates = self.get_adoption_rate(trait_idx)
        ax.plot(adoption_rates, linewidth=2)
        
        if trait_idx is not None:
            ax.set_title(f"Adoption Curve for Trait {trait_idx}")
        else:
            ax.set_title("Overall Adoption Curve")
        
        ax.set_xlabel("Time Steps")
        ax.set_ylabel("Adoption Rate")
        ax.grid(True, alpha=0.3)
        
        if ax is None:
            plt.show()
    
    def animate_diffusion(self, trait_idx: int = 0, filename: Optional[str] = None):
        """Create an animation of the diffusion process."""
        fig, ax = plt.subplots(figsize=(10, 8))
        pos = nx.spring_layout(self.network.graph)
        
        def update(frame):
            ax.clear()
            state = self.history[frame]
            node_colors = state[:, trait_idx]
            
            nodes = nx.draw_networkx_nodes(
                self.network.graph, pos, 
                node_color=node_colors,
                cmap='viridis',
                ax=ax,
                node_size=100,
                vmin=0,
                vmax=1
            )
            nx.draw_networkx_edges(self.network.graph, pos, alpha=0.2, ax=ax)
            ax.set_title(f"Cultural Diffusion - Step {frame}")
            
            return nodes,
        
        ani = FuncAnimation(fig, update, frames=len(self.history), blit=True, interval=200)
        
        if filename:
            ani.save(filename, writer='pillow', fps=5)
        
        plt.close()
        return ani
    
    def get_stats(self) -> Dict[str, Any]:
        """Calculate various statistics about the simulation."""
        current_state = self.history[-1]
        adoption_rates = np.mean(current_state > 0.5, axis=0)
        
        return {
            "total_steps": self.time_elapsed,
            "average_adoption": np.mean(adoption_rates),
            "trait_adoption_rates": adoption_rates,
            "cultural_diversity": self.get_diversity_index()[-1],
            "max_adoption": np.max(adoption_rates),
            "min_adoption": np.min(adoption_rates),
        }


def analyze_cultural_clusters(simulation: CulturalDiffusion) -> Dict[str, Any]:
    """Analyze cultural clusters in the population."""
    cultural_distances = simulation.get_cultural_distance_matrix()
    
    # Perform hierarchical clustering
    from scipy.cluster import hierarchy
    from scipy.spatial.distance import squareform
    
    condensed_dist = squareform(cultural_distances)
    Z = hierarchy.linkage(condensed_dist, 'ward')
    clusters = hierarchy.fcluster(Z, t=0.7, criterion='distance')
    
    # Calculate cluster statistics
    unique_clusters, cluster_sizes = np.unique(clusters, return_counts=True)
    
    return {
        "num_clusters": len(unique_clusters),
        "cluster_sizes": cluster_sizes,
        "cluster_assignments": clusters,
        "modularity": calculate_modularity(simulation, clusters)
    }


def calculate_modularity(simulation: CulturalDiffusion, clusters: np.ndarray) -> float:
    """Calculate modularity of cultural clusters relative to network structure."""
    cultural_similarity = 1 - simulation.get_cultural_distance_matrix() / np.sqrt(simulation.params.num_cultural_traits)
    cultural_modularity = 0.0
    
    unique_clusters = np.unique(clusters)
    m = np.sum(cultural_similarity) / 2  # Total "cultural strength"
    
    for cluster in unique_clusters:
        cluster_mask = clusters == cluster
        cluster_sim = np.sum(cultural_similarity[cluster_mask][:, cluster_mask])
        total_sim = np.sum(cultural_similarity[cluster_mask])
        cultural_modularity += cluster_sim / (2 * m) - (total_sim / (2 * m)) ** 2
    
    return cultural_modularity


def run_comparative_experiment(param_values: List[Any], param_name: str, 
                              num_agents: int = 100, steps: int = 50,
                              num_runs: int = 5) -> pd.DataFrame:
    """Run a comparative experiment varying one parameter."""
    results = []
    
    for value in param_values:
        for run in range(num_runs):
            params = SimulationParameters()
            setattr(params, param_name, value)
            
            sim = CulturalDiffusion(num_agents=num_agents, params=params)
            sim.simulate(steps=steps)
            
            stats = sim.get_stats()
            stats['run'] = run
            stats[param_name] = value
            
            results.append(stats)
    
    return pd.DataFrame(results)


# Example usage and demonstration
if __name__ == "__main__":
    # Create and run a simulation
    params = SimulationParameters(
        model_type=DiffusionModel.THRESHOLD,
        adoption_threshold=0.4,
        num_cultural_traits=4,
        initial_adoption_rate=0.05
    )
    
    sim = CulturalDiffusion(num_agents=100, params=params)
    sim.simulate(steps=30)
    
    # Visualize results
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 5))
    sim.visualize_adoption_curve(ax=ax1)
    sim.visualize_network(step=-1, ax=ax2)
    plt.show()
    
    # Show statistics
    print("Simulation Statistics:")
    for key, value in sim.get_stats().items():
        if isinstance(value, (int, float)):
            print(f"{key}: {value:.4f}")
    
    # Analyze clusters
    cluster_analysis = analyze_cultural_clusters(sim)
    print(f"\nCultural Clusters: {cluster_analysis['num_clusters']}")
    print(f"Cluster sizes: {cluster_analysis['cluster_sizes']}")