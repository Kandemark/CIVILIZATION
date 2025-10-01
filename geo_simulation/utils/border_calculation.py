"""Border calculation"""

# Module implementation

# utils/border_calculation.py
from enum import Enum
from typing import Dict, List, Tuple, Optional, Set, Any
from dataclasses import dataclass, field
import numpy as np
from .geometry import Point, Line, Polygon
import logging
import math
from collections import defaultdict

logger = logging.getLogger(__name__)


class BorderType(Enum):
    NATURAL = "natural"      # Rivers, mountains, coastlines
    POLITICAL = "political"  # Treaty-defined boundaries
    CULTURAL = "cultural"    # Ethnic/linguistic boundaries
    ECONOMIC = "economic"    # Trade zones, economic spheres
    MILITARY = "military"    # Occupation lines, demilitarized zones
    ADMINISTRATIVE = "administrative"  # Internal administrative boundaries
    HISTORICAL = "historical"  # Former boundaries with cultural significance
    DISPUTED = "disputed"    # Actively contested boundaries


class BorderStatus(Enum):
    STABLE = "stable"
    TENSE = "tense"
    DISPUTED = "disputed"
    MILITARIZED = "militarized"
    OPEN = "open"
    CLOSED = "closed"


@dataclass
class BorderSegment:
    """Represents a segment of a border with metadata."""
    points: List[Point]
    length: float
    border_type: BorderType
    status: BorderStatus = BorderStatus.STABLE
    fortification_level: float = 0.0  # 0.0 to 1.0
    crossing_points: List[Point] = field(default_factory=list)
    natural_barriers: List[str] = field(default_factory=list)
    last_incident: Optional[float] = None  # Timestamp
    
    def get_midpoint(self) -> Point:
        """Get the midpoint of this border segment."""
        if not self.points:
            return Point(0, 0)
        
        total_x = sum(p.x for p in self.points)
        total_y = sum(p.y for p in self.points)
        return Point(total_x / len(self.points), total_y / len(self.points))


@dataclass
class BorderConflict:
    """Represents a border dispute or conflict."""
    territory_a: str
    territory_b: str
    conflict_intensity: float  # 0.0 to 1.0
    disputed_area: float
    historical_claims: List[str]
    resolution_attempts: int = 0
    start_time: float = field(default_factory=lambda: 0.0)  # Simulation time
    economic_impact: float = 0.0
    military_engagements: int = 0
    diplomatic_penalties: Dict[str, float] = field(default_factory=dict)
    
    @property
    def is_active(self) -> bool:
        """Check if the conflict is still active."""
        return self.conflict_intensity > 0.1


@dataclass
class BorderAnalysis:
    """Comprehensive analysis of a border."""
    total_length: float
    segments: List[BorderSegment]
    average_fortification: float
    crossing_density: float  # Crossings per unit length
    natural_barrier_ratio: float
    stability_score: float
    tension_hotspots: List[Point]


