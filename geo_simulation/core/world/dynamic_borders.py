"""Handles border changes and disputes"""

# Module implementation

"""
Dynamic Borders System for Territory Boundaries

Handles border dynamics, territorial disputes, and border conflicts.
"""

from enum import Enum, auto
from typing import Dict, List, Optional, Set, Tuple, Any
from dataclasses import dataclass, field
from datetime import datetime, timedelta
import logging
import random
from collections import defaultdict
import math

logger = logging.getLogger(__name__)


class BorderType(Enum):
    """Types of borders between territories."""
    NATURAL = auto()        # Natural boundaries (rivers, mountains)
    POLITICAL = auto()      # Politically agreed boundaries
    DISPUTED = auto()       # Actively disputed borders
    MILITARIZED = auto()    # Heavily fortified borders
    OPEN = auto()           # Open borders with free movement
    CLOSED = auto()         # Closed borders with restrictions


class BorderConflict(Enum):
    """Types of border conflicts."""
    TERRITORIAL_DISPUTE = auto()   # Claim over territory
    RESOURCE_DISPUTE = auto()      # Conflict over resources
    BORDER_VIOLATION = auto()      # Military incursion
    SMUGGLING = auto()             # Illegal trade/traffic
    REFUGEE_CRISIS = auto()        # Mass migration issues


@dataclass
class BorderSegment:
    """A segment of border between two territories."""
    id: str
    territory_a: str
    territory_b: str
    start_point: Tuple[float, float]
    end_point: Tuple[float, float]
    border_type: BorderType = BorderType.POLITICAL
    fortification_level: float = 0.0  # 0-1.0
    tension_level: float = 0.0  # 0-1.0
    crossing_points: List[Tuple[float, float]] = field(default_factory=list)
    natural_features: List[str] = field(default_factory=list)
    last_incident: Optional[datetime] = None
    incident_history: List[Dict[str, Any]] = field(default_factory=list)


@dataclass
class BorderConflict:
    """An ongoing border conflict."""
    id: str
    border_segment_id: str
    conflict_type: BorderConflict
    severity: float = 0.0  # 0-1.0
    involved_parties: Set[str] = field(default_factory=set)
    start_date: datetime = field(default_factory=datetime.now)
    resolution_progress: float = 0.0  # 0-100%
    proposed_solutions: List[Dict[str, Any]] = field(default_factory=list)
    casualties: int = 0
    economic_impact: float = 0.0


