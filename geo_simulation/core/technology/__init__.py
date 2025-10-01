# Package initialization

"""
Technology Innovation System Package

A comprehensive framework for modeling technological development,
innovation systems, knowledge diffusion, and specialization patterns
across different technological eras.
"""

__version__ = "1.0.0"
__author__ = "Technology Innovation Team"

from .innovation import Innovation, InnovationType, DevelopmentStage
from .innovation_system import InnovationSystem, TechnologyNode, TechCategory, TechEra, BreakthroughType, ResearchTeam
from .knowledge_diffusion import KnowledgeDiffusion, KnowledgeEntity, DiffusionChannel, AdoptionCategory
from .research_areas import ResearchArea, ResearchAreaManager, ResearchIntensity, ResearchMaturity
from .specialization import SpecializationAnalyzer, SpecializationProfile, SpecializationType, SpecializationStrategy
from .technological_eras import TechnologicalEra, EraAnalyzer, EraType, TransitionType
from .technological_regression import RegressionEvent, RegressionAnalyzer, RegressionCause, RegressionPattern, RecoveryPotential
from .development_index import DevelopmentIndex

__all__ = [
    # Innovation core
    "Innovation",
    "InnovationType", 
    "DevelopmentStage",
    
    # Innovation system with tech trees
    "InnovationSystem",
    "TechnologyNode",
    "TechCategory", 
    "TechEra",
    "BreakthroughType",
    "ResearchTeam",
    
    # Knowledge diffusion
    "KnowledgeDiffusion",
    "KnowledgeEntity", 
    "DiffusionChannel",
    "AdoptionCategory",
    
    # Research areas
    "ResearchArea",
    "ResearchAreaManager",
    "ResearchIntensity",
    "ResearchMaturity",
    
    # Specialization
    "SpecializationAnalyzer",
    "SpecializationProfile", 
    "SpecializationType",
    "SpecializationStrategy",
    
    # Technological eras
    "TechnologicalEra",
    "EraAnalyzer",
    "EraType", 
    "TransitionType",
    
    # Technological regression
    "TechnologicalRegression",
    "RegressionEvent",
    "RegressionAnalyzer",
    "RegressionCause",
    "RegressionPattern", 
    "RecoveryPotential",
    
    # Development metrics
    "DevelopmentIndex",
]