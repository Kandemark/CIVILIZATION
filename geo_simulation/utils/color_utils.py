"""Color utilities"""

# Module implementation

# utils/color_utils.py
from typing import Dict, List, Tuple, Optional, Union, Any
from dataclasses import dataclass, field
import numpy as np
import matplotlib.colors as mcolors
import colorsys
import logging
from enum import Enum, auto
import math
import random

logger = logging.getLogger(__name__)


class ColorSpace(Enum):
    """Different color spaces for color operations."""
    RGB = auto()
    HSV = auto()
    HSL = auto()
    LAB = auto()


class ColorBlendMode(Enum):
    """Different color blending modes."""
    NORMAL = auto()
    MULTIPLY = auto()
    SCREEN = auto()
    OVERLAY = auto()
    DARKEN = auto()
    LIGHTEN = auto()


@dataclass
class RGBColor:
    """RGB color representation with utility methods."""
    r: int  # 0-255
    g: int  # 0-255
    b: int  # 0-255
    a: float = 1.0  # 0.0-1.0
    
    def __post_init__(self):
        self.r = max(0, min(255, self.r))
        self.g = max(0, min(255, self.g))
        self.b = max(0, min(255, self.b))
        self.a = max(0.0, min(1.0, self.a))
    
    def to_hex(self) -> str:
        """Convert to hex color code."""
        if self.a < 1.0:
            return f"#{self.r:02x}{self.g:02x}{self.b:02x}{int(self.a * 255):02x}"
        return f"#{self.r:02x}{self.g:02x}{self.b:02x}"
    
    def to_rgb_float(self) -> Tuple[float, float, float]:
        """Convert to normalized RGB tuple (0.0-1.0)."""
        return (self.r / 255.0, self.g / 255.0, self.b / 255.0)
    
    def to_hsv(self) -> Tuple[float, float, float]:
        """Convert to HSV color space."""
        return colorsys.rgb_to_hsv(*self.to_rgb_float())
    
    def to_hsl(self) -> Tuple[float, float, float]:
        """Convert to HSL color space."""
        return colorsys.rgb_to_hsl(*self.to_rgb_float())
    
    def adjust_brightness(self, factor: float) -> 'RGBColor':
        """Adjust brightness by factor."""
        h, s, v = self.to_hsv()
        v = max(0.0, min(1.0, v * factor))
        r, g, b = colorsys.hsv_to_rgb(h, s, v)
        return RGBColor(int(r * 255), int(g * 255), int(b * 255), self.a)
    
    def adjust_saturation(self, factor: float) -> 'RGBColor':
        """Adjust saturation by factor."""
        h, s, v = self.to_hsv()
        s = max(0.0, min(1.0, s * factor))
        r, g, b = colorsys.hsv_to_rgb(h, s, v)
        return RGBColor(int(r * 255), int(g * 255), int(b * 255), self.a)
    
    def blend(self, other: 'RGBColor', mode: ColorBlendMode = ColorBlendMode.NORMAL, 
              alpha: float = 0.5) -> 'RGBColor':
        """Blend with another color."""
        r1, g1, b1 = self.to_rgb_float()
        r2, g2, b2 = other.to_rgb_float()
        
        if mode == ColorBlendMode.NORMAL:
            r = r1 * (1 - alpha) + r2 * alpha
            g = g1 * (1 - alpha) + g2 * alpha
            b = b1 * (1 - alpha) + b2 * alpha
        elif mode == ColorBlendMode.MULTIPLY:
            r = r1 * r2
            g = g1 * g2
            b = b1 * b2
        elif mode == ColorBlendMode.SCREEN:
            r = 1 - (1 - r1) * (1 - r2)
            g = 1 - (1 - g1) * (1 - g2)
            b = 1 - (1 - b1) * (1 - b2)
        elif mode == ColorBlendMode.OVERLAY:
            r = 2 * r1 * r2 if r1 < 0.5 else 1 - 2 * (1 - r1) * (1 - r2)
            g = 2 * g1 * g2 if g1 < 0.5 else 1 - 2 * (1 - g1) * (1 - g2)
            b = 2 * b1 * b2 if b1 < 0.5 else 1 - 2 * (1 - b1) * (1 - b2)
        elif mode == ColorBlendMode.DARKEN:
            r = min(r1, r2)
            g = min(g1, g2)
            b = min(b1, b2)
        elif mode == ColorBlendMode.LIGHTEN:
            r = max(r1, r2)
            g = max(g1, g2)
            b = max(b1, b2)
        else:
            r, g, b = r1, g1, b1
        
        return RGBColor(int(r * 255), int(g * 255), int(b * 255), self.a)
    
    @classmethod
    def from_hex(cls, hex_code: str) -> 'RGBColor':
        """Create RGBColor from hex code."""
        hex_code = hex_code.lstrip('#')
        
        if len(hex_code) == 6:
            r = int(hex_code[0:2], 16)
            g = int(hex_code[2:4], 16)
            b = int(hex_code[4:6], 16)
            return cls(r, g, b)
        elif len(hex_code) == 8:
            r = int(hex_code[0:2], 16)
            g = int(hex_code[2:4], 16)
            b = int(hex_code[4:6], 16)
            a = int(hex_code[6:8], 16) / 255.0
            return cls(r, g, b, a)
        else:
            raise ValueError(f"Invalid hex code: #{hex_code}")


