"""Base economy class"""

# Module implementation

from abc import ABC, abstractmethod
from typing import Dict, List, Set, Optional, Any, Tuple
from enum import Enum
import random
import numpy as np
from dataclasses import dataclass
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class EconomicState(Enum):
    BOOM = "boom"           # Rapid growth, high confidence
    EXPANSION = "expansion" # Steady growth
    STABILITY = "stability" # Balanced economy
    STAGNATION = "stagnation" # No growth
    RECESSION = "recession" # Economic contraction
    DEPRESSION = "depression" # Severe contraction
    HYPERINFLATION = "hyperinflation" # Currency collapse
    DEFAULT = "default"     # Debt crisis

class DevelopmentLevel(Enum):
    PRIMITIVE = "primitive"      # Hunter-gatherer
    AGRARIAN = "agrarian"        # Agricultural society
    FEUDAL = "feudal"           # Manor-based economy
    MERCANTILE = "mercantile"   # Early trade focus
    INDUSTRIAL = "industrial"   # Manufacturing focus
    POST_INDUSTRIAL = "post_industrial" # Service focus
    DIGITAL = "digital"         # Information economy
    AUTOMATED = "automated"     # Full automation

class EconomicEntity(ABC):
    """Base class for all economic actors - can be regions, cities, corporations"""
    
    def __init__(self, entity_id: str, name: str, development_level: DevelopmentLevel):
        self.id = entity_id
        self.name = name
        self.development_level = development_level
        self.wealth: float = self._get_initial_wealth()
        self.inventory: Dict[str, float] = {}
        self.gdp: float = 0.0
        self.gdp_growth: float = 0.0
        self.inflation: float = 0.02  # 2% base inflation
        self.unemployment: float = 0.05
        self.currency_value: float = 1.0
        self.debt: float = 0.0
        self.tax_rate: float = 0.15
        self.economic_policies: Dict[str, Any] = {}
        self.connected_entities: List['EconomicEntity'] = []
        
    def _get_initial_wealth(self) -> float:
        """Get initial wealth based on development level"""
        wealth_map = {
            DevelopmentLevel.PRIMITIVE: 1000,
            DevelopmentLevel.AGRARIAN: 5000,
            DevelopmentLevel.FEUDAL: 10000,
            DevelopmentLevel.MERCANTILE: 50000,
            DevelopmentLevel.INDUSTRIAL: 200000,
            DevelopmentLevel.POST_INDUSTRIAL: 1000000,
            DevelopmentLevel.DIGITAL: 5000000,
            DevelopmentLevel.AUTOMATED: 20000000
        }
        return wealth_map.get(self.development_level, 1000)
    
    @abstractmethod
    def simulate_turn(self, global_economy: 'CivilizationEconomy') -> Dict[str, Any]:
        """Simulate one turn for this entity"""
        pass
    
    def calculate_economic_output(self) -> float:
        """Calculate GDP based on development level and assets"""
        base_output = self.wealth * 0.1  # 10% of wealth as annual output
        
        # Development multipliers
        dev_multipliers = {
            DevelopmentLevel.PRIMITIVE: 0.1,
            DevelopmentLevel.AGRARIAN: 0.3,
            DevelopmentLevel.FEUDAL: 0.5,
            DevelopmentLevel.MERCANTILE: 0.8,
            DevelopmentLevel.INDUSTRIAL: 1.2,
            DevelopmentLevel.POST_INDUSTRIAL: 1.5,
            DevelopmentLevel.DIGITAL: 2.0,
            DevelopmentLevel.AUTOMATED: 3.0
        }
        
        return base_output * dev_multipliers.get(self.development_level, 1.0)

@dataclass
class EconomicReport:
    """Comprehensive economic report for any entity"""
    entity_id: str
    gdp: float
    gdp_growth: float
    inflation: float
    unemployment: float
    wealth: float
    debt: float
    debt_to_gdp: float
    trade_balance: float
    currency_strength: float
    economic_state: EconomicState
    development_level: DevelopmentLevel
    key_industries: List[str]
    economic_health: float  # 0-1 score
    
    def to_dict(self) -> Dict[str, Any]:
        return {field: getattr(self, field) for field in self.__dataclass_fields__}
    
class CivilizationEconomy:
    """Manages the overall economy of the civilization"""
    
    def __init__(self):
        self.entities: Dict[str, EconomicEntity] = {}
        self.global_gdp: float = 0.0
        self.global_inflation: float = 0.02
        self.global_unemployment: float = 0.05
        self.global_trade_balance: float = 0.0
        self.global_currency_value: float = 1.0
        self.turn: int = 0
        
    def add_entity(self, entity: EconomicEntity):
        """Add an economic entity to the civilization"""
        self.entities[entity.id] = entity
        
    def simulate_turn(self) -> Dict[str, Any]:
        """Simulate one turn for the entire economy"""
        self.turn += 1
        logger.info(f"Simulating turn {self.turn}")
        
        total_gdp = 0.0
        total_inflation = 0.0
        total_unemployment = 0.0
        total_wealth = 0.0
        total_debt = 0.0
        
        reports = []
        
        for entity in self.entities.values():
            result = entity.simulate_turn(self)
            report = EconomicReport(
                entity_id=entity.id,
                gdp=entity.gdp,
                gdp_growth=entity.gdp_growth,
                inflation=entity.inflation,
                unemployment=entity.unemployment,
                wealth=entity.wealth,
                debt=entity.debt,
                debt_to_gdp=(entity.debt / entity.gdp) if entity.gdp > 0 else float('inf'),
                trade_balance=result.get('trade_balance', 0.0),
                currency_strength=entity.currency_value,
                economic_state=result.get('economic_state', EconomicState.STABILITY),
                development_level=entity.development_level,
                key_industries=result.get('key_industries', []),
                economic_health=result.get('economic_health', 0.5)
            )
            reports.append(report)
            
            total_gdp += entity.gdp
            total_inflation += entity.inflation
            total_unemployment += entity.unemployment
            total_wealth += entity.wealth
            total_debt += entity.debt
            
        num_entities = len(self.entities)
        self.global_gdp = total_gdp
        self.global_inflation = total_inflation / num_entities if num_entities > 0 else 0.02
        self.global_unemployment = total_unemployment / num_entities if num_entities > 0 else 0.05
        self.global_trade_balance = sum(r.trade_balance for r in reports)
        self.global_currency_value = np.mean([e.currency_value for e in self.entities.values()]) if num_entities > 0 else 1.0
        self.global_debt_to_gdp = (total_debt / total_gdp) if total_gdp > 0 else float('inf')

        logger.info(f"End of turn {self.turn}: Global GDP={self.global_gdp:.2f}, Inflation={self.global_inflation:.2%}, Unemployment={self.global_unemployment:.2%}, Trade Balance={self.global_trade_balance:.2f}")

class ResourceType(Enum):
    FOOD = "food"
    WATER = "water"
    ENERGY = "energy"
    MINERALS = "minerals"
    LUXURIES = "luxuries"
    TECHNOLOGY = "technology"
    SERVICES = "services"
    LABOR = "labor"
    