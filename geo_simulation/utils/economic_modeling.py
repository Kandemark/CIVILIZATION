"""Economic modeling"""

# Module implementation

# utils/economic_modeling.py
# utils/economic_modeling.py
from typing import Dict, List, Optional, Tuple, Set, Any
from dataclasses import dataclass, field
import numpy as np
import logging
from enum import Enum
import random
# from scipy.optimize import minimize
import uuid

logger = logging.getLogger(__name__)

class EconomicEventType(Enum):
    RECESSION = "recession"
    BOOM = "boom"
    FINANCIAL_CRISIS = "financial_crisis"
    CURRENCY_CRISIS = "currency_crisis"
    TRADE_WAR = "trade_war"
    NATURAL_DISASTER = "natural_disaster"
    TECHNOLOGICAL_BREAKTHROUGH = "technological_breakthrough"
    POLITICAL_INSTABILITY = "political_instability"
    RESOURCE_DISCOVERY = "resource_discovery"
    PANDEMIC = "pandemic"

class CurrencyRegime(Enum):
    FLOATING = "floating"
    PEGGED = "pegged"
    CURRENCY_BOARD = "currency_board"
    DOLLARIZED = "dollarized"
    MONETARY_UNION = "monetary_union"

class EconomicPolicy:
    """Economic policy settings for a nation."""
    def __init__(self):
        self.tax_rate_corporate = 0.25
        self.tax_rate_income = 0.30
        self.tax_rate_vat = 0.20
        self.government_spending_ratio = 0.35
        self.interest_rate = 0.04
        self.reserve_requirement = 0.10
        self.trade_tariff = 0.05
        self.export_subsidies = 0.0
        self.infrastructure_investment = 0.15
        self.education_spending = 0.10
        self.healthcare_spending = 0.08
        self.research_spending = 0.03

@dataclass
class EconomicEvent:
    """Random economic event that affects nations."""
    event_id: str
    event_type: EconomicEventType
    severity: float  # 0-1 scale
    duration: int  # in months
    affected_nations: List[str]
    global_impact: bool = False
    contagion_risk: float = 0.0
    description: str = ""
    effects: Dict[str, float] = field(default_factory=dict)

@dataclass
class Currency:
    """Currency representation with exchange rate regime."""
    code: str
    name: str
    base_value: float
    regime: CurrencyRegime
    pegged_to: Optional[str] = None
    volatility: float = 0.05
    central_bank_credibility: float = 0.8
    inflation_target: float = 0.02

@dataclass
class EconomicSector:
    """Detailed economic sector model."""
    name: str
    base_productivity: float
    employment_share: float
    capital_intensity: float
    growth_potential: float
    export_intensity: float
    import_dependency: float
    technology_level: float = 1.0
    skill_intensity: float = 0.5
    wage_level: float = 1.0
    profit_margin: float = 0.15
    innovation_rate: float = 0.02
    input_requirements: Dict[str, float] = field(default_factory=dict)
    output_goods: List[str] = field(default_factory=list)

@dataclass
class Market:
    """Dynamic market for goods and services."""
    good_type: str
    base_price: float
    price_elasticity: float
    supply: float
    demand: float
    price_volatility: float = 0.1
    tradeable: bool = True
    world_price: Optional[float] = None
    tariff_rate: float = 0.0
    domestic_production: float = 0.0
    imports: float = 0.0
    exports: float = 0.0

@dataclass
class InternationalLinkage:
    """Economic linkages between countries."""
    trade_share: float
    investment_flows: float
    debt_exposure: float
    technology_transfer: float
    migration_flows: float = 0.0
    diplomatic_relations: float = 0.5  # -1 to 1 scale

