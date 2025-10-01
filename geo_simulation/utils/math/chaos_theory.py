"""Chaos theory utilities"""

# Module implementation

"""
Chaos Theory Utilities for Complex System Simulation

Implements chaotic systems and sensitivity analysis for realistic simulation behavior.
"""

import numpy as np
from typing import Dict, List, Optional, Tuple, Callable
from dataclasses import dataclass, field
import logging
import random
from collections import defaultdict

logger = logging.getLogger(__name__)


@dataclass
class ChaosSystem:
    """A chaotic system for generating complex, deterministic-but-unpredictable behavior."""
    name: str
    equations: List[Callable]
    initial_conditions: List[float]
    parameters: Dict[str, float]
    state: List[float] = field(default_factory=list)
    history: List[List[float]] = field(default_factory=list)
    
    def __post_init__(self):
        self.state = self.initial_conditions.copy()
        self.history = [self.initial_conditions.copy()]
    
    def iterate(self, steps: int = 1) -> List[float]:
        """Iterate the chaotic system forward in time."""
        for _ in range(steps):
            new_state = []
            for eq in self.equations:
                new_state.append(eq(self.state, self.parameters))
            self.state = new_state
            self.history.append(new_state.copy())
        
        return self.state
    
    def sensitivity_analysis(self, parameter: str, variations: List[float]) -> Dict[float, List[float]]:
        """Analyze sensitivity to parameter changes."""
        results = {}
        original_value = self.parameters[parameter]
        
        for variation in variations:
            self.parameters[parameter] = variation
            self.state = self.initial_conditions.copy()
            self.history = [self.initial_conditions.copy()]
            
            # Run simulation
            final_states = []
            for _ in range(100):
                self.iterate()
                final_states.append(self.state.copy())
            
            results[variation] = final_states[-1]  # Final state
        
        # Restore original parameter
        self.parameters[parameter] = original_value
        self.state = self.initial_conditions.copy()
        self.history = [self.initial_conditions.copy()]
        
        return results
    
    def calculate_lyapunov_exponent(self, iterations: int = 1000) -> float:
        """Calculate the Lyapunov exponent to measure chaos."""
        if len(self.history) < 2:
            self.iterate(iterations)
        
        # Simplified Lyapunov exponent calculation
        divergences = []
        for i in range(1, len(self.history)):
            dist = np.linalg.norm(np.array(self.history[i]) - np.array(self.history[i-1]))
            if dist > 0:
                divergences.append(np.log(dist))
        
        return np.mean(divergences) if divergences else 0.0


class LorenzAttractor(ChaosSystem):
    """Lorenz attractor for atmospheric convection simulation."""
    
    def __init__(self, sigma: float = 10.0, rho: float = 28.0, beta: float = 8.0/3.0):
        equations = [
            lambda state, params: params['sigma'] * (state[1] - state[0]),
            lambda state, params: state[0] * (params['rho'] - state[2]) - state[1],
            lambda state, params: state[0] * state[1] - params['beta'] * state[2]
        ]
        
        super().__init__(
            name="Lorenz Attractor",
            equations=equations,
            initial_conditions=[1.0, 1.0, 1.0],
            parameters={'sigma': sigma, 'rho': rho, 'beta': beta}
        )


class LogisticMap(ChaosSystem):
    """Logistic map for population growth simulation."""
    
    def __init__(self, r: float = 3.9, x0: float = 0.5):
        equations = [
            lambda state, params: params['r'] * state[0] * (1 - state[0])
        ]
        
        super().__init__(
            name="Logistic Map",
            equations=equations,
            initial_conditions=[x0],
            parameters={'r': r}
        )
    
    def find_bifurcation_points(self, r_range: Tuple[float, float] = (2.5, 4.0), steps: int = 1000) -> Dict[float, List[float]]:
        """Find bifurcation points in the logistic map."""
        r_values = np.linspace(r_range[0], r_range[1], steps)
        bifurcation_data = {}
        
        for r in r_values:
            self.parameters['r'] = r
            self.state = [0.5]  # Reset to initial condition
            
            # Transient iterations
            for _ in range(1000):
                self.iterate()
            
            # Collect attractor points
            attractor = set()
            for _ in range(100):
                self.iterate()
                attractor.add(round(self.state[0], 6))
            
            bifurcation_data[r] = sorted(attractor)
        
        return bifurcation_data