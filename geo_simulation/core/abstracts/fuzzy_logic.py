"""Fuzzy logic system for vague concepts"""

# Module implementation

"""
Fuzzy logic system for handling vague concepts and gradual transitions.
"""

from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass
import numpy as np

@dataclass
class FuzzySet:
    """Represents a fuzzy set with membership function."""
    name: str
    a: float  # Left boundary
    b: float  # Peak start
    c: float  # Peak end
    d: float  # Right boundary
    
    def membership(self, x: float) -> float:
        """Calculate membership value for input x."""
        if x <= self.a or x >= self.d:
            return 0.0
        elif self.a < x <= self.b:
            return (x - self.a) / (self.b - self.a) if self.b != self.a else 1.0
        elif self.b < x <= self.c:
            return 1.0
        elif self.c < x < self.d:
            return (self.d - x) / (self.d - self.c) if self.d != self.c else 1.0
        return 0.0

class FuzzyVariable:
    """A fuzzy variable containing multiple fuzzy sets."""
    
    def __init__(self, name: str):
        self.name = name
        self.sets: Dict[str, FuzzySet] = {}
    
    def add_set(self, fuzzy_set: FuzzySet):
        """Add a fuzzy set to the variable."""
        self.sets[fuzzy_set.name] = fuzzy_set
    
    def fuzzify(self, x: float) -> Dict[str, float]:
        """Calculate membership values for all sets."""
        return {name: f_set.membership(x) for name, f_set in self.sets.items()}
    
    def defuzzify(self, membership_values: Dict[str, float], method: str = "centroid") -> float:
        """Convert fuzzy values back to crisp value."""
        if method == "centroid":
            return self._defuzzify_centroid(membership_values)
        elif method == "max":
            return self._defuzzify_max(membership_values)
        else:
            raise ValueError(f"Unknown defuzzification method: {method}")
    
    def _defuzzify_centroid(self, membership_values: Dict[str, float]) -> float:
        """Centroid defuzzification method."""
        total_area = 0
        weighted_sum = 0
        
        for name, membership in membership_values.items():
            if membership > 0:
                f_set = self.sets[name]
                # Simplified centroid calculation
                set_center = (f_set.b + f_set.c) / 2
                weighted_sum += set_center * membership
                total_area += membership
        
        return weighted_sum / total_area if total_area > 0 else 0
    
    def _defuzzify_max(self, membership_values: Dict[str, float]) -> float:
        """Maximum defuzzification method."""
        if not membership_values:
            return 0
        
        max_membership = max(membership_values.values())
        max_sets = [name for name, mem in membership_values.items() if mem == max_membership]
        
        # Return average of centers of sets with maximum membership
        centers = []
        for name in max_sets:
            f_set = self.sets[name]
            centers.append((f_set.b + f_set.c) / 2)
        
        return sum(centers) / len(centers)

class FuzzyRule:
    """A fuzzy rule for the inference system."""
    
    def __init__(self, antecedent: Dict[str, str], consequent: Dict[str, str], weight: float = 1.0):
        self.antecedent = antecedent  # {variable_name: set_name}
        self.consequent = consequent  # {variable_name: set_name}
        self.weight = weight
    
    def evaluate(self, inputs: Dict[str, float], variables: Dict[str, FuzzyVariable]) -> Dict[str, float]:
        """Evaluate the rule against input values."""
        # Calculate antecedent membership (AND operation using minimum)
        antecedent_strength = 1.0
        for var_name, set_name in self.antecedent.items():
            if var_name in inputs and var_name in variables:
                membership = variables[var_name].sets[set_name].membership(inputs[var_name])
                antecedent_strength = min(antecedent_strength, membership)
        
        # Apply rule weight
        rule_strength = antecedent_strength * self.weight
        
        # Return consequent with applied strength
        return {var_name: rule_strength for var_name in self.consequent.keys()}

class FuzzyInferenceSystem:
    """Complete fuzzy inference system."""
    
    def __init__(self):
        self.variables: Dict[str, FuzzyVariable] = {}
        self.rules: List[FuzzyRule] = []
    
    def add_variable(self, variable: FuzzyVariable):
        """Add a fuzzy variable to the system."""
        self.variables[variable.name] = variable
    
    def add_rule(self, rule: FuzzyRule):
        """Add a fuzzy rule to the system."""
        self.rules.append(rule)
    
    def infer(self, inputs: Dict[str, float]) -> Dict[str, float]:
        """Perform fuzzy inference on input values."""
        # Initialize output membership values
        output_membership = {var_name: {} for var_name in self.variables.keys()}
        
        # Evaluate all rules
        for rule in self.rules:
            rule_output = rule.evaluate(inputs, self.variables)
            
            for var_name, strength in rule_output.items():
                consequent_set = rule.consequent[var_name]
                if consequent_set not in output_membership[var_name]:
                    output_membership[var_name][consequent_set] = 0
                # OR operation using maximum
                output_membership[var_name][consequent_set] = max(
                    output_membership[var_name][consequent_set], strength
                )
        
        # Defuzzify outputs
        results = {}
        for var_name, memberships in output_membership.items():
            if var_name in self.variables and memberships:
                results[var_name] = self.variables[var_name].defuzzify(memberships)
        
        return results

# Example usage
def create_happiness_fuzzy_system() -> FuzzyInferenceSystem:
    """Create a fuzzy system for happiness assessment."""
    system = FuzzyInferenceSystem()
    
    # Create input variables
    economy = FuzzyVariable("economy")
    economy.add_set(FuzzySet("poor", 0.0, 0.0, 0.3, 0.5))
    economy.add_set(FuzzySet("average", 0.3, 0.5, 0.7, 0.9))
    economy.add_set(FuzzySet("strong", 0.6, 0.8, 1.0, 1.0))
    
    safety = FuzzyVariable("safety")
    safety.add_set(FuzzySet("low", 0.0, 0.0, 0.4, 0.6))
    safety.add_set(FuzzySet("medium", 0.4, 0.6, 0.8, 0.9))
    safety.add_set(FuzzySet("high", 0.7, 0.85, 1.0, 1.0))
    
    # Create output variable
    happiness = FuzzyVariable("happiness")
    happiness.add_set(FuzzySet("low", 0.0, 0.0, 0.3, 0.5))
    happiness.add_set(FuzzySet("medium", 0.3, 0.5, 0.7, 0.9))
    happiness.add_set(FuzzySet("high", 0.6, 0.8, 1.0, 1.0))
    
    # Add variables to system
    system.add_variable(economy)
    system.add_variable(safety)
    system.add_variable(happiness)
    
    # Add rules
    rules = [
        FuzzyRule(
            {"economy": "poor", "safety": "low"}, 
            {"happiness": "low"}, 0.9
        ),
        FuzzyRule(
            {"economy": "poor", "safety": "medium"}, 
            {"happiness": "low"}, 0.7
        ),
        FuzzyRule(
            {"economy": "average", "safety": "medium"}, 
            {"happiness": "medium"}, 0.8
        ),
        FuzzyRule(
            {"economy": "strong", "safety": "high"}, 
            {"happiness": "high"}, 0.9
        ),
        FuzzyRule(
            {"economy": "strong", "safety": "medium"}, 
            {"happiness": "high"}, 0.7
        ),
    ]
    
    for rule in rules:
        system.add_rule(rule)
    
    return system
