"""Internal political groups"""

# Module implementation

# politics/faction_system.py
from enum import Enum, auto
from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass, field
import numpy as np
from datetime import datetime
import logging

logger = logging.getLogger(__name__)

class Ideology(Enum):
    """Political ideologies that factions can adhere to"""
    CONSERVATIVE = auto()
    LIBERAL = auto()
    SOCIALIST = auto()
    LIBERTARIAN = auto()
    NATIONALIST = auto()
    RELIGIOUS = auto()
    MILITARIST = auto()
    ECOLOGIST = auto()
    POPULIST = auto()
    TECHNOCRATIC = auto()

class PoliticalAlignment(Enum):
    """Political alignment on various spectrums"""
    EXTREME_LEFT = -2
    LEFT = -1
    CENTER = 0
    RIGHT = 1
    EXTREME_RIGHT = 2

@dataclass
class FactionMember:
    """Represents a member of a political faction"""
    id: str
    name: str
    influence: float = 1.0
    loyalty: float = 0.8
    ambition: float = 0.5
    competence: float = 0.7
    traits: Set[str] = field(default_factory=set)
    position: str = "Member"

@dataclass
class Faction:
    """A political faction with members, ideology, and goals"""
    id: str
    name: str
    ideology: Ideology
    alignment: PoliticalAlignment
    influence: float = 10.0
    cohesion: float = 0.8
    popularity: float = 0.5
    treasury: float = 1000.0
    members: Dict[str, FactionMember] = field(default_factory=dict)
    goals: List[str] = field(default_factory=list)
    rivals: Set[str] = field(default_factory=set)
    allies: Set[str] = field(default_factory=set)
    created_date: datetime = field(default_factory=datetime.now)

    def add_member(self, member: FactionMember) -> bool:
        """Add a member to the faction"""
        if member.id in self.members:
            return False
        self.members[member.id] = member
        return True

    def remove_member(self, member_id: str) -> bool:
        """Remove a member from the faction"""
        if member_id not in self.members:
            return False
        del self.members[member_id]
        return True

    def calculate_power(self) -> float:
        """Calculate the faction's overall power"""
        member_power = sum(member.influence for member in self.members.values())
        return (self.influence * 0.4 + member_power * 0.3 + 
                self.treasury * 0.001 + self.popularity * 5.0)

