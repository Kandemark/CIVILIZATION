"""Coordinates system updates"""

# Module implementation

# simulation_engine/system_orchestrator.py
from typing import Dict, List, Any, Optional
from dataclasses import dataclass
import logging
import time
from concurrent.futures import ThreadPoolExecutor, as_completed

logger = logging.getLogger(__name__)

@dataclass
class SystemStatus:
    name: str
    enabled: bool
    health: float  # 0.0 to 1.0
    last_update: float
    update_count: int

class SystemOrchestrator:
    """
    Coordinates and manages all simulation systems, handling dependencies and execution order.
    """
    
    def __init__(self):
        self.systems: Dict[str, Any] = {}
        self.system_dependencies: Dict[str, List[str]] = {}
        self.system_status: Dict[str, SystemStatus] = {}
        self.execution_order: List[str] = []
        self.parallel_execution = False
        self.max_workers = 4
        
    def register_system(self, name: str, system: Any, dependencies: List[str] = None) -> None:
        """Register a system with optional dependencies"""
        self.systems[name] = system
        self.system_dependencies[name] = dependencies or []
        
        # Initialize status
        self.system_status[name] = SystemStatus(
            name=name,
            enabled=True,
            health=1.0,
            last_update=0.0,
            update_count=0
        )
        
        logger.info(f"Registered system: {name}")
        
    def unregister_system(self, name: str) -> None:
        """Unregister a system"""
        if name in self.systems:
            del self.systems[name]
            del self.system_dependencies[name]
            del self.system_status[name]
            logger.info(f"Unregistered system: {name}")
            
    def calculate_execution_order(self) -> List[str]:
        """Calculate the optimal execution order based on dependencies"""
        # Simple topological sort (in a real implementation, use a proper algorithm)
        order = []
        remaining = list(self.systems.keys())
        
        while remaining:
            # Find systems with all dependencies satisfied
            ready = []
            for system in remaining:
                deps = self.system_dependencies[system]
                if all(dep in order for dep in deps):
                    ready.append(system)
                    
            if not ready:
                logger.error("Circular dependency detected!")
                break
                
            order.extend(ready)
            remaining = [s for s in remaining if s not in ready]
            
        self.execution_order = order
        return order
        
    def update_systems(self, delta_time: float) -> Dict[str, Any]:
        """Update all registered systems"""
        results = {}
        
        if self.parallel_execution:
            results = self._update_parallel(delta_time)
        else:
            results = self._update_sequential(delta_time)
            
        return results
        
    def _update_sequential(self, delta_time: float) -> Dict[str, Any]:
        """Update systems sequentially in calculated order"""
        results = {}
        
        for system_name in self.execution_order:
            if not self.system_status[system_name].enabled:
                continue
                
            try:
                start_time = time.time()
                
                # Call the system's update method if it exists
                system = self.systems[system_name]
                if hasattr(system, 'update'):
                    result = system.update(delta_time)
                    results[system_name] = result
                elif hasattr(system, 'tick'):
                    result = system.tick(delta_time)
                    results[system_name] = result
                    
                # Update status
                update_time = time.time() - start_time
                self.system_status[system_name].last_update = update_time
                self.system_status[system_name].update_count += 1
                
            except Exception as e:
                logger.error(f"Error updating system {system_name}: {e}")
                self.system_status[system_name].health = max(0.0, self.system_status[system_name].health - 0.1)
                
        return results
        
    def _update_parallel(self, delta_time: float) -> Dict[str, Any]:
        """Update systems in parallel where possible"""
        results = {}
        
        with ThreadPoolExecutor(max_workers=self.max_workers) as executor:
            # Create future for each system
            future_to_system = {}
            for system_name in self.execution_order:
                if not self.system_status[system_name].enabled:
                    continue
                    
                system = self.systems[system_name]
                if hasattr(system, 'update'):
                    future = executor.submit(system.update, delta_time)
                    future_to_system[future] = system_name
                elif hasattr(system, 'tick'):
                    future = executor.submit(system.tick, delta_time)
                    future_to_system[future] = system_name
                    
            # Collect results
            for future in as_completed(future_to_system):
                system_name = future_to_system[future]
                try:
                    result = future.result()
                    results[system_name] = result
                    self.system_status[system_name].health = min(1.0, self.system_status[system_name].health + 0.01)
                except Exception as e:
                    logger.error(f"Error in parallel update for {system_name}: {e}")
                    self.system_status[system_name].health = max(0.0, self.system_status[system_name].health - 0.1)
                    
        return results
        
    def enable_system(self, system_name: str) -> bool:
        """Enable a system"""
        if system_name in self.system_status:
            self.system_status[system_name].enabled = True
            logger.info(f"Enabled system: {system_name}")
            return True
        return False
        
    def disable_system(self, system_name: str) -> bool:
        """Disable a system"""
        if system_name in self.system_status:
            self.system_status[system_name].enabled = False
            logger.info(f"Disabled system: {system_name}")
            return True
        return False
        
    def get_system_health(self, system_name: str) -> Optional[float]:
        """Get the health status of a system"""
        if system_name in self.system_status:
            return self.system_status[system_name].health
        return None
        
    def get_overall_health(self) -> float:
        """Get the overall health of all systems"""
        if not self.system_status:
            return 1.0
            
        total_health = sum(status.health for status in self.system_status.values())
        return total_health / len(self.system_status)
        
    def enable_parallel_execution(self, enabled: bool = True, max_workers: int = None) -> None:
        """Enable or disable parallel execution"""
        self.parallel_execution = enabled
        if max_workers is not None:
            self.max_workers = max_workers
        logger.info(f"Parallel execution {'enabled' if enabled else 'disabled'} with {self.max_workers} workers")