@dataclass
class EconomicState:
    """Complete economic state of a nation."""
    # Basic identifiers
    nation_id: str
    name: str
    
    # Core economic indicators
    gdp: float
    gdp_growth_rate: float
    gdp_per_capita: float
    inflation_rate: float
    unemployment_rate: float
    poverty_rate: float
    inequality_gini: float
    
    # Fiscal indicators
    government_debt: float
    budget_deficit: float
    tax_revenue: float
    government_spending: float
    
    # Monetary indicators
    money_supply: float
    interest_rate: float
    foreign_reserves: float
    
    # External sector
    current_account_balance: float
    trade_balance: float
    external_debt: float
    exchange_rate: float
    
    # Structural components
    population: int
    labor_force: int
    capital_stock: float
    technology_level: float
    human_capital_index: float
    infrastructure_quality: float
    
    # Dynamic components
    sectors: Dict[str, EconomicSector]
    markets: Dict[str, Market]
    currency: Currency
    international_links: Dict[str, InternationalLinkage]
    economic_policy: EconomicPolicy
    
    # Historical data
    gdp_history: List[float] = field(default_factory=list)
    inflation_history: List[float] = field(default_factory=list)
    unemployment_history: List[float] = field(default_factory=list)

class EconomicModel:
    """Comprehensive economic modeling system for multiple nations."""
    
    def __init__(self):
        self.nations: Dict[str, EconomicState] = {}
        self.economic_history: List[Dict] = []
        self.exchange_rates: Dict[Tuple[str, str], float] = {}
        self.global_events: List[EconomicEvent] = []
        self.time_period: int = 0
        self.world_gdp: float = 0.0
        self.global_inflation: float = 0.02
        self.commodity_prices: Dict[str, float] = {}
        
        # Configuration
        self.base_parameters = {
            'population_growth_rate': 0.01,
            'technological_progress': 0.02,
            'depreciation_rate': 0.05,
            'natural_unemployment': 0.04,
            'risk_free_rate': 0.02
        }
        
        logger.info("Economic Model Initialized")

    def add_nation(self, nation_state: EconomicState):
        """Add a nation to the economic model."""
        self.nations[nation_state.nation_id] = nation_state
        self._initialize_nation_markets(nation_state)
        logger.info(f"Added nation: {nation_state.name}")

    def _initialize_nation_markets(self, nation: EconomicState):
        """Initialize markets for a new nation."""
        basic_goods = {
            'food': Market('food', 1.0, -0.3, 1000, 1000, 0.15),
            'energy': Market('energy', 2.0, -0.2, 500, 500, 0.25),
            'manufactured_goods': Market('manufactured_goods', 1.5, -0.4, 800, 800, 0.12),
            'services': Market('services', 2.5, -0.5, 1200, 1200, 0.08),
            'housing': Market('housing', 3.0, -0.1, 600, 600, 0.05)
        }
        nation.markets = basic_goods

    def simulate_time_step(self):
        """Simulate one time step (e.g., one month) for all nations."""
        self.time_period += 1
        logger.info(f"Simulating time period {self.time_period}")
        
        # Generate random events
        self._generate_random_events()
        
        # Update global conditions
        self._update_global_conditions()
        
        # Process each nation
        for nation_id, nation in self.nations.items():
            self._simulate_nation_economy(nation)
        
        # Update international interactions
        self._update_international_relations()
        
        # Update exchange rates
        self._update_exchange_rates()
        
        # Record history
        self._record_economic_history()

    def _generate_random_events(self):
        """Generate random economic events."""
        event_probability = 0.15  # 15% chance of event per time period
        
        if random.random() < event_probability:
            event_type = random.choice(list(EconomicEventType))
            severity = random.uniform(0.1, 0.8)
            duration = random.randint(3, 24)  # 3-24 months
            
            # Select affected nations
            affected_nations = random.sample(
                list(self.nations.keys()), 
                k=min(3, len(self.nations))
            ) if len(self.nations) > 0 else []
            
            event = EconomicEvent(
                event_id=str(uuid.uuid4()),
                event_type=event_type,
                severity=severity,
                duration=duration,
                affected_nations=affected_nations,
                global_impact=severity > 0.5,
                contagion_risk=severity * 0.7,
                description=f"Global {event_type.value.capitalize()} Event"
            )
            
            self.global_events.append(event)
            self._apply_economic_event(event)
            logger.info(f"Economic event generated: {event.event_type}")

    def _apply_economic_event(self, event: EconomicEvent):
        """Apply economic event effects to nations."""
        for nation_id in event.affected_nations:
            if nation_id in self.nations:
                nation = self.nations[nation_id]
                effects = self._calculate_event_effects(event, nation)
                self._modify_nation_economy(nation, effects)

    def _calculate_event_effects(self, event: EconomicEvent, nation: EconomicState) -> Dict[str, float]:
        """Calculate specific effects of an economic event on a nation."""
        base_effect = event.severity
        
        if event.event_type == EconomicEventType.RECESSION:
            return {
                'gdp_growth_rate': -base_effect * 0.08,
                'unemployment_rate': base_effect * 0.05,
                'inflation_rate': -base_effect * 0.02
            }
        elif event.event_type == EconomicEventType.BOOM:
            return {
                'gdp_growth_rate': base_effect * 0.06,
                'unemployment_rate': -base_effect * 0.03,
                'inflation_rate': base_effect * 0.03
            }
        elif event.event_type == EconomicEventType.FINANCIAL_CRISIS:
            return {
                'gdp_growth_rate': -base_effect * 0.10,
                'unemployment_rate': base_effect * 0.08,
                'interest_rate': base_effect * 0.05,
                'government_debt': base_effect * 0.15
            }
        # Add more event types...
        
        return {}

    def _modify_nation_economy(self, nation: EconomicState, effects: Dict[str, float]):
        """Modify nation's economy based on effects."""
        for attribute, change in effects.items():
            if hasattr(nation, attribute):
                current_value = getattr(nation, attribute)
                setattr(nation, attribute, current_value * (1 + change))

    def _simulate_nation_economy(self, nation: EconomicState):
        """Simulate economic development for a single nation."""
        try:
            # Update population and labor force
            self._update_demographics(nation)
            
            # Calculate sectoral outputs
            sector_outputs = self._calculate_sectoral_outputs(nation)
            
            # Update GDP
            new_gdp = self._calculate_gdp(nation, sector_outputs)
            nation.gdp_history.append(new_gdp)
            nation.gdp = new_gdp
            
            # Update inflation
            nation.inflation_rate = self._calculate_inflation(nation)
            nation.inflation_history.append(nation.inflation_rate)
            
            # Update employment
            nation.unemployment_rate = self._calculate_unemployment(nation)
            nation.unemployment_history.append(nation.unemployment_rate)
            
            # Update fiscal position
            self._update_fiscal_position(nation)
            
            # Update monetary conditions
            self._update_monetary_conditions(nation)
            
            # Update external sector
            self._update_external_sector(nation)
            
            # Update markets
            self._update_markets(nation)
            
            # Technological progress
            nation.technology_level *= (1 + self.base_parameters['technological_progress'])
            
            logger.debug(f"Updated economy for {nation.name}: GDP={nation.gdp:.2f}")
            
        except Exception as e:
            logger.error(f"Error simulating economy for {nation.name}: {str(e)}")

    def _update_demographics(self, nation: EconomicState):
        """Update population and labor force dynamics."""
        # Natural population growth
        growth_rate = self.base_parameters['population_growth_rate']
        nation.population = int(nation.population * (1 + growth_rate + random.uniform(-0.005, 0.005)))
        
        # Labor force participation
        participation_rate = 0.65 + (nation.human_capital_index * 0.1)
        nation.labor_force = int(nation.population * participation_rate)

    def _calculate_sectoral_outputs(self, nation: EconomicState) -> Dict[str, float]:
        """Calculate output for each economic sector."""
        sector_outputs = {}
        total_output = 0.0
        
        for sector_name, sector in nation.sectors.items():
            # Enhanced production function
            labor_input = nation.labor_force * sector.employment_share
            capital_input = nation.capital_stock * sector.capital_intensity
            
            # Technology-enhanced production
            technology_factor = nation.technology_level * sector.technology_level
            human_capital_factor = nation.human_capital_index ** 0.3
            
            output = (sector.base_productivity * 
                     (labor_input ** 0.6) * 
                     (capital_input ** 0.3) * 
                     (technology_factor ** 0.1) *
                     human_capital_factor)
            
            # Apply random shocks and policy effects
            policy_effect = 1.0 + (nation.economic_policy.research_spending * 0.5)
            random_shock = random.uniform(0.95, 1.05)
            
            final_output = output * policy_effect * random_shock
            sector_outputs[sector_name] = final_output
            total_output += final_output
        
        return sector_outputs

    def _calculate_gdp(self, nation: EconomicState, sector_outputs: Dict[str, float]) -> float:
        """Calculate GDP from sector outputs with international adjustments."""
        nominal_gdp = sum(sector_outputs.values())
        
        # International competitiveness adjustment
        competitiveness = self._calculate_competitiveness(nation)
        real_gdp = nominal_gdp * competitiveness
        
        # Calculate growth rate
        if nation.gdp_history:
            previous_gdp = nation.gdp_history[-1] if nation.gdp_history else nation.gdp
            nation.gdp_growth_rate = (real_gdp - previous_gdp) / previous_gdp
        else:
            nation.gdp_growth_rate = 0.03  # Initial growth rate
        
        return real_gdp

    def _calculate_competitiveness(self, nation: EconomicState) -> float:
        """Calculate international competitiveness factor."""
        base_competitiveness = 1.0
        
        # Exchange rate effect
        exchange_rate_effect = 1.0 / (1.0 + abs(nation.exchange_rate - 1.0) * 0.2)
        
        # Technology effect
        technology_effect = nation.technology_level ** 0.2
        
        # Infrastructure effect
        infrastructure_effect = nation.infrastructure_quality ** 0.15
        
        # Human capital effect
        human_capital_effect = nation.human_capital_index ** 0.1
        
        return base_competitiveness * exchange_rate_effect * technology_effect * infrastructure_effect * human_capital_effect

    def _calculate_inflation(self, nation: EconomicState) -> float:
        """Calculate inflation rate using multiple factors."""
        # Monetary inflation
        money_growth = (nation.money_supply - (nation.gdp * 0.3)) / (nation.gdp * 0.3)
        monetary_inflation = money_growth * 0.7
        
        # Demand-pull inflation
        output_gap = (nation.gdp - self._calculate_potential_gdp(nation)) / self._calculate_potential_gdp(nation)
        demand_inflation = max(0, output_gap) * 0.5
        
        # Cost-push inflation (import prices)
        import_inflation = 0.0
        for linkage in nation.international_links.values():
            import_inflation += linkage.trade_share * 0.1
        
        # Expected inflation (adaptive expectations)
        expected_inflation = nation.inflation_history[-1] if nation.inflation_history else 0.02
        
        total_inflation = (monetary_inflation + demand_inflation + 
                          import_inflation + expected_inflation * 0.3)
        
        # Add random shock
        inflation_shock = random.uniform(-0.01, 0.01)
        
        return max(-0.02, total_inflation + inflation_shock)  # Prevent deflation below -2%

    def _calculate_potential_gdp(self, nation: EconomicState) -> float:
        """Calculate potential GDP."""
        return (nation.labor_force ** 0.6) * (nation.capital_stock ** 0.3) * (nation.technology_level ** 0.1)

    def _calculate_unemployment(self, nation: EconomicState) -> float:
        """Calculate unemployment rate."""
        natural_unemployment = self.base_parameters['natural_unemployment']
        
        # Cyclical unemployment based on output gap
        output_gap = (nation.gdp - self._calculate_potential_gdp(nation)) / self._calculate_potential_gdp(nation)
        cyclical_unemployment = max(0, -output_gap * 0.5)
        
        # Structural unemployment based on skills mismatch
        structural_unemployment = (1 - nation.human_capital_index) * 0.03
        
        total_unemployment = natural_unemployment + cyclical_unemployment + structural_unemployment
        
        return min(0.3, total_unemployment)  # Cap at 30%

    def _update_fiscal_position(self, nation: EconomicState):
        """Update government fiscal position."""
        # Calculate tax revenue
        tax_revenue = (nation.gdp * nation.economic_policy.tax_rate_corporate * 0.4 +
                      nation.gdp * nation.economic_policy.tax_rate_income * 0.3 +
                      nation.gdp * nation.economic_policy.tax_rate_vat * 0.3)
        
        # Calculate government spending
        government_spending = nation.gdp * nation.economic_policy.government_spending_ratio
        
        # Update budget deficit
        nation.budget_deficit = government_spending - tax_revenue
        
        # Update government debt
        nation.government_debt += nation.budget_deficit
        
        nation.tax_revenue = tax_revenue
        nation.government_spending = government_spending

    def _update_monetary_conditions(self, nation: EconomicState):
        """Update monetary conditions."""
        # Money supply growth based on GDP growth and inflation target
        target_money_growth = nation.gdp_growth_rate + nation.currency.inflation_target
        current_money_growth = random.uniform(target_money_growth - 0.02, target_money_growth + 0.02)
        
        nation.money_supply *= (1 + current_money_growth)
        
        # Interest rate adjustment based on inflation and growth
        inflation_gap = nation.inflation_rate - nation.currency.inflation_target
        output_gap = (nation.gdp - self._calculate_potential_gdp(nation)) / self._calculate_potential_gdp(nation)
        
        nation.interest_rate = (self.base_parameters['risk_free_rate'] + 
                               inflation_gap * 0.5 + 
                               output_gap * 0.3)

    def _update_external_sector(self, nation: EconomicState):
        """Update external sector indicators."""
        # Calculate trade balance
        exports = sum(sector.export_intensity * output for sector, output in 
                     zip(nation.sectors.values(), self._calculate_sectoral_outputs(nation).values()))
        imports = nation.gdp * 0.25  # Simplified import calculation
        
        nation.trade_balance = exports - imports
        nation.current_account_balance = nation.trade_balance
        
        # Update foreign reserves
        reserve_change = nation.current_account_balance * 0.1
        nation.foreign_reserves += reserve_change

    def _update_markets(self, nation: EconomicState):
        """Update market prices and quantities."""
        for market in nation.markets.values():
            # Basic supply-demand price adjustment
            imbalance = (market.demand - market.supply) / market.supply if market.supply > 0 else 0
            price_change = imbalance * market.price_elasticity
            
            # Add inflation effect
            price_change += nation.inflation_rate
            
            # Add random volatility
            volatility = random.normalvariate(0, market.price_volatility)
            
            new_price = market.base_price * (1 + price_change + volatility)
            market.base_price = max(0.01, new_price)
            
            # Adjust supply and demand
            market.supply *= (1 + nation.gdp_growth_rate * 0.5)
            market.demand *= (1 + nation.gdp_growth_rate * 0.7 + random.uniform(-0.05, 0.05))

    def _update_international_relations(self):
        """Update international economic relations."""
        for nation_id, nation in self.nations.items():
            for partner_id, linkage in nation.international_links.items():
                if partner_id in self.nations:
                    partner = self.nations[partner_id]
                    
                    # Update trade based on relative competitiveness
                    relative_growth = nation.gdp_growth_rate - partner.gdp_growth_rate
                    linkage.trade_share *= (1 + relative_growth * 0.1)
                    
                    # Update investment flows based on interest rate differential
                    interest_diff = nation.interest_rate - partner.interest_rate
                    linkage.investment_flows *= (1 - interest_diff * 0.2)
                    
                    # Diplomatic relations affect economic links
                    economic_cooperation = (linkage.diplomatic_relations + 1) / 2
                    linkage.trade_share *= economic_cooperation
                    linkage.investment_flows *= economic_cooperation

    def _update_exchange_rates(self):
        """Update exchange rates between currencies."""
        currencies = {nation_id: nation.currency for nation_id, nation in self.nations.items()}
        
        for (curr_a, curr_b), current_rate in self.exchange_rates.items():
            if curr_a == curr_b:
                continue
                
            if curr_a in currencies and curr_b in currencies:
                currency_a = currencies[curr_a]
                currency_b = currencies[curr_b]
                
                # Basic purchasing power parity adjustment
                nation_a = self.nations[curr_a]
                nation_b = self.nations[curr_b]
                
                inflation_diff = nation_a.inflation_rate - nation_b.inflation_rate
                growth_diff = nation_a.gdp_growth_rate - nation_b.gdp_growth_rate
                interest_diff = nation_a.interest_rate - nation_b.interest_rate
                
                new_rate = current_rate * (1 - inflation_diff * 0.3 + growth_diff * 0.2 + interest_diff * 0.1)
                
                # Add random noise
                noise = random.normalvariate(0, (currency_a.volatility + currency_b.volatility) / 2)
                new_rate *= (1 + noise)
                
                self.exchange_rates[(curr_a, curr_b)] = max(0.001, new_rate)

    def _update_global_conditions(self):
        """Update global economic conditions."""
        # Calculate world GDP
        self.world_gdp = sum(nation.gdp for nation in self.nations.values())
        
        # Global inflation as weighted average
        if self.nations:
            total_gdp = sum(nation.gdp for nation in self.nations.values())
            weighted_inflation = sum(nation.inflation_rate * nation.gdp for nation in self.nations.values()) / total_gdp
            self.global_inflation = weighted_inflation

    def _record_economic_history(self):
        """Record current economic state for all nations."""
        period_data = {
            'time_period': self.time_period,
            'nations': {}
        }
        
        for nation_id, nation in self.nations.items():
            period_data['nations'][nation_id] = {
                'gdp': nation.gdp,
                'inflation': nation.inflation_rate,
                'unemployment': nation.unemployment_rate,
                'growth_rate': nation.gdp_growth_rate
            }
        
        self.economic_history.append(period_data)

    def apply_policy_changes(self, nation_id: str, policy_changes: Dict[str, float]):
        """Apply policy changes to a nation."""
        if nation_id in self.nations:
            nation = self.nations[nation_id]
            policy = nation.economic_policy
            
            for policy_field, new_value in policy_changes.items():
                if hasattr(policy, policy_field):
                    setattr(policy, policy_field, new_value)
            
            logger.info(f"Applied policy changes to {nation.name}: {policy_changes}")

    def get_economic_indicators(self, nation_id: str) -> Dict[str, Any]:
        """Get comprehensive economic indicators for a nation."""
        if nation_id not in self.nations:
            return {}
        
        nation = self.nations[nation_id]
        
        return {
            'gdp': nation.gdp,
            'gdp_growth': nation.gdp_growth_rate,
            'gdp_per_capita': nation.gdp_per_capita,
            'inflation': nation.inflation_rate,
            'unemployment': nation.unemployment_rate,
            'budget_deficit': nation.budget_deficit,
            'government_debt': nation.government_debt,
            'trade_balance': nation.trade_balance,
            'current_account': nation.current_account_balance,
            'foreign_reserves': nation.foreign_reserves,
            'external_debt': nation.external_debt,
            'exchange_rate': nation.exchange_rate,
            'interest_rate': nation.interest_rate,
            'money_supply': nation.money_supply,
            'technology_level': nation.technology_level,
            'human_capital': nation.human_capital_index,
            'infrastructure_quality': nation.infrastructure_quality
        }

    def predict_economic_outlook(self, nation_id: str, periods: int = 12) -> Dict[str, List[float]]:
        """Predict economic outlook for the next periods."""
        if nation_id not in self.nations:
            return {}
        
        nation = self.nations[nation_id]
        predictions = {
            'gdp': [],
            'inflation': [],
            'unemployment': [],
            'growth': []
        }
        
        # Simple trend-based prediction (in real implementation, use more sophisticated models)
        current_gdp = nation.gdp
        current_inflation = nation.inflation_rate
        current_unemployment = nation.unemployment_rate
        current_growth = nation.gdp_growth_rate
        
        for i in range(periods):
            # Add some noise and trend persistence
            growth_pred = current_growth * 0.8 + random.uniform(-0.01, 0.01)
            inflation_pred = current_inflation * 0.7 + random.uniform(-0.005, 0.005)
            unemployment_pred = current_unemployment * 0.9 + random.uniform(-0.005, 0.005)
            
            predictions['gdp'].append(current_gdp * (1 + growth_pred))
            predictions['inflation'].append(inflation_pred)
            predictions['unemployment'].append(unemployment_pred)
            predictions['growth'].append(growth_pred)
            
            current_gdp = predictions['gdp'][-1]
            current_growth = growth_pred
            current_inflation = inflation_pred
            current_unemployment = unemployment_pred
        
        return predictions

