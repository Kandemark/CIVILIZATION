"""Focused technological development"""

# Module implementation

"""
Specialization Module

Analyzes and models patterns of technological and research specialization
across different entities, regions, and time periods.
"""

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
import math
import statistics
from collections import defaultdict


class SpecializationType(Enum):
    """Types of specialization patterns"""
    GENERALIST = "generalist"
    SPECIALIST = "specialist"
    HYBRID = "hybrid"
    FOCUSED = "focused"
    DIVERSIFIED = "diversified"


class SpecializationStrategy(Enum):
    """Strategies for specialization"""
    BREADTH_FIRST = "breadth_first"
    DEPTH_FIRST = "depth_first"
    ADAPTIVE = "adaptive"
    NICHE = "niche"
    PLATFORM = "platform"


@dataclass
class SpecializationProfile:
    """Represents the specialization pattern of an entity"""
    
    entity_id: str
    entity_name: str
    domains: Set[str]  # Knowledge/technology domains
    capabilities: Dict[str, float]  # Domain -> capability level (0.0 to 1.0)
    specialization_type: SpecializationType
    strategy: SpecializationStrategy
    
    # Performance metrics
    productivity: float  # 0.0 to 1.0
    innovation_rate: float  # 0.0 to 1.0
    adaptability: float  # 0.0 to 1.0
    
    def __post_init__(self):
        """Validate specialization profile"""
        assert all(0.0 <= capability <= 1.0 for capability in self.capabilities.values()), \
            "All capabilities must be between 0 and 1"
        assert 0.0 <= self.productivity <= 1.0, "Productivity must be between 0 and 1"
        assert 0.0 <= self.innovation_rate <= 1.0, "Innovation rate must be between 0 and 1"
        assert 0.0 <= self.adaptability <= 1.0, "Adaptability must be between 0 and 1"
    
    @property
    def specialization_index(self) -> float:
        """Calculate specialization index (0=generalist, 1=specialist)"""
        if len(self.capabilities) <= 1:
            return 0.0
        
        capability_values = list(self.capabilities.values())
        total_capability = sum(capability_values)
        
        if total_capability == 0:
            return 0.0
        
        # Calculate Herfindahl index for concentration
        shares = [cap / total_capability for cap in capability_values]
        hhi = sum(share * share for share in shares)
        
        # Normalize to 0-1 range
        min_hhi = 1 / len(capability_values)
        normalized_hhi = (hhi - min_hhi) / (1 - min_hhi) if (1 - min_hhi) > 0 else 0.0
        
        return normalized_hhi
    
    @property
    def domain_breadth(self) -> int:
        """Number of domains with significant capability (> 0.3)"""
        return sum(1 for capability in self.capabilities.values() if capability > 0.3)
    
    @property
    def domain_depth(self) -> float:
        """Average capability in top 3 domains"""
        if not self.capabilities:
            return 0.0
        
        top_capabilities = sorted(self.capabilities.values(), reverse=True)[:3]
        return statistics.mean(top_capabilities)
    
    def get_comparative_advantage(self, other_profile: 'SpecializationProfile') -> List[Tuple[str, float]]:
        """
        Calculate comparative advantage domains compared to another profile.
        
        Returns:
            List of (domain, advantage_score) tuples
        """
        advantages = []
        
        for domain, capability in self.capabilities.items():
            other_capability = other_profile.capabilities.get(domain, 0.0)
            
            if capability > other_capability:
                advantage_score = capability - other_capability
                advantages.append((domain, advantage_score))
        
        # Sort by advantage score
        advantages.sort(key=lambda x: x[1], reverse=True)
        return advantages


