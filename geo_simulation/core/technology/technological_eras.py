"""Progressive advancement stages"""

# Module implementation

"""
Technological Eras Module

Defines and analyzes different technological eras, their characteristics,
and transitions between eras.
"""

from typing import Dict, List, Optional, Set
from dataclasses import dataclass
from enum import Enum
from datetime import datetime, timedelta
import statistics


class EraType(Enum):
    """Types of technological eras"""
    PRE_INDUSTRIAL = "pre_industrial"
    INDUSTRIAL = "industrial"
    ELECTRO_MECHANICAL = "electro_mechanical"
    INFORMATION = "information"
    DIGITAL = "digital"
    BIOTECH = "biotech"
    AI = "ai"
    QUANTUM = "quantum"
    POST_SCARCITY = "post_scarcity"


class TransitionType(Enum):
    """Types of era transitions"""
    GRADUAL = "gradual"
    DISRUPTIVE = "disruptive"
    REVOLUTIONARY = "revolutionary"
    CONVERGENT = "convergent"
    REGRESSIVE = "regressive"


@dataclass
class TechnologicalEra:
    """Represents a specific technological era"""
    
    name: str
    era_type: EraType
    description: str
    
    # Temporal boundaries
    start_year: int
    end_year: Optional[int]  # None for current era
    
    # Core characteristics
    key_technologies: Set[str]
    dominant_paradigms: Set[str]
    infrastructure: Set[str]
    social_structures: Set[str]
    
    # Quantitative metrics
    innovation_rate: float  # 0.0 to 1.0
    knowledge_accumulation: float  # 0.0 to 1.0
    technological_complexity: float  # 0.0 to 1.0
    economic_growth_potential: float  # 0.0 to 1.0
    
    # Transition properties
    transition_from: Optional[EraType]
    transition_type: TransitionType
    
    def __post_init__(self):
        """Validate era data"""
        assert self.start_year > 0, "Start year must be positive"
        if self.end_year is not None:
            assert self.end_year >= self.start_year, "End year must be after start year"
        assert 0.0 <= self.innovation_rate <= 1.0, "Innovation rate must be between 0 and 1"
        assert 0.0 <= self.knowledge_accumulation <= 1.0, "Knowledge accumulation must be between 0 and 1"
        assert 0.0 <= self.technological_complexity <= 1.0, "Technological complexity must be between 0 and 1"
        assert 0.0 <= self.economic_growth_potential <= 1.0, "Economic growth potential must be between 0 and 1"
    
    @property
    def duration(self) -> Optional[int]:
        """Calculate era duration in years"""
        if self.end_year is None:
            return None
        return self.end_year - self.start_year
    
    @property
    def is_current(self) -> bool:
        """Check if this is the current technological era"""
        current_year = datetime.now().year
        if self.end_year is None:
            return current_year >= self.start_year
        return self.start_year <= current_year <= self.end_year
    
    @property
    def era_score(self) -> float:
        """Calculate overall era development score"""
        weights = {
            'innovation': 0.3,
            'knowledge': 0.25,
            'complexity': 0.25,
            'economic': 0.2
        }
        
        score = (
            weights['innovation'] * self.innovation_rate +
            weights['knowledge'] * self.knowledge_accumulation +
            weights['complexity'] * self.technological_complexity +
            weights['economic'] * self.economic_growth_potential
        )
        
        return round(score, 4)
    
    def get_transition_complexity(self) -> float:
        """Calculate complexity of transition to this era"""
        transition_complexities = {
            TransitionType.GRADUAL: 0.3,
            TransitionType.DISRUPTIVE: 0.6,
            TransitionType.REVOLUTIONARY: 0.9,
            TransitionType.CONVERGENT: 0.7,
            TransitionType.REGRESSIVE: 0.5
        }
        
        return transition_complexities.get(self.transition_type, 0.5)