@dataclass
class ColorGradient:
    """Creates color gradients for visualization."""
    name: str
    colors: List[Union[str, RGBColor]]
    positions: Optional[List[float]] = None
    color_space: ColorSpace = ColorSpace.RGB
    
    def __post_init__(self):
        # Convert hex strings to RGBColor objects
        processed_colors = []
        for color in self.colors:
            if isinstance(color, str):
                processed_colors.append(RGBColor.from_hex(color))
            else:
                processed_colors.append(color)
        self.colors = processed_colors
        
        if self.positions is None:
            self.positions = np.linspace(0, 1, len(self.colors)).tolist()
        elif len(self.positions) != len(self.colors):
            raise ValueError("Positions list must match colors list length")
    
    def get_color(self, value: float, value_range: Tuple[float, float] = (0.0, 1.0)) -> RGBColor:
        """Get color for a value within range."""
        min_val, max_val = value_range
        
        # Normalize value
        if max_val == min_val:
            normalized = 0.0
        else:
            normalized = (value - min_val) / (max_val - min_val)
        normalized = max(0.0, min(1.0, normalized))
        
        # Find segment
        for i in range(len(self.positions) - 1):
            if self.positions[i] <= normalized <= self.positions[i + 1]:
                # Interpolate between colors
                segment_pos = (normalized - self.positions[i]) / (self.positions[i + 1] - self.positions[i])
                return self._interpolate_colors(self.colors[i], self.colors[i + 1], segment_pos)
        
        return self.colors[-1]  # Fallback
    
    def _interpolate_colors(self, color1: RGBColor, color2: RGBColor, t: float) -> RGBColor:
        """Interpolate between two colors."""
        if self.color_space == ColorSpace.RGB:
            # RGB interpolation
            r = int(color1.r + (color2.r - color1.r) * t)
            g = int(color1.g + (color2.g - color1.g) * t)
            b = int(color1.b + (color2.b - color1.b) * t)
            a = color1.a + (color2.a - color1.a) * t
            return RGBColor(r, g, b, a)
        
        elif self.color_space == ColorSpace.HSV:
            # HSV interpolation (often gives better gradients)
            h1, s1, v1 = color1.to_hsv()
            h2, s2, v2 = color2.to_hsv()
            
            # Handle hue wrapping
            if abs(h2 - h1) > 0.5:
                if h2 > h1:
                    h1 += 1.0
                else:
                    h2 += 1.0
            
            h = (h1 + (h2 - h1) * t) % 1.0
            s = s1 + (s2 - s1) * t
            v = v1 + (v2 - v1) * t
            
            r, g, b = colorsys.hsv_to_rgb(h, s, v)
            a = color1.a + (color2.a - color1.a) * t
            return RGBColor(int(r * 255), int(g * 255), int(b * 255), a)
        
        else:
            # Default to RGB
            return self._interpolate_colors(color1, color2, t)
    
    def get_discrete_colors(self, count: int) -> List[RGBColor]:
        """Get discrete colors from gradient."""
        return [self.get_color(i / (count - 1)) for i in range(count)]
    
    def reverse(self) -> 'ColorGradient':
        """Create reversed gradient."""
        return ColorGradient(
            name=f"{self.name} (Reversed)",
            colors=list(reversed(self.colors)),
            positions=list(reversed([1 - p for p in self.positions])),
            color_space=self.color_space
        )


