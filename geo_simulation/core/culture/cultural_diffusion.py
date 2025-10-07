"""Spread of cultural elements"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
import random
import math

@dataclass
class DiffusionPath:
    culture_a: str
    culture_b: str
    connection_strength: float
    diffusion_types: Set[str]  # "trade", "migration", "conquest", "diplomacy"

class InfiniteDiffusionSystem:
    """Handles organic cultural diffusion without predefined patterns"""
    
    def __init__(self):
        self.diffusion_paths: Dict[Tuple[str, str], DiffusionPath] = {}
        self.diffusion_history: List[Dict] = []
        self.connection_network: Dict[str, Set[str]] = {}
        
    def establish_connection(self, culture_a: str, culture_b: str, 
                           connection_type: str, strength: float):
        """Establish a connection between two cultures"""
        key = self._get_path_key(culture_a, culture_b)
        
        if key in self.diffusion_paths:
            # Strengthen existing connection
            path = self.diffusion_paths[key]
            path.connection_strength = min(1.0, path.connection_strength + strength * 0.1)
            path.diffusion_types.add(connection_type)
        else:
            # Create new connection
            path = DiffusionPath(
                culture_a=culture_a,
                culture_b=culture_b,
                connection_strength=strength,
                diffusion_types={connection_type}
            )
            self.diffusion_paths[key] = path
            
            # Update network
            if culture_a not in self.connection_network:
                self.connection_network[culture_a] = set()
            if culture_b not in self.connection_network:
                self.connection_network[culture_b] = set()
            
            self.connection_network[culture_a].add(culture_b)
            self.connection_network[culture_b].add(culture_a)
    
    def simulate_diffusion_turn(self, cultures: Dict[str, any]) -> Dict[str, List[Dict]]:
        """Simulate one turn of cultural diffusion"""
        diffusion_results = {}
        
        for (culture_a, culture_b), path in self.diffusion_paths.items():
            culture_a_obj = cultures.get(culture_a)
            culture_b_obj = cultures.get(culture_b)
            
            if not culture_a_obj or not culture_b_obj:
                continue
            
            # Simulate bidirectional diffusion
            results_a_to_b = self._diffuse_between_cultures(culture_a_obj, culture_b_obj, path)
            results_b_to_a = self._diffuse_between_cultures(culture_b_obj, culture_a_obj, path)
            
            if culture_a not in diffusion_results:
                diffusion_results[culture_a] = []
            if culture_b not in diffusion_results:
                diffusion_results[culture_b] = []
            
            diffusion_results[culture_a].extend(results_b_to_a)
            diffusion_results[culture_b].extend(results_a_to_b)
            
            # Weaken connections over time (cultures can become isolated)
            if random.random() < 0.05:
                path.connection_strength *= 0.95
        
        return diffusion_results
    
    def _diffuse_between_cultures(self, source_culture, target_culture, path: DiffusionPath) -> List[Dict]:
        """Simulate diffusion from source to target culture"""
        diffusion_results = []
        base_diffusion_chance = path.connection_strength * 0.1
        
        # Cultural traits diffusion
        if hasattr(source_culture, 'emergent_traits') and hasattr(target_culture, 'emergent_traits'):
            for trait, strength in source_culture.emergent_traits.items():
                if random.random() < base_diffusion_chance * strength:
                    current = target_culture.emergent_traits.get(trait, 0.0)
                    diffusion_amount = strength * path.connection_strength * 0.05
                    target_culture.emergent_traits[trait] = min(1.0, current + diffusion_amount)
                    
                    diffusion_results.append({
                        "type": "trait_diffusion",
                        "trait": trait,
                        "strength": diffusion_amount,
                        "direction": "incoming"
                    })
        
        # Tradition diffusion
        if (hasattr(source_culture, 'generated_traditions') and 
            hasattr(target_culture, 'generated_traditions')):
            
            for tradition in source_culture.generated_traditions:
                if (random.random() < base_diffusion_chance * 0.5 and 
                    tradition not in target_culture.generated_traditions):
                    
                    target_culture.generated_traditions.add(tradition)
                    diffusion_results.append({
                        "type": "tradition_adoption",
                        "tradition": tradition,
                        "direction": "incoming"
                    })
        
        # Innovation vector influence
        if (hasattr(source_culture, 'innovation_vectors') and 
            hasattr(target_culture, 'innovation_vectors')):
            
            for vector, strength in source_culture.innovation_vectors.items():
                if random.random() < base_diffusion_chance * strength * 0.2:
                    current = target_culture.innovation_vectors.get(vector, 0.0)
                    influence = strength * path.connection_strength * 0.1
                    target_culture.innovation_vectors[vector] = min(1.0, current + influence)
                    
                    diffusion_results.append({
                        "type": "innovation_influence",
                        "vector": vector,
                        "influence": influence,
                        "direction": "incoming"
                    })
        
        return diffusion_results
    
    def _get_path_key(self, culture_a: str, culture_b: str) -> Tuple[str, str]:
        """Get consistent key for culture pair regardless of order"""
        return tuple(sorted([culture_a, culture_b]))
    
    def calculate_cultural_influence(self, culture_id: str) -> float:
        """Calculate a culture's overall influence on others"""
        if culture_id not in self.connection_network:
            return 0.0
        
        total_influence = 0.0
        for other_culture in self.connection_network[culture_id]:
            key = self._get_path_key(culture_id, other_culture)
            path = self.diffusion_paths.get(key)
            if path:
                total_influence += path.connection_strength
        
        return total_influence
    
    def find_cultural_bridges(self) -> List[Tuple[str, str, float]]:
        """Find cultures that serve as bridges between different groups"""
        bridges = []
        
        for culture_id, connections in self.connection_network.items():
            if len(connections) >= 3:  # At least 3 connections to be a bridge
                # Calculate betweenness (simplified)
                bridge_score = len(connections) * 0.5
                
                # Check if connects disparate groups
                connection_strengths = []
                for other_id in connections:
                    key = self._get_path_key(culture_id, other_id)
                    path = self.diffusion_paths.get(key)
                    if path:
                        connection_strengths.append(path.connection_strength)
                
                if connection_strengths:
                    avg_strength = sum(connection_strengths) / len(connection_strengths)
                    bridge_score *= avg_strength
                
                bridges.append((culture_id, bridge_score))
        
        return sorted(bridges, key=lambda x: x[1], reverse=True)[:10]  # Top 10 bridges