# Example usage and helper functions
# def create_sample_nation(nation_id: str, name: str, development_level: str = "developed") -> EconomicState:
#     """Create a sample nation for testing."""
    
#     # Base parameters based on development level
#     if development_level == "developed":
#         gdp = 2000000000000  # $2 trillion
#         tech_level = 1.2
#         human_capital = 0.8
#         infrastructure = 0.85
#     elif development_level == "developing":
#         gdp = 500000000000   # $500 billion
#         tech_level = 0.8
#         human_capital = 0.6
#         infrastructure = 0.6
#     else:  # undeveloped
#         gdp = 100000000000   # $100 billion
#         tech_level = 0.5
#         human_capital = 0.4
#         infrastructure = 0.4
    
#     currency = Currency(
#         code=name[:3].upper(),
#         name=f"{name} Dollar",
#         base_value=1.0,
#         regime=CurrencyRegime.FLOATING,
#         volatility=0.03,
#         inflation_target=0.02
#     )
    
#     sectors = {
#         "agriculture": EconomicSector(
#             name="Agriculture",
#             base_productivity=0.8,
#             employment_share=0.1 if development_level == "developed" else 0.3,
#             capital_intensity=0.3,
#             growth_potential=0.02,
#             export_intensity=0.2,
#             import_dependency=0.1,
#             technology_level=tech_level * 0.8
#         ),
#         "manufacturing": EconomicSector(
#             name="Manufacturing",
#             base_productivity=1.2,
#             employment_share=0.2,
#             capital_intensity=0.6,
#             growth_potential=0.04,
#             export_intensity=0.5,
#             import_dependency=0.3,
#             technology_level=tech_level
#         ),
#         "services": EconomicSector(
#             name="Services",
#             base_productivity=1.5,
#             employment_share=0.7 if development_level == "developed" else 0.5,
#             capital_intensity=0.4,
#             growth_potential=0.05,
#             export_intensity=0.1,
#             import_dependency=0.05,
#             technology_level=tech_level * 1.1
#         )
#     }
    
