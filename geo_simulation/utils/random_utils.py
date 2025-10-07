"""Random utilities"""

# Module implementation

# utils/random_utils.py
import random
import numpy as np
from typing import List, Dict, Any, Optional, Union
from enum import Enum
import logging

logger = logging.getLogger(__name__)

class RandomGenerator:
    """Advanced random number generation with multiple distributions"""
    
    def __init__(self, seed: Optional[int] = None):
        self.seed = seed
        if seed is not None:
            random.seed(seed)
            np.random.seed(seed)
    
    def random_int(self, min_val: int, max_val: int) -> int:
        """Generate random integer in range [min_val, max_val]"""
        return random.randint(min_val, max_val)
    
    def random_float(self, min_val: float, max_val: float) -> float:
        """Generate random float in range [min_val, max_val]"""
        return random.uniform(min_val, max_val)
    
    def random_choice(self, items: List[Any]) -> Any:
        """Random choice from list"""
        return random.choice(items)
    
    def random_sample(self, items: List[Any], k: int) -> List[Any]:
        """Random sample without replacement"""
        return random.sample(items, k)
    
    def shuffle(self, items: List[Any]) -> List[Any]:
        """Shuffle list in place"""
        shuffled = items.copy()
        random.shuffle(shuffled)
        return shuffled
    
    def normal_distribution(self, mean: float, std_dev: float, size: int = 1) -> np.ndarray:
        """Generate values from normal distribution"""
        return np.random.normal(mean, std_dev, size)
    
    def exponential_distribution(self, scale: float, size: int = 1) -> np.ndarray:
        """Generate values from exponential distribution"""
        return np.random.exponential(scale, size)

class WeightedRandom:
    """Weighted random selection utilities"""
    
    @staticmethod
    def weighted_choice(items: List[Any], weights: List[float]) -> Any:
        """Weighted random choice"""
        if len(items) != len(weights):
            raise ValueError("Items and weights must have same length")
        
        total = sum(weights)
        if total == 0:
            return random.choice(items)
        
        r = random.uniform(0, total)
        current = 0
        
        for item, weight in zip(items, weights):
            current += weight
            if r <= current:
                return item
        
        return items[-1]  # Fallback
    
    @staticmethod
    def weighted_sample(items: List[Any], weights: List[float], k: int) -> List[Any]:
        """Weighted random sample without replacement"""
        if len(items) != len(weights):
            raise ValueError("Items and weights must have same length")
        
        if k > len(items):
            raise ValueError("Sample size cannot exceed population size")
        
        # Simple implementation using repeated weighted choice
        sampled = []
        remaining_items = items.copy()
        remaining_weights = weights.copy()
        
        for _ in range(k):
            if not remaining_items:
                break
            
            choice = WeightedRandom.weighted_choice(remaining_items, remaining_weights)
            index = remaining_items.index(choice)
            
            sampled.append(choice)
            remaining_items.pop(index)
            remaining_weights.pop(index)
        
        return sampled
    
    @staticmethod
    def normalize_weights(weights: List[float]) -> List[float]:
        """Normalize weights to sum to 1"""
        total = sum(weights)
        if total == 0:
            return [1.0 / len(weights)] * len(weights)
        return [w / total for w in weights]

class SeededRandom:
    """Seeded random number generator for reproducible results"""
    
    def __init__(self, seed: int):
        self.seed = seed
        self.random_state = random.getstate()
        self.np_random_state = np.random.get_state()
        
        random.seed(seed)
        np.random.seed(seed)
    
    def save_state(self) -> Dict[str, Any]:
        """Save current random state"""
        return {
            'random_state': random.getstate(),
            'np_random_state': np.random.get_state()
        }
    
    def restore_state(self, state: Dict[str, Any]):
        """Restore random state"""
        random.setstate(state['random_state'])
        np.random.set_state(state['np_random_state'])
    
    def reset(self):
        """Reset to initial seed"""
        random.seed(self.seed)
        np.random.seed(self.seed)

# Specialized random utilities
class RandomUtils:
    """Specialized random utilities for simulations"""
    
    @staticmethod
    def random_point_in_circle(center_x: float, center_y: float, 
                              radius: float) -> tuple[float, float]:
        """Generate random point within circle"""
        angle = random.uniform(0, 2 * np.pi)
        r = radius * np.sqrt(random.uniform(0, 1))
        x = center_x + r * np.cos(angle)
        y = center_y + r * np.sin(angle)
        return x, y
    
    @staticmethod
    def random_point_in_rectangle(min_x: float, min_y: float, max_x: float, max_y: float) -> tuple[float, float]:
        """Generate random point within rectangle"""
        x = random.uniform(min_x, max_x)
        y = random.uniform(min_y, max_y)
        return x, y
    
    @staticmethod
    def random_color() -> str:
        """Generate random hex color"""
        return f"#{random.randint(0, 0xFFFFFF):06x}"
    
    @staticmethod
    def random_name() -> str:
        """Generate random name"""
        syllables = ['ba', 'be', 'bi', 'bo', 'bu', 'ca', 'ce', 'ci', 'co', 'cu',
                    'da', 'de', 'di', 'do', 'du', 'fa', 'fe', 'fi', 'fo', 'fu']
        name_length = random.randint(2, 4)
        name = ''.join(random.choice(syllables) for _ in range(name_length))
        return name.capitalize()