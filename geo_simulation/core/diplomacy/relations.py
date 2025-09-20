"""Bilateral relations"""

# Module implementation

"""
Diplomacy system with relations, treaties, alliances, and espionage.
"""

from typing import Dict, List, Optional, Set
from dataclasses import dataclass
from enum import Enum
import random
from datetime import datetime, timedelta

class RelationLevel(Enum):
    """Levels of diplomatic relations."""
    WAR = -2
    HOSTILE = -1
    NEUTRAL = 0
    FRIENDLY = 1
    ALLIED = 2

class TreatyType(Enum):
    """Types of diplomatic treaties."""
    TRADE_AGREEMENT = "trade_agreement"
    NON_AGGRESSION = "non_aggression"
    DEFENSIVE_PACT = "defensive_pact"
    MILITARY_ALLIANCE = "military_alliance"
    RESEARCH_PARTNERSHIP = "research_partnership"

@dataclass
class DiplomaticRelation:
    """Represents diplomatic relations between two nations."""
    nation_a: str
    nation_b: str
    relation_level: RelationLevel
    trust: float  # 0.0 to 1.0
    last_updated: datetime
    historical_events: List[Dict]

@dataclass
class Treaty:
    """Represents a diplomatic treaty."""
    treaty_id: str
    treaty_type: TreatyType
    signatories: Set[str]
    terms: Dict
    start_date: datetime
    duration_days: int
    active: bool
    
    @property
    def expires_on(self) -> datetime:
        return self.start_date + timedelta(days=self.duration_days)
    
    def is_expired(self, current_date: datetime) -> bool:
        return current_date > self.expires_on

