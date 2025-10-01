"""Tech spreading between nations"""

# Module implementation

"""
Knowledge Diffusion Module

Models the spread and adoption of knowledge across different entities,
regions, and time periods within technological systems.
"""

from typing import Dict, List, Set, Optional, Tuple
from datetime import datetime, timedelta
from enum import Enum
import math
import statistics
from dataclasses import dataclass


class DiffusionChannel(Enum):
    """Channels through which knowledge diffuses"""
    ACADEMIC = "academic"
    INDUSTRIAL = "industrial"
    GOVERNMENT = "government"
    SOCIAL = "social"
    DIGITAL = "digital"
    MIGRATION = "migration"


class AdoptionCategory(Enum):
    """Categories of technology adoption"""
    INNOVATORS = "innovators"
    EARLY_ADOPTERS = "early_adopters"
    EARLY_MAJORITY = "early_majority"
    LATE_MAJORITY = "late_majority"
    LAGGARDS = "laggards"


@dataclass
class KnowledgeEntity:
    """Represents an entity that can possess and transfer knowledge"""
    
    id: str
    name: str
    knowledge_level: float  # 0.0 to 1.0
    adoption_profile: AdoptionCategory
    connectivity: float  # 0.0 to 1.0 (network connectivity)
    learning_capacity: float  # 0.0 to 1.0
    location: Tuple[float, float]  # (x, y) coordinates for spatial diffusion
    
    # Knowledge domains possessed
    knowledge_domains: Set[str]
    
    def __post_init__(self):
        """Validate entity parameters"""
        assert 0.0 <= self.knowledge_level <= 1.0, "Knowledge level must be between 0 and 1"
        assert 0.0 <= self.connectivity <= 1.0, "Connectivity must be between 0 and 1"
        assert 0.0 <= self.learning_capacity <= 1.0, "Learning capacity must be between 0 and 1"