class BorderCalculator:
    """Calculates and manages borders between territories."""
    
    def __init__(self):
        self.borders: Dict[Tuple[str, str], List[BorderSegment]] = {}
        self.border_types: Dict[Tuple[str, str], BorderType] = {}
        self.conflicts: Dict[Tuple[str, str], BorderConflict] = {}
        self.border_history: Dict[Tuple[str, str], List[Dict]] = defaultdict(list)
        
    def calculate_border(self, poly_a: Polygon, poly_b: Polygon, 
                        border_type: BorderType = BorderType.POLITICAL,
                        resolution: int = 100) -> List[BorderSegment]:
        """Calculate border between two polygons with high precision."""
        # Find all intersection points
        intersection_points = self._find_polygon_intersection(poly_a, poly_b)
        
        if not intersection_points:
            # No direct intersection, create border based on proximity
            return self._create_proximity_border(poly_a, poly_b, border_type)
        
        # Sort points to form continuous border
        sorted_points = self._sort_border_points(intersection_points, poly_a, poly_b)
        
        # Create border segments
        segments = self._create_border_segments(sorted_points, border_type)
        
        # Store border
        key = (poly_a.name, poly_b.name) if hasattr(poly_a, 'name') else ('unknown', 'unknown')
        self.borders[key] = segments
        self.border_types[key] = border_type
        
        # Record in history
        self.border_history[key].append({
            'timestamp': 0.0,  # Would be simulation time
            'segments': segments,
            'type': border_type,
            'length': sum(seg.length for seg in segments)
        })
        
        return segments
    
    def _find_polygon_intersection(self, poly_a: Polygon, poly_b: Polygon) -> List[Point]:
        """Find all intersection points between two polygons."""
        intersections = []
        processed_edges = set()
        
        for i in range(len(poly_a.points)):
            edge_a = Line(poly_a.points[i], poly_a.points[(i + 1) % len(poly_a.points)])
            edge_key = (min(edge_a.start, edge_a.end), max(edge_a.start, edge_a.end))
            
            if edge_key in processed_edges:
                continue
            processed_edges.add(edge_key)
            
            for j in range(len(poly_b.points)):
                edge_b = Line(poly_b.points[j], poly_b.points[(j + 1) % len(poly_b.points)])
                
                intersection = edge_a.intersection(edge_b)
                if intersection and intersection not in intersections:
                    intersections.append(intersection)
        
        return intersections
    
    def _sort_border_points(self, points: List[Point], poly_a: Polygon, poly_b: Polygon) -> List[Point]:
        """Sort border points to form a continuous line."""
        if len(points) <= 1:
            return points
        
        # Find the point closest to poly_a's centroid as starting point
        centroid_a = poly_a.centroid()
        start_point = min(points, key=lambda p: p.distance_to(centroid_a))
        
        sorted_points = [start_point]
        remaining_points = [p for p in points if p != start_point]
        
        while remaining_points:
            last_point = sorted_points[-1]
            next_point = min(remaining_points, key=lambda p: last_point.distance_to(p))
            sorted_points.append(next_point)
            remaining_points.remove(next_point)
        
        return sorted_points
    
    def _create_border_segments(self, points: List[Point], border_type: BorderType) -> List[BorderSegment]:
        """Create border segments from sorted points."""
        if len(points) < 2:
            return []
        
        segments = []
        for i in range(len(points) - 1):
            segment_points = [points[i], points[i + 1]]
            length = points[i].distance_to(points[i + 1])
            
            segment = BorderSegment(
                points=segment_points,
                length=length,
                border_type=border_type,
                status=BorderStatus.STABLE
            )
            segments.append(segment)
        
        return segments
    
    def _create_proximity_border(self, poly_a: Polygon, poly_b: Polygon, 
                               border_type: BorderType) -> List[BorderSegment]:
        """Create border for non-intersecting polygons based on proximity."""
        # Find closest edges between polygons
        min_distance = float('inf')
        closest_edge_pair = None
        
        for i in range(len(poly_a.points)):
            edge_a = Line(poly_a.points[i], poly_a.points[(i + 1) % len(poly_a.points)])
            
            for j in range(len(poly_b.points)):
                edge_b = Line(poly_b.points[j], poly_b.points[(j + 1) % len(poly_b.points)])
                
                distance = self._edge_distance(edge_a, edge_b)
                if distance < min_distance:
                    min_distance = distance
                    closest_edge_pair = (edge_a, edge_b)
        
        if closest_edge_pair:
            edge_a, edge_b = closest_edge_pair
            # Create border points at midpoints of closest edges
            midpoint_a = Point(
                (edge_a.start.x + edge_a.end.x) / 2,
                (edge_a.start.y + edge_a.end.y) / 2
            )
            midpoint_b = Point(
                (edge_b.start.x + edge_b.end.x) / 2,
                (edge_b.start.y + edge_b.end.y) / 2
            )
            
            segment = BorderSegment(
                points=[midpoint_a, midpoint_b],
                length=midpoint_a.distance_to(midpoint_b),
                border_type=border_type,
                status=BorderStatus.OPEN
            )
            return [segment]
        
        return []
    
    def _edge_distance(self, edge1: Line, edge2: Line) -> float:
        """Calculate minimum distance between two edges."""
        # Check distance between all point combinations
        distances = [
            edge1.start.distance_to(edge2.start),
            edge1.start.distance_to(edge2.end),
            edge1.end.distance_to(edge2.start),
            edge1.end.distance_to(edge2.end)
        ]
        
        # Check for edge intersections (would be 0 distance)
        if edge1.intersection(edge2):
            return 0.0
        
        return min(distances)
    
    def add_border_conflict(self, territory_a: str, territory_b: str, 
                          intensity: float, disputed_area: float,
                          historical_claims: List[str] = None) -> BorderConflict:
        """Add a border conflict between territories."""
        key = (territory_a, territory_b)
        reverse_key = (territory_b, territory_a)
        
        conflict = BorderConflict(
            territory_a=territory_a,
            territory_b=territory_b,
            conflict_intensity=max(0.0, min(1.0, intensity)),
            disputed_area=max(0.0, disputed_area),
            historical_claims=historical_claims or [],
            start_time=0.0  # Would be simulation time
        )
        
        self.conflicts[key] = conflict
        self.conflicts[reverse_key] = conflict  # Store both ways
        
        # Update border status to disputed
        if key in self.borders:
            for segment in self.borders[key]:
                segment.status = BorderStatus.DISPUTED
        
        logger.info(f"Border conflict added between {territory_a} and {territory_b} "
                   f"(intensity: {intensity:.2f})")
        
        return conflict
    
    def resolve_border_conflict(self, territory_a: str, territory_b: str, 
                              resolution: Dict[str, Any]) -> Dict[str, Any]:
        """Attempt to resolve a border conflict with detailed outcome."""
        key = (territory_a, territory_b)
        if key not in self.conflicts:
            return {'success': False, 'reason': 'No conflict found'}
        
        conflict = self.conflicts[key]
        conflict.resolution_attempts += 1
        
        # Calculate success probability based on multiple factors
        base_success = resolution.get('diplomatic_pressure', 0.5)
        intensity_penalty = conflict.conflict_intensity * 0.3
        attempts_penalty = conflict.resolution_attempts * 0.1
        economic_factor = resolution.get('economic_incentives', 0.0) * 0.2
        
        success_probability = max(0.1, base_success - intensity_penalty - attempts_penalty + economic_factor)
        success = np.random.random() < success_probability
        
        result = {
            'success': success,
            'probability': success_probability,
            'attempt_number': conflict.resolution_attempts,
            'remaining_intensity': conflict.conflict_intensity
        }
        
        if success:
            # Successful resolution
            del self.conflicts[key]
            reverse_key = (territory_b, territory_a)
            if reverse_key in self.conflicts:
                del self.conflicts[reverse_key]
            
            # Update border status
            if key in self.borders:
                for segment in self.borders[key]:
                    segment.status = BorderStatus.STABLE
            
            result['message'] = f"Border conflict resolved between {territory_a} and {territory_b}"
            logger.info(result['message'])
        else:
            # Failed resolution - conflict may intensify
            escalation_factor = resolution.get('escalation_risk', 0.2)
            conflict.conflict_intensity = min(1.0, conflict.conflict_intensity + escalation_factor)
            
            result['message'] = f"Border resolution failed between {territory_a} and {territory_b}"
            result['new_intensity'] = conflict.conflict_intensity
            logger.warning(result['message'])
        
        return result
    
    def calculate_border_stability(self, territory_a: str, territory_b: str) -> float:
        """Calculate comprehensive stability score for a border (0.0 to 1.0)."""
        key = (territory_a, territory_b)
        
        # Base stability from border type
        type_stability = {
            BorderType.NATURAL: 0.8,
            BorderType.POLITICAL: 0.6,
            BorderType.CULTURAL: 0.7,
            BorderType.ECONOMIC: 0.5,
            BorderType.MILITARY: 0.3,
            BorderType.ADMINISTRATIVE: 0.9,
            BorderType.HISTORICAL: 0.4,
            BorderType.DISPUTED: 0.1
        }.get(self.border_types.get(key, BorderType.POLITICAL), 0.5)
        
        # Adjust for conflict presence and intensity
        if key in self.conflicts:
            conflict = self.conflicts[key]
            conflict_penalty = conflict.conflict_intensity * 0.6
            type_stability *= (1.0 - conflict_penalty)
        
        # Adjust for border segment status
        if key in self.borders:
            segments = self.borders[key]
            status_penalties = {
                BorderStatus.STABLE: 0.0,
                BorderStatus.TENSE: 0.2,
                BorderStatus.DISPUTED: 0.5,
                BorderStatus.MILITARIZED: 0.3,
                BorderStatus.OPEN: -0.1,  # Slight bonus for open borders
                BorderStatus.CLOSED: 0.4
            }
            
            avg_status_penalty = sum(status_penalties[seg.status] for seg in segments) / len(segments)
            type_stability *= (1.0 - avg_status_penalty)
        
        return max(0.0, min(1.0, type_stability))
    
    def analyze_border(self, territory_a: str, territory_b: str) -> BorderAnalysis:
        """Perform comprehensive analysis of a border."""
        key = (territory_a, territory_b)
        
        if key not in self.borders:
            return BorderAnalysis(
                total_length=0.0,
                segments=[],
                average_fortification=0.0,
                crossing_density=0.0,
                natural_barrier_ratio=0.0,
                stability_score=0.0,
                tension_hotspots=[]
            )
        
        segments = self.borders[key]
        total_length = sum(seg.length for seg in segments)
        
        # Calculate various metrics
        avg_fortification = sum(seg.fortification_level for seg in segments) / len(segments)
        
        total_crossings = sum(len(seg.crossing_points) for seg in segments)
        crossing_density = total_crossings / total_length if total_length > 0 else 0.0
        
        segments_with_barriers = sum(1 for seg in segments if seg.natural_barriers)
        natural_barrier_ratio = segments_with_barriers / len(segments) if segments else 0.0
        
        stability_score = self.calculate_border_stability(territory_a, territory_b)
        
        # Identify tension hotspots (segments with recent incidents)
        tension_hotspots = []
        for seg in segments:
            if seg.last_incident and seg.fortification_level > 0.7:
                tension_hotspots.append(seg.get_midpoint())
        
        return BorderAnalysis(
            total_length=total_length,
            segments=segments,
            average_fortification=avg_fortification,
            crossing_density=crossing_density,
            natural_barrier_ratio=natural_barrier_ratio,
            stability_score=stability_score,
            tension_hotspots=tension_hotspots
        )
    
    def add_border_fortification(self, territory_a: str, territory_b: str,
                               segment_index: int, fortification_level: float) -> bool:
        """Add fortification to a border segment."""
        key = (territory_a, territory_b)
        
        if key not in self.borders or segment_index >= len(self.borders[key]):
            return False
        
        segment = self.borders[key][segment_index]
        segment.fortification_level = max(0.0, min(1.0, fortification_level))
        
        # Update status based on fortification level
        if fortification_level > 0.7:
            segment.status = BorderStatus.MILITARIZED
        elif fortification_level > 0.3:
            segment.status = BorderStatus.TENSE
        
        return True
    
    def find_border_crossings(self, territory_a: str, territory_b: str,
                            max_crossings: int = 5) -> List[Point]:
        """Find optimal locations for border crossings."""
        key = (territory_a, territory_b)
        
        if key not in self.borders:
            return []
        
        segments = self.borders[key]
        crossings = []
        
        # Simple strategy: midpoints of longest segments
        sorted_segments = sorted(segments, key=lambda s: s.length, reverse=True)
        
        for segment in sorted_segments[:max_crossings]:
            midpoint = segment.get_midpoint()
            crossings.append(midpoint)
            segment.crossing_points.append(midpoint)
        
        return crossings
    
    def get_shared_borders(self, territory: str) -> List[Tuple[str, List[BorderSegment]]]:
        """Get all borders shared by a territory."""
        shared = []
        
        for (terr_a, terr_b), segments in self.borders.items():
            if terr_a == territory:
                shared.append((terr_b, segments))
            elif terr_b == territory:
                shared.append((terr_a, segments))
        
        return shared
    
    def calculate_border_economic_impact(self, territory_a: str, territory_b: str,
                                      trade_volume: float) -> Dict[str, float]:
        """Calculate economic impact of border configuration."""
        stability = self.calculate_border_stability(territory_a, territory_b)
        analysis = self.analyze_border(territory_a, territory_b)
        
        # Base trade efficiency based on stability and crossing density
        base_efficiency = stability * (1.0 + analysis.crossing_density * 0.5)
        
        # Penalties for fortification and tension
        fortification_penalty = analysis.average_fortification * 0.3
        tension_penalty = len(analysis.tension_hotspots) * 0.1
        
        effective_efficiency = max(0.1, base_efficiency - fortification_penalty - tension_penalty)
        
        return {
            'trade_efficiency': effective_efficiency,
            'effective_trade_volume': trade_volume * effective_efficiency,
            'border_costs': trade_volume * (1 - effective_efficiency),
            'stability_bonus': stability * 0.1 * trade_volume
        }