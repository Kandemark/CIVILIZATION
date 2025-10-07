"""Internal and external trade"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple, Any
from dataclasses import dataclass
from enum import Enum
import random
from base_economy import ResourceType, EconomicEntity

class TradePolicy(Enum):
    FREE_TRADE = "free_trade"        # No restrictions
    PROTECTIONIST = "protectionist"  # High tariffs
    MERCANTILIST = "mercantilist"    # Export focus
    ISOLATIONIST = "isolationist"    # Minimal trade
    MANAGED = "managed"              # Government controlled

class TariffType(Enum):
    IMPORT = "import"
    EXPORT = "export"
    TRANSIT = "transit"

@dataclass
class Tariff:
    tariff_id: str
    name: str
    tariff_type: TariffType
    resource: ResourceType
    rate: float  # 0-1, percentage
    target_entity: Optional[str] = None  # Specific entity tariff

@dataclass
class TradeDeal:
    deal_id: str
    entity_a: str
    entity_b: str
    terms: Dict[ResourceType, float]  # resource -> amount per turn
    duration: int  # turns
    tariffs: List[Tariff]
    special_conditions: Dict[str, Any]
    active: bool = True

class TradeManager:
    """Manages trade policies, tariffs, and international trade deals"""
    
    def __init__(self):
        self.trade_policies: Dict[str, TradePolicy] = {}  # entity -> policy
        self.tariffs: Dict[str, List[Tariff]] = {}  # entity -> tariffs
        self.trade_deals: List[TradeDeal] = []
        self.trade_embargoes: Set[Tuple[str, str]] = set()  # (imposer, target)
        self.export_quotas: Dict[Tuple[str, ResourceType], float] = {}  # (entity, resource) -> max_export
        self.import_restrictions: Dict[Tuple[str, ResourceType], float] = {}  # max_import
        
    def set_trade_policy(self, entity_id: str, policy: TradePolicy):
        """Set trade policy for an entity"""
        self.trade_policies[entity_id] = policy
        
        # Apply policy effects
        if policy == TradePolicy.ISOLATIONIST:
            # High tariffs across the board
            self._set_isolationist_tariffs(entity_id)
        elif policy == TradePolicy.FREE_TRADE:
            # Remove most tariffs
            self._set_free_trade_tariffs(entity_id)
    
    def _set_isolationist_tariffs(self, entity_id: str):
        """Set high tariffs for isolationist policy"""
        high_tariffs = []
        for resource in ResourceType:
            high_tariffs.append(Tariff(
                f"iso_import_{resource.value}", f"Import {resource.value}",
                TariffType.IMPORT, resource, 0.5  # 50% tariff
            ))
        self.tariffs[entity_id] = high_tariffs
    
    def _set_free_trade_tariffs(self, entity_id: str):
        """Set low tariffs for free trade policy"""
        low_tariffs = []
        for resource in ResourceType:
            low_tariffs.append(Tariff(
                f"free_import_{resource.value}", f"Import {resource.value}",
                TariffType.IMPORT, resource, 0.05  # 5% tariff
            ))
        self.tariffs[entity_id] = low_tariffs
    
    def calculate_tariff_cost(self, importer: str, exporter: str, 
                            resource: ResourceType, amount: float, base_price: float) -> float:
        """Calculate total tariff cost for a trade"""
        total_tariff = 0.0
        
        # Importer tariffs
        importer_tariffs = self.tariffs.get(importer, [])
        for tariff in importer_tariffs:
            if (tariff.tariff_type == TariffType.IMPORT and 
                tariff.resource == resource and
                (tariff.target_entity is None or tariff.target_entity == exporter)):
                total_tariff += tariff.rate
        
        # Exporter tariffs (export taxes)
        exporter_tariffs = self.tariffs.get(exporter, [])
        for tariff in exporter_tariffs:
            if (tariff.tariff_type == TariffType.EXPORT and 
                tariff.resource == resource):
                total_tariff += tariff.rate
        
        return base_price * amount * total_tariff
    
    def create_trade_deal(self, entity_a: str, entity_b: str, 
                         terms: Dict[ResourceType, float], duration: int = 10) -> TradeDeal:
        """Create a new trade deal between entities"""
        deal = TradeDeal(
            deal_id=f"deal_{len(self.trade_deals)}",
            entity_a=entity_a,
            entity_b=entity_b,
            terms=terms,
            duration=duration,
            tariffs=[],
            special_conditions={}
        )
        
        self.trade_deals.append(deal)
        return deal
    
    def check_trade_restrictions(self, exporter: str, importer: str, 
                               resource: ResourceType, amount: float) -> bool:
        """Check if trade is allowed under current restrictions"""
        # Check embargoes
        if (exporter, importer) in self.trade_embargoes or (importer, exporter) in self.trade_embargoes:
            return False
        
        # Check export quotas
        export_quota = self.export_quotas.get((exporter, resource))
        if export_quota and amount > export_quota:
            return False
        
        # Check import restrictions
        import_restriction = self.import_restrictions.get((importer, resource))
        if import_restriction and amount > import_restriction:
            return False
        
        return True
    
    def simulate_trade_deals(self) -> Dict[str, Dict[ResourceType, float]]:
        """Execute all active trade deals"""
        trade_flows = {}
        
        for deal in self.trade_deals:
            if not deal.active:
                continue
                
            # Execute terms for both directions
            for resource, amount in deal.terms.items():
                # Entity A to Entity B
                if amount > 0:
                    self._execute_trade_flow(deal.entity_a, deal.entity_b, resource, amount, trade_flows)
                # Entity B to Entity A  
                elif amount < 0:
                    self._execute_trade_flow(deal.entity_b, deal.entity_a, resource, abs(amount), trade_flows)
            
            # Update deal duration
            deal.duration -= 1
            if deal.duration <= 0:
                deal.active = False
        
        return trade_flows
    
    def _execute_trade_flow(self, exporter: str, importer: str, resource: ResourceType,
                          amount: float, trade_flows: Dict[str, Dict[ResourceType, float]]):
        """Execute a single trade flow"""
        if exporter not in trade_flows:
            trade_flows[exporter] = {}
        if importer not in trade_flows:
            trade_flows[importer] = {}
        
        # Update export/import records
        trade_flows[exporter][resource] = trade_flows[exporter].get(resource, 0) - amount
        trade_flows[importer][resource] = trade_flows[importer].get(resource, 0) + amount
    
    def impose_embargo(self, imposer: str, target: str):
        """Impose trade embargo"""
        self.trade_embargoes.add((imposer, target))
        
        # Cancel any existing trade deals
        for deal in self.trade_deals:
            if ((deal.entity_a == imposer and deal.entity_b == target) or
                (deal.entity_a == target and deal.entity_b == imposer)):
                deal.active = False
    
    def set_export_quota(self, entity: str, resource: ResourceType, max_export: float):
        """Set export quota for a resource"""
        self.export_quotas[(entity, resource)] = max_export
    
    def set_import_restriction(self, entity: str, resource: ResourceType, max_import: float):
        """Set import restriction for a resource"""
        self.import_restrictions[(entity, resource)] = max_import