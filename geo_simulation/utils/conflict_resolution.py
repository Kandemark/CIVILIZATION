"""Conflict resolution"""

# Module implementation

# utils/conflict_resolution.py
from enum import Enum
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass, field
import numpy as np
from datetime import datetime
import logging

logger = logging.getLogger(__name__)

class DiplomaticWeight(Enum):
    """Factors that influence diplomatic weight in conflicts"""
    MILITARY_STRENGTH = "military_strength"
    ECONOMIC_POWER = "economic_power"
    POLITICAL_INFLUENCE = "political_influence"
    STRATEGIC_POSITION = "strategic_position"
    ALLIANCE_NETWORK = "alliance_network"

@dataclass
class NegotiationParty:
    """A party involved in conflict resolution"""
    faction_id: str
    demands: List[str]
    concessions: List[str]
    diplomatic_weight: float
    resolve_willingness: float  # 0.0 to 1.0

@dataclass
class NegotiationRound:
    """A round of negotiation in conflict resolution"""
    round_number: int
    proposals: Dict[str, Dict[str, float]]  # faction_id -> {demand: concession_value}
    acceptance_status: Dict[str, bool]
    mediator_influence: float = 0.0

class ConflictResolver:
    """Advanced conflict resolution system for diplomatic negotiations"""
    
    def __init__(self):
        self.negotiation_history: List[Dict] = []
        self.mediator_effectiveness: float = 0.5
        self.base_resolution_chance: float = 0.3
    
    def initiate_negotiation(self, parties: List[NegotiationParty], conflict_intensity: float) -> NegotiationRound:
        """Initiate a new negotiation round"""
        round_number = len(self.negotiation_history) + 1
        
        # Generate initial proposals based on party positions
        proposals = {}
        for party in parties:
            proposals[party.faction_id] = self._generate_initial_proposal(party, conflict_intensity)
        
        round_data = NegotiationRound(
            round_number=round_number,
            proposals=proposals,
            acceptance_status={p.faction_id: False for p in parties}
        )
        
        logger.info(f"Negotiation round {round_number} initiated with {len(parties)} parties")
        return round_data
    
    def _generate_initial_proposal(self, party: NegotiationParty, conflict_intensity: float) -> Dict[str, float]:
        """Generate initial negotiation proposal"""
        proposal = {}
        
        # More aggressive demands for higher conflict intensity
        aggression_factor = 0.5 + (conflict_intensity * 0.5)
        
        for demand in party.demands:
            # Base value influenced by diplomatic weight and aggression
            base_value = party.diplomatic_weight * aggression_factor
            # Add some randomness
            value = np.clip(base_value + np.random.normal(0, 0.1), 0.1, 1.0)
            proposal[demand] = value
        
        return proposal
    
    def evaluate_proposal(self, round_data: NegotiationRound, parties: List[NegotiationParty]) -> Dict[str, bool]:
        """Evaluate if parties accept the current proposal"""
        acceptance = {}
        
        for party in parties:
            party_proposal = round_data.proposals.get(party.faction_id, {})
            
            # Calculate satisfaction score
            satisfaction = self._calculate_satisfaction(party, party_proposal)
            
            # Willingness to resolve affects acceptance threshold
            threshold = 0.6 - (party.resolve_willingness * 0.3)
            acceptance[party.faction_id] = satisfaction >= threshold
        
        return acceptance
    
    def _calculate_satisfaction(self, party: NegotiationParty, proposal: Dict[str, float]) -> float:
        """Calculate how satisfied a party is with a proposal"""
        if not proposal:
            return 0.0
        
        # Compare demands with concessions
        demand_satisfaction = 0.0
        for demand, value in proposal.items():
            if demand in party.demands:
                # Higher value for own demands is good
                demand_satisfaction += value
            elif demand in party.concessions:
                # Lower value for concessions is good
                demand_satisfaction += (1.0 - value)
        
        return demand_satisfaction / len(proposal)
    
    def mediate_negotiation(self, round_data: NegotiationRound, parties: List[NegotiationParty]) -> NegotiationRound:
        """Apply mediation to improve negotiation chances"""
        new_proposals = round_data.proposals.copy()
        
        for party in parties:
            current_proposal = new_proposals[party.faction_id]
            mediated_proposal = {}
            
            for demand, value in current_proposal.items():
                # Mediator tries to find middle ground
                if demand in party.demands:
                    # Slightly reduce aggressive demands
                    mediated_value = value * (1.0 - self.mediator_effectiveness * 0.2)
                elif demand in party.concessions:
                    # Slightly increase minimal concessions
                    mediated_value = value * (1.0 + self.mediator_effectiveness * 0.1)
                else:
                    mediated_value = value
                
                mediated_proposal[demand] = np.clip(mediated_value, 0.0, 1.0)
            
            new_proposals[party.faction_id] = mediated_proposal
        
        return NegotiationRound(
            round_number=round_data.round_number,
            proposals=new_proposals,
            acceptance_status=round_data.acceptance_status,
            mediator_influence=self.mediator_effectiveness
        )
    
    def calculate_resolution_probability(self, parties: List[NegotiationParty], conflict_intensity: float) -> float:
        """Calculate probability of successful conflict resolution"""
        base_prob = self.base_resolution_chance
        
        # Factors affecting resolution probability
        total_willingness = sum(p.resolve_willingness for p in parties) / len(parties)
        weight_balance = 1.0 - abs(max(p.diplomatic_weight for p in parties) - 
                                 min(p.diplomatic_weight for p in parties))
        
        # Higher conflict intensity reduces resolution chance
        intensity_penalty = conflict_intensity * 0.5
        
        resolution_prob = (base_prob + 
                         total_willingness * 0.3 + 
                         weight_balance * 0.2 - 
                         intensity_penalty)
        
        return np.clip(resolution_prob, 0.0, 1.0)

class NegotiationSystem:
    """Comprehensive negotiation system for conflict resolution"""
    
    def __init__(self):
        self.resolver = ConflictResolver()
        self.active_negotiations: Dict[str, List[NegotiationRound]] = {}
        self.successful_resolutions: List[Dict] = []
    
    def conduct_negotiation(self, conflict_id: str, parties: List[NegotiationParty], max_rounds: int = 5) -> Tuple[bool, Dict]:
        """Conduct a complete negotiation process"""
        conflict_intensity = 1.0 - (sum(p.resolve_willingness for p in parties) / len(parties))
        
        if conflict_id not in self.active_negotiations:
            self.active_negotiations[conflict_id] = []
        
        resolution_prob = self.resolver.calculate_resolution_probability(parties, conflict_intensity)
        
        for round_num in range(max_rounds):
            # Initiate or continue negotiation
            if round_num == 0:
                current_round = self.resolver.initiate_negotiation(parties, conflict_intensity)
            else:
                current_round = self.resolver.mediate_negotiation(current_round, parties)
            
            # Evaluate acceptance
            acceptance = self.resolver.evaluate_proposal(current_round, parties)
            current_round.acceptance_status = acceptance
            
            self.active_negotiations[conflict_id].append(current_round)
            
            # Check if all parties accept
            if all(acceptance.values()):
                resolution = {
                    'conflict_id': conflict_id,
                    'rounds': round_num + 1,
                    'final_proposal': current_round.proposals,
                    'timestamp': datetime.now()
                }
                self.successful_resolutions.append(resolution)
                logger.info(f"Conflict {conflict_id} resolved successfully in {round_num + 1} rounds")
                return True, resolution
        
        logger.info(f"Conflict {conflict_id} negotiation failed after {max_rounds} rounds")
        return False, {'conflict_id': conflict_id, 'rounds': max_rounds}