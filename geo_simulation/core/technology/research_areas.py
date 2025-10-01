"""Fields of research"""

# Module implementation

"""
Research Areas Module

Defines research domains, their interrelationships, and methods for
analyzing research focus and specialization patterns.
"""

from typing import Dict, List, Set, Optional, Tuple
import math
from dataclasses import dataclass
from enum import Enum
import statistics
from datetime import datetime


class ResearchIntensity(Enum):
    """Levels of research intensity"""
    LOW = "low"
    MEDIUM = "medium"
    HIGH = "high"
    VERY_HIGH = "very_high"


class ResearchMaturity(Enum):
    """Maturity levels of research areas"""
    EMERGING = "emerging"
    GROWING = "growing"
    MATURE = "mature"
    DECLINING = "declining"
    SATURATED = "saturated"


@dataclass
class ResearchArea:
    """Represents a specific research domain or field"""
    
    id: str
    name: str
    description: str
    parent_area: Optional[str]  # ID of parent research area
    maturity: ResearchMaturity
    research_intensity: ResearchIntensity
    
    # Quantitative metrics
    publication_count: int
    citation_count: int
    researcher_count: int
    funding_level: float  # 0.0 to 1.0
    
    # Knowledge properties
    key_concepts: Set[str]
    methodologies: Set[str]
    applications: Set[str]
    
    # Interdisciplinary connections
    related_areas: Set[str]  # IDs of related research areas
    convergence_potential: float  # 0.0 to 1.0
    
    def __post_init__(self):
        """Validate research area data"""
        assert self.publication_count >= 0, "Publication count cannot be negative"
        assert self.citation_count >= 0, "Citation count cannot be negative"
        assert self.researcher_count >= 0, "Researcher count cannot be negative"
        assert 0.0 <= self.funding_level <= 1.0, "Funding level must be between 0 and 1"
        assert 0.0 <= self.convergence_potential <= 1.0, "Convergence potential must be between 0 and 1"
    
    @property
    def research_impact(self) -> float:
        """Calculate research impact score"""
        if self.publication_count == 0:
            return 0.0
        
        citations_per_publication = self.citation_count / self.publication_count
        
        # Normalize researcher count impact
        researcher_impact = min(1.0, self.researcher_count / 1000)  # Cap at 1000 researchers
        
        impact_score = (
            citations_per_publication * 0.4 +
            researcher_impact * 0.3 +
            self.funding_level * 0.3
        )
        
        return round(impact_score, 4)
    
    @property
    def growth_potential(self) -> float:
        """Calculate growth potential based on current state"""
        maturity_factors = {
            ResearchMaturity.EMERGING: 0.9,
            ResearchMaturity.GROWING: 0.7,
            ResearchMaturity.MATURE: 0.3,
            ResearchMaturity.DECLINING: 0.1,
            ResearchMaturity.SATURATED: 0.05
        }
        
        intensity_factors = {
            ResearchIntensity.LOW: 0.8,
            ResearchIntensity.MEDIUM: 0.6,
            ResearchIntensity.HIGH: 0.4,
            ResearchIntensity.VERY_HIGH: 0.2
        }
        
        base_potential = (
            maturity_factors.get(self.maturity, 0.5) *
            intensity_factors.get(self.research_intensity, 0.5)
        )
        
        # Adjust for convergence potential
        adjusted_potential = base_potential * (1 + self.convergence_potential * 0.5)
        
        return min(1.0, adjusted_potential)
    
    def is_interdisciplinary(self, threshold: float = 0.7) -> bool:
        """Check if research area is highly interdisciplinary"""
        return self.convergence_potential >= threshold