class DiplomacySystem:
    """Manages diplomatic relations between nations."""
    
    def __init__(self):
        self.relations: Dict[str, Dict[str, DiplomaticRelation]] = {}
        self.treaties: Dict[str, Treaty] = {}
        self.espionage_network: Dict[str, List[Dict]] = {}
        self.diplomatic_events: List[Dict] = []
    
    def initialize_relations(self, nation_ids: List[str]):
        """Initialize diplomatic relations between all nations."""
        for nation_a in nation_ids:
            self.relations[nation_a] = {}
            for nation_b in nation_ids:
                if nation_a != nation_b:
                    self.relations[nation_a][nation_b] = DiplomaticRelation(
                        nation_a=nation_a,
                        nation_b=nation_b,
                        relation_level=RelationLevel.NEUTRAL,
                        trust=0.5,
                        last_updated=datetime.now(),
                        historical_events=[]
                    )
    
    def update_relations(self, current_date: datetime, economic_data: Dict, military_data: Dict):
        """Update all diplomatic relations."""
        for nation_a, relations in self.relations.items():
            for nation_b, relation in relations.items():
                if nation_a != nation_b:
                    self._update_single_relation(relation, current_date, economic_data, military_data)
    
    def _update_single_relation(self, relation: DiplomaticRelation, current_date: datetime, 
                               economic_data: Dict, military_data: Dict):
        """Update a single diplomatic relation."""
        # Economic factors
        economic_similarity = 1.0 - abs(
            economic_data.get(relation.nation_a, {}).get('gdp_per_capita', 0.5) - 
            economic_data.get(relation.nation_b, {}).get('gdp_per_capita', 0.5)
        )
        
        # Military balance
        military_balance = (
            military_data.get(relation.nation_a, 0.0) / 
            max(1.0, military_data.get(relation.nation_b, 0.1))
        )
        military_tension = 1.0 - min(1.0, abs(1.0 - military_balance) * 2.0)
        
        # Cultural and historical factors (simplified)
        cultural_similarity = random.uniform(0.4, 0.8)
        
        # Calculate relation change
        relation_change = (
            economic_similarity * 0.4 +
            military_tension * 0.3 +
            cultural_similarity * 0.3 -
            0.5  # Center around neutral
        )
        
        # Apply change to trust
        relation.trust = max(0.0, min(1.0, relation.trust + relation_change * 0.1))
        
        # Update relation level based on trust
        if relation.trust < 0.3:
            new_level = RelationLevel.HOSTILE
        elif relation.trust < 0.4:
            new_level = RelationLevel.NEUTRAL
        elif relation.trust < 0.7:
            new_level = RelationLevel.FRIENDLY
        else:
            new_level = RelationLevel.ALLIED
        
        if relation.relation_level != new_level:
            self.diplomatic_events.append({
                'type': 'relation_change',
                'nations': (relation.nation_a, relation.nation_b),
                'old_level': relation.relation_level,
                'new_level': new_level,
                'date': current_date
            })
            relation.relation_level = new_level
        
        relation.last_updated = current_date
    
    def propose_treaty(self, proposer: str, recipient: str, treaty_type: TreatyType, 
                      terms: Dict, duration_days: int = 365) -> bool:
        """Propose a new treaty."""
        relation = self.relations[proposer][recipient]
        
        # Base acceptance chance based on relations
        base_chance = relation.trust
        
        # Modify based on treaty type
        type_modifiers = {
            TreatyType.TRADE_AGREEMENT: 0.2,
            TreatyType.NON_AGGRESSION: 0.0,
            TreatyType.DEFENSIVE_PACT: -0.2,
            TreatyType.MILITARY_ALLIANCE: -0.3,
            TreatyType.RESEARCH_PARTNERSHIP: 0.1
        }
        
        acceptance_chance = max(0.1, min(0.9, base_chance + type_modifiers[treaty_type]))
        
        if random.random() < acceptance_chance:
            treaty_id = f"treaty_{len(self.treaties)}_{random.randint(1000, 9999)}"
            new_treaty = Treaty(
                treaty_id=treaty_id,
                treaty_type=treaty_type,
                signatories={proposer, recipient},
                terms=terms,
                start_date=datetime.now(),
                duration_days=duration_days,
                active=True
            )
            
            self.treaties[treaty_id] = new_treaty
            
            self.diplomatic_events.append({
                'type': 'treaty_signed',
                'treaty_id': treaty_id,
                'treaty_type': treaty_type.value,
                'signatories': [proposer, recipient],
                'date': datetime.now()
            })
            
            return True
        
        return False
    
    def conduct_espionage(self, spy_nation: str, target_nation: str, mission_type: str) -> Dict:
        """Conduct an espionage mission."""
        success_chance = 0.6
        risk_level = 0.3
        
        if random.random() < success_chance:
            # Mission success
            intelligence = {
                'military_strength': random.uniform(0.7, 1.3),
                'economic_data': random.uniform(0.8, 1.2),
                'technology_level': random.uniform(0.9, 1.1)
            }
            
            self.espionage_network.setdefault(spy_nation, []).append({
                'target': target_nation,
                'mission_type': mission_type,
                'success': True,
                'intelligence': intelligence,
                'date': datetime.now()
            })
            
            return {'success': True, 'intelligence': intelligence}
        else:
            # Mission failure
            detection_chance = risk_level * (1.0 - success_chance)
            
            self.espionage_network.setdefault(spy_nation, []).append({
                'target': target_nation,
                'mission_type': mission_type,
                'success': False,
                'detected': random.random() < detection_chance,
                'date': datetime.now()
            })
            
            return {'success': False, 'detected': random.random() < detection_chance}
    
    def get_relations_summary(self, nation_id: str) -> Dict:
        """Get diplomatic relations summary for a nation."""
        if nation_id not in self.relations:
            return {}
        
        summary = {
            'allies': [],
            'friends': [],
            'neutrals': [],
            'hostiles': [],
            'enemies': []
        }
        
        for other_nation, relation in self.relations[nation_id].items():
            if relation.relation_level == RelationLevel.ALLIED:
                summary['allies'].append(other_nation)
            elif relation.relation_level == RelationLevel.FRIENDLY:
                summary['friends'].append(other_nation)
            elif relation.relation_level == RelationLevel.NEUTRAL:
                summary['neutrals'].append(other_nation)
            elif relation.relation_level == RelationLevel.HOSTILE:
                summary['hostiles'].append(other_nation)
            elif relation.relation_level == RelationLevel.WAR:
                summary['enemies'].append(other_nation)
        
        return summary