class DynamicBorders:
    """
    Manages dynamic borders, territorial disputes, and border conflicts.
    """
    
    def __init__(self):
        self.border_segments: Dict[str, BorderSegment] = {}
        self.active_conflicts: Dict[str, BorderConflict] = {}
        self.border_claims: Dict[str, Set[str]] = defaultdict(set)  # civ_id -> claimed_territories
        self.diplomatic_relations: Dict[Tuple[str, str], float] = {}  # (civ_a, civ_b) -> relation_score
        
        logger.info("Dynamic borders system initialized")
    
    def create_border_segment(self, territory_a: str, territory_b: str,
                            start_point: Tuple[float, float], end_point: Tuple[float, float],
                            border_type: BorderType = BorderType.POLITICAL) -> BorderSegment:
        """Create a new border segment between two territories."""
        border_id = f"border_{territory_a}_{territory_b}_{len(self.border_segments) + 1}"
        
        segment = BorderSegment(
            id=border_id,
            territory_a=territory_a,
            territory_b=territory_b,
            start_point=start_point,
            end_point=end_point,
            border_type=border_type
        )
        
        self.border_segments[border_id] = segment
        logger.info(f"Created border segment {border_id} between {territory_a} and {territory_b}")
        
        return segment
    
    def update_border_tensions(self, delta_time: float) -> None:
        """Update tension levels for all border segments."""
        for segment in self.border_segments.values():
            self._update_segment_tension(segment, delta_time)
            
            # Check for conflict escalation
            if segment.tension_level > 0.7 and random.random() < 0.01:
                self._escalate_to_conflict(segment)
    
    def _update_segment_tension(self, segment: BorderSegment, delta_time: float) -> None:
        """Update tension level for a border segment."""
        base_tension_change = 0.0
        
        # Get diplomatic relations between the two territories
        relation_key = (segment.territory_a, segment.territory_b)
        relation_score = self.diplomatic_relations.get(relation_key, 50.0)
        
        # Relations affect tension
        if relation_score < 30.0:
            base_tension_change += 0.1 * delta_time
        elif relation_score > 70.0:
            base_tension_change -= 0.05 * delta_time
        
        # Border type affects tension
        if segment.border_type == BorderType.DISPUTED:
            base_tension_change += 0.2 * delta_time
        elif segment.border_type == BorderType.NATURAL:
            base_tension_change -= 0.1 * delta_time
        
        # Recent incidents increase tension
        if segment.last_incident and (datetime.now() - segment.last_incident).days < 30:
            base_tension_change += 0.3 * delta_time
        
        segment.tension_level = max(0.0, min(1.0, segment.tension_level + base_tension_change))
    
    def _escalate_to_conflict(self, segment: BorderSegment) -> None:
        """Escalate border tension to an active conflict."""
        conflict_id = f"conflict_{len(self.active_conflicts) + 1}"
        
        # Determine conflict type based on situation
        conflict_type = random.choice(list(BorderConflict))
        
        conflict = BorderConflict(
            id=conflict_id,
            border_segment_id=segment.id,
            conflict_type=conflict_type,
            severity=segment.tension_level,
            involved_parties={segment.territory_a, segment.territory_b}
        )
        
        self.active_conflicts[conflict_id] = conflict
        logger.warning(f"Border conflict erupted on segment {segment.id}: {conflict_type.name}")
    
    def add_border_incident(self, border_id: str, incident_type: str, severity: float,
                          description: str, involved_parties: List[str]) -> None:
        """Add a border incident to a segment's history."""
        if border_id not in self.border_segments:
            return
        
        segment = self.border_segments[border_id]
        
        incident = {
            "type": incident_type,
            "severity": severity,
            "description": description,
            "involved_parties": involved_parties,
            "timestamp": datetime.now()
        }
        
        segment.incident_history.append(incident)
        segment.last_incident = datetime.now()
        segment.tension_level = min(1.0, segment.tension_level + severity * 0.3)
        
        logger.info(f"Border incident on {border_id}: {description}")
    
    def resolve_conflict(self, conflict_id: str, resolution_type: str, 
                       terms: Dict[str, Any]) -> bool:
        """Attempt to resolve a border conflict."""
        if conflict_id not in self.active_conflicts:
            return False
        
        conflict = self.active_conflicts[conflict_id]
        segment = self.border_segments.get(conflict.border_segment_id)
        
        if not segment:
            return False
        
        # Apply resolution effects
        if resolution_type == "military_victory":
            segment.tension_level = 0.8  # Victor imposes terms
            conflict.resolution_progress = 100.0
        elif resolution_type == "diplomatic":
            segment.tension_level *= 0.5  # Reduced tension
            conflict.resolution_progress = 100.0
        elif resolution_type == "compromise":
            segment.tension_level *= 0.7
            conflict.resolution_progress = 100.0
        
        # Update diplomatic relations
        for party in conflict.involved_parties:
            for other_party in conflict.involved_parties:
                if party != other_party:
                    relation_key = (party, other_party)
                    current_relation = self.diplomatic_relations.get(relation_key, 50.0)
                    
                    if resolution_type == "diplomatic":
                        new_relation = min(100.0, current_relation + 10.0)
                    else:
                        new_relation = max(0.0, current_relation - 5.0)
                    
                    self.diplomatic_relations[relation_key] = new_relation
        
        del self.active_conflicts[conflict_id]
        logger.info(f"Conflict {conflict_id} resolved via {resolution_type}")
        
        return True
    
    def claim_territory(self, civilization_id: str, territory_id: str) -> bool:
        """Claim a territory for a civilization."""
        # Check if territory is already claimed
        for civ, territories in self.border_claims.items():
            if territory_id in territories and civ != civilization_id:
                # Territory dispute!
                self._create_territorial_dispute(civilization_id, civ, territory_id)
                return False
        
        self.border_claims[civilization_id].add(territory_id)
        logger.info(f"Territory {territory_id} claimed by {civilization_id}")
        return True
    
    def _create_territorial_dispute(self, claimant_a: str, claimant_b: str, territory_id: str) -> None:
        """Create a territorial dispute between two claimants."""
        # Find or create border segment between the claimants
        border_id = None
        for seg_id, segment in self.border_segments.items():
            if {segment.territory_a, segment.territory_b} == {claimant_a, claimant_b}:
                border_id = seg_id
                break
        
        if not border_id:
            # Create a new border segment for the dispute
            segment = self.create_border_segment(claimant_a, claimant_b, (0, 0), (1, 1))
            border_id = segment.id
        
        # Mark as disputed
        segment = self.border_segments[border_id]
        segment.border_type = BorderType.DISPUTED
        segment.tension_level = 0.8
        
        logger.warning(f"Territorial dispute over {territory_id} between {claimant_a} and {claimant_b}")
    
    def get_border_status(self, territory_a: str, territory_b: str) -> Optional[Dict[str, Any]]:
        """Get status of border between two territories."""
        for segment in self.border_segments.values():
            if {segment.territory_a, segment.territory_b} == {territory_a, territory_b}:
                return {
                    "tension_level": segment.tension_level,
                    "border_type": segment.border_type.name,
                    "fortification_level": segment.fortification_level,
                    "last_incident": segment.last_incident,
                    "incident_count": len(segment.incident_history)
                }
        return None
    
    def build_fortification(self, border_id: str, level_increase: float = 0.1) -> bool:
        """Build fortifications on a border segment."""
        if border_id not in self.border_segments:
            return False
        
        segment = self.border_segments[border_id]
        segment.fortification_level = min(1.0, segment.fortification_level + level_increase)
        
        if segment.fortification_level > 0.5:
            segment.border_type = BorderType.MILITARIZED
        
        logger.info(f"Fortifications built on border {border_id}, level: {segment.fortification_level:.2f}")
        return True  