class PoliticalMapColors:
    """Standard colors for political maps with advanced features."""
    
    def __init__(self):
        self.faction_colors = {
            'player': '#1f77b4',      # Blue
            'enemy': '#d62728',       # Red
            'neutral': '#7f7f7f',     # Gray
            'ally': '#2ca02c',        # Green
            'rebel': '#ff7f0e',       # Orange
            'barbarian': '#8c564b',   # Brown
            'merchant': '#9467bd',    # Purple
            'religious': '#e377c2',   # Pink
            'imperial': '#bcbd22',    # Yellow-Green
            'tribal': '#17becf'       # Cyan
        }
        
        self.terrain_colors = {
            'plains': '#8fbc8f',
            'forest': '#228b22',
            'mountains': '#a9a9a9',
            'desert': '#f0e68c',
            'water': '#1e90ff',
            'urban': '#8b4513',
            'swamp': '#2f4f4f',
            'tundra': '#f5f5dc',
            'jungle': '#006400',
            'hills': '#daa520',
            'volcanic': '#8b0000',
            'glacier': '#e6f2ff'
        }
        
        self.border_colors = {
            'national': '#000000',
            'regional': '#666666',
            'disputed': '#ff0000',
            'natural': '#2e8b57',
            'water': '#0000ff'
        }
        
        # Create gradients for different map elements
        self.elevation_gradient = ColorGradient(
            name="Elevation",
            colors=['#003366', '#0066cc', '#66ccff', '#ffffcc', '#ccffcc', '#99cc66', '#663300', '#ffffff'],
            color_space=ColorSpace.HSV
        )
        
        self.temperature_gradient = ColorGradient(
            name="Temperature",
            colors=['#0000ff', '#00ffff', '#ffff00', '#ff0000'],  # Cold to Hot
            color_space=ColorSpace.HSV
        )
        
        self.wealth_gradient = ColorGradient(
            name="Wealth",
            colors=['#8b0000', '#ff0000', '#ffff00', '#00ff00', '#0000ff'],  # Poor to Rich
            color_space=ColorSpace.HSV
        )
    
    def get_faction_color(self, faction_id: str, default: str = '#cccccc') -> RGBColor:
        """Get color for a faction with fallback."""
        hex_color = self.faction_colors.get(faction_id, default)
        return RGBColor.from_hex(hex_color)
    
    def get_terrain_color(self, terrain_type: str, default: str = '#888888') -> RGBColor:
        """Get color for a terrain type with fallback."""
        hex_color = self.terrain_colors.get(terrain_type, default)
        return RGBColor.from_hex(hex_color)
    
    def get_border_color(self, border_type: str, default: str = '#000000') -> RGBColor:
        """Get color for a border type."""
        hex_color = self.border_colors.get(border_type, default)
        return RGBColor.from_hex(hex_color)
    
    def generate_unique_faction_color(self, faction_id: str, used_colors: List[RGBColor]) -> RGBColor:
        """Generate a unique color for a faction that isn't in used_colors."""
        base_color = self.get_faction_color(faction_id)
        
        # If base color is not used, return it
        if base_color not in used_colors:
            return base_color
        
        # Otherwise, generate variations
        for hue_shift in np.linspace(0, 1, 12):  # Try 12 different hues
            h, s, v = base_color.to_hsv()
            h = (h + hue_shift) % 1.0
            r, g, b = colorsys.hsv_to_rgb(h, s, v)
            candidate = RGBColor(int(r * 255), int(g * 255), int(b * 255))
            
            if candidate not in used_colors:
                return candidate
        
        # If all else fails, return a random color
        return RGBColor(
            random.randint(0, 255),
            random.randint(0, 255),
            random.randint(0, 255)
        )


