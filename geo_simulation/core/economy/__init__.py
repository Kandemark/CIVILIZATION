# Package initialization

from .base_economy import *
from .resources import *
from .production import *
from .production_chains import *
from .market_dynamics import *
from .trade import *
from .trade_networks import *
from .economic_crises import *
from .wealth_distribution import *
from .infrastructure import *
from .urbanization import *
from .distribution import *

class CivilizationEconomy:
    """Complete economic simulation engine for Civilization game"""
    
    def __init__(self, game_world):
        self.game_world = game_world
        self.turn_count = 0
        
        # Core economic systems
        self.resource_manager = ResourceManager()
        self.production_manager = ProductionManager()
        self.production_chain_manager = ProductionChainManager()
        self.market_dynamics = MarketDynamics()
        self.trade_manager = TradeManager()
        self.trade_network = TradeNetwork()
        self.crisis_manager = CrisisManager()
        # self.distribution_manager = DistributionManager()
        
        # Entity management
        self.nations: Dict[str, NationEconomy] = {}
        self.regions: Dict[str, RegionalEconomy] = {}
        self.corporations: Dict[str, Corporation] = {}
        
        # Global economic state
        self.global_gdp = 0.0
        self.global_trade_volume = 0.0
        self.global_inflation = 0.02
        self.economic_cycles: List[EconomicState] = [EconomicState.STABILITY]
        
        # Historical data
        self.economic_history: Dict[int, Dict] = {}
        
    def add_nation(self, nation_id: str, name: str, development_level: DevelopmentLevel):
        """Add a nation to the economic simulation"""
        nation = NationEconomy(nation_id, name, development_level)
        self.nations[nation_id] = nation
        return nation
    
    def add_region(self, region_id: str, name: str, nation_id: str, development_level: DevelopmentLevel):
        """Add a region to a nation"""
        if nation_id in self.nations:
            region = RegionalEconomy(region_id, name, development_level)
            self.nations[nation_id].add_region(region)
            self.regions[region_id] = region
            return region
        return None
    
    def simulate_economic_turn(self) -> Dict[str, Any]:
        """Simulate one complete economic turn for the entire world"""
        print(f"🏛️ ECONOMIC TURN {self.turn_count}")
        turn_results = {
            'global_metrics': {},
            'nation_performance': {},
            'crises_occurred': [],
            'major_events': []
        }
        
        # 1. Update global economic conditions
        global_conditions = self._update_global_economy()
        turn_results['global_metrics'] = global_conditions
        
        # 2. Simulate each nation's economy
        for nation_id, nation in self.nations.items():
            nation_results = nation.simulate_nation_turn(self)
            turn_results['nation_performance'][nation_id] = nation_results
            
            # Track major economic events
            if nation_results.get('major_event'):
                turn_results['major_events'].append(nation_results['major_event'])
        
        # 3. Simulate international trade
        trade_results = self._simulate_international_trade()
        turn_results['trade_flows'] = trade_results
        
        # 4. Handle economic crises
        crisis_results = self.crisis_manager.simulate_crisis_turn(self)
        turn_results['crises_occurred'] = crisis_results
        
        # 5. Update global markets
        market_results = self._update_global_markets()
        turn_results['market_conditions'] = market_results
        
        # 6. Record history
        self.economic_history[self.turn_count] = turn_results
        
        self.turn_count += 1
        return turn_results
    
    def _update_global_economy(self) -> Dict[str, float]:
        """Update global economic indicators"""
        total_gdp = 0.0
        total_population = 0
        total_trade = 0.0
        
        for nation in self.nations.values():
            total_gdp += nation.calculate_gdp()
            total_population += nation.total_population
            total_trade += nation.export_volume + nation.import_volume
        
        self.global_gdp = total_gdp
        self.global_trade_volume = total_trade
        
        # Calculate global inflation (weighted average)
        weighted_inflation = 0.0
        total_weight = 0.0
        for nation in self.nations.values():
            weight = nation.calculate_gdp() / total_gdp if total_gdp > 0 else 1.0 / len(self.nations)
            weighted_inflation += nation.inflation * weight
            total_weight += weight
        
        self.global_inflation = weighted_inflation / total_weight if total_weight > 0 else 0.02
        
        return {
            'global_gdp': total_gdp,
            'global_gdp_growth': self._calculate_global_growth(),
            'global_inflation': self.global_inflation,
            'global_trade_volume': total_trade,
            'global_unemployment': self._calculate_global_unemployment(),
            'economic_cycle': self._determine_global_cycle()
        }
    
    def _calculate_global_growth(self) -> float:
        """Calculate global GDP growth rate"""
        if self.turn_count < 2:
            return 0.03  # Default growth
        
        current_gdp = self.global_gdp
        previous_gdp = self.economic_history.get(self.turn_count - 1, {}).get('global_metrics', {}).get('global_gdp', current_gdp)
        
        if previous_gdp > 0:
            return (current_gdp - previous_gdp) / previous_gdp
        return 0.03
    
    def _calculate_global_unemployment(self) -> float:
        """Calculate global unemployment rate"""
        if not self.nations:
            return 0.05
        
        total_workforce = 0
        total_unemployed = 0
        
        for nation in self.nations.values():
            workforce = nation.total_population * 0.6  # Assume 60% workforce
            unemployed = workforce * nation.unemployment_rate
            total_workforce += workforce
            total_unemployed += unemployed
        
        return total_unemployed / total_workforce if total_workforce > 0 else 0.05
    
    def _determine_global_cycle(self) -> EconomicState:
        """Determine current global economic cycle state"""
        growth = self._calculate_global_growth()
        inflation = self.global_inflation
        
        if growth > 0.05 and inflation < 0.04:
            return EconomicState.BOOM
        elif growth > 0.02:
            return EconomicState.EXPANSION
        elif growth > -0.02:
            return EconomicState.STABILITY
        elif growth > -0.05:
            return EconomicState.RECESSION
        else:
            return EconomicState.DEPRESSION
    
    def _simulate_international_trade(self) -> Dict[str, Any]:
        """Simulate trade between nations"""
        trade_flows = {}
        trade_disputes = []
        
        # Generate trade demands between nations
        trade_demands = {}
        for nation_a in self.nations.values():
            for nation_b in self.nations.values():
                if nation_a.id != nation_b.id:
                    # Nations trade based on comparative advantage
                    trade_volume = self._calculate_trade_potential(nation_a, nation_b)
                    if trade_volume:
                        key = (nation_a.id, nation_b.id)
                        trade_demands[key] = trade_volume
        
        # Execute trade through trade network
        trade_results = self.trade_network.simulate_network_turn(trade_demands)
        
        # Apply trade policies and tariffs
        for (importer_id, exporter_id), resources in trade_demands.items():
            importer = self.nations.get(importer_id)
            exporter = self.nations.get(exporter_id)
            
            if importer and exporter:
                # Check trade restrictions
                can_trade = self.trade_manager.check_trade_restrictions(
                    exporter.id, importer.id, list(resources.keys())[0], list(resources.values())[0]
                )
                
                if not can_trade:
                    trade_disputes.append(f"Trade blocked between {exporter.name} and {importer.name}")
                    continue
                
                # Calculate tariff costs
                total_tariff = 0.0
                for resource, amount in resources.items():
                    base_value = self.resource_manager.resources[resource].current_value
                    tariff_cost = self.trade_manager.calculate_tariff_cost(
                        importer.id, exporter.id, resource, amount, base_value
                    )
                    total_tariff += tariff_cost
                
                # Record successful trade
                if importer.id not in trade_flows:
                    trade_flows[importer.id] = {'imports': {}, 'exports': {}, 'tariff_revenue': 0}
                if exporter.id not in trade_flows:
                    trade_flows[exporter.id] = {'imports': {}, 'exports': {}, 'tariff_revenue': 0}
                
                for resource, amount in resources.items():
                    trade_flows[importer.id]['imports'][resource] = trade_flows[importer.id]['imports'].get(resource, 0) + amount
                    trade_flows[exporter.id]['exports'][resource] = trade_flows[exporter.id]['exports'].get(resource, 0) + amount
                
                trade_flows[importer.id]['tariff_revenue'] += total_tariff
        
        return {
            'trade_flows': trade_flows,
            'disputes': trade_disputes,
            'total_volume': sum(sum(flow['exports'].values()) for flow in trade_flows.values())
        }
    
    def _calculate_trade_potential(self, nation_a, nation_b) -> Dict[ResourceType, float]:
        """Calculate potential trade between two nations based on comparative advantage"""
        trade_volume = {}
        
        # Nation A's export candidates (resources it produces efficiently)
        a_efficient_resources = nation_a.get_competitive_resources()
        # Nation B's import needs (resources it lacks)
        b_import_needs = nation_b.get_import_needs()
        
        # Find matching resources
        for resource in a_efficient_resources:
            if resource in b_import_needs:
                # Base trade volume on production capacity and demand
                production_capacity = nation_a.get_resource_production(resource)
                import_demand = b_import_needs[resource]
                trade_amount = min(production_capacity * 0.2, import_demand)  # 20% of production for export
                
                if trade_amount > 0:
                    trade_volume[resource] = trade_amount
        
        return trade_volume
    
    def _update_global_markets(self) -> Dict[str, Any]:
        """Update global resource markets and prices"""
        # Calculate global supply and demand
        global_supply = {}
        global_demand = {}
        
        for nation in self.nations.values():
            nation_supply = nation.calculate_total_production()
            nation_demand = nation.calculate_total_consumption()
            
            for resource, supply in nation_supply.items():
                global_supply[resource] = global_supply.get(resource, 0) + supply
            for resource, demand in nation_demand.items():
                global_demand[resource] = global_demand.get(resource, 0) + demand
        
        # Update resource manager with global conditions
        self.resource_manager.update_global_supply_demand(global_demand, global_supply)
        
        # Get current prices
        current_prices = {}
        for resource_type, resource in self.resource_manager.resources.items():
            current_prices[resource_type] = resource.current_value
        
        # Update market dynamics
        self.market_dynamics.update_price_histories(current_prices)
        
        # Detect speculative bubbles
        fundamental_values = self._calculate_fundamental_values()
        bubbles = self.market_dynamics.detect_bubbles(current_prices, fundamental_values)
        
        return {
            'prices': current_prices,
            'supply': global_supply,
            'demand': global_demand,
            'market_sentiment': self.market_dynamics.get_market_sentiment(),
            'speculative_bubbles': bubbles
        }
    
    def _calculate_fundamental_values(self) -> Dict[ResourceType, float]:
        """Calculate fundamental values of resources based on utility and scarcity"""
        fundamental_values = {}
        
        for resource_type, resource in self.resource_manager.resources.items():
            # Base value adjusted for scarcity and utility
            scarcity_factor = 1.0 + resource.scarcity
            utility_factor = self._calculate_resource_utility(resource_type)
            fundamental_values[resource_type] = resource.base_value * scarcity_factor * utility_factor
        
        return fundamental_values
    
    def _calculate_resource_utility(self, resource_type: ResourceType) -> float:
        """Calculate the economic utility of a resource"""
        utility_factors = {
            ResourceType.FOOD: 2.0,      # Essential for survival
            ResourceType.WOOD: 1.2,      # Construction and fuel
            ResourceType.IRON: 1.5,      # Industrial applications
            ResourceType.GOLD: 0.8,      # Mostly monetary value
            ResourceType.OIL: 1.8,       # Energy and industry
            ResourceType.ELECTRONICS: 1.6, # Technology and communication
        }
        return utility_factors.get(resource_type, 1.0)
    
    def get_economic_report(self, entity_id: str) -> Optional[EconomicReport]:
        """Generate comprehensive economic report for any entity"""
        if entity_id in self.nations:
            return self.nations[entity_id].generate_economic_report()
        elif entity_id in self.regions:
            return self.regions[entity_id].generate_economic_report()
        return None
    
    def implement_economic_policy(self, nation_id: str, policy_type: str, 
                                parameters: Dict[str, Any]) -> Dict[str, Any]:
        """Allow nations to implement economic policies"""
        if nation_id not in self.nations:
            return {'success': False, 'error': 'Nation not found'}
        
        nation = self.nations[nation_id]
        return nation.implement_policy(policy_type, parameters, self)

