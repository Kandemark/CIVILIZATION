# Package initialization

# utils/__init__.py
"""
Advanced Utilities for Simulation Systems
Mathematical, geometric, and utility functions for complex simulations.
"""

__version__ = "1.0.0"
__author__ = "Simulation Utilities Team"

"""
Utility Module for Civilization Simulation

General utilities and helper functions for various simulation systems.
"""

# --- Core utilities ---
from .border_calculation import BorderCalculator, BorderConflict, BorderType
from .color_utils import ColorGradient, PoliticalMapColors, TerrainColors
from .conflict_resolution import ConflictResolver, NegotiationSystem, DiplomaticWeight
from .cultural_similarity import CulturalTraits, CulturalSimilarity
from .decorators import timer, cache, retry, validate_input, log_execution
from .demographic_calculations import DemographicModel, PopulationPyramid, MigrationCalculator
from .economic_modeling import (
    EconomicEventType,
    CurrencyRegime,
    EconomicPolicy,
    EconomicEvent,
    Currency,
    EconomicSector,
    Market,
    InternationalLinkage,
    EconomicState,
    EconomicModel,
)
from .file_utils import JSONHandler, YAMLConfig, SaveGameManager, DataSerializer
from .geometric_operations import GeometryUtils, PolygonOperations, DistanceCalculator
from .geometry import Point, Line, Polygon, Circle, BoundingBox
from .logger import SimulationLogger, LogLevel, LogFormatter
from .math_utils import MathUtils, Statistics, VectorCalculus, MatrixOperations
from .pathfinding import Pathfinder, TerrainWeighting, RouteOptimizer
from .perf_monitor import PerformanceMonitor, MemoryTracker, ExecutionProfiler
from .random_utils import RandomGenerator, WeightedRandom, SeededRandom
from .validators import InputValidator, SchemaValidator, RangeValidator

# --- Math subpackage exports ---
from .math.chaos_theory import ChaosSystem, LorenzAttractor, LogisticMap
from .math.fuzzy_math import FuzzySet, FuzzyLogic, FuzzyVariable
from .math.graph_theory import Graph, Node, Edge, shortest_path, minimum_spanning_tree
from .math.probability_distributions import (
    ProbabilityDistribution, NormalDistribution, PoissonDistribution
)
from .math.systems_dynamics import SystemDynamics, FeedbackLoop, Stock, Flow


__all__ = [
    # Borders & Geography
    "BorderCalculator", "BorderConflict", "BorderType",
    "ColorGradient", "PoliticalMapColors", "TerrainColors",
    "GeometryUtils", "PolygonOperations", "DistanceCalculator",
    "Point", "Line", "Polygon", "Circle", "BoundingBox",

    # Conflict & Diplomacy
    "ConflictResolver", "NegotiationSystem", "DiplomaticWeight",

    # Culture
    "CulturalTraits", "CulturalSimilarity",

    # Demographics
    "DemographicModel", "PopulationPyramid", "MigrationCalculator",

    # Economy
    "EconomicEventType", "CurrencyRegime", "EconomicPolicy", "EconomicEvent",
    "Currency", "EconomicSector", "Market", "InternationalLinkage",
    "EconomicState", "EconomicModel",

    # Math & Computation (base)
    "MathUtils", "Statistics", "VectorCalculus", "MatrixOperations",

    # Math Subpackage (advanced)
    "ChaosSystem", "LorenzAttractor", "LogisticMap",
    "FuzzySet", "FuzzyLogic", "FuzzyVariable",
    "Graph", "Node", "Edge", "shortest_path", "minimum_spanning_tree",
    "ProbabilityDistribution", "NormalDistribution", "PoissonDistribution",
    "SystemDynamics", "FeedbackLoop", "Stock", "Flow",

    # Pathfinding
    "Pathfinder", "TerrainWeighting", "RouteOptimizer",

    # Performance Monitoring
    "PerformanceMonitor", "MemoryTracker", "ExecutionProfiler",

    # Randomness
    "RandomGenerator", "WeightedRandom", "SeededRandom",

    # Logging
    "SimulationLogger", "LogLevel", "LogFormatter",

    # File & Data
    "JSONHandler", "YAMLConfig", "SaveGameManager", "DataSerializer",

    # Validation
    "InputValidator", "SchemaValidator", "RangeValidator",

    # Decorators
    "timer", "cache", "retry", "validate_input", "log_execution",
]
