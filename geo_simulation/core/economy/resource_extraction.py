"""Gathering natural resources"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple, Any
from dataclasses import dataclass
from enum import Enum
import random
from base_economy import ResourceType, EconomicEntity, DevelopmentLevel

class ExtractionMethod(Enum):
    SURFACE_MINING = "surface_mining"      # Open-pit, strip mining
    UNDERGROUND_MINING = "underground_mining" # Shaft mining, tunneling
    DRILLING = "drilling"                 # Oil, gas, geothermal
    QUARRYING = "quarrying"               # Stone, marble
    LOGGING = "logging"                   # Forestry
    HARVESTING = "harvesting"             # Agriculture, fishing
    PUMPING = "pumping"                   # Water, oil
    FRACKING = "fracking"                 # Hydraulic fracturing
    DEEP_SEA_MINING = "deep_sea_mining"   # Ocean floor extraction
    SPACE_MINING = "space_mining"         # Asteroid mining

class ExtractionEfficiency(Enum):
    PRIMITIVE = "primitive"    # 0.3 efficiency
    BASIC = "basic"           # 0.5 efficiency  
    ADVANCED = "advanced"     # 0.7 efficiency
    MODERN = "modern"         # 0.85 efficiency
    FUTURISTIC = "futuristic" # 0.95 efficiency

@dataclass
class ResourceDeposit:
    deposit_id: str
    resource_type: ResourceType
    location: str
    total_reserves: float
    current_reserves: float
    concentration: float  # 0-1, how concentrated the resource is
    accessibility: float  # 0-1, how easy to extract
    depth: float         # meters below surface
    quality: float       # 0-1, purity/quality of resource
    
    @property
    def depletion_rate(self) -> float:
        """Calculate natural depletion rate based on reserves"""
        if self.total_reserves <= 0:
            return 0.0
        return (self.total_reserves - self.current_reserves) / self.total_reserves
    
    @property
    def economic_viability(self) -> float:
        """Calculate how economically viable this deposit is"""
        viability = (self.concentration * 0.4 + 
                    self.accessibility * 0.3 + 
                    self.quality * 0.3)
        # Adjust for depletion
        return viability * (1.0 - self.depletion_rate * 0.5)

class ExtractionSite:
    """Represents a physical resource extraction operation"""
    
    def __init__(self, site_id: str, deposit: ResourceDeposit, 
                 extraction_method: ExtractionMethod, efficiency: ExtractionEfficiency):
        self.site_id = site_id
        self.deposit = deposit
        self.extraction_method = extraction_method
        self.efficiency = efficiency
        self.workforce = 0
        self.max_workforce = self._calculate_max_workforce()
        self.current_production = 0.0
        self.operational_cost = 0.0
        self.infrastructure_level = 1.0
        self.environmental_impact = 0.0
        self.active = True
        
        # Technology and equipment
        self.equipment_quality = 1.0
        self.automation_level = 0.0
        self.safety_standards = 0.7
        
    def _calculate_max_workforce(self) -> int:
        """Calculate maximum workforce based on extraction method and deposit size"""
        base_workforce = {
            ExtractionMethod.SURFACE_MINING: 50,
            ExtractionMethod.UNDERGROUND_MINING: 80,
            ExtractionMethod.DRILLING: 20,
            ExtractionMethod.QUARRYING: 40,
            ExtractionMethod.LOGGING: 30,
            ExtractionMethod.HARVESTING: 25,
            ExtractionMethod.PUMPING: 15,
            ExtractionMethod.FRACKING: 35,
            ExtractionMethod.DEEP_SEA_MINING: 60,
            ExtractionMethod.SPACE_MINING: 100
        }
        return base_workforce.get(self.extraction_method, 30)
    
    @property
    def effective_efficiency(self) -> float:
        """Calculate effective extraction efficiency"""
        base_efficiency = {
            ExtractionEfficiency.PRIMITIVE: 0.3,
            ExtractionEfficiency.BASIC: 0.5,
            ExtractionEfficiency.ADVANCED: 0.7,
            ExtractionEfficiency.MODERN: 0.85,
            ExtractionEfficiency.FUTURISTIC: 0.95
        }
        
        efficiency = base_efficiency.get(self.efficiency, 0.5)
        
        # Adjust for workforce
        workforce_factor = min(1.0, self.workforce / self.max_workforce) if self.max_workforce > 0 else 0
        efficiency *= workforce_factor
        
        # Adjust for infrastructure
        efficiency *= self.infrastructure_level
        
        # Adjust for equipment
        efficiency *= self.equipment_quality
        
        # Adjust for automation
        efficiency *= (1.0 + self.automation_level * 0.5)
        
        return efficiency
    
    def calculate_production_capacity(self) -> float:
        """Calculate maximum possible production per turn"""
        if not self.active or self.deposit.current_reserves <= 0:
            return 0.0
        
        base_capacity = self.deposit.concentration * 1000  # Base production rate
        
        # Method-specific multipliers
        method_multipliers = {
            ExtractionMethod.SURFACE_MINING: 1.2,
            ExtractionMethod.UNDERGROUND_MINING: 0.8,
            ExtractionMethod.DRILLING: 1.5,
            ExtractionMethod.QUARRYING: 1.1,
            ExtractionMethod.LOGGING: 0.9,
            ExtractionMethod.HARVESTING: 1.0,
            ExtractionMethod.PUMPING: 1.3,
            ExtractionMethod.FRACKING: 1.7,
            ExtractionMethod.DEEP_SEA_MINING: 1.4,
            ExtractionMethod.SPACE_MINING: 2.0
        }
        
        capacity = base_capacity * method_multipliers.get(self.extraction_method, 1.0)
        capacity *= self.effective_efficiency
        capacity *= self.deposit.accessibility
        
        # Reduce capacity as deposit depletes
        depletion_penalty = 1.0 - (self.deposit.depletion_rate * 0.5)
        capacity *= depletion_penalty
        
        return capacity
    
    def extract_resources(self, desired_amount: float) -> float:
        """Extract resources from the deposit, returns actual amount extracted"""
        if not self.active or self.deposit.current_reserves <= 0:
            return 0.0
        
        production_capacity = self.calculate_production_capacity()
        actual_extraction = min(desired_amount, production_capacity, self.deposit.current_reserves)
        
        # Update deposit
        self.deposit.current_reserves -= actual_extraction
        self.current_production = actual_extraction
        
        # Calculate operational costs
        self._calculate_operational_costs(actual_extraction)
        
        # Increase environmental impact
        self._update_environmental_impact(actual_extraction)
        
        # Check if deposit is exhausted
        if self.deposit.current_reserves <= 0:
            self.active = False
            print(f"⛏️ Resource deposit exhausted at site {self.site_id}")
        
        return actual_extraction
    
    def _calculate_operational_costs(self, extraction_amount: float):
        """Calculate operational costs for extraction"""
        base_cost_per_unit = 0.1
        
        # Method-specific cost multipliers
        cost_multipliers = {
            ExtractionMethod.UNDERGROUND_MINING: 1.5,
            ExtractionMethod.DEEP_SEA_MINING: 2.0,
            ExtractionMethod.SPACE_MINING: 5.0,
            ExtractionMethod.FRACKING: 1.8,
            ExtractionMethod.DRILLING: 1.3
        }
        
        multiplier = cost_multipliers.get(self.extraction_method, 1.0)
        
        # Efficiency reduces costs
        cost_reduction = self.effective_efficiency * 0.3
        
        # Automation reduces labor costs
        labor_savings = self.automation_level * 0.4
        
        self.operational_cost = (extraction_amount * base_cost_per_unit * multiplier * 
                               (1.0 - cost_reduction) * (1.0 - labor_savings))
    
    def _update_environmental_impact(self, extraction_amount: float):
        """Update environmental impact of extraction operations"""
        base_impact_per_unit = 0.01
        
        # Method-specific environmental impact
        impact_multipliers = {
            ExtractionMethod.SURFACE_MINING: 1.5,
            ExtractionMethod.FRACKING: 2.0,
            ExtractionMethod.DEEP_SEA_MINING: 1.8,
            ExtractionMethod.SPACE_MINING: 0.1,  # Lower terrestrial impact
            ExtractionMethod.LOGGING: 1.2,
            ExtractionMethod.HARVESTING: 0.8
        }
        
        multiplier = impact_multipliers.get(self.extraction_method, 1.0)
        
        # Safety standards reduce environmental impact
        impact_reduction = self.safety_standards * 0.3
        
        impact_increase = extraction_amount * base_impact_per_unit * multiplier * (1.0 - impact_reduction)
        self.environmental_impact = min(1.0, self.environmental_impact + impact_increase)
    
    def upgrade_equipment(self, improvement: float) -> float:
        """Upgrade extraction equipment, returns cost"""
        upgrade_cost = improvement * 1000  # Cost scales with improvement
        self.equipment_quality = min(2.0, self.equipment_quality + improvement)
        return upgrade_cost
    
    def increase_automation(self, automation_increase: float) -> float:
        """Increase automation level, returns cost"""
        automation_cost = automation_increase * 2000  # Higher cost for automation
        self.automation_level = min(1.0, self.automation_level + automation_increase)
        
        # Automation reduces required workforce
        self.max_workforce = int(self._calculate_max_workforce() * (1.0 - self.automation_level * 0.6))
        
        return automation_cost

class ResourceExtractionManager:
    """Manages all resource extraction operations for an economic entity"""
    
    def __init__(self, entity_id: str):
        self.entity_id = entity_id
        self.extraction_sites: Dict[str, ExtractionSite] = {}
        self.known_deposits: Dict[str, ResourceDeposit] = {}
        self.extraction_technology: Dict[ExtractionMethod, ExtractionEfficiency] = {}
        self.initialize_extraction_technology()
    
    def initialize_extraction_technology(self):
        """Initialize starting extraction technology levels"""
        self.extraction_technology = {
            ExtractionMethod.SURFACE_MINING: ExtractionEfficiency.BASIC,
            ExtractionMethod.UNDERGROUND_MINING: ExtractionEfficiency.PRIMITIVE,
            ExtractionMethod.DRILLING: ExtractionEfficiency.BASIC,
            ExtractionMethod.QUARRYING: ExtractionEfficiency.BASIC,
            ExtractionMethod.LOGGING: ExtractionEfficiency.BASIC,
            ExtractionMethod.HARVESTING: ExtractionEfficiency.BASIC,
            ExtractionMethod.PUMPING: ExtractionEfficiency.BASIC,
            ExtractionMethod.FRACKING: ExtractionEfficiency.PRIMITIVE,
            ExtractionMethod.DEEP_SEA_MINING: ExtractionEfficiency.PRIMITIVE,
            ExtractionMethod.SPACE_MINING: ExtractionEfficiency.PRIMITIVE
        }
    
    def discover_deposit(self, resource_type: ResourceType, location: str, 
                        size_multiplier: float = 1.0) -> ResourceDeposit:
        """Discover a new resource deposit"""
        # Generate random deposit characteristics
        total_reserves = random.uniform(50000, 500000) * size_multiplier
        concentration = random.uniform(0.1, 0.9)
        accessibility = random.uniform(0.3, 0.95)
        depth = random.uniform(0, 5000)
        quality = random.uniform(0.5, 1.0)
        
        deposit = ResourceDeposit(
            deposit_id=f"deposit_{len(self.known_deposits)}",
            resource_type=resource_type,
            location=location,
            total_reserves=total_reserves,
            current_reserves=total_reserves,
            concentration=concentration,
            accessibility=accessibility,
            depth=depth,
            quality=quality
        )
        
        self.known_deposits[deposit.deposit_id] = deposit
        print(f"📍 Discovered {resource_type.value} deposit at {location} "
              f"(Reserves: {total_reserves:,.0f}, Quality: {quality:.2f})")
        
        return deposit
    
    def establish_extraction_site(self, deposit_id: str, 
                                extraction_method: ExtractionMethod) -> Optional[ExtractionSite]:
        """Establish a new extraction site at a deposit"""
        if deposit_id not in self.known_deposits:
            return None
        
        deposit = self.known_deposits[deposit_id]
        
        # Check if method is appropriate for deposit
        if not self._is_method_suitable(extraction_method, deposit):
            return None
        
        efficiency = self.extraction_technology.get(extraction_method, ExtractionEfficiency.PRIMITIVE)
        
        site = ExtractionSite(
            site_id=f"site_{len(self.extraction_sites)}",
            deposit=deposit,
            extraction_method=extraction_method,
            efficiency=efficiency
        )
        
        self.extraction_sites[site.site_id] = site
        print(f"🏭 Established {extraction_method.value} site at {deposit.location} "
              f"for {deposit.resource_type.value}")
        
        return site
    
    def _is_method_suitable(self, method: ExtractionMethod, deposit: ResourceDeposit) -> bool:
        """Check if extraction method is suitable for deposit"""
        # Depth constraints
        if method == ExtractionMethod.SURFACE_MINING and deposit.depth > 100:
            return False
        if method == ExtractionMethod.UNDERGROUND_MINING and deposit.depth < 50:
            return False
        
        # Resource-type constraints
        if (method == ExtractionMethod.LOGGING and 
            deposit.resource_type != ResourceType.WOOD):
            return False
        if (method in [ExtractionMethod.DRILLING, ExtractionMethod.PUMPING, ExtractionMethod.FRACKING] and
            deposit.resource_type not in [ResourceType.OIL, ResourceType.WATER]):
            return False
        
        return True
    
    def simulate_extraction_turn(self, workforce_allocation: Dict[str, int] = None) -> Dict[ResourceType, float]:
        """Simulate one turn of resource extraction"""
        total_extraction = {}
        
        # Default workforce allocation if not provided
        if workforce_allocation is None:
            workforce_allocation = {}
            for site_id, site in self.extraction_sites.items():
                if site.active:
                    workforce_allocation[site_id] = site.max_workforce // 2  # 50% staffing by default
        
        # Extract from each active site
        for site_id, site in self.extraction_sites.items():
            if not site.active:
                continue
            
            # Allocate workforce
            allocated_workforce = workforce_allocation.get(site_id, 0)
            site.workforce = min(allocated_workforce, site.max_workforce)
            
            # Calculate desired extraction based on workforce
            desired_extraction = site.calculate_production_capacity()
            
            # Extract resources
            actual_extraction = site.extract_resources(desired_extraction)
            
            if actual_extraction > 0:
                resource_type = site.deposit.resource_type
                total_extraction[resource_type] = total_extraction.get(resource_type, 0) + actual_extraction
        
        return total_extraction
    
    def get_extraction_report(self) -> Dict[str, Any]:
        """Generate comprehensive extraction report"""
        report = {
            'active_sites': 0,
            'total_production': {},
            'depletion_rates': {},
            'operational_costs': 0.0,
            'environmental_impact': 0.0,
            'efficiency_metrics': {}
        }
        
        for site in self.extraction_sites.values():
            if site.active:
                report['active_sites'] += 1
                report['operational_costs'] += site.operational_cost
                report['environmental_impact'] += site.environmental_impact
                
                # Production by resource
                resource = site.deposit.resource_type
                report['total_production'][resource] = report['total_production'].get(resource, 0) + site.current_production
                
                # Depletion rates
                depletion = site.deposit.depletion_rate
                report['depletion_rates'][resource] = max(report['depletion_rates'].get(resource, 0.0), depletion)
                
                # Efficiency metrics
                efficiency = site.effective_efficiency
                report['efficiency_metrics'][site.site_id] = {
                    'method': site.extraction_method.value,
                    'efficiency': efficiency,
                    'workforce_utilization': site.workforce / site.max_workforce if site.max_workforce > 0 else 0,
                    'reserves_remaining': site.deposit.current_reserves
                }
        
        return report
    
    def research_extraction_technology(self, method: ExtractionMethod, 
                                    research_investment: float) -> bool:
        """Invest in research to improve extraction technology"""
        current_level = self.extraction_technology.get(method)
        if not current_level:
            return False
        
        # Research costs for each level
        research_costs = {
            ExtractionEfficiency.PRIMITIVE: 1000,
            ExtractionEfficiency.BASIC: 5000,
            ExtractionEfficiency.ADVANCED: 20000,
            ExtractionEfficiency.MODERN: 50000,
            ExtractionEfficiency.FUTURISTIC: 100000
        }
        
        current_cost = research_costs.get(current_level, 0)
        next_level = self._get_next_efficiency_level(current_level)
        
        if next_level and research_investment >= current_cost:
            self.extraction_technology[method] = next_level
            print(f"🔬 Improved {method.value} technology to {next_level.value}")
            return True
        
        return False
    
    def _get_next_efficiency_level(self, current: ExtractionEfficiency) -> Optional[ExtractionEfficiency]:
        """Get the next efficiency level"""
        levels = list(ExtractionEfficiency)
        current_index = levels.index(current)
        if current_index < len(levels) - 1:
            return levels[current_index + 1]
        return None
    
    def optimize_extraction_operations(self, budget: float) -> Dict[str, float]:
        """Optimize extraction operations with given budget"""
        improvements = {}
        remaining_budget = budget
        
        # Prioritize sites with high economic viability but low efficiency
        sites_by_priority = sorted(
            [s for s in self.extraction_sites.values() if s.active],
            key=lambda x: (x.deposit.economic_viability * (1.0 - x.effective_efficiency)),
            reverse=True
        )
        
        for site in sites_by_priority:
            if remaining_budget <= 0:
                break
            
            # Equipment upgrade
            if site.equipment_quality < 1.5 and remaining_budget >= 500:
                upgrade_amount = min(0.3, 1.5 - site.equipment_quality)
                cost = site.upgrade_equipment(upgrade_amount)
                if cost <= remaining_budget:
                    improvements[f"{site.site_id}_equipment"] = upgrade_amount
                    remaining_budget -= cost
            
            # Automation increase
            if site.automation_level < 0.8 and remaining_budget >= 1000:
                automation_increase = min(0.2, 0.8 - site.automation_level)
                cost = site.increase_automation(automation_increase)
                if cost <= remaining_budget:
                    improvements[f"{site.site_id}_automation"] = automation_increase
                    remaining_budget -= cost
        
        return improvements