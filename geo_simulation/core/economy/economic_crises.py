"""Recessions and depressions"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
import random
import numpy as np
from base_economy import ResourceType, EconomicEntity, DevelopmentLevel, EconomicState

class CrisisSeverity(Enum):
    MILD = "mild"        # Minor economic disruption
    MODERATE = "moderate" # Significant but manageable
    SEVERE = "severe"    # Major economic damage
    CATASTROPHIC = "catastrophic" # Economic collapse

class CrisisType(Enum):
    RECESSION = "recession"              # General economic contraction
    DEPRESSION = "depression"            # Severe, prolonged recession
    HYPERINFLATION = "hyperinflation"    # Currency collapse
    MARKET_CRASH = "market_crash"        # Asset price collapse
    DEBT_CRISIS = "debt_crisis"          # Sovereign/default crisis
    BANKING_CRISIS = "banking_crisis"    # Financial system failure
    CURRENCY_CRISIS = "currency_crisis"  # Exchange rate collapse
    COMMODITY_CRISIS = "commodity_crisis" # Resource price shock
    TRADE_WAR = "trade_war"              # Protectionism escalation
    SUPPLY_CHAIN_CRISIS = "supply_chain_crisis" # Production disruption

@dataclass
class EconomicCrisis:
    crisis_id: str
    crisis_type: CrisisType
    severity: CrisisSeverity
    duration: int
    origin_entity: str
    affected_entities: Set[str]
    contagion_risk: float  # 0-1, likelihood of spreading
    resolution_progress: float = 0.0
    turns_active: int = 0
    active: bool = True
    
    # Crisis-specific effects
    gdp_impact: float = 0.0
    inflation_impact: float = 0.0
    unemployment_impact: float = 0.0
    wealth_destruction: float = 0.0
    trade_disruption: float = 0.0
    confidence_impact: float = 0.0
    
    def apply_effects(self, economy, entity_id: str) -> Dict[str, float]:
        """Apply crisis effects to a specific entity"""
        if entity_id not in self.affected_entities:
            return {}
            
        severity_multiplier = self._get_severity_multiplier()
        effects = {}
        
        if self.crisis_type == CrisisType.RECESSION:
            effects = self._apply_recession_effects(severity_multiplier)
        elif self.crisis_type == CrisisType.HYPERINFLATION:
            effects = self._apply_hyperinflation_effects(severity_multiplier)
        elif self.crisis_type == CrisisType.MARKET_CRASH:
            effects = self._apply_market_crash_effects(severity_multiplier)
        elif self.crisis_type == CrisisType.DEBT_CRISIS:
            effects = self._apply_debt_crisis_effects(severity_multiplier)
        elif self.crisis_type == CrisisType.BANKING_CRISIS:
            effects = self._apply_banking_crisis_effects(severity_multiplier)
        elif self.crisis_type == CrisisType.SUPPLY_CHAIN_CRISIS:
            effects = self._apply_supply_chain_effects(severity_multiplier)
        
        self.turns_active += 1
        return effects
    
    def _get_severity_multiplier(self) -> float:
        """Get multiplier based on crisis severity"""
        multipliers = {
            CrisisSeverity.MILD: 0.3,
            CrisisSeverity.MODERATE: 0.6,
            CrisisSeverity.SEVERE: 1.0,
            CrisisSeverity.CATASTROPHIC: 1.8
        }
        return multipliers.get(self.severity, 1.0)
    
    def _apply_recession_effects(self, severity: float) -> Dict[str, float]:
        """Apply recession effects - reduced demand, unemployment"""
        return {
            'gdp_growth_reduction': -0.04 * severity,
            'unemployment_increase': 0.03 * severity,
            'confidence_reduction': -0.2 * severity,
            'investment_reduction': -0.15 * severity
        }
    
    def _apply_hyperinflation_effects(self, severity: float) -> Dict[str, float]:
        """Apply hyperinflation effects - price spiral, currency devaluation"""
        return {
            'inflation_increase': 0.5 * severity,
            'currency_devaluation': 0.3 * severity,
            'savings_destruction': 0.4 * severity,
            'trade_disruption': 0.2 * severity
        }
    
    def _apply_market_crash_effects(self, severity: float) -> Dict[str, float]:
        """Apply market crash effects - wealth destruction, credit crunch"""
        return {
            'wealth_destruction': 0.25 * severity,
            'investment_reduction': -0.3 * severity,
            'confidence_reduction': -0.4 * severity,
            'credit_availability': -0.35 * severity
        }
    
    def _apply_debt_crisis_effects(self, severity: float) -> Dict[str, float]:
        """Apply debt crisis effects - borrowing costs, austerity"""
        return {
            'borrowing_cost_increase': 0.08 * severity,
            'credit_availability': -0.5 * severity,
            'government_spending_reduction': -0.25 * severity,
            'currency_volatility': 0.15 * severity
        }
    
    def _apply_banking_crisis_effects(self, severity: float) -> Dict[str, float]:
        """Apply banking crisis effects - credit freeze, bank failures"""
        return {
            'credit_availability': -0.6 * severity,
            'wealth_destruction': 0.2 * severity,
            'confidence_reduction': -0.5 * severity,
            'liquidity_crisis': 0.4 * severity
        }
    
    def _apply_supply_chain_effects(self, severity: float) -> Dict[str, float]:
        """Apply supply chain crisis effects - production disruption, shortages"""
        return {
            'production_efficiency_reduction': -0.3 * severity,
            'resource_availability_reduction': -0.4 * severity,
            'inflation_increase': 0.2 * severity,
            'trade_disruption': 0.35 * severity
        }
    
    def update_resolution(self, policy_response: float, economic_health: float):
        """Update crisis resolution progress"""
        # Base resolution rate
        base_resolution = 0.05
        
        # Policy response effectiveness
        policy_effectiveness = policy_response * 0.1
        
        # Economic resilience
        resilience_effect = economic_health * 0.05
        
        # Time decay - crises naturally resolve over time
        time_effect = min(0.1, self.turns_active * 0.01)
        
        resolution_increment = base_resolution + policy_effectiveness + resilience_effect + time_effect
        self.resolution_progress += resolution_increment
        
        if self.resolution_progress >= 1.0:
            self.active = False

class CrisisManager:
    """Manages economic crises, their propagation, and resolution"""
    
    def __init__(self):
        self.active_crises: List[EconomicCrisis] = []
        self.crisis_history: List[EconomicCrisis] = []
        self.global_contagion_risk = 0.1
        self.crisis_probabilities: Dict[CrisisType, float] = self._initialize_probabilities()
        self.crisis_interdependencies: Dict[CrisisType, List[CrisisType]] = self._initialize_interdependencies()
    
    def _initialize_probabilities(self) -> Dict[CrisisType, float]:
        """Initialize base crisis probabilities"""
        return {
            CrisisType.RECESSION: 0.08,
            CrisisType.DEPRESSION: 0.02,
            CrisisType.HYPERINFLATION: 0.03,
            CrisisType.MARKET_CRASH: 0.05,
            CrisisType.DEBT_CRISIS: 0.04,
            CrisisType.BANKING_CRISIS: 0.03,
            CrisisType.CURRENCY_CRISIS: 0.04,
            CrisisType.COMMODITY_CRISIS: 0.06,
            CrisisType.TRADE_WAR: 0.05,
            CrisisType.SUPPLY_CHAIN_CRISIS: 0.07
        }
    
    def _initialize_interdependencies(self) -> Dict[CrisisType, List[CrisisType]]:
        """Define how crises can trigger other crises"""
        return {
            CrisisType.MARKET_CRASH: [CrisisType.RECESSION, CrisisType.BANKING_CRISIS],
            CrisisType.DEBT_CRISIS: [CrisisType.CURRENCY_CRISIS, CrisisType.RECESSION],
            CrisisType.BANKING_CRISIS: [CrisisType.RECESSION, CrisisType.MARKET_CRASH],
            CrisisType.TRADE_WAR: [CrisisType.RECESSION, CrisisType.SUPPLY_CHAIN_CRISIS],
            CrisisType.HYPERINFLATION: [CrisisType.CURRENCY_CRISIS, CrisisType.DEBT_CRISIS]
        }
    
    def assess_crisis_risk(self, entity: EconomicEntity, global_economy) -> Dict[CrisisType, float]:
        """Assess risk of different crisis types for an entity"""
        risks = {}
        
        # Base probabilities
        for crisis_type, base_prob in self.crisis_probabilities.items():
            risk = base_prob
            
            # Entity-specific risk factors
            if crisis_type == CrisisType.DEBT_CRISIS:
                debt_ratio = entity.debt / max(entity.gdp, 1)
                risk += debt_ratio * 0.5
                
            elif crisis_type == CrisisType.HYPERINFLATION:
                if entity.inflation > 0.1:  # 10% inflation
                    risk += entity.inflation * 2
                    
            elif crisis_type == CrisisType.MARKET_CRASH:
                # High asset prices increase crash risk
                if hasattr(entity, 'asset_bubbles'):
                    bubble_risk = sum(entity.asset_bubbles.values()) / len(entity.asset_bubbles) if entity.asset_bubbles else 0
                    risk += bubble_risk * 0.3
            
            elif crisis_type == CrisisType.TRADE_WAR:
                trade_openness = getattr(entity, 'trade_openness', 0.5)
                risk += trade_openness * 0.2
            
            risks[crisis_type] = min(0.95, risk)  # Cap at 95%
        
        return risks
    
    def trigger_crisis(self, crisis_type: CrisisType, origin_entity: str, 
                      severity: CrisisSeverity, duration: int) -> EconomicCrisis:
        """Trigger a new economic crisis"""
        crisis = EconomicCrisis(
            crisis_id=f"crisis_{len(self.crisis_history)}",
            crisis_type=crisis_type,
            severity=severity,
            duration=duration,
            origin_entity=origin_entity,
            affected_entities={origin_entity},
            contagion_risk=random.uniform(0.3, 0.8)
        )
        
        self.active_crises.append(crisis)
        self.crisis_history.append(crisis)
        
        # Log crisis
        print(f"🌪️ ECONOMIC CRISIS TRIGGERED: {crisis_type.value} in {origin_entity} "
              f"(Severity: {severity.value}, Duration: {duration} turns)")
        
        return crisis
    
    def simulate_crisis_propagation(self, global_economy):
        """Simulate crisis propagation to other entities"""
        for crisis in self.active_crises[:]:
            if not crisis.active:
                continue
                
            # Check for contagion to other entities
            for entity in global_economy.economic_entities:
                if entity.id not in crisis.affected_entities:
                    contagion_prob = crisis.contagion_risk * self.global_contagion_risk
                    
                    # Increase probability for closely connected entities
                    if hasattr(entity, 'trade_partners') and crisis.origin_entity in getattr(entity, 'trade_partners', []):
                        contagion_prob *= 2
                    
                    if random.random() < contagion_prob:
                        crisis.affected_entities.add(entity.id)
                        print(f"📈 Crisis contagion: {crisis.crisis_type.value} spread to {entity.name}")
    
    def apply_crisis_effects(self, entity: EconomicEntity, global_economy) -> Dict[str, float]:
        """Apply all active crisis effects to an entity"""
        total_effects = {}
        
        for crisis in self.active_crises:
            if crisis.active and entity.id in crisis.affected_entities:
                effects = crisis.apply_effects(global_economy, entity.id)
                
                # Combine effects
                for effect, value in effects.items():
                    total_effects[effect] = total_effects.get(effect, 0) + value
        
        return total_effects
    
    def implement_policy_responses(self, entity: EconomicEntity, crisis: EconomicCrisis) -> float:
        """Calculate effectiveness of policy responses to crisis"""
        policy_effectiveness = 0.0
        
        # Monetary policy responses
        if crisis.crisis_type in [CrisisType.RECESSION, CrisisType.MARKET_CRASH]:
            if hasattr(entity, 'interest_rate_policy'):
                policy_effectiveness += 0.3
        
        # Fiscal policy responses
        if crisis.crisis_type in [CrisisType.RECESSION, CrisisType.DEBT_CRISIS]:
            if hasattr(entity, 'fiscal_stimulus'):
                policy_effectiveness += 0.4
        
        # Banking system responses
        if crisis.crisis_type == CrisisType.BANKING_CRISIS:
            if hasattr(entity, 'banking_bailouts'):
                policy_effectiveness += 0.5
        
        # Trade policy responses
        if crisis.crisis_type == CrisisType.TRADE_WAR:
            if hasattr(entity, 'trade_negotiations'):
                policy_effectiveness += 0.3
        
        return min(1.0, policy_effectiveness)
    
    def simulate_crisis_turn(self, global_economy):
        """Simulate one turn of crisis dynamics"""
        # Update existing crises
        for crisis in self.active_crises[:]:
            if crisis.active:
                # Apply policy responses and update resolution
                for entity in global_economy.economic_entities:
                    if entity.id in crisis.affected_entities:
                        policy_response = self.implement_policy_responses(entity, crisis)
                        economic_health = self._calculate_economic_health(entity)
                        crisis.update_resolution(policy_response, economic_health)
            
            # Remove resolved crises
            if not crisis.active:
                self.active_crises.remove(crisis)
                print(f"✅ Crisis resolved: {crisis.crisis_type.value} in {crisis.origin_entity}")
        
        # Check for new crises
        self._check_new_crises(global_economy)
        
        # Simulate crisis propagation
        self.simulate_crisis_propagation(global_economy)
    
    def _check_new_crises(self, global_economy):
        """Check if new crises should be triggered"""
        for entity in global_economy.economic_entities:
            risks = self.assess_crisis_risk(entity, global_economy)
            
            for crisis_type, risk in risks.items():
                if random.random() < risk:
                    severity = self._determine_crisis_severity(entity, crisis_type)
                    duration = random.randint(5, 20)
                    self.trigger_crisis(crisis_type, entity.id, severity, duration)
    
    def _determine_crisis_severity(self, entity: EconomicEntity, crisis_type: CrisisType) -> CrisisSeverity:
        """Determine the severity of a potential crisis"""
        economic_health = self._calculate_economic_health(entity)
        
        if economic_health > 0.7:
            return CrisisSeverity.MILD
        elif economic_health > 0.4:
            return CrisisSeverity.MODERATE
        elif economic_health > 0.2:
            return CrisisSeverity.SEVERE
        else:
            return CrisisSeverity.CATASTROPHIC
    
    def _calculate_economic_health(self, entity: EconomicEntity) -> float:
        """Calculate overall economic health (0-1)"""
        health_indicators = []
        
        # GDP growth (positive is good)
        health_indicators.append(max(0, min(1, (entity.gdp_growth + 0.1) / 0.2)))
        
        # Low inflation is good
        health_indicators.append(max(0, min(1, 1.0 - (entity.inflation / 0.2))))
        
        # Low unemployment is good
        health_indicators.append(max(0, min(1, 1.0 - (entity.unemployment / 0.2))))
        
        # Manageable debt
        debt_ratio = entity.debt / max(entity.gdp, 1)
        health_indicators.append(max(0, min(1, 1.0 - (debt_ratio / 2.0))))
        
        return sum(health_indicators) / len(health_indicators)