"""System dynamics modeling"""

# Module implementation

"""
Systems Dynamics for Complex System Modeling

Implements system dynamics modeling with stocks, flows, and feedback loops.
"""

from typing import Dict, List, Optional, Tuple, Callable, Any
from dataclasses import dataclass, field
import numpy as np
# from scipy.integrate import odeint
import logging

logger = logging.getLogger(__name__)


@dataclass
class Stock:
    """A stock (accumulation) in a system dynamics model."""
    name: str
    initial_value: float
    current_value: float = 0.0
    min_value: float = 0.0
    max_value: float = float('inf')
    units: str = "units"
    
    def __post_init__(self):
        self.current_value = self.initial_value
    
    def update(self, net_flow: float, dt: float) -> None:
        """Update stock value based on net flow over time dt."""
        new_value = self.current_value + net_flow * dt
        self.current_value = max(self.min_value, min(self.max_value, new_value))


@dataclass
class Flow:
    """A flow (rate) between stocks in a system dynamics model."""
    name: str
    source: Optional[str] = None  # Source stock name
    target: Optional[str] = None  # Target stock name
    rate_function: Optional[Callable] = None  # Function that calculates flow rate
    constant_rate: float = 0.0  # Constant flow rate (if no function)
    
    def calculate_rate(self, stocks: Dict[str, Stock], time: float) -> float:
        """Calculate current flow rate."""
        if self.rate_function:
            return self.rate_function(stocks, time)
        else:
            return self.constant_rate


@dataclass
class FeedbackLoop:
    """A feedback loop in a system dynamics model."""
    name: str
    loop_type: str  # "reinforcing" or "balancing"
    stocks_involved: List[str]
    strength: float = 1.0  # 0-1.0
    description: str = ""


@dataclass
class SystemDynamics:
    """A system dynamics model with stocks and flows."""
    name: str
    stocks: Dict[str, Stock] = field(default_factory=dict)
    flows: Dict[str, Flow] = field(default_factory=dict)
    feedback_loops: List[FeedbackLoop] = field(default_factory=list)
    time: float = 0.0
    history: Dict[str, List[float]] = field(default_factory=dict)
    
    def add_stock(self, stock: Stock) -> None:
        """Add a stock to the model."""
        self.stocks[stock.name] = stock
        self.history[stock.name] = [stock.initial_value]
    
    def add_flow(self, flow: Flow) -> None:
        """Add a flow to the model."""
        self.flows[flow.name] = flow
    
    def add_feedback_loop(self, loop: FeedbackLoop) -> None:
        """Add a feedback loop to the model."""
        self.feedback_loops.append(loop)
    
    def calculate_net_flows(self, time: float) -> Dict[str, float]:
        """Calculate net flows for all stocks."""
        net_flows = {stock_name: 0.0 for stock_name in self.stocks}
        
        for flow in self.flows.values():
            rate = flow.calculate_rate(self.stocks, time)
            
            if flow.source:
                net_flows[flow.source] -= rate
            if flow.target:
                net_flows[flow.target] += rate
        
        return net_flows
    
    def step(self, dt: float) -> None:
        """Advance the simulation by time step dt."""
        net_flows = self.calculate_net_flows(self.time)
        
        # Update stocks
        for stock_name, stock in self.stocks.items():
            stock.update(net_flows[stock_name], dt)
            self.history[stock_name].append(stock.current_value)
        
        self.time += dt
    
    def simulate(self, duration: float, dt: float = 1.0) -> None:
        """Run simulation for specified duration."""
        num_steps = int(duration / dt)
        
        for step in range(num_steps):
            self.step(dt)
    
    def get_equilibrium(self, tolerance: float = 1e-6, max_iterations: int = 1000) -> Optional[Dict[str, float]]:
        """Find equilibrium state of the system."""
        original_state = {name: stock.current_value for name, stock in self.stocks.items()}
        
        for iteration in range(max_iterations):
            # Store previous state
            previous_state = {name: stock.current_value for name, stock in self.stocks.items()}
            
            # Take a small step
            self.step(0.1)
            
            # Check for convergence
            max_change = max(abs(self.stocks[name].current_value - previous_state[name]) 
                           for name in self.stocks)
            
            if max_change < tolerance:
                equilibrium = {name: stock.current_value for name, stock in self.stocks.items()}
                
                # Restore original state
                for name, stock in self.stocks.items():
                    stock.current_value = original_state[name]
                
                return equilibrium
        
        # Restore original state if no convergence
        for name, stock in self.stocks.items():
            stock.current_value = original_state[name]
        
        return None
    
    def sensitivity_analysis(self, parameter_name: str, variations: List[float], 
                           duration: float = 100.0) -> Dict[float, Dict[str, List[float]]]:
        """Perform sensitivity analysis on a parameter."""
        results = {}
        original_flows = self.flows.copy()
        
        for variation in variations:
            # Modify the parameter in all relevant flows
            for flow_name, flow in self.flows.items():
                if hasattr(flow, parameter_name):
                    setattr(flow, parameter_name, variation)
            
            # Reset and run simulation
            self.reset()
            self.simulate(duration)
            
            # Store results
            results[variation] = {name: values.copy() for name, values in self.history.items()}
        
        # Restore original flows
        self.flows = original_flows
        self.reset()
        
        return results
    
    def reset(self) -> None:
        """Reset the model to initial conditions."""
        for stock in self.stocks.values():
            stock.current_value = stock.initial_value
        
        self.time = 0.0
        for stock_name in self.history:
            self.history[stock_name] = [self.stocks[stock_name].initial_value]
    
    def identify_feedback_loops(self) -> List[FeedbackLoop]:
        """Automatically identify feedback loops in the system."""
        # This is a simplified implementation
        # In a full implementation, this would use graph theory to find cycles
        
        loops = []
        
        # Look for simple two-stock balancing loops
        for flow1 in self.flows.values():
            for flow2 in self.flows.values():
                if (flow1.source == flow2.target and flow1.target == flow2.source):
                    loop = FeedbackLoop(
                        name=f"Balancing_{flow1.source}_{flow1.target}",
                        loop_type="balancing",
                        stocks_involved=[flow1.source, flow1.target],
                        strength=0.5
                    )
                    loops.append(loop)
        
        return loops


# Example flow rate functions
def linear_flow(stocks: Dict[str, Stock], time: float) -> float:
    """Linear flow rate based on source stock."""
    # Example implementation
    if 'population' in stocks:
        return stocks['population'].current_value * 0.1
    return 0.0


def logistic_flow(stocks: Dict[str, Stock], time: float) -> float:
    """Logistic flow rate with carrying capacity."""
    if 'population' in stocks and 'carrying_capacity' in stocks:
        pop = stocks['population'].current_value
        capacity = stocks['carrying_capacity'].current_value
        return pop * (1 - pop / capacity) * 0.1
    return 0.0


def delayed_flow(stocks: Dict[str, Stock], time: float, delay: float = 10.0) -> Callable:
    """Create a flow with time delay."""
    history = []
    
    def flow_function(stocks: Dict[str, Stock], current_time: float) -> float:
        nonlocal history
        
        # Store current value
        if 'source_stock' in stocks:
            current_value = stocks['source_stock'].current_value
            history.append((current_time, current_value))
            
            # Remove old entries
            history = [(t, v) for t, v in history if current_time - t <= delay]
            
            # Find delayed value (simplified)
            if history:
                # Use oldest value in history as delayed value
                delayed_value = history[0][1]
                return delayed_value * 0.1
        
        return 0.0
    
    return flow_function