class SpecializationAnalyzer:
    """Analyzes specialization patterns across multiple entities"""
    
    def __init__(self):
        self.profiles: Dict[str, SpecializationProfile] = {}
        self.domain_landscape: Dict[str, List[float]] = defaultdict(list)
    
    def add_profile(self, profile: SpecializationProfile) -> bool:
        """Add a specialization profile to the analyzer"""
        if profile.entity_id in self.profiles:
            return False
        
        self.profiles[profile.entity_id] = profile
        
        # Update domain landscape
        for domain, capability in profile.capabilities.items():
            self.domain_landscape[domain].append(capability)
        
        return True
    
    def remove_profile(self, entity_id: str) -> bool:
        """Remove a specialization profile"""
        if entity_id not in self.profiles:
            return False
        
        profile = self.profiles[entity_id]
        
        # Remove from domain landscape
        for domain in profile.capabilities:
            if domain in self.domain_landscape and entity_id in [p.entity_id for p in self.profiles.values()]:
                # Rebuild domain landscape for this domain
                self.domain_landscape[domain] = [
                    p.capabilities[domain] for p in self.profiles.values() 
                    if domain in p.capabilities
                ]
        
        del self.profiles[entity_id]
        return True
    
    def calculate_domain_competitiveness(self, domain: str) -> float:
        """
        Calculate competitiveness of a specific domain.
        
        Returns:
            Competitiveness score (0.0 to 1.0)
        """
        if domain not in self.domain_landscape or not self.domain_landscape[domain]:
            return 0.0
        
        capabilities = self.domain_landscape[domain]
        
        # Competitiveness based on average capability and distribution
        avg_capability = statistics.mean(capabilities)
        capability_std = statistics.stdev(capabilities) if len(capabilities) > 1 else 0.0
        
        # High average and low std indicates high competitiveness
        competitiveness = avg_capability * (1 - capability_std)
        return max(0.0, min(1.0, competitiveness))
    
    def find_niche_opportunities(self, min_entities: int = 3) -> List[Tuple[str, float]]:
        """
        Find domains with low competition and high potential.
        
        Returns:
            List of (domain, opportunity_score) tuples
        """
        opportunities = []
        
        for domain, capabilities in self.domain_landscape.items():
            if len(capabilities) < min_entities:
                continue
            
            # Calculate opportunity score
            avg_capability = statistics.mean(capabilities)
            participation_rate = len(capabilities) / len(self.profiles)
            
            # Opportunity: low current capability but potential for high capability
            opportunity_score = (1 - avg_capability) * (1 - participation_rate)
            
            if opportunity_score > 0.2:  # Threshold for meaningful opportunities
                opportunities.append((domain, opportunity_score))
        
        # Sort by opportunity score
        opportunities.sort(key=lambda x: x[1], reverse=True)
        return opportunities
    
    def calculate_specialization_metrics(self) -> Dict[str, float]:
        """Calculate overall specialization metrics across all profiles"""
        if not self.profiles:
            return {}
        
        profiles = list(self.profiles.values())
        
        metrics = {
            'total_entities': len(profiles),
            'average_specialization': statistics.mean(p.specialization_index for p in profiles),
            'average_breadth': statistics.mean(p.domain_breadth for p in profiles),
            'average_depth': statistics.mean(p.domain_depth for p in profiles),
            'specialization_diversity': self._calculate_specialization_diversity(),
            'strategic_alignment': self._calculate_strategic_alignment(),
        }
        
        # Performance correlations
        specialization_productivity_corr = self._calculate_correlation(
            [p.specialization_index for p in profiles],
            [p.productivity for p in profiles]
        )
        metrics['specialization_productivity_correlation'] = specialization_productivity_corr
        
        return metrics
    
    def _calculate_specialization_diversity(self) -> float:
        """Calculate diversity of specialization types"""
        type_counts = defaultdict(int)
        
        for profile in self.profiles.values():
            type_counts[profile.specialization_type] += 1
        
        total_profiles = len(self.profiles)
        if total_profiles <= 1:
            return 0.0
        
        # Calculate Shannon entropy
        entropy = 0.0
        for count in type_counts.values():
            probability = count / total_profiles
            entropy -= probability * math.log(probability)
        
        # Normalize by maximum possible entropy
        max_entropy = math.log(len(SpecializationType))
        normalized_entropy = entropy / max_entropy if max_entropy > 0 else 0.0
        
        return normalized_entropy
    
    def _calculate_strategic_alignment(self) -> float:
        """Calculate how well specialization strategies align with performance"""
        aligned_count = 0
        total_count = 0
        
        strategy_performance_map = {
            SpecializationStrategy.BREADTH_FIRST: lambda p: p.adaptability > 0.7,
            SpecializationStrategy.DEPTH_FIRST: lambda p: p.productivity > 0.7,
            SpecializationStrategy.ADAPTIVE: lambda p: p.innovation_rate > 0.6,
            SpecializationStrategy.NICHE: lambda p: p.specialization_index > 0.7,
            SpecializationStrategy.PLATFORM: lambda p: p.domain_breadth > 3,
        }
        
        for profile in self.profiles.values():
            performance_check = strategy_performance_map.get(profile.strategy)
            if performance_check and performance_check(profile):
                aligned_count += 1
            total_count += 1
        
        return aligned_count / total_count if total_count > 0 else 0.0
    
    def _calculate_correlation(self, x: List[float], y: List[float]) -> float:
        """Calculate Pearson correlation coefficient between two lists"""
        if len(x) != len(y) or len(x) < 2:
            return 0.0
        
        try:
            n = len(x)
            sum_x = sum(x)
            sum_y = sum(y)
            sum_xy = sum(xi * yi for xi, yi in zip(x, y))
            sum_x2 = sum(xi * xi for xi in x)
            sum_y2 = sum(yi * yi for yi in y)
            
            numerator = n * sum_xy - sum_x * sum_y
            denominator = math.sqrt((n * sum_x2 - sum_x * sum_x) * (n * sum_y2 - sum_y * sum_y))
            
            if denominator == 0:
                return 0.0
            
            correlation = numerator / denominator
            return max(-1.0, min(1.0, correlation))
        except (ValueError, ZeroDivisionError):
            return 0.0
    
    def get_optimal_specialization_recommendations(self, entity_id: str) -> List[Dict]:
        """
        Get recommendations for optimal specialization based on current profile.
        
        Args:
            entity_id: ID of the entity to analyze
            
        Returns:
            List of specialization recommendations
        """
        if entity_id not in self.profiles:
            return []
        
        profile = self.profiles[entity_id]
        recommendations = []
        
        # Analyze current strengths
        strengths = sorted(profile.capabilities.items(), key=lambda x: x[1], reverse=True)[:3]
        
        for domain, capability in strengths:
            if capability > 0.7:
                # Recommend deepening expertise
                recommendations.append({
                    'type': 'deepen_expertise',
                    'domain': domain,
                    'current_capability': capability,
                    'recommendation': f"Deepen expertise in {domain} to maintain competitive advantage",
                    'priority': 'high'
                })
        
        # Identify diversification opportunities
        opportunities = self.find_niche_opportunities()
        for domain, opportunity_score in opportunities[:2]:
            if domain not in profile.capabilities or profile.capabilities[domain] < 0.3:
                recommendations.append({
                    'type': 'diversify',
                    'domain': domain,
                    'opportunity_score': opportunity_score,
                    'recommendation': f"Consider diversifying into {domain} (high opportunity)",
                    'priority': 'medium'
                })
        
        # Strategic alignment recommendation
        current_performance = (profile.productivity + profile.innovation_rate + profile.adaptability) / 3
        if current_performance < 0.6:
            recommendations.append({
                'type': 'strategic_review',
                'current_strategy': profile.strategy.value,
                'current_performance': current_performance,
                'recommendation': "Review specialization strategy for better performance alignment",
                'priority': 'high'
            })
        
        return recommendations