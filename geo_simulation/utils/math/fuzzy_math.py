"""Fuzzy math operations"""

# Module implementation

"""
Fuzzy Mathematics for Uncertainty Modeling

Implements fuzzy sets, fuzzy logic, and approximate reasoning for simulation uncertainty.
"""

from typing import Dict, List, Optional, Tuple, Union, Callable, DefaultDict
from dataclasses import dataclass, field
import numpy as np
import logging

logger = logging.getLogger(__name__)


@dataclass
class FuzzySet:
    """A fuzzy set with membership function."""
    name: str
    universe: Tuple[float, float]
    membership_function: Callable[[float], float]
    values: Dict[float, float] = field(default_factory=dict)
    
    def __post_init__(self):
        """Initialize the fuzzy set by sampling the membership function."""
        min_val, max_val = self.universe
        samples = np.linspace(min_val, max_val, 100)
        self.values = {x: self.membership_function(x) for x in samples}
    
    def membership(self, x: float) -> float:
        """Get membership degree of x in the fuzzy set."""
        x = max(self.universe[0], min(self.universe[1], x))
        
        # Find closest sampled points for interpolation
        sampled_x = np.array(list(self.values.keys()))
        sampled_y = np.array(list(self.values.values()))
        
        # Linear interpolation
        idx = np.searchsorted(sampled_x, x)
        if idx == 0:
            return sampled_y[0]
        elif idx == len(sampled_x):
            return sampled_y[-1]
        else:
            x0, x1 = sampled_x[idx-1], sampled_x[idx]
            y0, y1 = sampled_y[idx-1], sampled_y[idx]
            return y0 + (y1 - y0) * (x - x0) / (x1 - x0)
    
    def alpha_cut(self, alpha: float) -> List[float]:
        """Get alpha-cut of the fuzzy set."""
        return [x for x, mu in self.values.items() if mu >= alpha]
    
    def centroid(self) -> float:
        """Calculate centroid (center of gravity) of the fuzzy set."""
        x_vals = np.array(list(self.values.keys()))
        mu_vals = np.array(list(self.values.values()))
        
        if np.sum(mu_vals) == 0:
            return np.mean(x_vals)
        
        return np.sum(x_vals * mu_vals) / np.sum(mu_vals)
    
    def union(self, other: 'FuzzySet') -> 'FuzzySet':
        """Union of two fuzzy sets."""
        if self.universe != other.universe:
            raise ValueError("Fuzzy sets must have the same universe")
        
        def union_membership(x):
            return max(self.membership(x), other.membership(x))
        
        return FuzzySet(
            name=f"{self.name}_union_{other.name}",
            universe=self.universe,
            membership_function=union_membership
        )
    
    def intersection(self, other: 'FuzzySet') -> 'FuzzySet':
        """Intersection of two fuzzy sets."""
        if self.universe != other.universe:
            raise ValueError("Fuzzy sets must have the same universe")
        
        def intersection_membership(x):
            return min(self.membership(x), other.membership(x))
        
        return FuzzySet(
            name=f"{self.name}_intersection_{other.name}",
            universe=self.universe,
            membership_function=intersection_membership
        )


@dataclass
class FuzzyVariable:
    """A linguistic variable with multiple fuzzy sets."""
    name: str
    universe: Tuple[float, float]
    terms: Dict[str, FuzzySet] = field(default_factory=dict)
    
    def add_term(self, term_name: str, membership_function: Callable[[float], float]) -> None:
        """Add a linguistic term to the variable."""
        self.terms[term_name] = FuzzySet(
            name=term_name,
            universe=self.universe,
            membership_function=membership_function
        )
    
    def fuzzify(self, value: float) -> Dict[str, float]:
        """Fuzzify a crisp value into membership degrees."""
        return {term: fs.membership(value) for term, fs in self.terms.items()}
    
    def defuzzify(self, membership_values: Dict[str, float], method: str = 'centroid') -> float:
        """Defuzzify membership values to a crisp value."""
        if method == 'centroid':
            # Weighted average of centroids
            total_weight = 0.0
            weighted_sum = 0.0
            
            for term, mu in membership_values.items():
                if term in self.terms and mu > 0:
                    centroid = self.terms[term].centroid()
                    weighted_sum += centroid * mu
                    total_weight += mu
            
            return weighted_sum / total_weight if total_weight > 0 else np.mean(self.universe)
        
        elif method == 'max_membership':
            # Use the term with maximum membership
            max_term = max(membership_values.items(), key=lambda x: x[1])
            return self.terms[max_term[0]].centroid()
        
        else:
            raise ValueError(f"Unknown defuzzification method: {method}")


class FuzzyLogic:
    """Fuzzy logic inference system."""
    
    def __init__(self):
        self.rules: List[Dict] = []
        self.variables: Dict[str, FuzzyVariable] = {}
    
    def add_variable(self, variable: FuzzyVariable) -> None:
        """Add a fuzzy variable to the system."""
        self.variables[variable.name] = variable
    
    def add_rule(self, antecedents: Dict[str, str], consequent: Dict[str, str]) -> None:
        """Add a fuzzy rule."""
        self.rules.append({
            'if': antecedents,  # {variable_name: term_name}
            'then': consequent  # {variable_name: term_name}
        })
    
    def infer(self, inputs: Dict[str, float]) -> Dict[str, float]:
        """Perform fuzzy inference."""
        # Step 1: Fuzzify inputs
        fuzzified_inputs = {}
        for var_name, value in inputs.items():
            if var_name in self.variables:
                fuzzified_inputs[var_name] = self.variables[var_name].fuzzify(value)
        
        # Step 2: Apply rules
        rule_strengths = []
        consequent_memberships = defaultdict(lambda: defaultdict(float))
        
        for rule in self.rules:
            # Calculate rule strength (min of antecedent memberships)
            strength = 1.0
            for var_name, term_name in rule['if'].items():
                if var_name in fuzzified_inputs and term_name in fuzzified_inputs[var_name]:
                    strength = min(strength, fuzzified_inputs[var_name][term_name])
                else:
                    strength = 0.0
                    break
            
            if strength > 0:
                rule_strengths.append(strength)
                # Apply to consequent
                for var_name, term_name in rule['then'].items():
                    consequent_memberships[var_name][term_name] = max(
                        consequent_memberships[var_name][term_name],
                        strength
                    )
        
        # Step 3: Defuzzify outputs
        outputs = {}
        for var_name, memberships in consequent_memberships.items():
            if var_name in self.variables:
                outputs[var_name] = self.variables[var_name].defuzzify(memberships)
        
        return outputs


# Common membership functions
def triangular_mf(a: float, b: float, c: float) -> Callable[[float], float]:
    """Create a triangular membership function."""
    def mf(x):
        if x <= a or x >= c:
            return 0.0
        elif a < x <= b:
            return (x - a) / (b - a)
        else:  # b < x < c
            return (c - x) / (c - b)
    return mf


def trapezoidal_mf(a: float, b: float, c: float, d: float) -> Callable[[float], float]:
    """Create a trapezoidal membership function."""
    def mf(x):
        if x <= a or x >= d:
            return 0.0
        elif a < x < b:
            return (x - a) / (b - a)
        elif b <= x <= c:
            return 1.0
        else:  # c < x < d
            return (d - x) / (d - c)
    return mf


def gaussian_mf(mean: float, std: float) -> Callable[[float], float]:
    """Create a Gaussian membership function."""
    def mf(x):
        return np.exp(-0.5 * ((x - mean) / std) ** 2)
    return mf