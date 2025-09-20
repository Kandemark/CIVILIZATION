"""Reproducible simulations"""

# Module implementation

# simulation_engine/deterministic_mode.py
import numpy as np
from typing import Optional, Dict, Any
import logging

logger = logging.getLogger(__name__)

class DeterministicMode:
    """
    Provides deterministic simulation mode for reproducible results.
    Useful for testing, debugging, and scenario replay.
    """
    
    def __init__(self):
        self.enabled = False
        self.seed = None
        self.random_state = None
        self.deterministic_functions = {}
        
    def enable(self, seed: int = 42) -> None:
        """Enable deterministic mode with a specific seed"""
        self.enabled = True
        self.seed = seed
        np.random.seed(seed)
        self.random_state = np.random.get_state()
        logger.info(f"Deterministic mode enabled with seed: {seed}")
        
    def disable(self) -> None:
        """Disable deterministic mode"""
        self.enabled = False
        self.seed = None
        self.random_state = None
        logger.info("Deterministic mode disabled")
        
    def save_state(self) -> Dict[str, Any]:
        """Save the current random state for later restoration"""
        return {
            'numpy_state': np.random.get_state(),
            'seed': self.seed,
            'enabled': self.enabled
        }
        
    def restore_state(self, state: Dict[str, Any]) -> None:
        """Restore a previously saved random state"""
        if state['enabled']:
            np.random.set_state(state['numpy_state'])
            self.enabled = True
            self.seed = state['seed']
        else:
            self.disable()
            
    def register_deterministic_function(self, func_name: str, func: callable) -> None:
        """Register a function to be made deterministic"""
        self.deterministic_functions[func_name] = func
        
    def get_deterministic_variant(self, base_value: float, variation: float = 0.1) -> float:
        """
        Get a deterministic variant of a value within specified bounds
        Useful for creating predictable but varied outcomes
        """
        if not self.enabled:
            return base_value * (1 + np.random.uniform(-variation, variation))
        
        # Use hash of current state for deterministic but varied results
        state_hash = hash(str(self.random_state))
        variation_factor = (state_hash % 1000) / 1000  # 0.0 to 1.0
        return base_value * (1 + variation * (variation_factor - 0.5) * 2)