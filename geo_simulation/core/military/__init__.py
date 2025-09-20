# Package initialization

# __init__.py
"""
Advanced Military Simulation System
A comprehensive framework for simulating military operations, logistics, and strategy.
"""

__version__ = "2.0.0"
__author__ = "Military Simulation Team"

from .combat import CombatSystem
from .doctrine import MilitaryDoctrine
from .formations import FormationSystem
from .logistics import LogisticsSystem
from .military_doctrine import DoctrineManager
from .military_technology import TechnologyTree
from .morale import MoraleSystem
from .occupation_system import OccupationManager
from .rebellion_system import RebellionSystem
from .recruitment import RecruitmentSystem
from .units import Unit, UnitManager
from .war_exhaustion import WarExhaustionSystem
from .warfare_system import WarfareCoordinator

__all__ = [
    'CombatSystem',
    'MilitaryDoctrine',
    'FormationSystem',
    'LogisticsSystem',
    'DoctrineManager',
    'TechnologyTree',
    'MoraleSystem',
    'OccupationManager',
    'RebellionSystem',
    'RecruitmentSystem',
    'Unit',
    'UnitManager',
    'WarExhaustionSystem',
    'WarfareCoordinator'
]