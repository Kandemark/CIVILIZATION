"""Roads, ports, and buildings"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple, Any
from dataclasses import dataclass
from enum import Enum
import random
from base_economy import ResourceType, EconomicEntity, DevelopmentLevel

class InfrastructureType(Enum):
    TRANSPORTATION = "transportation"
    ENERGY = "energy"
    WATER = "water"
    COMMUNICATION = "communication"
    EDUCATION = "education"
    HEALTHCARE = "healthcare"
    PUBLIC_SAFETY = "public_safety"
    WASTE_MANAGEMENT = "waste_management"
    DIGITAL = "digital"

class InfrastructureQuality(Enum):
    CRUMBLING = "crumbling"    # < 0.3
    POOR = "poor"              # 0.3-0.5
    ADEQUATE = "adequate"      # 0.5-0.7
    GOOD = "good"              # 0.7-0.9
    EXCELLENT = "excellent"    # > 0.9

@dataclass
class InfrastructureProject:
    project_id: str
    name: str
    infra_type: InfrastructureType
    location: str
    construction_cost: Dict[ResourceType, float]
    maintenance_cost: Dict[ResourceType, float]
    construction_time: int
    economic_benefits: Dict[str, float]
    required_tech: Optional[str] = None
    progress: float = 0.0
    active: bool = False
    completed: bool = False

class InfrastructureSystem:
    """Represents a specific infrastructure system in a region"""
    
    def __init__(self, system_id: str, infra_type: InfrastructureType, 
                 base_capacity: float, location: str):
        self.system_id = system_id
        self.infra_type = infra_type
        self.base_capacity = base_capacity
        self.location = location
        self.condition = 1.0
        self.quality = InfrastructureQuality.EXCELLENT
        self.utilization = 0.7  # 70% average utilization
        self.maintenance_backlog = 0.0
        self.upgrade_level = 1.0
        self.connected_systems: List[str] = []
        
    @property
    def effective_capacity(self) -> float:
        """Calculate effective capacity considering condition and utilization"""
        condition_factor = 0.3 + (self.condition * 0.7)  # 30-100% based on condition
        upgrade_multiplier = 1.0 + (self.upgrade_level - 1.0) * 0.5
        return self.base_capacity * condition_factor * upgrade_multiplier * self.utilization
    
    @property
    def maintenance_need(self) -> float:
        """Calculate maintenance needs based on condition and utilization"""
        base_maintenance = 0.01  # 1% per turn base
        utilization_stress = max(0, self.utilization - 0.8) * 0.05  # Extra stress above 80%
        condition_deterioration = (1.0 - self.condition) * 0.02
        return base_maintenance + utilization_stress + condition_deterioration + self.maintenance_backlog
    
    def calculate_economic_impact(self) -> Dict[str, float]:
        """Calculate economic impact of this infrastructure"""
        impacts = {}
        
        if self.infra_type == InfrastructureType.TRANSPORTATION:
            impacts['trade_efficiency'] = self.condition * 0.3
            impacts['production_efficiency'] = self.condition * 0.2
            impacts['regional_connectivity'] = self.condition * 0.4
            
        elif self.infra_type == InfrastructureType.ENERGY:
            impacts['industrial_output'] = self.condition * 0.5
            impacts['production_efficiency'] = self.condition * 0.3
            impacts['quality_of_life'] = self.condition * 0.2
            
        elif self.infra_type == InfrastructureType.EDUCATION:
            impacts['research_capacity'] = self.condition * 0.4
            impacts['workforce_quality'] = self.condition * 0.6
            impacts['innovation_rate'] = self.condition * 0.3
            
        elif self.infra_type == InfrastructureType.DIGITAL:
            impacts['information_efficiency'] = self.condition * 0.8
            impacts['research_capacity'] = self.condition * 0.4
            impacts['trade_efficiency'] = self.condition * 0.3
        
        return impacts
    
    def degrade(self, degradation_rate: float = 0.01):
        """Degrade infrastructure condition"""
        self.condition = max(0.0, self.condition - degradation_rate)
        self.maintenance_backlog += degradation_rate * 2  # Backlog grows faster than degradation
        
        # Update quality rating
        if self.condition < 0.3:
            self.quality = InfrastructureQuality.CRUMBLING
        elif self.condition < 0.5:
            self.quality = InfrastructureQuality.POOR
        elif self.condition < 0.7:
            self.quality = InfrastructureQuality.ADEQUATE
        elif self.condition < 0.9:
            self.quality = InfrastructureQuality.GOOD
        else:
            self.quality = InfrastructureQuality.EXCELLENT
    
    def perform_maintenance(self, maintenance_effort: float) -> float:
        """Perform maintenance and return actual maintenance performed"""
        actual_maintenance = min(maintenance_effort, self.maintenance_need)
        maintenance_effect = actual_maintenance * 2  # Maintenance improves condition twice as fast as degradation
        
        self.condition = min(1.0, self.condition + maintenance_effect)
        self.maintenance_backlog = max(0, self.maintenance_backlog - actual_maintenance)
        
        return actual_maintenance

class InfrastructureManager:
    """Manages all infrastructure systems and projects for an economic entity"""
    
    def __init__(self, entity_id: str):
        self.entity_id = entity_id
        self.systems: Dict[InfrastructureType, List[InfrastructureSystem]] = {}
        self.projects: List[InfrastructureProject] = []
        self.maintenance_budget: float = 0.1  # 10% of revenue for maintenance
        self.construction_budget: float = 0.05  # 5% for new construction
        self.infrastructure_quality: Dict[InfrastructureType, float] = {}
        self.initialize_infrastructure()
    
    def initialize_infrastructure(self):
        """Initialize basic infrastructure systems"""
        # Start with basic infrastructure based on development level
        basic_systems = [
            (InfrastructureType.TRANSPORTATION, 1000),
            (InfrastructureType.ENERGY, 500),
            (InfrastructureType.WATER, 800),
        ]
        
        for infra_type, capacity in basic_systems:
            if infra_type not in self.systems:
                self.systems[infra_type] = []
            
            system = InfrastructureSystem(
                f"{self.entity_id}_{infra_type.value}_1",
                infra_type,
                capacity,
                self.entity_id
            )
            self.systems[infra_type].append(system)
    
    def calculate_infrastructure_quality(self) -> Dict[InfrastructureType, float]:
        """Calculate overall quality for each infrastructure type"""
        quality_scores = {}
        
        for infra_type, systems in self.systems.items():
            if systems:
                avg_condition = sum(sys.condition for sys in systems) / len(systems)
                capacity_per_capita = sum(sys.effective_capacity for sys in systems) / max(len(systems), 1)
                quality_scores[infra_type] = (avg_condition + min(1.0, capacity_per_capita / 1000)) / 2
            else:
                quality_scores[infra_type] = 0.1  # Minimal quality if no systems
        
        self.infrastructure_quality = quality_scores
        return quality_scores
    
    def get_infrastructure_bottlenecks(self) -> List[Tuple[InfrastructureType, float]]:
        """Identify infrastructure bottlenecks (type, severity)"""
        bottlenecks = []
        quality_scores = self.calculate_infrastructure_quality()
        
        for infra_type, quality in quality_scores.items():
            if quality < 0.5:  # Below adequate quality
                severity = 1.0 - (quality / 0.5)  # 0-1 severity
                bottlenecks.append((infra_type, severity))
        
        return sorted(bottlenecks, key=lambda x: x[1], reverse=True)
    
    def propose_infrastructure_project(self, infra_type: InfrastructureType, 
                                     scale: float = 1.0) -> InfrastructureProject:
        """Propose a new infrastructure project"""
        # Calculate costs based on type and scale
        base_costs = self._calculate_project_costs(infra_type, scale)
        benefits = self._calculate_project_benefits(infra_type, scale)
        
        project = InfrastructureProject(
            project_id=f"project_{len(self.projects)}",
            name=f"{infra_type.value.capitalize()} Expansion",
            infra_type=infra_type,
            location=self.entity_id,
            construction_cost=base_costs,
            maintenance_cost={k: v * 0.1 for k, v in base_costs.items()},  # 10% maintenance
            construction_time=int(scale * 10),
            economic_benefits=benefits
        )
        
        self.projects.append(project)
        return project
    
    def _calculate_project_costs(self, infra_type: InfrastructureType, scale: float) -> Dict[ResourceType, float]:
        """Calculate construction costs for a project"""
        base_costs = {
            InfrastructureType.TRANSPORTATION: {ResourceType.STONE: 1000, ResourceType.WOOD: 500, ResourceType.IRON: 200},
            InfrastructureType.ENERGY: {ResourceType.COAL: 800, ResourceType.IRON: 400, ResourceType.MACHINERY: 100},
            InfrastructureType.DIGITAL: {ResourceType.ELECTRONICS: 300, ResourceType.COPPER: 200, ResourceType.MACHINERY: 150},
            InfrastructureType.EDUCATION: {ResourceType.WOOD: 400, ResourceType.STONE: 300, ResourceType.FOOD: 200},
        }
        
        costs = base_costs.get(infra_type, {ResourceType.STONE: 500, ResourceType.WOOD: 300})
        return {res: amount * scale for res, amount in costs.items()}
    
    def _calculate_project_benefits(self, infra_type: InfrastructureType, scale: float) -> Dict[str, float]:
        """Calculate economic benefits of a project"""
        base_benefits = {
            InfrastructureType.TRANSPORTATION: {'trade_efficiency': 0.3, 'production_efficiency': 0.2},
            InfrastructureType.ENERGY: {'industrial_output': 0.4, 'quality_of_life': 0.2},
            InfrastructureType.DIGITAL: {'research_capacity': 0.5, 'information_efficiency': 0.6},
            InfrastructureType.EDUCATION: {'workforce_quality': 0.4, 'innovation_rate': 0.3},
        }
        
        benefits = base_benefits.get(infra_type, {'economic_growth': 0.1})
        return {k: v * scale for k, v in benefits.items()}
    
    def simulate_infrastructure_turn(self, available_resources: Dict[ResourceType, float], 
                                  revenue: float) -> Dict[str, Any]:
        """Simulate one turn of infrastructure management"""
        results = {
            'maintenance_performed': {},
            'projects_completed': [],
            'bottlenecks': [],
            'economic_impact': {}
        }
        
        # Perform maintenance
        maintenance_budget = revenue * self.maintenance_budget
        results['maintenance_performed'] = self._perform_maintenance(maintenance_budget)
        
        # Advance construction projects
        results['projects_completed'] = self._advance_projects(available_resources)
        
        # Identify bottlenecks
        results['bottlenecks'] = self.get_infrastructure_bottlenecks()
        
        # Calculate economic impact
        results['economic_impact'] = self._calculate_total_economic_impact()
        
        # Degrade all systems
        for system_list in self.systems.values():
            for system in system_list:
                system.degrade()
        
        return results
    
    def _perform_maintenance(self, budget: float) -> Dict[InfrastructureType, float]:
        """Perform maintenance on all systems within budget"""
        maintenance_performed = {}
        total_maintenance_needed = 0
        
        # Calculate total maintenance need
        for infra_type, systems in self.systems.items():
            for system in systems:
                total_maintenance_needed += system.maintenance_need
        
        if total_maintenance_needed == 0:
            return maintenance_performed
        
        # Allocate budget proportionally to need
        for infra_type, systems in self.systems.items():
            type_maintenance = 0
            for system in systems:
                maintenance_share = (system.maintenance_need / total_maintenance_needed) * budget
                actual_maintenance = system.perform_maintenance(maintenance_share)
                type_maintenance += actual_maintenance
            
            maintenance_performed[infra_type] = type_maintenance
        
        return maintenance_performed
    
    def _advance_projects(self, available_resources: Dict[ResourceType, float]) -> List[str]:
        """Advance construction projects and return completed project IDs"""
        completed_projects = []
        
        for project in self.projects[:]:
            if not project.active:
                # Check if we can start project
                can_start = True
                for resource, cost in project.construction_cost.items():
                    if available_resources.get(resource, 0) < cost * 0.1:  # Need 10% to start
                        can_start = False
                        break
                
                if can_start:
                    project.active = True
            
            if project.active and not project.completed:
                # Advance construction
                progress_increment = 1.0 / project.construction_time
                project.progress += progress_increment
                
                # Consume resources
                for resource, total_cost in project.construction_cost.items():
                    cost_per_turn = total_cost / project.construction_time
                    available_resources[resource] = max(0, available_resources.get(resource, 0) - cost_per_turn)
                
                # Check completion
                if project.progress >= 1.0:
                    project.completed = True
                    completed_projects.append(project.project_id)
                    
                    # Add new infrastructure system
                    new_system = InfrastructureSystem(
                        f"{self.entity_id}_{project.infra_type.value}_{len(self.systems.get(project.infra_type, [])) + 1}",
                        project.infra_type,
                        1000,  # Base capacity
                        project.location
                    )
                    
                    if project.infra_type not in self.systems:
                        self.systems[project.infra_type] = []
                    self.systems[project.infra_type].append(new_system)
        
        # Remove completed projects
        self.projects = [p for p in self.projects if not p.completed]
        
        return completed_projects
    
    def _calculate_total_economic_impact(self) -> Dict[str, float]:
        """Calculate total economic impact of all infrastructure"""
        total_impact = {}
        
        for systems in self.systems.values():
            for system in systems:
                impact = system.calculate_economic_impact()
                for key, value in impact.items():
                    total_impact[key] = total_impact.get(key, 0) + value
        
        return total_impact