class EraAnalyzer:
    """Analyzes technological eras and their transitions"""
    
    def __init__(self):
        self.eras: Dict[EraType, TechnologicalEra] = {}
        self.era_sequence: List[EraType] = []
    
    def add_era(self, era: TechnologicalEra) -> bool:
        """Add a technological era to the analyzer"""
        if era.era_type in self.eras:
            return False
        
        self.eras[era.era_type] = era
        self._update_era_sequence()
        return True
    
    def remove_era(self, era_type: EraType) -> bool:
        """Remove a technological era"""
        if era_type not in self.eras:
            return False
        
        del self.eras[era_type]
        self._update_era_sequence()
        return True
    
    def _update_era_sequence(self):
        """Update the chronological sequence of eras"""
        eras_list = list(self.eras.values())
        eras_list.sort(key=lambda x: x.start_year)
        self.era_sequence = [era.era_type for era in eras_list]
    
    def get_current_era(self) -> Optional[TechnologicalEra]:
        """Get the current technological era"""
        for era in self.eras.values():
            if era.is_current:
                return era
        return None
    
    def get_era_transitions(self) -> List[Dict]:
        """Analyze transitions between consecutive eras"""
        transitions = []
        
        for i in range(1, len(self.era_sequence)):
            from_era_type = self.era_sequence[i-1]
            to_era_type = self.era_sequence[i]
            
            from_era = self.eras[from_era_type]
            to_era = self.eras[to_era_type]
            
            transition = {
                'from_era': from_era_type.value,
                'to_era': to_era_type.value,
                'transition_type': to_era.transition_type.value,
                'duration_years': to_era.start_year - from_era.end_year if from_era.end_year else None,
                'complexity': to_era.get_transition_complexity(),
                'development_gap': to_era.era_score - from_era.era_score,
                'key_changes': self._analyze_era_changes(from_era, to_era)
            }
            
            transitions.append(transition)
        
        return transitions
    
    def _analyze_era_changes(self, from_era: TechnologicalEra, to_era: TechnologicalEra) -> Dict[str, float]:
        """Analyze changes between two consecutive eras"""
        changes = {}
        
        # Technology changes
        tech_overlap = len(from_era.key_technologies.intersection(to_era.key_technologies))
        tech_change = 1 - (tech_overlap / max(len(from_era.key_technologies), 1))
        changes['technology_turnover'] = tech_change
        
        # Paradigm changes
        paradigm_overlap = len(from_era.dominant_paradigms.intersection(to_era.dominant_paradigms))
        paradigm_change = 1 - (paradigm_overlap / max(len(from_era.dominant_paradigms), 1))
        changes['paradigm_shift'] = paradigm_change
        
        # Infrastructure changes
        infra_overlap = len(from_era.infrastructure.intersection(to_era.infrastructure))
        infra_change = 1 - (infra_overlap / max(len(from_era.infrastructure), 1))
        changes['infrastructure_replacement'] = infra_change
        
        # Metric changes
        changes['innovation_acceleration'] = to_era.innovation_rate - from_era.innovation_rate
        changes['complexity_increase'] = to_era.technological_complexity - from_era.technological_complexity
        
        return changes
    
    def calculate_era_metrics(self) -> Dict[str, float]:
        """Calculate overall metrics across all technological eras"""
        if not self.eras:
            return {}
        
        eras_list = list(self.eras.values())
        
        metrics = {
            'total_eras': len(eras_list),
            'average_era_duration': statistics.mean(
                era.duration for era in eras_list if era.duration is not None
            ) if any(era.duration for era in eras_list) else 0,
            'average_era_score': statistics.mean(era.era_score for era in eras_list),
            'technological_acceleration': self._calculate_acceleration_rate(),
            'era_volatility': self._calculate_era_volatility(),
        }
        
        # Transition analysis
        transitions = self.get_era_transitions()
        if transitions:
            metrics['average_transition_complexity'] = statistics.mean(
                t['complexity'] for t in transitions
            )
            metrics['average_development_gap'] = statistics.mean(
                t['development_gap'] for t in transitions
            )
        
        return metrics
    
    def _calculate_acceleration_rate(self) -> float:
        """Calculate rate of technological acceleration across eras"""
        if len(self.era_sequence) < 2:
            return 0.0
        
        innovation_rates = [self.eras[era_type].innovation_rate for era_type in self.era_sequence]
        
        # Calculate acceleration as change in innovation rate over time
        acceleration_sum = 0.0
        for i in range(1, len(innovation_rates)):
            acceleration = innovation_rates[i] - innovation_rates[i-1]
            acceleration_sum += acceleration
        
        average_acceleration = acceleration_sum / (len(innovation_rates) - 1)
        return max(0.0, average_acceleration)
    
    def _calculate_era_volatility(self) -> float:
        """Calculate volatility of era transitions"""
        transitions = self.get_era_transitions()
        if not transitions:
            return 0.0
        
        # Volatility based on transition complexity and development gaps
        complexity_scores = [t['complexity'] for t in transitions]
        gap_absolutes = [abs(t['development_gap']) for t in transitions]
        
        volatility = statistics.mean(complexity_scores) * statistics.mean(gap_absolutes)
        return volatility
    
    def predict_next_era(self, current_trends: Dict[str, float]) -> Dict:
        """
        Predict characteristics of the next technological era.
        
        Args:
            current_trends: Dictionary of current trend indicators
            
        Returns:
            Prediction of next era characteristics
        """
        current_era = self.get_current_era()
        if not current_era:
            return {}
        
        # Base prediction on current era and trends
        next_era_type = self._get_next_era_type(current_era.era_type)
        
        prediction = {
            'predicted_era_type': next_era_type.value if next_era_type else 'unknown',
            'estimated_start': datetime.now().year + self._estimate_transition_time(current_era),
            'key_technologies': self._predict_emerging_technologies(current_trends),
            'expected_innovation_rate': min(1.0, current_era.innovation_rate * 1.2),
            'expected_complexity': min(1.0, current_era.technological_complexity * 1.15),
            'transition_likelihood': self._calculate_transition_likelihood(current_trends),
        }
        
        return prediction
    
    def _get_next_era_type(self, current_era_type: EraType) -> Optional[EraType]:
        """Get the most likely next era type"""
        era_order = list(EraType)
        try:
            current_index = era_order.index(current_era_type)
            if current_index < len(era_order) - 1:
                return era_order[current_index + 1]
        except ValueError:
            pass
        return None
    
    def _estimate_transition_time(self, current_era: TechnologicalEra) -> int:
        """Estimate time until next era transition"""
        # Base estimation on current era characteristics and historical patterns
        base_transition_time = 20  # years
        
        # Adjust based on innovation rate (higher rate = faster transitions)
        innovation_adjustment = (1 - current_era.innovation_rate) * 10
        
        # Adjust based on transition type of current era
        transition_adjustments = {
            TransitionType.GRADUAL: 10,
            TransitionType.DISRUPTIVE: -5,
            TransitionType.REVOLUTIONARY: -10,
            TransitionType.CONVERGENT: 0,
            TransitionType.REGRESSIVE: 15
        }
        transition_adjustment = transition_adjustments.get(current_era.transition_type, 0)
        
        estimated_time = base_transition_time + innovation_adjustment + transition_adjustment
        return max(5, int(estimated_time))  # Minimum 5 years
    
    def _predict_emerging_technologies(self, current_trends: Dict[str, float]) -> List[str]:
        """Predict which technologies will dominate the next era"""
        # This is a simplified prediction - in practice, this would use more sophisticated analysis
        emerging_techs = []
        
        trend_threshold = 0.7
        for tech, trend_strength in current_trends.items():
            if trend_strength >= trend_threshold:
                emerging_techs.append(tech)
        
        return emerging_techs
    
    def _calculate_transition_likelihood(self, current_trends: Dict[str, float]) -> float:
        """Calculate likelihood of era transition occurring soon"""
        if not current_trends:
            return 0.5
        
        # Calculate average trend strength
        avg_trend = statistics.mean(current_trends.values()) if current_trends else 0.0
        
        # Higher trends indicate higher transition likelihood
        likelihood = min(1.0, avg_trend * 1.5)
        return likelihood