# Package initialization

# politics/__init__.py
"""
Advanced Political Simulation System
A comprehensive framework for simulating political dynamics, factions, and power struggles.
"""

__version__ = "1.0.0"
__author__ = "Political Simulation Team"

from .faction_system import Faction, FactionSystem, Ideology, PoliticalAlignment
from .legitimacy import LegitimacySystem, LegitimacySource, LegitimacyLevel
from .political_crises import PoliticalCrisis, CrisisSystem, CrisisType, CrisisSeverity
from .power_struggles import PowerStruggle, StruggleSystem, StruggleType, StruggleOutcome
from .succession_systems import SuccessionSystem, SuccessionType, SuccessionCrisis

__all__ = [
    'Faction', 'FactionSystem', 'Ideology', 'PoliticalAlignment',
    'LegitimacySystem', 'LegitimacySource', 'LegitimacyLevel',
    'PoliticalCrisis', 'CrisisSystem', 'CrisisType', 'CrisisSeverity',
    'PowerStruggle', 'StruggleSystem', 'StruggleType', 'StruggleOutcome',
    'SuccessionSystem', 'SuccessionType', 'SuccessionCrisis'
]