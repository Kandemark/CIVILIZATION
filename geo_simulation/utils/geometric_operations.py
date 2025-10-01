"""Geometric operations"""

# Module implementation

# utils/geometric_operations.py
import numpy as np
from typing import List, Optional, Tuple
from .geometry import Point, Line, Polygon, Circle, BoundingBox

class GeometryUtils:
    """Advanced geometric operations and calculations"""
    
    @staticmethod
    def calculate_centroid(points: List[Point]) -> Point:
        """Calculate centroid of a set of points"""
        if not points:
            return Point(0, 0)
        
        sum_x = sum(p.x for p in points)
        sum_y = sum(p.y for p in points)
        n = len(points)
        
        return Point(sum_x / n, sum_y / n)
    
    @staticmethod
    def convex_hull(points: List[Point]) -> List[Point]:
        """Calculate convex hull using Graham scan"""
        if len(points) <= 3:
            return points
        
        # Find the point with the lowest y-coordinate
        start = min(points, key=lambda p: (p.y, p.x))
        
        # Sort points by polar angle with start point
        def polar_angle(p: Point) -> float:
            return np.arctan2(p.y - start.y, p.x - start.x)
        
        sorted_points = sorted(points, key=polar_angle)
        
        # Build convex hull
        hull = [start, sorted_points[0]]
        
        for point in sorted_points[1:]:
            while len(hull) > 1 and GeometryUtils._cross(hull[-2], hull[-1], point) <= 0:
                hull.pop()
            hull.append(point)
        
        return hull
    
    @staticmethod
    def _cross(a: Point, b: Point, c: Point) -> float:
        """2D cross product of vectors AB and AC"""
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)
    
    @staticmethod
    def point_line_distance(point: Point, line: Line) -> float:
        """Calculate shortest distance from point to line"""
        # Vector from line start to point
        v1 = Point(point.x - line.start.x, point.y - line.start.y)
        # Vector from line start to line end
        v2 = Point(line.end.x - line.start.x, line.end.y - line.start.y)
        
        # Projection of v1 onto v2
        dot_product = v1.x * v2.x + v1.y * v2.y
        v2_length_squared = v2.x ** 2 + v2.y ** 2
        
        if v2_length_squared == 0:
            return point.distance_to(line.start)
        
        t = max(0, min(1, dot_product / v2_length_squared))
        
        # Closest point on line
        closest = Point(
            line.start.x + t * v2.x,
            line.start.y + t * v2.y
        )
        
        return point.distance_to(closest)

class PolygonOperations:
    """Advanced polygon operations"""
    
    @staticmethod
    def simplify_polygon(polygon: Polygon, tolerance: float) -> Polygon:
        """Simplify polygon using Douglas-Peucker algorithm"""
        if len(polygon.points) <= 2:
            return polygon
        
        def douglas_peucker(points: List[Point], epsilon: float) -> List[Point]:
            if len(points) <= 2:
                return points
            
            # Find the point with the maximum distance
            dmax = 0.0
            index = 0
            line = Line(points[0], points[-1])
            
            for i in range(1, len(points) - 1):
                d = GeometryUtils.point_line_distance(points[i], line)
                if d > dmax:
                    index = i
                    dmax = d
            
            # If max distance is greater than epsilon, recursively simplify
            if dmax > epsilon:
                rec_results1 = douglas_peucker(points[:index+1], epsilon)
                rec_results2 = douglas_peucker(points[index:], epsilon)
                return rec_results1[:-1] + rec_results2
            else:
                return [points[0], points[-1]]
        
        simplified_points = douglas_peucker(polygon.points, tolerance)
        return Polygon(simplified_points)
    
    @staticmethod
    def offset_polygon(polygon: Polygon, offset: float) -> Polygon:
        """Offset polygon by a given distance"""
        # Simplified implementation - in production use proper offset algorithms
        centroid = GeometryUtils.calculate_centroid(polygon.points)
        
        offset_points = []
        for point in polygon.points:
            # Calculate direction from centroid to point
            direction = Point(point.x - centroid.x, point.y - centroid.y)
            length = point.distance_to(centroid)
            
            if length > 0:
                # Normalize and scale by offset
                normalized = Point(direction.x / length, direction.y / length)
                offset_point = Point(
                    point.x + normalized.x * offset,
                    point.y + normalized.y * offset
                )
                offset_points.append(offset_point)
            else:
                offset_points.append(point)
        
        return Polygon(offset_points)

class DistanceCalculator:
    """Advanced distance calculations"""
    
    @staticmethod
    def haversine_distance(point1: Point, point2: Point, earth_radius: float = 6371.0) -> float:
        """Calculate great-circle distance between two points on Earth"""
        # Convert degrees to radians
        lat1 = np.radians(point1.y)
        lon1 = np.radians(point1.x)
        lat2 = np.radians(point2.y)
        lon2 = np.radians(point2.x)
        
        # Haversine formula
        dlat = lat2 - lat1
        dlon = lon2 - lon1
        
        a = np.sin(dlat/2)**2 + np.cos(lat1) * np.cos(lat2) * np.sin(dlon/2)**2
        c = 2 * np.arctan2(np.sqrt(a), np.sqrt(1-a))
        
        return earth_radius * c
    
    @staticmethod
    def manhattan_distance(point1: Point, point2: Point) -> float:
        """Calculate Manhattan distance between two points"""
        return abs(point1.x - point2.x) + abs(point1.y - point2.y)
    
    @staticmethod
    def path_length(points: List[Point]) -> float:
        """Calculate total length of a path through points"""
        if len(points) < 2:
            return 0.0
        
        total_length = 0.0
        for i in range(len(points) - 1):
            total_length += points[i].distance_to(points[i + 1])
        
        return total_length