class FactionSystem:
    """Manages all political factions and their interactions"""
    
    def __init__(self):
        self.factions: Dict[str, Faction] = {}
        self.ideology_relationships: Dict[Tuple[Ideology, Ideology], float] = {}
        self.political_events: List[Dict] = []
        self._initialize_ideology_relationships()

    def _initialize_ideology_relationships(self):
        """Initialize relationship modifiers between different ideologies"""
        # Base relationship matrix
        ideologies = list(Ideology)
        for i, ideo1 in enumerate(ideologies):
            for j, ideo2 in enumerate(ideologies):
                if ideo1 == ideo2:
                    self.ideology_relationships[(ideo1, ideo2)] = 1.0
                else:
                    # Simple relationship model - could be more complex
                    similarity = 1.0 - (abs(i - j) / len(ideologies))
                    self.ideology_relationships[(ideo1, ideo2)] = similarity * 0.5 + 0.5

    def create_faction(self, name: str, ideology: Ideology, alignment: PoliticalAlignment) -> Faction:
        """Create a new political faction"""
        faction_id = f"faction_{len(self.factions) + 1}"
        faction = Faction(
            id=faction_id,
            name=name,
            ideology=ideology,
            alignment=alignment
        )
        self.factions[faction_id] = faction
        logger.info(f"Created new faction: {name} ({ideology.name})")
        return faction

    def disband_faction(self, faction_id: str) -> bool:
        """Disband a political faction"""
        if faction_id not in self.factions:
            return False
        
        faction = self.factions[faction_id]
        logger.info(f"Disbanded faction: {faction.name}")
        del self.factions[faction_id]
        
        # Remove from other factions' relationships
        for other_faction in self.factions.values():
            other_faction.rivals.discard(faction_id)
            other_faction.allies.discard(faction_id)
        
        return True

    def update_faction_relationships(self):
        """Update relationships between factions based on ideologies and events"""
        for faction_id, faction in self.factions.items():
            for other_id, other_faction in self.factions.items():
                if faction_id == other_id:
                    continue
                
                # Base relationship from ideology compatibility
                ideology_mod = self.ideology_relationships[
                    (faction.ideology, other_faction.ideology)
                ]
                
                # Alignment similarity
                alignment_similarity = 1.0 - abs(faction.alignment.value - other_faction.alignment.value) / 4.0
                
                # Current relationship state
                is_rival = other_id in faction.rivals
                is_ally = other_id in faction.allies
                
                # Calculate new relationship score
                relationship_score = (ideology_mod * 0.6 + alignment_similarity * 0.4)
                
                # Update relationships based on score
                if relationship_score < 0.3 and not is_rival:
                    faction.rivals.add(other_id)
                    faction.allies.discard(other_id)
                    logger.debug(f"{faction.name} now rivals {other_faction.name}")
                elif relationship_score > 0.7 and not is_ally:
                    faction.allies.add(other_id)
                    faction.rivals.discard(other_id)
                    logger.debug(f"{faction.name} now allies with {other_faction.name}")

    def simulate_faction_growth(self, time_delta: float = 1.0):
        """Simulate faction development over time"""
        for faction in self.factions.values():
            # Influence growth based on popularity and cohesion
            influence_growth = (faction.popularity * 0.1 + faction.cohesion * 0.05) * time_delta
            faction.influence = min(100.0, faction.influence + influence_growth)
            
            # Treasury changes based on influence and popularity
            income = faction.influence * faction.popularity * 0.1 * time_delta
            expenses = len(faction.members) * 0.5 * time_delta
            faction.treasury += income - expenses
            
            # Cohesion changes based on size and external threats
            cohesion_change = (0.01 - len(faction.members) * 0.001) * time_delta
            if faction.rivals:
                cohesion_change += 0.02 * time_delta  # External threats increase cohesion
            faction.cohesion = np.clip(faction.cohesion + cohesion_change, 0.1, 1.0)

    def get_faction_power_distribution(self) -> Dict[str, float]:
        """Get the distribution of power among factions"""
        total_power = sum(faction.calculate_power() for faction in self.factions.values())
        if total_power == 0:
            return {}
        
        return {
            faction_id: faction.calculate_power() / total_power
            for faction_id, faction in self.factions.items()
        }

    def form_coalition(self, faction_ids: List[str]) -> Optional[str]:
        """Form a political coalition between factions"""
        if len(faction_ids) < 2:
            return None
        
        # Check if factions can form a coalition (not rivals)
        for i, fid1 in enumerate(faction_ids):
            for fid2 in faction_ids[i+1:]:
                if fid2 in self.factions[fid1].rivals:
                    return None
        
        # Create coalition faction
        coalition = self.create_faction(
            name="Coalition Government",
            ideology=Ideology.POPULIST,  # Temporary ideology
            alignment=PoliticalAlignment.CENTER
        )
        
        # Add members from coalition factions
        for faction_id in faction_ids:
            if faction_id in self.factions:
                source_faction = self.factions[faction_id]
                for member in source_faction.members.values():
                    coalition.add_member(member)
                # Mark original faction as part of coalition
                source_faction.allies.add(coalition.id)
        
        return coalition.id

    def trigger_internal_conflict(self, faction_id: str, severity: float = 0.5) -> bool:
        """Trigger internal conflict within a faction"""
        if faction_id not in self.factions:
            return False
        
        faction = self.factions[faction_id]
        
        # Members with high ambition and low loyalty are more likely to cause conflict
        conflicted_members = [
            member for member in faction.members.values()
            if member.ambition > 0.7 and member.loyalty < 0.6
        ]
        
        if not conflicted_members:
            return False
        
        # Severity affects how many members are involved
        num_involved = max(1, int(len(conflicted_members) * severity))
        involved_members = np.random.choice(conflicted_members, num_involved, replace=False)
        
        # Reduce cohesion and influence
        faction.cohesion = max(0.1, faction.cohesion - severity * 0.3)
        faction.influence = max(1.0, faction.influence - severity * 5.0)
        
        logger.info(f"Internal conflict in {faction.name}: {len(involved_members)} members involved")
        return True