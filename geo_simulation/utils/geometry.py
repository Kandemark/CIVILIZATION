"""Geometry utilities"""

# Module implementation

# utils/geometry.py
from typing import List, Optional, Tuple
from dataclasses import dataclass
import math
import numpy as np

@dataclass
class Point:
    """2D point with x,y coordinates"""
    x: float
    y: float
    
    def distance_to(self, other: 'Point') -> float:
        """Calculate Euclidean distance to another point"""
        return math.sqrt((self.x - other.x) ** 2 + (self.y - other.y) ** 2)
    
    def __add__(self, other: 'Point') -> 'Point':
        return Point(self.x + other.x, self.y + other.y)
    
    def __sub__(self, other: 'Point') -> 'Point':
        return Point(self.x - other.x, self.y - other.y)
    
    def __mul__(self, scalar: float) -> 'Point':
        return Point(self.x * scalar, self.y * scalar)
    
    def __truediv__(self, scalar: float) -> 'Point':
        return Point(self.x / scalar, self.y / scalar)

@dataclass
class Line:
    """Line segment between two points"""
    start: Point
    end: Point
    
    def length(self) -> float:
        """Calculate line length"""
        return self.start.distance_to(self.end)
    
    def intersection(self, other: 'Line') -> Optional[Point]:
        """Find intersection point with another line"""
        # Implementation of line intersection algorithm
        x1, y1 = self.start.x, self.start.y
        x2, y2 = self.end.x, self.end.y
        x3, y3 = other.start.x, other.start.y
        x4, y4 = other.end.x, other.end.y
        
        denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
        if abs(denom) < 1e-10:
            return None  # Lines are parallel
        
        t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom
        u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom
        
        if 0 <= t <= 1 and 0 <= u <= 1:
            x = x1 + t * (x2 - x1)
            y = y1 + t * (y2 - y1)
            return Point(x, y)
        
        return None

@dataclass
class Polygon:
    """Polygon defined by a list of points"""
    points: List[Point]
    
    def area(self) -> float:
        """Calculate polygon area using shoelace formula"""
        if len(self.points) < 3:
            return 0.0
        
        area = 0.0
        n = len(self.points)
        
        for i in range(n):
            j = (i + 1) % n
            area += self.points[i].x * self.points[j].y
            area -= self.points[j].x * self.points[i].y
        
        return abs(area) / 2.0
    
    def perimeter(self) -> float:
        """Calculate polygon perimeter"""
        if len(self.points) < 2:
            return 0.0
        
        perimeter = 0.0
        n = len(self.points)
        
        for i in range(n):
            j = (i + 1) % n
            perimeter += self.points[i].distance_to(self.points[j])
        
        return perimeter
    
    def contains_point(self, point: Point) -> bool:
        """Check if polygon contains a point using ray casting"""
        if len(self.points) < 3:
            return False
        
        inside = False
        n = len(self.points)
        
        for i in range(n):
            j = (i + 1) % n
            if ((self.points[i].y > point.y) != (self.points[j].y > point.y) and
                (point.x < (self.points[j].x - self.points[i].x) * (point.y - self.points[i].y) / 
                 (self.points[j].y - self.points[i].y) + self.points[i].x)):
                inside = not inside
        
        return inside
    
    def intersects(self, other: 'Polygon') -> bool:
        """Check if this polygon intersects another polygon"""
        # Check if any edges intersect
        for i in range(len(self.points)):
            edge1 = Line(self.points[i], self.points[(i + 1) % len(self.points)])
            
            for j in range(len(other.points)):
                edge2 = Line(other.points[j], other.points[(j + 1) % len(other.points)])
                
                if edge1.intersection(edge2) is not None:
                    return True
        
        # Check if one polygon is completely inside the other
        if self.contains_point(other.points[0]) or other.contains_point(self.points[0]):
            return True
        
        return False
    
    def bounding_box(self) -> Optional['BoundingBox']:
        """Calculate bounding box of polygon"""
        if not self.points:
            return None
        
        min_x = min(p.x for p in self.points)
        max_x = max(p.x for p in self.points)
        min_y = min(p.y for p in self.points)
        max_y = max(p.y for p in self.points)
        
        return BoundingBox(Point(min_x, min_y), Point(max_x, max_y))

@dataclass
class Circle:
    """Circle defined by center and radius"""
    center: Point
    radius: float
    
    def area(self) -> float:
        """Calculate circle area"""
        return math.pi * self.radius ** 2
    
    def circumference(self) -> float:
        """Calculate circle circumference"""
        return 2 * math.pi * self.radius
    
    def contains_point(self, point: Point) -> bool:
        """Check if circle contains a point"""
        return self.center.distance_to(point) <= self.radius
    
    def intersects_circle(self, other: 'Circle') -> bool:
        """Check if circle intersects another circle"""
        distance = self.center.distance_to(other.center)
        return distance <= (self.radius + other.radius)

@dataclass
class BoundingBox:
    """Axis-aligned bounding box"""
    min_point: Point
    max_point: Point
    
    def width(self) -> float:
        return self.max_point.x - self.min_point.x
    
    def height(self) -> float:
        return self.max_point.y - self.min_point.y
    
    def area(self) -> float:
        return self.width() * self.height()
    
    def contains_point(self, point: Point) -> bool:
        return (self.min_point.x <= point.x <= self.max_point.x and
                self.min_point.y <= point.y <= self.max_point.y)
    
    def intersects(self, other: 'BoundingBox') -> bool:
        return not (self.max_point.x < other.min_point.x or
                   self.min_point.x > other.max_point.x or
                   self.max_point.y < other.min_point.y or
                   self.min_point.y > other.max_point.y)