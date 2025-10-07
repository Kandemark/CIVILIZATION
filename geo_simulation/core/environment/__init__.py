# Package initialization

"""
Environmental Science Module

A comprehensive package for modeling and analyzing environmental systems,
climate change, ecological dynamics, and resource management.
"""

__version__ = "1.0.0"
__author__ = "Environmental Science Team"

from .climate import Climate
from .climate_change import ClimateChange
from .climate_system import ClimateSystem
from .ecology import Ecosystem, Species
from .environmental_degradation import EnvironmentalDegradation
from .geography import Geography
from .natural_disasters import NaturalDisaster, DisasterManager
from .resource_depletion import ResourceDepletion
from .resources import NaturalResource, ResourceManager

__all__ = [
    'Climate',
    'ClimateChange', 
    'ClimateSystem',
    'Ecosystem',
    'Species',
    'EnvironmentalDegradation',
    'Geography',
    'NaturalDisaster',
    'DisasterManager',
    'ResourceDepletion',
    'NaturalResource',
    'ResourceManager'
]