class TerrainColors:
    """Advanced terrain coloring with elevation, moisture, and other factors."""
    
    def __init__(self):
        # Elevation gradient (sea floor to mountain peaks)
        self.elevation_gradient = ColorGradient(
            name="Elevation",
            colors=[
                '#000080',  # Deep ocean (-10000m)
                '#0000ff',  # Ocean (-5000m)
                '#1e90ff',  # Shallow water (-1000m)
                '#87ceeb',  # Coastal waters (-100m)
                '#ffffcc',  # Beach (0m)
                '#90ee90',  # Lowlands (100m)
                '#32cd32',  # Plains (500m)
                '#228b22',  # Hills (1000m)
                '#8b4513',  # Mountains (3000m)
                '#a9a9a9',  # High mountains (5000m)
                '#ffffff'   # Snow caps (7000m+)
            ],
            positions=[0.0, 0.1, 0.2, 0.3, 0.35, 0.4, 0.5, 0.6, 0.8, 0.9, 1.0],
            color_space=ColorSpace.HSV
        )
        
        # Moisture gradient for climate coloring
        self.moisture_gradient = ColorGradient(
            name="Moisture",
            colors=[
                '#8b0000',  # Arid
                '#ff8c00',  # Semi-arid
                '#ffd700',  # Dry
                '#adff2f',  # Moderate
                '#32cd32',  # Moist
                '#006400',  # Wet
                '#0000ff'   # Very wet
            ],
            color_space=ColorSpace.HSV
        )
        
        # Temperature gradient
        self.temperature_gradient = ColorGradient(
            name="Temperature",
            colors=[
                '#0000ff',  # Very cold
                '#87ceeb',  # Cold
                '#ffffff',  # Cool
                '#ffff00',  # Temperate
                '#ffa500',  # Warm
                '#ff4500',  # Hot
                '#8b0000'   # Very hot
            ],
            color_space=ColorSpace.HSV
        )
    
    def get_elevation_color(self, elevation: float, sea_level: float = 0.0) -> RGBColor:
        """Get color based on elevation relative to sea level."""
        # Normalize elevation to 0-1 range for the gradient
        min_elev = -10000  # Deepest ocean
        max_elev = 9000    # Highest mountains
        
        normalized_elev = (elevation - min_elev) / (max_elev - min_elev)
        return self.elevation_gradient.get_color(normalized_elev)
    
    def get_biome_color(self, elevation: float, temperature: float, moisture: float) -> RGBColor:
        """Get biome color based on elevation, temperature, and moisture."""
        # Base color from elevation
        base_color = self.get_elevation_color(elevation)
        
        # Adjust for temperature
        temp_color = self.temperature_gradient.get_color(temperature, (0.0, 1.0))
        base_color = base_color.blend(temp_color, ColorBlendMode.OVERLAY, 0.3)
        
        # Adjust for moisture
        moisture_color = self.moisture_gradient.get_color(moisture, (0.0, 1.0))
        base_color = base_color.blend(moisture_color, ColorBlendMode.MULTIPLY, 0.2)
        
        return base_color
    
    def get_climate_zone_color(self, latitude: float, elevation: float, 
                             continentality: float = 0.5) -> RGBColor:
        """Get color for climate zone based on latitude and other factors."""
        # Normalize latitude to 0-1 (south pole to north pole)
        normalized_lat = (latitude + 90) / 180
        
        # Base color from latitude
        if normalized_lat < 0.1:  # Polar
            base_color = RGBColor.from_hex('#ffffff')
        elif normalized_lat < 0.3:  # Subpolar
            base_color = RGBColor.from_hex('#d3d3d3')
        elif normalized_lat < 0.5:  # Temperate
            base_color = RGBColor.from_hex('#90ee90')
        elif normalized_lat < 0.7:  # Subtropical
            base_color = RGBColor.from_hex('#ffd700')
        else:  # Tropical
            base_color = RGBColor.from_hex('#ff4500')
        
        # Adjust for elevation
        if elevation > 2000:
            base_color = base_color.blend(RGBColor.from_hex('#a9a9a9'), ColorBlendMode.OVERLAY, 0.5)
        
        # Adjust for continentality (maritime vs continental)
        if continentality > 0.7:  # Very continental
            base_color = base_color.adjust_saturation(1.3)
        elif continentality < 0.3:  # Very maritime
            base_color = base_color.adjust_brightness(0.8)
        
        return base_color