class ResearchAreaManager:
    """Manages multiple research areas and their relationships"""
    
    def __init__(self):
        self.research_areas: Dict[str, ResearchArea] = {}
        self.area_graph: Dict[Tuple[str, str], float] = {}  # (area1, area2) -> relatedness
    
    def add_research_area(self, area: ResearchArea) -> bool:
        """Add a research area to the manager"""
        if area.id in self.research_areas:
            return False
        
        self.research_areas[area.id] = area
        
        # Add relationships to graph
        for related_id in area.related_areas:
            if related_id in self.research_areas:
                self.add_relationship(area.id, related_id)
        
        return True
    
    def remove_research_area(self, area_id: str) -> bool:
        """Remove a research area"""
        if area_id not in self.research_areas:
            return False
        
        del self.research_areas[area_id]
        
        # Remove related connections
        connections_to_remove = [
            (a1, a2) for a1, a2 in self.area_graph.keys()
            if a1 == area_id or a2 == area_id
        ]
        for connection in connections_to_remove:
            del self.area_graph[connection]
        
        return True
    
    def add_relationship(self, area1_id: str, area2_id: str, strength: float = 0.5) -> bool:
        """Add relationship between two research areas"""
        if area1_id not in self.research_areas or area2_id not in self.research_areas:
            return False
        
        connection = (area1_id, area2_id)
        self.area_graph[connection] = max(0.0, min(1.0, strength))
        return True
    
    def get_related_areas(self, area_id: str, min_strength: float = 0.3) -> List[ResearchArea]:
        """Get research areas related to the specified area"""
        if area_id not in self.research_areas:
            return []
        
        related_areas = []
        for (a1, a2), strength in self.area_graph.items():
            if a1 == area_id and strength >= min_strength:
                related_area = self.research_areas.get(a2)
                if related_area:
                    related_areas.append(related_area)
        
        return related_areas
    
    def find_interdisciplinary_bridges(self) -> List[Tuple[str, str, float]]:
        """Find pairs of research areas that serve as interdisciplinary bridges"""
        bridges = []
        
        for area1_id, area1 in self.research_areas.items():
            for area2_id, area2 in self.research_areas.items():
                if area1_id >= area2_id:  # Avoid duplicates
                    continue
                
                # Calculate bridge score based on convergence potential and relationship
                relationship_strength = self.area_graph.get((area1_id, area2_id), 0.0)
                bridge_score = (
                    area1.convergence_potential *
                    area2.convergence_potential *
                    relationship_strength
                )
                
                if bridge_score > 0.3:  # Threshold for meaningful bridges
                    bridges.append((area1_id, area2_id, bridge_score))
        
        # Sort by bridge score
        bridges.sort(key=lambda x: x[2], reverse=True)
        return bridges
    
    def calculate_research_landscape(self) -> Dict[str, float]:
        """Calculate overall metrics of the research landscape"""
        if not self.research_areas:
            return {}
        
        areas = list(self.research_areas.values())
        
        metrics = {
            'total_areas': len(areas),
            'average_impact': statistics.mean(area.research_impact for area in areas),
            'average_growth_potential': statistics.mean(area.growth_potential for area in areas),
            'interdisciplinary_ratio': sum(1 for area in areas if area.is_interdisciplinary()) / len(areas),
            'landscape_diversity': self._calculate_landscape_diversity(),
            'research_concentration': self._calculate_research_concentration(),
        }
        
        # Maturity distribution
        maturity_dist = {}
        for maturity in ResearchMaturity:
            maturity_dist[maturity.value] = sum(1 for area in areas if area.maturity == maturity) / len(areas)
        metrics['maturity_distribution'] = maturity_dist
        
        return metrics
    
    def _calculate_landscape_diversity(self) -> float:
        """Calculate diversity of research areas based on relationships"""
        if len(self.research_areas) <= 1:
            return 0.0
        
        # Calculate relationship diversity using Shannon entropy
        relationship_strengths = list(self.area_graph.values())
        if not relationship_strengths:
            return 0.0
        
        total_strength = sum(relationship_strengths)
        if total_strength == 0:
            return 0.0
        
        # Normalize strengths to probabilities
        probabilities = [strength / total_strength for strength in relationship_strengths]
        
        # Calculate Shannon entropy
        entropy = -sum(p * math.log(p) for p in probabilities if p > 0)
        
        # Normalize by maximum possible entropy
        max_entropy = math.log(len(probabilities))
        normalized_entropy = entropy / max_entropy if max_entropy > 0 else 0.0
        
        return normalized_entropy
    
    def _calculate_research_concentration(self) -> float:
        """Calculate concentration of research impact"""
        impact_scores = [area.research_impact for area in self.research_areas.values()]
        if not impact_scores:
            return 0.0
        
        # Calculate Herfindahl-Hirschman Index (HHI)
        total_impact = sum(impact_scores)
        if total_impact == 0:
            return 0.0
        
        market_shares = [impact / total_impact for impact in impact_scores]
        hhi = sum(share * share for share in market_shares)
        
        return hhi
    
    def get_emerging_areas(self, top_n: int = 5) -> List[ResearchArea]:
        """Get research areas with highest growth potential"""
        areas = list(self.research_areas.values())
        areas.sort(key=lambda x: x.growth_potential, reverse=True)
        return areas[:top_n]
    
    def get_impact_leaders(self, top_n: int = 5) -> List[ResearchArea]:
        """Get research areas with highest research impact"""
        areas = list(self.research_areas.values())
        areas.sort(key=lambda x: x.research_impact, reverse=True)
        return areas[:top_n]