class NationEconomy(EconomicEntity):
    """Represents the complete economy of a civilization nation"""
    
    def __init__(self, nation_id: str, name: str, development_level: DevelopmentLevel):
        super().__init__(nation_id, name, development_level)
        self.regions: Dict[str, RegionalEconomy] = {}
        self.wealth_distribution = WealthDistribution(nation_id)
        self.infrastructure_manager = InfrastructureManager(nation_id)
        self.trade_policy = TradePolicy.MANAGED
        
        # National economic indicators
        self.total_population = 100000
        self.gdp = self.calculate_economic_output()
        self.gdp_growth = 0.03
        self.inflation = 0.02
        self.unemployment_rate = 0.05
        self.export_volume = 0.0
        self.import_volume = 0.0
        self.trade_balance = 0.0
        self.budget_deficit = 0.0
        
        # Economic sectors
        self.agriculture_share = 0.4
        self.industry_share = 0.3
        self.services_share = 0.3
        
    def add_region(self, region: RegionalEconomy):
        """Add a region to the nation"""
        self.regions[region.region_id] = region
    
    def simulate_nation_turn(self, global_economy: CivilizationEconomy) -> Dict[str, Any]:
        """Simulate one economic turn for the nation"""
        results = {
            'gdp': self.gdp,
            'growth': self.gdp_growth,
            'inflation': self.inflation,
            'unemployment': self.unemployment_rate,
            'trade_balance': self.trade_balance,
            'regional_performance': {},
            'sector_performance': {},
            'major_event': None
        }
        
        # Simulate each region
        total_regional_gdp = 0
        for region_id, region in self.regions.items():
            region_results = region.calculate_regional_economy()
            results['regional_performance'][region_id] = region_results
            total_regional_gdp += region_results['regional_gdp']
        
        # Update national GDP
        old_gdp = self.gdp
        self.gdp = total_regional_gdp
        self.gdp_growth = (self.gdp - old_gdp) / old_gdp if old_gdp > 0 else 0.03
        
        # Update other economic indicators
        self._update_economic_indicators(global_economy)
        
        # Apply wealth distribution effects
        wealth_impact = self.wealth_distribution.get_economic_impact()
        
        # Simulate infrastructure
        infra_results = self.infrastructure_manager.simulate_infrastructure_turn(
            self._get_national_resources(), self.gdp * 0.2  # 20% of GDP for infrastructure
        )
        
        # Check for economic events
        event = self._check_economic_events(global_economy)
        if event:
            results['major_event'] = event
        
        return results
    
    def _update_economic_indicators(self, global_economy: CivilizationEconomy):
        """Update nation's economic indicators"""
        # Inflation based on global inflation and domestic factors
        domestic_inflation_pressure = max(0, self.gdp_growth - 0.03) * 0.5
        self.inflation = global_economy.global_inflation * 0.7 + domestic_inflation_pressure * 0.3
        
        # Unemployment based on growth and development
        base_unemployment = 0.15 - (self.development_level.value * 0.02)
        cyclical_unemployment = max(0, -self.gdp_growth * 2)
        self.unemployment_rate = base_unemployment + cyclical_unemployment
        
        # Update wealth distribution
        total_wealth = self.gdp * 5  # Assume wealth is 5x GDP
        economic_conditions = {
            'gdp_growth': self.gdp_growth,
            'inflation': self.inflation,
            'unemployment': self.unemployment_rate
        }
        self.wealth_distribution.update_distribution(total_wealth, [], economic_conditions)
    
    def _get_national_resources(self) -> Dict[ResourceType, float]:
        """Get total resources available nationwide"""
        national_resources = {}
        for region in self.regions.values():
            # Sum resources from all regions (simplified)
            for city in region.cities.values():
                for resource, amount in city.inventory.items():
                    national_resources[resource] = national_resources.get(resource, 0) + amount
        return national_resources
    
    def _check_economic_events(self, global_economy: CivilizationEconomy) -> Optional[str]:
        """Check for significant economic events"""
        # Economic boom
        if self.gdp_growth > 0.08:
            return f"Economic boom in {self.name}! GDP growth: {self.gdp_growth:.1%}"
        
        # Recession
        elif self.gdp_growth < -0.03:
            return f"Economic recession in {self.name}. GDP contraction: {self.gdp_growth:.1%}"
        
        # Hyperinflation
        elif self.inflation > 0.25:
            return f"Hyperinflation crisis in {self.name}! Inflation: {self.inflation:.1%}"
        
        return None
    
    def generate_economic_report(self) -> EconomicReport:
        """Generate comprehensive economic report"""
        return EconomicReport(
            entity_id=self.id,
            gdp=self.gdp,
            gdp_growth=self.gdp_growth,
            inflation=self.inflation,
            unemployment=self.unemployment_rate,
            wealth=self.gdp * 5,  # Estimate total wealth
            debt=self.debt,
            debt_to_gdp=self.debt / self.gdp if self.gdp > 0 else 0,
            trade_balance=self.trade_balance,
            currency_strength=1.0 / (1.0 + self.inflation),  # Simplified
            economic_state=self._get_economic_state(),
            development_level=self.development_level,
            key_industries=list(self.wealth_distribution.segments.keys())[:3],
            economic_health=self._calculate_economic_health()
        )
    
    def _get_economic_state(self) -> EconomicState:
        """Determine nation's economic state"""
        if self.gdp_growth > 0.06:
            return EconomicState.BOOM
        elif self.gdp_growth > 0.03:
            return EconomicState.EXPANSION
        elif self.gdp_growth > -0.02:
            return EconomicState.STABILITY
        elif self.gdp_growth > -0.05:
            return EconomicState.RECESSION
        else:
            return EconomicState.DEPRESSION
    
    def _calculate_economic_health(self) -> float:
        """Calculate overall economic health (0-1)"""
        health_indicators = [
            max(0, min(1, (self.gdp_growth + 0.1) / 0.2)),  # Growth indicator
            max(0, min(1, 1.0 - (self.inflation / 0.2))),   # Inflation indicator
            max(0, min(1, 1.0 - (self.unemployment_rate / 0.2))),  # Employment
            max(0, min(1, 1.0 - abs(self.trade_balance) / self.gdp)) if self.gdp > 0 else 0.5,  # Trade balance
            1.0 - (self.wealth_distribution.gini_coefficient * 0.8)  # Equality
        ]
        return sum(health_indicators) / len(health_indicators)
    
