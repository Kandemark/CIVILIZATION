"""Random discoveries"""

# Module implementation

"""
Core Innovation Module

Defines the fundamental Innovation class and related components
that represent technological innovations and their properties.
"""

from dataclasses import dataclass
from typing import List, Dict, Optional
from enum import Enum
import datetime


class InnovationType(Enum):
    """Types of technological innovations"""
    INCREMENTAL = "incremental"
    RADICAL = "radical"
    ARCHITECTURAL = "architectural"
    DISRUPTIVE = "disruptive"
    SUSTAINING = "sustaining"


class DevelopmentStage(Enum):
    """Stages of innovation development"""
    RESEARCH = "research"
    DEVELOPMENT = "development" 
    DEMONSTRATION = "demonstration"
    DEPLOYMENT = "deployment"
    MATURE = "mature"


@dataclass
class Innovation:
    """
    Represents a technological innovation with its properties
    and development characteristics.
    """
    
    id: str
    name: str
    description: str
    innovation_type: InnovationType
    development_stage: DevelopmentStage
    
    # Core metrics
    technical_feasibility: float  # 0.0 to 1.0
    economic_viability: float     # 0.0 to 1.0  
    social_acceptance: float      # 0.0 to 1.0
    environmental_impact: float   # -1.0 to 1.0
    
    # Knowledge properties
    required_knowledge: List[str]
    knowledge_domain: str
    complexity: float  # 0.0 to 1.0
    
    # Temporal properties
    creation_date: datetime.datetime
    expected_maturation: datetime.datetime
    
    # Relationships
    dependencies: List[str]  # IDs of prerequisite innovations
    related_areas: List[str]  # Research areas
    
    def __post_init__(self):
        """Validate innovation data after initialization"""
        self._validate_metrics()
    
    def _validate_metrics(self):
        """Validate that all metrics are within expected ranges"""
        assert 0.0 <= self.technical_feasibility <= 1.0, "Technical feasibility must be between 0 and 1"
        assert 0.0 <= self.economic_viability <= 1.0, "Economic viability must be between 0 and 1"
        assert 0.0 <= self.social_acceptance <= 1.0, "Social acceptance must be between 0 and 1"
        assert -1.0 <= self.environmental_impact <= 1.0, "Environmental impact must be between -1 and 1"
        assert 0.0 <= self.complexity <= 1.0, "Complexity must be between 0 and 1"
    
    @property
    def overall_potential(self) -> float:
        """
        Calculate overall innovation potential based on multiple factors.
        
        Returns:
            float: Overall potential score (0.0 to 1.0)
        """
        weights = {
            'technical': 0.3,
            'economic': 0.3, 
            'social': 0.2,
            'environmental': 0.2
        }
        
        # Normalize environmental impact to 0-1 scale
        env_score = (self.environmental_impact + 1) / 2
        
        potential = (
            weights['technical'] * self.technical_feasibility +
            weights['economic'] * self.economic_viability +
            weights['social'] * self.social_acceptance +
            weights['environmental'] * env_score
        )
        
        return round(potential, 4)
    
    def advance_stage(self) -> bool:
        """
        Advance the innovation to the next development stage.
        
        Returns:
            bool: True if advancement was successful, False otherwise
        """
        stage_order = list(DevelopmentStage)
        current_index = stage_order.index(self.development_stage)
        
        if current_index < len(stage_order) - 1:
            self.development_stage = stage_order[current_index + 1]
            return True
        return False
    
    def is_viable(self, threshold: float = 0.6) -> bool:
        """
        Check if innovation is viable based on overall potential.
        
        Args:
            threshold: Minimum potential score for viability
            
        Returns:
            bool: True if innovation is viable
        """
        return self.overall_potential >= threshold
    
    def to_dict(self) -> Dict:
        """Convert innovation to dictionary representation"""
        return {
            'id': self.id,
            'name': self.name,
            'description': self.description,
            'innovation_type': self.innovation_type.value,
            'development_stage': self.development_stage.value,
            'technical_feasibility': self.technical_feasibility,
            'economic_viability': self.economic_viability,
            'social_acceptance': self.social_acceptance,
            'environmental_impact': self.environmental_impact,
            'complexity': self.complexity,
            'overall_potential': self.overall_potential,
            'required_knowledge': self.required_knowledge,
            'knowledge_domain': self.knowledge_domain,
            'dependencies': self.dependencies,
            'related_areas': self.related_areas,
            'creation_date': self.creation_date.isoformat(),
            'expected_maturation': self.expected_maturation.isoformat()
        }