class KnowledgeDiffusion:
    """
    Simulates and analyzes the diffusion of knowledge across entities
    and through different channels over time.
    """
    
    def __init__(self):
        self.entities: Dict[str, KnowledgeEntity] = {}
        self.diffusion_network: Dict[Tuple[str, str], float] = {}  # (entity1, entity2) -> strength
        self.diffusion_history: List[Dict] = []
        
    def add_entity(self, entity: KnowledgeEntity) -> bool:
        """Add a knowledge entity to the diffusion system"""
        if entity.id in self.entities:
            return False
        self.entities[entity.id] = entity
        return True
    
    def remove_entity(self, entity_id: str) -> bool:
        """Remove a knowledge entity from the system"""
        if entity_id not in self.entities:
            return False
        del self.entities[entity_id]
        
        # Remove related network connections
        connections_to_remove = [
            (e1, e2) for e1, e2 in self.diffusion_network.keys()
            if e1 == entity_id or e2 == entity_id
        ]
        for connection in connections_to_remove:
            del self.diffusion_network[connection]
            
        return True
    
    def add_connection(self, entity1_id: str, entity2_id: str, strength: float = 0.5) -> bool:
        """Add a connection between two entities for knowledge diffusion"""
        if entity1_id not in self.entities or entity2_id not in self.entities:
            return False
        
        connection = (entity1_id, entity2_id)
        self.diffusion_network[connection] = max(0.0, min(1.0, strength))
        return True
    
    def calculate_diffusion_probability(self, source_id: str, target_id: str, 
                                      knowledge_domain: str) -> float:
        """
        Calculate probability of knowledge diffusion between two entities.
        
        Args:
            source_id: ID of knowledge source entity
            target_id: ID of knowledge target entity
            knowledge_domain: Domain of knowledge being diffused
            
        Returns:
            Diffusion probability (0.0 to 1.0)
        """
        if source_id not in self.entities or target_id not in self.entities:
            return 0.0
        
        source = self.entities[source_id]
        target = self.entities[target_id]
        
        # Base connection strength
        connection_strength = self.diffusion_network.get((source_id, target_id), 0.1)
        
        # Knowledge gap (source must know more than target)
        knowledge_gap = max(0.0, source.knowledge_level - target.knowledge_level)
        
        # Domain compatibility
        domain_compatibility = 1.0 if knowledge_domain in target.knowledge_domains else 0.3
        
        # Adoption profile modifier
        profile_modifiers = {
            AdoptionCategory.INNOVATORS: 1.2,
            AdoptionCategory.EARLY_ADOPTERS: 1.1,
            AdoptionCategory.EARLY_MAJORITY: 1.0,
            AdoptionCategory.LATE_MAJORITY: 0.8,
            AdoptionCategory.LAGGARDS: 0.6
        }
        adoption_modifier = profile_modifiers.get(target.adoption_profile, 1.0)
        
        # Calculate diffusion probability
        diffusion_prob = (
            connection_strength * 
            knowledge_gap * 
            domain_compatibility * 
            target.learning_capacity * 
            adoption_modifier
        )
        
        return min(1.0, diffusion_prob)
    
    def simulate_diffusion_step(self, knowledge_domain: str, 
                              diffusion_rate: float = 0.1) -> Dict[str, float]:
        """
        Simulate one step of knowledge diffusion across all entities.
        
        Args:
            knowledge_domain: Domain of knowledge to diffuse
            diffusion_rate: Base rate of diffusion
            
        Returns:
            Dictionary of knowledge level changes for each entity
        """
        changes = {}
        
        for target_id, target_entity in self.entities.items():
            total_influence = 0.0
            influence_weight = 0.0
            
            for source_id, source_entity in self.entities.items():
                if source_id == target_id:
                    continue
                
                diffusion_prob = self.calculate_diffusion_probability(
                    source_id, target_id, knowledge_domain
                )
                
                if diffusion_prob > 0:
                    influence = source_entity.knowledge_level * diffusion_prob
                    total_influence += influence
                    influence_weight += diffusion_prob
            
            if influence_weight > 0:
                average_influence = total_influence / influence_weight
                knowledge_gain = average_influence * diffusion_rate * target_entity.learning_capacity
                
                # Update entity knowledge level
                old_level = target_entity.knowledge_level
                new_level = min(1.0, old_level + knowledge_gain)
                target_entity.knowledge_level = new_level
                
                changes[target_id] = new_level - old_level
        
        # Record diffusion step
        step_record = {
            'timestamp': datetime.now(),
            'knowledge_domain': knowledge_domain,
            'changes': changes.copy(),
            'average_knowledge': self.get_average_knowledge_level()
        }
        self.diffusion_history.append(step_record)
        
        return changes
    
    def get_average_knowledge_level(self) -> float:
        """Calculate average knowledge level across all entities"""
        if not self.entities:
            return 0.0
        return statistics.mean(entity.knowledge_level for entity in self.entities.values())
    
    def get_knowledge_distribution(self) -> Dict[AdoptionCategory, float]:
        """Get average knowledge level by adoption category"""
        distribution = {}
        
        for category in AdoptionCategory:
            category_entities = [
                entity for entity in self.entities.values()
                if entity.adoption_profile == category
            ]
            
            if category_entities:
                distribution[category] = statistics.mean(
                    entity.knowledge_level for entity in category_entities
                )
            else:
                distribution[category] = 0.0
        
        return distribution
    
    def find_knowledge_hubs(self, top_n: int = 5) -> List[Tuple[str, float]]:
        """
        Identify entities with the highest network influence.
        
        Returns:
            List of (entity_id, influence_score) tuples
        """
        influence_scores = {}
        
        for entity_id, entity in self.entities.items():
            # Calculate outgoing connection strength
            outgoing_strength = sum(
                strength for (src, tgt), strength in self.diffusion_network.items()
                if src == entity_id
            )
            
            # Calculate influence score based on knowledge and connectivity
            influence_score = (
                entity.knowledge_level * 
                entity.connectivity * 
                (1 + outgoing_strength)
            )
            influence_scores[entity_id] = influence_score
        
        # Return top N influential entities
        sorted_hubs = sorted(influence_scores.items(), key=lambda x: x[1], reverse=True)
        return sorted_hubs[:top_n]
    
    def calculate_diffusion_metrics(self) -> Dict[str, float]:
        """Calculate comprehensive diffusion metrics"""
        if not self.entities:
            return {}
        
        metrics = {
            'total_entities': len(self.entities),
            'average_knowledge': self.get_average_knowledge_level(),
            'knowledge_inequality': self._calculate_gini_coefficient(),
            'network_density': len(self.diffusion_network) / max(1, len(self.entities) * (len(self.entities) - 1)),
            'connectivity_index': statistics.mean(entity.connectivity for entity in self.entities.values()),
        }
        
        return metrics
    
    def _calculate_gini_coefficient(self) -> float:
        """Calculate Gini coefficient for knowledge distribution"""
        knowledge_levels = [entity.knowledge_level for entity in self.entities.values()]
        if not knowledge_levels:
            return 0.0
        
        # Sort knowledge levels
        sorted_levels = sorted(knowledge_levels)
        n = len(sorted_levels)
        
        # Calculate Gini coefficient
        numerator = sum((i + 1) * level for i, level in enumerate(sorted_levels))
        denominator = n * sum(sorted_levels)
        
        if denominator == 0:
            return 0.0
        
        gini = (2 * numerator) / denominator - (n + 1) / n
        return gini