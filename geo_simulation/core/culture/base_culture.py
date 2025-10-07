from abc import ABC, abstractmethod
from typing import Dict, List, Set, Optional, Tuple
from enum import Enum
import random
import math

class DevelopmentPhase(Enum):
    """Phases instead of fixed stages - cultures can progress infinitely"""
    FORMATIVE = "formative"
    EXPANSIVE = "expansive" 
    INTEGRATIVE = "integrative"
    TRANSFORMATIVE = "transformative"
    TRANSCENDENT = "transcendent"
    # No final stage - can keep evolving forever

class EmergentCulturalEntity(ABC):
    """Base class for cultures that evolve without caps"""
    
    def __init__(self, entity_id: str, name: str, geographic_context: str, climate_zone: str):
        self.id = entity_id
        self.name = name
        self.development_phase = DevelopmentPhase.FORMATIVE
        self.civilization_index = 0.0  # Continuous measure of development
        self.emergent_traits: Dict[str, float] = {}
        self.generated_traditions: Set[str] = set()
        self.innovation_vectors: Dict[str, float] = {}  # Direction of cultural development
        
        # Infinite scaling metrics
        self.cultural_complexity = 1.0
        self.social_organization = 1.0
        self.knowledge_integration = 1.0
        self.artistic_expression = 1.0
        
        # Generate initial traits from environment
        self._generate_environmental_foundation(geographic_context, climate_zone)
        
        self.adaptability = random.uniform(0.3, 0.7)
        self.innovation_potential = random.uniform(0.2, 0.8)
        self.turn_created = 0
        
    def _generate_environmental_foundation(self, geographic_context: str, climate_zone: str):
        """Generate initial cultural foundation based on environment"""
        # Environmental influences create starting point, not limits
        if geographic_context == "coastal":
            self.innovation_vectors["maritime_development"] = 0.6
            self.emergent_traits["ocean_navigation"] = 0.5
        elif geographic_context == "mountain":
            self.innovation_vectors["vertical_engineering"] = 0.7
            self.emergent_traits["altitude_adaptation"] = 0.6
        elif geographic_context == "river_valley":
            self.innovation_vectors["hydraulic_engineering"] = 0.8
            self.emergent_traits["sedentary_agriculture"] = 0.7
            
        # Climate influences
        if climate_zone == "arctic":
            self.innovation_vectors["thermal_management"] = 0.9
        elif climate_zone == "tropical":
            self.innovation_vectors["biological_diversity_utilization"] = 0.7
        
    def advance_civilization_index(self, progress_factors: Dict[str, float]):
        """Advance civilization development without upper bound"""
        base_progress = 0.01
        
        # Each factor contributes to progress
        for factor, strength in progress_factors.items():
            progress_contribution = strength * 0.05
            base_progress += progress_contribution
            
            # Strong factors can create new innovation vectors
            if strength > 0.8 and random.random() < 0.1:
                new_vector = f"{factor}_breakthrough"
                self.innovation_vectors[new_vector] = random.uniform(0.3, 0.7)
        
        # Apply progress with diminishing returns but no hard cap
        current_index = self.civilization_index
        progress_multiplier = 1.0 / (1.0 + math.log(1.0 + current_index * 0.1))
        actual_progress = base_progress * progress_multiplier
        
        self.civilization_index += actual_progress
        
        # Check for phase transitions (not caps, just milestones)
        self._check_phase_transition()
        
        # Cultural metrics scale with civilization index
        self._scale_cultural_metrics()
    
    def _check_phase_transition(self):
        """Transition between development phases (milestones, not endpoints)"""
        old_phase = self.development_phase
        phases = list(DevelopmentPhase)
        
        phase_thresholds = [0, 10, 25, 50, 100, 200]  # Can add more thresholds infinitely
        
        for i, threshold in enumerate(reversed(phase_thresholds)):
            if self.civilization_index >= threshold and i < len(phases) - 1:
                new_phase_index = len(phases) - 1 - i
                self.development_phase = phases[new_phase_index]
                break
        
        if old_phase != self.development_phase:
            self._phase_transition_effects(old_phase, self.development_phase)
    
    def _phase_transition_effects(self, old_phase: DevelopmentPhase, new_phase: DevelopmentPhase):
        """Effects when transitioning between phases"""
        phase_innovations = {
            DevelopmentPhase.EXPANSIVE: ["territorial_organization", "trade_networks"],
            DevelopmentPhase.INTEGRATIVE: ["cultural_synthesis", "institutional_complexity"],
            DevelopmentPhase.TRANSFORMATIVE: ["paradigm_shifts", "transcendent_art"],
            DevelopmentPhase.TRANSCENDENT: ["cosmic_perspective", "reality_manipulation"]
        }
        
        innovations = phase_innovations.get(new_phase, [])
        for innovation in innovations:
            if random.random() < 0.7:
                self.innovation_vectors[innovation] = random.uniform(0.4, 0.8)
    
    def _scale_cultural_metrics(self):
        """Scale cultural metrics with civilization development"""
        # All metrics can grow infinitely with diminishing returns
        log_index = math.log(1.0 + self.civilization_index)
        
        self.cultural_complexity = 1.0 + log_index * 2.0
        self.social_organization = 1.0 + log_index * 1.5
        self.knowledge_integration = 1.0 + log_index * 2.5
        self.artistic_expression = 1.0 + log_index * 1.2
    
    def generate_cultural_innovation(self, domain: str, intensity: float) -> str:
        """Generate new cultural innovations in any domain"""
        innovation_id = f"innovation_{domain}_{random.randint(1000, 9999)}"
        
        # Innovation strength scales with civilization index
        base_strength = intensity * (1.0 + self.civilization_index * 0.01)
        
        # Create new trait or enhance existing one
        if domain in self.emergent_traits:
            # Enhance existing trait
            current = self.emergent_traits[domain]
            enhancement = base_strength * 0.1
            self.emergent_traits[domain] = min(1.0, current + enhancement)
        else:
            # Create new trait
            self.emergent_traits[domain] = base_strength
        
        return innovation_id
    
    @abstractmethod
    def evolve(self, current_turn: int, environmental_conditions: Dict[str, float]):
        """Evolve culture based on infinite timeline"""
        pass

