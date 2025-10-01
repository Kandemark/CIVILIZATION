"""Abstract tech measurement (0-∞)"""

# Module implementation

"""
Development Index Module

Provides tools for calculating and analyzing technological development
indices and progress metrics.
"""

from typing import Dict, List, Optional
from datetime import datetime, timedelta
import statistics
from .innovation import Innovation, DevelopmentStage
from .innovation import InnovationType


class DevelopmentIndex:
    """
    Calculates and tracks technological development indices
    across different dimensions and time periods.
    """
    
    def __init__(self, name: str, weights: Optional[Dict[str, float]] = None):
        self.name = name
        self.weights = weights or {
            'technical': 0.25,
            'economic': 0.25,
            'social': 0.20,
            'environmental': 0.15,
            'diversity': 0.15
        }
        
        # Validate weights sum to 1.0
        total_weight = sum(self.weights.values())
        if abs(total_weight - 1.0) > 0.001:
            raise ValueError(f"Weights must sum to 1.0, got {total_weight}")
        
        self.history: List[Dict] = []
        
    def calculate_index(self, innovations: List[Innovation]) -> Dict[str, float]:
        """
        Calculate comprehensive development index for a set of innovations.
        
        Args:
            innovations: List of Innovation objects
            
        Returns:
            Dictionary containing index scores for different dimensions
        """
        if not innovations:
            return self._get_empty_scores()
        
        # Calculate component scores
        technical_score = self._calculate_technical_score(innovations)
        economic_score = self._calculate_economic_score(innovations)
        social_score = self._calculate_social_score(innovations)
        environmental_score = self._calculate_environmental_score(innovations)
        diversity_score = self._calculate_diversity_score(innovations)
        
        # Calculate weighted overall score
        overall_score = (
            self.weights['technical'] * technical_score +
            self.weights['economic'] * economic_score +
            self.weights['social'] * social_score +
            self.weights['environmental'] * environmental_score +
            self.weights['diversity'] * diversity_score
        )
        
        scores = {
            'overall': round(overall_score, 4),
            'technical': round(technical_score, 4),
            'economic': round(economic_score, 4),
            'social': round(social_score, 4),
            'environmental': round(environmental_score, 4),
            'diversity': round(diversity_score, 4),
            'timestamp': datetime.now(),
            'innovation_count': len(innovations)
        }
        
        # Store in history
        self.history.append(scores)
        
        return scores
    
    def _calculate_technical_score(self, innovations: List[Innovation]) -> float:
        """Calculate technical development score"""
        if not innovations:
            return 0.0
            
        # Average technical feasibility weighted by development stage
        stage_weights = {
            DevelopmentStage.RESEARCH: 0.2,
            DevelopmentStage.DEVELOPMENT: 0.4,
            DevelopmentStage.DEMONSTRATION: 0.6,
            DevelopmentStage.DEPLOYMENT: 0.8,
            DevelopmentStage.MATURE: 1.0
        }
        
        weighted_scores = []
        for innovation in innovations:
            stage_weight = stage_weights.get(innovation.development_stage, 0.5)
            weighted_score = innovation.technical_feasibility * stage_weight
            weighted_scores.append(weighted_score)
        
        return statistics.mean(weighted_scores) if weighted_scores else 0.0
    
    def _calculate_economic_score(self, innovations: List[Innovation]) -> float:
        """Calculate economic development score"""
        if not innovations:
            return 0.0
            
        economic_scores = [innov.economic_viability for innov in innovations]
        return statistics.mean(economic_scores)
    
    def _calculate_social_score(self, innovations: List[Innovation]) -> float:
        """Calculate social development score"""
        if not innovations:
            return 0.0
            
        social_scores = [innov.social_acceptance for innov in innovations]
        return statistics.mean(social_scores)
    
    def _calculate_environmental_score(self, innovations: List[Innovation]) -> float:
        """Calculate environmental development score"""
        if not innovations:
            return 0.5  # Neutral score for no innovations
            
        # Convert environmental impact from [-1, 1] to [0, 1] scale
        env_scores = [(innov.environmental_impact + 1) / 2 for innov in innovations]
        return statistics.mean(env_scores)
    
    def _calculate_diversity_score(self, innovations: List[Innovation]) -> float:
        """Calculate innovation diversity score"""
        if len(innovations) <= 1:
            return 0.0
            
        # Count unique knowledge domains
        domains = set(innov.knowledge_domain for innov in innovations)
        domain_diversity = len(domains) / len(innovations)
        
        # Count innovation types
        types = set(innov.innovation_type for innov in innovations)
        type_diversity = len(types) / len(InnovationType)
        
        # Combine diversity measures
        diversity_score = (domain_diversity + type_diversity) / 2
        return min(diversity_score, 1.0)  # Cap at 1.0
    
    def _get_empty_scores(self) -> Dict[str, float]:
        """Return scores for empty innovation set"""
        return {
            'overall': 0.0,
            'technical': 0.0,
            'economic': 0.0,
            'social': 0.0,
            'environmental': 0.5,
            'diversity': 0.0,
            'timestamp': datetime.now(),
            'innovation_count': 0
        }
    
    def get_trend(self, window: int = 5) -> Dict[str, float]:
        """
        Calculate trend of development indices over recent history.
        
        Args:
            window: Number of recent entries to consider
            
        Returns:
            Dictionary of trend slopes for each score type
        """
        if len(self.history) < 2:
            return {}
            
        recent_history = self.history[-window:]
        
        trends = {}
        for score_type in ['overall', 'technical', 'economic', 'social', 'environmental', 'diversity']:
            scores = [entry[score_type] for entry in recent_history]
            if len(scores) >= 2:
                # Simple linear trend calculation
                x = list(range(len(scores)))
                slope = self._calculate_slope(x, scores)
                trends[score_type] = round(slope, 4)
        
        return trends
    
    def _calculate_slope(self, x: List[float], y: List[float]) -> float:
        """Calculate slope of simple linear regression"""
        n = len(x)
        if n < 2:
            return 0.0
            
        sum_x = sum(x)
        sum_y = sum(y)
        sum_xy = sum(xi * yi for xi, yi in zip(x, y))
        sum_x2 = sum(xi * xi for xi in x)
        
        denominator = n * sum_x2 - sum_x * sum_x
        if denominator == 0:
            return 0.0
            
        slope = (n * sum_xy - sum_x * sum_y) / denominator
        return slope
    
    def get_comparison(self, other_innovations: List[Innovation]) -> Dict[str, Dict[str, float]]:
        """
        Compare current state with another set of innovations.
        
        Args:
            other_innovations: List of innovations to compare against
            
        Returns:
            Dictionary with 'current' and 'comparison' scores
        """
        if not self.history:
            return {}
            
        current_scores = self.history[-1]
        comparison_scores = self.calculate_index(other_innovations)
        
        return {
            'current': current_scores,
            'comparison': comparison_scores,
            'differences': {
                score_type: comparison_scores[score_type] - current_scores[score_type]
                for score_type in ['overall', 'technical', 'economic', 'social', 'environmental', 'diversity']
            }
        }