@dataclass
class BorderEvolution:
    """Tracks the historical evolution of borders."""
    id: str
    border_segment_id: str
    changes: List[Dict[str, Any]] = field(default_factory=list)
    current_shape: List[Tuple[float, float]] = field(default_factory=list)
    historical_shapes: List[Tuple[datetime, List[Tuple[float, float]]]] = field(default_factory=list)
    evolution_factors: Dict[str, float] = field(default_factory=dict)  # Factor -> influence
    
    def add_change(self, change_type: str, description: str, 
                  new_shape: List[Tuple[float, float]], factors: Dict[str, float]) -> None:
        """Add a border change to the evolution history."""
        change_record = {
            "timestamp": datetime.now(),
            "type": change_type,
            "description": description,
            "old_shape": self.current_shape.copy(),
            "new_shape": new_shape.copy(),
            "factors": factors.copy()
        }
        
        self.changes.append(change_record)
        self.historical_shapes.append((datetime.now(), new_shape.copy()))
        self.current_shape = new_shape.copy()
        
        # Update evolution factors
        for factor, influence in factors.items():
            self.evolution_factors[factor] = self.evolution_factors.get(factor, 0.0) + influence
    
    def get_shape_at_date(self, target_date: datetime) -> Optional[List[Tuple[float, float]]]:
        """Get the border shape at a specific historical date."""
        if not self.historical_shapes:
            return None
        
        # Find the most recent shape before the target date
        for i, (shape_date, shape) in enumerate(self.historical_shapes):
            if shape_date > target_date:
                if i == 0:
                    return None
                return self.historical_shapes[i-1][1]
        
        return self.current_shape
    
    def calculate_stability_score(self) -> float:
        """Calculate border stability based on evolution history."""
        if not self.changes:
            return 100.0  # Perfectly stable if no changes
        
        # Recent changes reduce stability more
        recent_changes = [c for c in self.changes 
                         if (datetime.now() - c["timestamp"]).days < 365]
        
        stability = 100.0
        stability -= len(recent_changes) * 10  # -10 per recent change
        stability -= len(self.changes) * 2     # -2 per historical change
        
        # Factor-based stability adjustments
        if "war" in self.evolution_factors:
            stability -= self.evolution_factors["war"] * 20
        if "diplomacy" in self.evolution_factors:
            stability += self.evolution_factors["diplomacy"] * 5
        
        return max(0.0, min(100.0, stability))
    
    def predict_future_shape(self, years_ahead: int = 10) -> List[Tuple[float, float]]:
        """Predict future border shape based on evolution patterns."""
        if len(self.historical_shapes) < 2:
            return self.current_shape
        
        # Simple linear prediction based on recent trends
        recent_shapes = self.historical_shapes[-5:]  # Last 5 changes
        if len(recent_shapes) < 2:
            return self.current_shape
        
        # Calculate average movement per year
        total_movement = 0.0
        movement_vectors = []
        
        for i in range(1, len(recent_shapes)):
            time_diff = (recent_shapes[i][0] - recent_shapes[i-1][0]).days / 365.0
            if time_diff == 0:
                continue
                
            shape_diff = self._calculate_shape_difference(recent_shapes[i-1][1], recent_shapes[i][1])
            annual_movement = shape_diff / time_diff
            total_movement += annual_movement
            movement_vectors.append(annual_movement)
        
        if not movement_vectors:
            return self.current_shape
        
        avg_movement = total_movement / len(movement_vectors)
        predicted_movement = avg_movement * years_ahead
        
        # Apply movement to current shape (simplified)
        predicted_shape = []
        for point in self.current_shape:
            # Simple expansion/contraction based on movement trend
            movement_direction = 1.0 if predicted_movement > 0 else -1.0
            movement_magnitude = min(abs(predicted_movement), 10.0)  # Cap movement
            
            new_point = (
                point[0] + movement_direction * movement_magnitude * random.uniform(0.8, 1.2),
                point[1] + movement_direction * movement_magnitude * random.uniform(0.8, 1.2)
            )
            predicted_shape.append(new_point)
        
        return predicted_shape
    
    def _calculate_shape_difference(self, shape1: List[Tuple[float, float]], 
                                  shape2: List[Tuple[float, float]]) -> float:
        """Calculate the difference between two shapes."""
        if len(shape1) != len(shape2):
            return 10.0  # Large difference if point counts differ
        
        total_distance = 0.0
        for p1, p2 in zip(shape1, shape2):
            distance = math.sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)
            total_distance += distance
        
        return total_distance / len(shape1) 