class Corporation:
    """Represents a multinational corporation"""
    
    def __init__(self, corp_id: str, name: str, headquarters: str):
        self.id = corp_id
        self.name = name
        self.headquarters = headquarters  # Nation ID
        self.subsidiaries: Dict[str, RegionalEconomy] = {}
        self.global_revenue = 0.0
        self.global_profit = 0.0
        self.global_employees = 0
        self.industry_sector = "General"
        self.market_share = 0.0  # Percentage of global market in sector
        
    def add_subsidiary(self, region: RegionalEconomy):
        """Add a regional subsidiary"""
        self.subsidiaries[region.region_id] = region
    
    def simulate_corporate_turn(self, global_economy: CivilizationEconomy) -> Dict[str, Any]:
        """Simulate one turn for the corporation"""
        results = {
            'revenue': self.global_revenue,
            'profit': self.global_profit,
            'employees': self.global_employees,
            'market_share': self.market_share,
            'subsidiary_performance': {}
        }
        
        total_revenue = 0.0
        total_profit = 0.0
        total_employees = 0
        
        for region_id, subsidiary in self.subsidiaries.items():
            sub_results = subsidiary.calculate_regional_economy()
            results['subsidiary_performance'][region_id] = sub_results
            
            # Simplified corporate metrics based on regional economy
            revenue = sub_results['regional_gdp'] * 0.1  # Assume 10% of regional GDP
            profit = revenue * 0.2  # Assume 20% profit margin
            employees = int(subsidiary.total_population * 0.05)  # Employ 5% of population
            
            total_revenue += revenue
            total_profit += profit
            total_employees += employees
        
        self.global_revenue = total_revenue
        self.global_profit = total_profit
        self.global_employees = total_employees
        
        # Update market share (simplified)
        industry_total = sum(corp.global_revenue for corp in global_economy.corporations.values() if corp.industry_sector == self.industry_sector)
        if industry_total > 0:
            self.market_share = self.global_revenue / industry_total
        else:
            self.market_share = 0.0
            results['market_share'] = self.market_share

            return results
        
