"""Manufacturing and processing"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
from base_economy import ResourceType, DevelopmentLevel

class ChainComplexity(Enum):
    SIMPLE = "simple"        # 1-2 steps
    MODERATE = "moderate"    # 3-4 steps
    COMPLEX = "complex"      # 5-6 steps
    ADVANCED = "advanced"    # 7+ steps

@dataclass
class ProductionStep:
    step_id: str
    process: str
    inputs: Dict[ResourceType, float]
    outputs: Dict[ResourceType, float]
    time_required: int
    workforce_required: int
    required_technology: Optional[str] = None
    efficiency: float = 1.0

class ProductionChain:
    """Represents a complete production chain from raw materials to finished goods"""
    
    def __init__(self, chain_id: str, name: str, final_product: ResourceType,
                 complexity: ChainComplexity, development_required: DevelopmentLevel):
        self.chain_id = chain_id
        self.name = name
        self.final_product = final_product
        self.complexity = complexity
        self.development_required = development_required
        self.steps: List[ProductionStep] = []
        self.dependencies: Set[str] = set()  # Other chains this depends on
        self.overall_efficiency = 1.0
        self.bottlenecks: List[Tuple[str, float]] = []  # (step_id, severity)
    
    def add_step(self, step: ProductionStep):
        """Add a production step to the chain"""
        self.steps.append(step)
        # Recalculate overall efficiency
        self._calculate_efficiency()
    
    def _calculate_efficiency(self):
        """Calculate overall chain efficiency"""
        if not self.steps:
            self.overall_efficiency = 0.0
            return
        
        # Chain efficiency is product of step efficiencies
        self.overall_efficiency = 1.0
        for step in self.steps:
            self.overall_efficiency *= step.efficiency
    
    def simulate_production(self, available_resources: Dict[ResourceType, float],
                          workforce: int) -> Dict[ResourceType, float]:
        """Simulate production through the entire chain"""
        if not self.steps:
            return {}
        
        current_resources = available_resources.copy()
        output = {}
        self.bottlenecks = []
        
        for i, step in enumerate(self.steps):
            # Check if we can execute this step
            can_produce = True
            for resource, amount in step.inputs.items():
                if current_resources.get(resource, 0) < amount:
                    can_produce = False
                    bottleneck_severity = 1.0 - (current_resources.get(resource, 0) / amount)
                    self.bottlenecks.append((step.step_id, bottleneck_severity))
                    break
            
            workforce_available = workforce >= step.workforce_required
            if not workforce_available:
                can_produce = False
                self.bottlenecks.append((step.step_id, 0.5))
            
            if can_produce:
                # Consume inputs
                for resource, amount in step.inputs.items():
                    current_resources[resource] -= amount
                
                # Produce outputs
                for resource, amount in step.outputs.items():
                    produced = amount * step.efficiency
                    output[resource] = output.get(resource, 0) + produced
                    current_resources[resource] = current_resources.get(resource, 0) + produced
        
        return output
    
    def identify_optimization_opportunities(self) -> List[Tuple[str, float, str]]:
        """Identify opportunities to optimize the production chain"""
        opportunities = []
        
        for step in self.steps:
            if step.efficiency < 0.8:
                opportunities.append((
                    step.step_id,
                    1.0 - step.efficiency,
                    f"Efficiency improvement potential: {((1.0 - step.efficiency) * 100):.1f}%"
                ))
            
            # Check for technology upgrades
            if step.required_technology:
                opportunities.append((
                    step.step_id,
                    0.3,
                    f"Technology upgrade available: {step.required_technology}"
                ))
        
        return sorted(opportunities, key=lambda x: x[1], reverse=True)

class ProductionChainManager:
    """Manages all production chains in the economy"""
    
    def __init__(self):
        self.chains: Dict[str, ProductionChain] = {}
        self.chain_dependencies: Dict[str, Set[str]] = {}
        self.initialize_core_chains()
    
    def initialize_core_chains(self):
        """Initialize fundamental production chains"""
        
        # Food production chain
        food_chain = ProductionChain("food_chain", "Food Production", 
                                   ResourceType.FOOD, ChainComplexity.MODERATE, 
                                   DevelopmentLevel.AGRARIAN)
        
        food_chain.add_step(ProductionStep(
            "farming", "Crop Farming",
            inputs={ResourceType.FOOD: 0.1},  # Seeds
            outputs={ResourceType.FOOD: 5.0},
            time_required=2,
            workforce_required=20
        ))
        
        food_chain.add_step(ProductionStep(
            "processing", "Food Processing",
            inputs={ResourceType.FOOD: 3.0},
            outputs={ResourceType.FOOD: 4.0},  # Value-added food
            time_required=1,
            workforce_required=10,
            efficiency=0.9
        ))
        
        self.chains["food_chain"] = food_chain
        
        # Steel production chain
        steel_chain = ProductionChain("steel_chain", "Steel Production",
                                    ResourceType.MACHINERY, ChainComplexity.COMPLEX,
                                    DevelopmentLevel.INDUSTRIAL)
        
        steel_chain.add_step(ProductionStep(
            "mining", "Iron Mining",
            inputs={},
            outputs={ResourceType.IRON: 8.0},
            time_required=1,
            workforce_required=15
        ))
        
        steel_chain.add_step(ProductionStep(
            "coal_mining", "Coal Mining",
            inputs={},
            outputs={ResourceType.COAL: 6.0},
            time_required=1,
            workforce_required=12
        ))
        
        steel_chain.add_step(ProductionStep(
            "smelting", "Steel Smelting",
            inputs={ResourceType.IRON: 5.0, ResourceType.COAL: 3.0},
            outputs={ResourceType.MACHINERY: 3.0},
            time_required=2,
            workforce_required=25,
            required_technology="blast_furnace"
        ))
        
        self.chains["steel_chain"] = steel_chain
        
        # Electronics production chain
        electronics_chain = ProductionChain("electronics_chain", "Electronics Manufacturing",
                                          ResourceType.ELECTRONICS, ChainComplexity.ADVANCED,
                                          DevelopmentLevel.DIGITAL)
        
        electronics_chain.add_step(ProductionStep(
            "copper_mining", "Copper Mining",
            inputs={},
            outputs={ResourceType.COPPER: 10.0},
            time_required=1,
            workforce_required=8
        ))
        
        electronics_chain.add_step(ProductionStep(
            "silicon_production", "Silicon Production",
            inputs={ResourceType.STONE: 8.0},
            outputs={ResourceType.ELECTRONICS: 2.0},
            time_required=2,
            workforce_required=15,
            required_technology="semiconductor_tech"
        ))
        
        electronics_chain.add_step(ProductionStep(
            "assembly", "Component Assembly",
            inputs={ResourceType.COPPER: 6.0, ResourceType.ELECTRONICS: 1.5},
            outputs={ResourceType.ELECTRONICS: 4.0},
            time_required=3,
            workforce_required=20,
            efficiency=0.85
        ))
        
        self.chains["electronics_chain"] = electronics_chain
    
    def get_available_chains(self, development_level: DevelopmentLevel, 
                           known_techs: Set[str]) -> List[ProductionChain]:
        """Get production chains available based on development and technology"""
        available = []
        
        for chain in self.chains.values():
            # Check development level
            if chain.development_required.value > development_level.value:
                continue
            
            # Check technology requirements for all steps
            tech_requirements_met = True
            for step in chain.steps:
                if step.required_technology and step.required_technology not in known_techs:
                    tech_requirements_met = False
                    break
            
            if tech_requirements_met:
                available.append(chain)
        
        return available
    
    def simulate_all_chains(self, available_resources: Dict[ResourceType, float],
                          total_workforce: int) -> Dict[ResourceType, float]:
        """Simulate all production chains"""
        total_output = {}
        resources = available_resources.copy()
        
        # Execute chains in order of complexity (simplest first)
        chains_by_complexity = sorted(self.chains.values(), 
                                    key=lambda x: x.complexity.value)
        
        for chain in chains_by_complexity:
            # Allocate workforce proportionally
            chain_workforce = total_workforce // len(chains_by_complexity)
            chain_output = chain.simulate_production(resources, chain_workforce)
            
            # Add outputs to total and available resources
            for resource, amount in chain_output.items():
                total_output[resource] = total_output.get(resource, 0) + amount
                resources[resource] = resources.get(resource, 0) + amount
        
        return total_output
    
    def analyze_supply_chain_risks(self) -> Dict[str, List[Tuple[str, float]]]:
        """Analyze risks and vulnerabilities in production chains"""
        risks = {}
        
        for chain_id, chain in self.chains.items():
            chain_risks = []
            
            # Single points of failure
            if len(chain.steps) == 1:
                chain_risks.append(("Single point of failure", 0.8))
            
            # Resource dependencies
            critical_resources = set()
            for step in chain.steps:
                for resource in step.inputs.keys():
                    critical_resources.add(resource)
            
            if len(critical_resources) < 3:  # Limited resource diversity
                chain_risks.append(("Resource concentration", 0.6))
            
            # Efficiency bottlenecks
            for step in chain.steps:
                if step.efficiency < 0.7:
                    chain_risks.append((f"Efficiency bottleneck: {step.process}", 
                                      1.0 - step.efficiency))
            
            risks[chain_id] = chain_risks
        
        return risks
    
    def optimize_chain(self, chain_id: str, improvement_budget: float) -> Dict[str, float]:
        """Optimize a production chain with given budget"""
        if chain_id not in self.chains:
            return {}
        
        chain = self.chains[chain_id]
        improvements = {}
        remaining_budget = improvement_budget
        
        # Improve least efficient steps first
        steps_by_efficiency = sorted(chain.steps, key=lambda x: x.efficiency)
        
        for step in steps_by_efficiency:
            if remaining_budget <= 0:
                break
            
            improvement_cost = (1.0 - step.efficiency) * 1000
            if improvement_cost <= remaining_budget:
                improvement = min(0.2, 1.0 - step.efficiency)  # Max 20% improvement per optimization
                step.efficiency += improvement
                improvements[step.step_id] = improvement
                remaining_budget -= improvement_cost
        
        chain._calculate_efficiency()
        return improvements