#     return EconomicState(
#         nation_id=nation_id,
#         name=name,
#         gdp=gdp,
#         gdp_growth_rate=0.03,
#         gdp_per_capita=gdp / 50000000,  # Assuming 50 million population
#         inflation_rate=0.02,
#         unemployment_rate=0.05,
#         poverty_rate=0.1 if development_level == "developed" else 0.3,
#         inequality_gini=0.3 if development_level == "developed" else 0.5,
#         government_debt=gdp * 0.6,
#         budget_deficit=gdp * 0.03,
#         tax_revenue=gdp * 0.3,
#         government_spending=gdp * 0.33,
#         money_supply=gdp * 0.8,
#         interest_rate=0.04,
#         foreign_reserves=gdp * 0.1,
#         current_account_balance=gdp * 0.02,
#         trade_balance=gdp * 0.01,
#         external_debt=gdp * 0.4,
#         exchange_rate=1.0,
#         population=50000000,
#         labor_force=30000000,
#         capital_stock=gdp * 2.5,
#         technology_level=tech_level,
#         human_capital_index=human_capital,
#         infrastructure_quality=infrastructure,
#         sectors=sectors,
#         markets={},
#         currency=currency,
#         international_links={},
#         economic_policy=EconomicPolicy()
#     )

# Initialize the global economic model
# global_economic_model = EconomicModel()

# def initialize_world_economy():
#     """Initialize the world with sample nations."""
#     nations = [
#         create_sample_nation("usa", "United States", "developed"),
#         create_sample_nation("china", "China", "developing"),
#         create_sample_nation("germany", "Germany", "developed"),
#         create_sample_nation("india", "India", "developing"),
#         create_sample_nation("brazil", "Brazil", "developing"),
#         create_sample_nation("nigeria", "Nigeria", "undeveloped")
#     ]
    
#     for nation in nations:
#         global_economic_model.add_nation(nation)
    
#     logger.info("World economy initialized with sample nations")

# Initialize when module is imported
# initialize_world_economy()