class ColorAccessibility:
    """Tools for ensuring color accessibility and contrast."""
    
    @staticmethod
    def calculate_luminance(color: RGBColor) -> float:
        """Calculate relative luminance of a color (WCAG formula)."""
        r, g, b = color.to_rgb_float()
        
        # Convert to linear RGB
        r = r / 12.92 if r <= 0.03928 else ((r + 0.055) / 1.055) ** 2.4
        g = g / 12.92 if g <= 0.03928 else ((g + 0.055) / 1.055) ** 2.4
        b = b / 12.92 if b <= 0.03928 else ((b + 0.055) / 1.055) ** 2.4
        
        return 0.2126 * r + 0.7152 * g + 0.0722 * b
    
    @staticmethod
    def calculate_contrast_ratio(color1: RGBColor, color2: RGBColor) -> float:
        """Calculate contrast ratio between two colors (WCAG)."""
        l1 = ColorAccessibility.calculate_luminance(color1)
        l2 = ColorAccessibility.calculate_luminance(color2)
        
        lighter = max(l1, l2)
        darker = min(l1, l2)
        
        return (lighter + 0.05) / (darker + 0.05)
    
    @staticmethod
    def is_accessible_contrast(color1: RGBColor, color2: RGBColor, 
                             level: str = 'AA') -> bool:
        """Check if contrast meets accessibility standards."""
        ratio = ColorAccessibility.calculate_contrast_ratio(color1, color2)
        
        if level == 'AA':
            return ratio >= 4.5
        elif level == 'AAA':
            return ratio >= 7.0
        else:
            return ratio >= 3.0  # Minimum for large text
    
    @staticmethod
    def find_accessible_color(base_color: RGBColor, background_color: RGBColor,
                            level: str = 'AA') -> RGBColor:
        """Find an accessible color that contrasts well with background."""
        if ColorAccessibility.is_accessible_contrast(base_color, background_color, level):
            return base_color
        
        # Try adjusting brightness
        h, s, v = base_color.to_hsv()
        target_ratio = 4.5 if level == 'AA' else 7.0
        
        # Try lighter version
        for brightness in np.linspace(v, 1.0, 10):
            test_color = base_color.adjust_brightness(brightness / v)
            if ColorAccessibility.is_accessible_contrast(test_color, background_color, level):
                return test_color
        
        # Try darker version
        for brightness in np.linspace(v, 0.0, 10):
            test_color = base_color.adjust_brightness(brightness / v)
            if ColorAccessibility.is_accessible_contrast(test_color, background_color, level):
                return test_color
        
        # If nothing works, return extreme contrast
        bg_luminance = ColorAccessibility.calculate_luminance(background_color)
        if bg_luminance > 0.5:
            return RGBColor(0, 0, 0)  # Black
        else:
            return RGBColor(255, 255, 255)  # White


# Predefined color palettes for common use cases
PREDEFINED_GRADIENTS = {
    'viridis': ColorGradient(
        name="Viridis",
        colors=['#440154', '#482878', '#3e4989', '#31688e', '#26828e', 
                '#1f9e89', '#35b779', '#6ece58', '#b5de2b', '#fde725'],
        color_space=ColorSpace.HSV
    ),
    'plasma': ColorGradient(
        name="Plasma",
        colors=['#0d0887', '#46039f', '#7201a8', '#9c179e', '#bd3786',
                '#d8576b', '#ed7953', '#fb9f3a', '#fdca26', '#f0f921'],
        color_space=ColorSpace.HSV
    ),
    'terrain': ColorGradient(
        name="Terrain",
        colors=['#00a000', '#c0c000', '#c08000', '#c06000', '#c04000',
                '#c02000', '#c00000', '#a00000', '#800000', '#600000'],
        color_space=ColorSpace.HSV
    )
}


def hex_to_rgb(hex_code: str) -> RGBColor:
    """Convert hex color code to RGBColor."""
    return RGBColor.from_hex(hex_code)


def rgb_to_hex(r: int, g: int, b: int, a: float = 1.0) -> str:
    """Convert RGB values to hex color code."""
    return RGBColor(r, g, b, a).to_hex()


def create_categorical_palette(count: int, saturation: float = 0.8, 
                             value: float = 0.9) -> List[RGBColor]:
    """Create a categorical color palette with distinct colors."""
    colors = []
    for i in range(count):
        hue = i / count
        r, g, b = colorsys.hsv_to_rgb(hue, saturation, value)
        colors.append(RGBColor(int(r * 255), int(g * 255), int(b * 255)))
    return colors