class InfiniteCulture(EmergentCulturalEntity):
    """A culture that can evolve without any caps or endpoints"""
    
    def __init__(self, culture_id: str, name: str, geographic_context: str, climate_zone: str):
        super().__init__(culture_id, name, geographic_context, climate_zone)
        
        # Infinite progression trackers
        self.technological_mastery = 0.0
        self.philosophical_depth = 0.0
        self.aesthetic_sophistication = 0.0
        self.social_harmony = 0.0
        
        # Emergent properties that scale infinitely
        self.emergent_properties: Dict[str, float] = {}
        
    def evolve(self, current_turn: int, environmental_conditions: Dict[str, float]):
        """Continuous evolution without endpoints"""
        turns_existed = current_turn - self.turn_created
        
        # Base progress from existence
        existence_progress = math.log(1.0 + turns_existed) * 0.01
        
        # Environmental influences
        environmental_progress = sum(environmental_conditions.values()) * 0.02
        
        # Internal development factors
        progress_factors = {
            "temporal_accumulation": existence_progress,
            "environmental_adaptation": environmental_progress,
            "internal_innovation": self.innovation_potential * 0.01,
            "cultural_momentum": self.cultural_complexity * 0.001
        }
        
        # Advance civilization
        self.advance_civilization_index(progress_factors)
        
        # Scale all infinite metrics
        self._scale_infinite_metrics()
        
        # Generate emergent properties at higher levels
        self._generate_emergent_properties()
        
        # Cultural drift and innovation
        self._continuous_innovation()
    
    def _scale_infinite_metrics(self):
        """Scale all metrics that can grow without bound"""
        # Use logarithmic scaling to prevent explosion but allow infinite growth
        log_civ = math.log(1.0 + self.civilization_index)
        
        self.technological_mastery = log_civ * 3.0
        self.philosophical_depth = log_civ * 2.0
        self.aesthetic_sophistication = log_civ * 1.5
        self.social_harmony = log_civ * 1.2
        
        # Innovation potential grows with mastery
        self.innovation_potential = min(0.95, 0.2 + log_civ * 0.1)
    
    def _generate_emergent_properties(self):
        """Generate new emergent properties at higher civilization levels"""
        property_thresholds = [
            (10, "meta_cognition"),
            (25, "reality_modeling"), 
            (50, "dimensional_awareness"),
            (100, "temporal_manipulation"),
            (200, "consciousness_engineering"),
            (500, "universe_simulation"),
            (1000, "multiversal_perspective")
            # Can keep adding thresholds infinitely
        ]
        
        for threshold, property_name in property_thresholds:
            if (self.civilization_index >= threshold and 
                property_name not in self.emergent_properties):
                
                property_strength = (self.civilization_index - threshold) * 0.1
                self.emergent_properties[property_name] = property_strength
                
                # Create associated innovation vector
                self.innovation_vectors[f"{property_name}_development"] = 0.5
    
    def _continuous_innovation(self):
        """Continuous cultural innovation without repetition"""
        innovation_chance = 0.01 + (self.innovation_potential * 0.02)
        
        if random.random() < innovation_chance:
            # Choose innovation domain based on current vectors
            if self.innovation_vectors:
                domain = random.choice(list(self.innovation_vectors.keys()))
                strength = self.innovation_vectors[domain]
            else:
                domain = f"domain_{random.randint(1, 100)}"
                strength = random.uniform(0.3, 0.7)
            
            self.generate_cultural_innovation(domain, strength)
            
            # Innovation can create new domains
            if random.random() < 0.2:
                new_domain = f"emergent_domain_{random.randint(1000, 9999)}"
                self.innovation_vectors[new_domain] = random.uniform(0.2, 0.6)