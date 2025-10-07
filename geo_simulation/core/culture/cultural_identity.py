"""Cultural traits and values"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
import random
import math

class EmergentIdentityStrength(Enum):
    """Identity strength that emerges from cultural dynamics"""
    FLUID = "fluid"          # Constantly changing, highly adaptable
    FORMING = "forming"      # Developing consistency
    COHESIVE = "cohesive"    # Stable but open to evolution
    RESILIENT = "resilient"  # Strong but can adapt under pressure
    CRYSTALLIZED = "crystallized" # Very stable, resistant to change
    TRANSCENDENT = "transcendent" # Beyond normal identity constraints

@dataclass
class EmergentSymbol:
    """Cultural symbols that emerge organically"""
    symbol_id: str
    name: str
    origin_story: str  # How this symbol emerged
    cultural_resonance: float  # 0-∞, can grow infinitely
    recognition_level: float  # 0-1
    emotional_charge: float  # -1 to 1, negative to positive emotional weight
    complexity: float = 1.0  # Can become infinitely complex
    
    def evolve_symbol(self, cultural_development: float):
        """Evolve symbol over time"""
        # Symbols gain resonance with cultural development
        resonance_growth = cultural_development * 0.01
        self.cultural_resonance += resonance_growth
        
        # Recognition naturally increases but can decrease if neglected
        recognition_change = random.uniform(-0.02, 0.05)
        self.recognition_level = max(0.0, min(1.0, self.recognition_level + recognition_change))
        
        # Symbols can become more complex over time
        complexity_growth = cultural_development * 0.005
        self.complexity += complexity_growth

class InfiniteCulturalIdentity:
    """Cultural identity that evolves without predefined limits"""
    
    def __init__(self, culture_id: str, name: str, environmental_context: str):
        self.culture_id = culture_id
        self.name = name
        self.strength = EmergentIdentityStrength.FORMING
        self.cohesion = 0.5  # 0-1, but can be reinforced infinitely
        self.identity_complexity = 1.0  # Can grow without bound
        
        # Emergent identity components
        self.self_narratives: Dict[str, float] = {}  # narrative -> belief_strength
        self.external_reflections: Dict[str, Dict[str, float]] = {}  # other_culture -> perception
        self.symbols: Dict[str, EmergentSymbol] = {}
        self.founding_myths: List[Tuple[str, float]] = []  # (myth, significance)
        self.cultural_boundaries: Set[str] = set()  # What defines "us" vs "them"
        self.identity_anchors: Dict[str, float] = {}  # Core identity elements
        
        # Infinite scaling metrics
        self.narrative_depth = 1.0
        self.symbolic_richness = 1.0
        self.boundary_strength = 0.5
        
        # Generate initial identity from environment
        self._generate_environmental_identity(environmental_context)
    
    def _generate_environmental_identity(self, environmental_context: str):
        """Generate initial identity foundations from environment"""
        environmental_identities = {
            "coastal": ["sea_people", "navigators", "traders"],
            "mountain": ["high_people", "resilient_ones", "isolated_community"],
            "forest": ["forest_guardians", "nature_people", "hunter_gatherers"],
            "desert": ["dune_walkers", "water_seekers", "sun_children"],
            "plains": ["horizon_watchers", "nomads", "grassland_people"]
        }
        
        identity_elements = environmental_identities.get(environmental_context, ["emerging_people"])
        for element in identity_elements:
            self.self_narratives[element] = random.uniform(0.6, 0.9)
        
        # Create initial boundary
        self.cultural_boundaries.add("speaks_our_language")
        self.cultural_boundaries.add("shares_our_land")
    
    def evolve_identity(self, cultural_development: float, external_pressures: Dict[str, float]):
        """Evolve identity based on cultural development and external factors"""
        # Identity naturally becomes more complex
        self._increase_complexity(cultural_development)
        
        # Respond to external pressures
        self._pressure_response(external_pressures)
        
        # Evolve narratives and symbols
        self._evolve_narratives()
        self._evolve_symbols(cultural_development)
        
        # Update identity strength
        self._update_identity_strength()
        
        # Cultural boundaries can evolve
        self._evolve_boundaries()
    
    def _increase_complexity(self, cultural_development: float):
        """Increase identity complexity without upper bound"""
        complexity_growth = math.log(1.0 + cultural_development) * 0.1
        self.identity_complexity += complexity_growth
        
        # Narrative depth increases with complexity
        self.narrative_depth += complexity_growth * 0.5
        
        # Symbolic richness grows
        self.symbolic_richness += complexity_growth * 0.3
    
    def _pressure_response(self, external_pressures: Dict[str, float]):
        """Respond to external pressures on identity"""
        for pressure_type, intensity in external_pressures.items():
            if intensity > 0.3:
                if pressure_type == "assimilation_pressure":
                    self._respond_to_assimilation_pressure(intensity)
                elif pressure_type == "cultural_contact":
                    self._respond_to_cultural_contact(intensity)
                elif pressure_type == "existential_threat":
                    self._respond_to_existential_threat(intensity)
    
    def _respond_to_assimilation_pressure(self, intensity: float):
        """Respond to pressure to assimilate into other cultures"""
        if random.random() < intensity * 0.5:
            # Strengthen boundaries and identity markers
            new_boundary = f"resistance_to_{random.randint(1,100)}"
            self.cultural_boundaries.add(new_boundary)
            self.boundary_strength = min(1.0, self.boundary_strength + intensity * 0.2)
            
            # Create defensive narratives
            defensive_narrative = f"we_persist_against_odds_{random.randint(1,100)}"
            self.self_narratives[defensive_narrative] = intensity
    
    def _respond_to_cultural_contact(self, intensity: float):
        """Respond to contact with other cultures"""
        if random.random() < intensity * 0.3:
            # Identity can incorporate external elements
            incorporated_element = f"adopted_trait_{random.randint(1,100)}"
            self.self_narratives[incorporated_element] = intensity * 0.5
            
            # Boundaries might become more permeable
            if random.random() < 0.2:
                self.boundary_strength = max(0.1, self.boundary_strength - intensity * 0.1)
    
    def _respond_to_existential_threat(self, intensity: float):
        """Respond to existential threats to the culture"""
        if random.random() < intensity * 0.7:
            # Create survival narratives
            survival_narrative = f"survival_against_{random.randint(1,100)}"
            self.self_narratives[survival_narrative] = intensity
            
            # Strengthen cohesion dramatically
            self.cohesion = min(1.0, self.cohesion + intensity * 0.3)
            
            # Create powerful symbols of resistance
            if random.random() < 0.5:
                self._create_crisis_symbol(intensity)
    
    def _evolve_narratives(self):
        """Evolve cultural narratives over time"""
        # Narratives can strengthen, weaken, or transform
        for narrative, strength in list(self.self_narratives.items()):
            # Natural narrative drift
            drift = random.uniform(-0.02, 0.03)
            new_strength = max(0.0, min(1.0, strength + drift))
            self.self_narratives[narrative] = new_strength
            
            # Very weak narratives might be forgotten
            if new_strength < 0.1 and random.random() < 0.1:
                del self.self_narratives[narrative]
        
        # New narratives can emerge
        if random.random() < 0.05:
            new_narrative = f"emerging_belief_{random.randint(1000,9999)}"
            self.self_narratives[new_narrative] = random.uniform(0.2, 0.5)
    
    def _evolve_symbols(self, cultural_development: float):
        """Evolve cultural symbols"""
        for symbol in self.symbols.values():
            symbol.evolve_symbol(cultural_development)
            
            # Symbols can transform or gain new meanings
            if random.random() < 0.01:
                symbol.emotional_charge += random.uniform(-0.1, 0.1)
                symbol.emotional_charge = max(-1.0, min(1.0, symbol.emotional_charge))
        
        # New symbols can emerge
        if random.random() < 0.02:
            self._create_emergent_symbol()
    
    def _evolve_boundaries(self):
        """Evolve cultural boundaries"""
        # Boundaries can strengthen, weaken, or change
        boundary_change = random.uniform(-0.01, 0.02)
        self.boundary_strength = max(0.0, min(1.0, self.boundary_strength + boundary_change))
        
        # New boundaries can form
        if random.random() < 0.03:
            new_boundary = f"distinction_{random.randint(1000,9999)}"
            self.cultural_boundaries.add(new_boundary)
        
        # Old boundaries can dissolve
        if self.cultural_boundaries and random.random() < 0.02:
            boundary_to_remove = random.choice(list(self.cultural_boundaries))
            self.cultural_boundaries.remove(boundary_to_remove)
    
    def _update_identity_strength(self):
        """Update identity strength based on current state"""
        stability_score = self._calculate_identity_stability()
        
        if stability_score > 0.9 and self.identity_complexity > 50:
            self.strength = EmergentIdentityStrength.TRANSCENDENT
        elif stability_score > 0.8:
            self.strength = EmergentIdentityStrength.CRYSTALLIZED
        elif stability_score > 0.6:
            self.strength = EmergentIdentityStrength.RESILIENT
        elif stability_score > 0.4:
            self.strength = EmergentIdentityStrength.COHESIVE
        elif stability_score > 0.2:
            self.strength = EmergentIdentityStrength.FORMING
        else:
            self.strength = EmergentIdentityStrength.FLUID
    
    def _calculate_identity_stability(self) -> float:
        """Calculate how stable this identity is"""
        base_stability = self.cohesion
        
        # Narrative consistency contributes to stability
        narrative_strength = sum(self.self_narratives.values())
        narrative_consistency = min(0.3, narrative_strength * 0.1)
        
        # Symbolic resonance increases stability
        symbol_resonance = sum(symbol.cultural_resonance for symbol in self.symbols.values())
        symbol_stability = min(0.2, math.log(1.0 + symbol_resonance) * 0.05)
        
        # Boundary clarity affects stability
        boundary_clarity = self.boundary_strength * 0.2
        
        total_stability = base_stability + narrative_consistency + symbol_stability + boundary_clarity
        return min(1.0, total_stability)
    
    def _create_emergent_symbol(self):
        """Create a new emergent cultural symbol"""
        symbol_id = f"symbol_{len(self.symbols)}"
        
        symbol_types = ["animal", "element", "concept", "artifact", "location"]
        symbol_origins = [
            "dream_vision", "historical_event", "natural_phenomenon", 
            "artistic_inspiration", "philosophical_insight"
        ]
        
        symbol = EmergentSymbol(
            symbol_id=symbol_id,
            name=f"Symbol_{random.randint(100,999)}",
            origin_story=random.choice(symbol_origins),
            cultural_resonance=random.uniform(0.1, 0.5),
            recognition_level=0.1,
            emotional_charge=random.uniform(-0.3, 0.8)
        )
        
        self.symbols[symbol_id] = symbol
        return symbol
    
    def _create_crisis_symbol(self, intensity: float):
        """Create a symbol during cultural crisis"""
        symbol_id = f"crisis_symbol_{len(self.symbols)}"
        
        symbol = EmergentSymbol(
            symbol_id=symbol_id,
            name="Resilience_Emblem",
            origin_story="forged_in_crisis",
            cultural_resonance=intensity * 2.0,
            recognition_level=0.8,  # Crisis symbols gain quick recognition
            emotional_charge=0.9  # Highly positive emotional charge
        )
        
        self.symbols[symbol_id] = symbol
    
    def add_historical_memory(self, event: str, impact: float, is_positive: bool):
        """Add a historical memory to cultural identity"""
        memory_strength = impact * (1.0 + self.identity_complexity * 0.01)
        
        if is_positive:
            # Positive memories increase cohesion
            self.cohesion = min(1.0, self.cohesion + impact * 0.1)
            # Create triumphant narratives
            triumph_narrative = f"triumph_{event}_{random.randint(1,100)}"
            self.self_narratives[triumph_narrative] = memory_strength
        else:
            # Negative memories can create trauma but also resilience
            self.cohesion = max(0.0, self.cohesion - impact * 0.05)
            # Create narratives of survival
            survival_narrative = f"survival_{event}_{random.randint(1,100)}"
            self.self_narratives[survival_narrative] = memory_strength * 0.8
            
            # Very impactful events create permanent boundaries
            if impact > 0.7:
                self.cultural_boundaries.add(f"never_again_{event}")
    
    def promote_symbol(self, symbol_id: str, promotion_effort: float):
        """Increase recognition and resonance of a symbol"""
        if symbol_id in self.symbols:
            symbol = self.symbols[symbol_id]
            symbol.recognition_level = min(1.0, symbol.recognition_level + promotion_effort)
            symbol.cultural_resonance += promotion_effort * 10  # Significant boost
    
    def calculate_assimilation_resistance(self) -> float:
        """Calculate resistance to cultural assimilation"""
        base_resistance = self.cohesion * 0.3
        boundary_resistance = self.boundary_strength * 0.4
        narrative_resistance = len(self.self_narratives) * 0.01
        symbol_resistance = sum(s.cultural_resonance for s in self.symbols.values()) * 0.001
        
        total_resistance = (base_resistance + boundary_resistance + 
                          narrative_resistance + symbol_resistance)
        
        # Complex identities are harder to assimilate
        complexity_bonus = math.log(1.0 + self.identity_complexity) * 0.1
        
        return min(2.0, total_resistance + complexity_bonus)  # Cap at 2.0 for balance

class EmergentCulturalGroup:
    """A cultural group with emergent, infinite identity"""
    
    def __init__(self, group_id: str, name: str, environmental_context: str):
        self.id = group_id
        self.name = name
        self.identity = InfiniteCulturalIdentity(group_id, name, environmental_context)
        self.civilization_index = 0.0
        self.population_share = 1.0
        self.adaptability = random.uniform(0.3, 0.7)
        
        # Subcultural dynamics
        self.subcultures: Dict[str, 'EmergentCulturalGroup'] = {}
        self.parent_group: Optional['EmergentCulturalGroup'] = None
        
        # Innovation in identity formation
        self.identity_innovation_rate = random.uniform(0.2, 0.6)
    
    def evolve_group_identity(self, cultural_development: float, external_pressures: Dict[str, float]):
        """Evolve the group's cultural identity"""
        self.civilization_index = cultural_development
        
        # Evolve core identity
        self.identity.evolve_identity(cultural_development, external_pressures)
        
        # Identity innovation
        self._innovate_identity()
        
        # Subcultural dynamics
        self._evolve_subcultures(cultural_development)
    
    def _innovate_identity(self):
        """Innovate new identity elements"""
        innovation_chance = self.identity_innovation_rate * 0.01
        
        if random.random() < innovation_chance:
            # Create new narrative
            new_narrative = f"innovative_belief_{random.randint(10000,99999)}"
            self.identity.self_narratives[new_narrative] = random.uniform(0.3, 0.6)
        
        if random.random() < innovation_chance * 0.5:
            # Create new symbol
            self.identity._create_emergent_symbol()
    
    def _evolve_subcultures(self, cultural_development: float):
        """Evolve subcultures within the group"""
        # Subcultures can emerge naturally
        emergence_chance = cultural_development * 0.001
        
        if random.random() < emergence_chance and len(self.subcultures) < 10:  # Reasonable limit
            self._create_emergent_subculture()
        
        # Evolve existing subcultures
        for subculture in self.subcultures.values():
            subculture_pressures = {
                "assimilation_pressure": 0.1,  # Constant pressure from parent culture
                "cultural_contact": 0.2  # Contact with sibling subcultures
            }
            subculture.evolve_group_identity(cultural_development * 0.8, subculture_pressures)
    
    def _create_emergent_subculture(self):
        """Create a new emergent subculture"""
        subculture_id = f"subculture_{self.id}_{len(self.subcultures)}"
        subculture_name = f"{self.name}_Variant_{random.randint(1,100)}"
        
        subculture = EmergentCulturalGroup(subculture_id, subculture_name, "urban")  # Subcultures often urban
        
        # Subculture starts with parent's identity but differentiated
        subculture.identity.self_narratives = self.identity.self_narratives.copy()
        subculture.identity.cultural_boundaries = self.identity.cultural_boundaries.copy()
        
        # Apply differentiation
        differentiation = random.uniform(0.1, 0.4)
        for narrative in list(subculture.identity.self_narratives.keys()):
            change = random.uniform(-differentiation, differentiation)
            subculture.identity.self_narratives[narrative] = max(0.1, 
                min(1.0, subculture.identity.self_narratives[narrative] + change))
        
        # Add unique subcultural elements
        unique_narratives = random.randint(1, 2)
        for i in range(unique_narratives):
            unique_narrative = f"subcultural_focus_{random.randint(1000,9999)}"
            subculture.identity.self_narratives[unique_narrative] = random.uniform(0.5, 0.8)
        
        subculture.parent_group = self
        self.subcultures[subculture_id] = subculture
        
        return subculture
    
    def calculate_identity_coherence(self) -> float:
        """Calculate overall coherence of the cultural group's identity"""
        main_coherence = self.identity._calculate_identity_stability()
        
        # Subcultures affect overall coherence
        if self.subcultures:
            subculture_coherence = sum(sub.identity._calculate_identity_stability() 
                                     for sub in self.subcultures.values()) / len(self.subcultures)
            # Diversity can both strengthen and challenge coherence
            coherence_balance = (main_coherence + subculture_coherence) / 2
            diversity_effect = len(self.subcultures) * 0.05  # More subcultures = more complex identity
            return min(1.0, coherence_balance - diversity_effect * 0.1)
        
        return main_coherence