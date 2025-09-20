"""Handles different time scales"""

# Module implementation

"""
Time management system for the civilization simulation.
Handles different time scales and synchronization between systems.
"""

from enum import Enum, auto
from typing import Dict, List, Callable, Any
import time
from datetime import datetime, timedelta

class TimeScale(Enum):
    """Different time scales for simulation."""
    PAUSED = auto()
    SLOW = auto()      # 1 day = 1 second real time
    NORMAL = auto()    # 1 day = 0.5 second real time
    FAST = auto()      # 1 day = 0.1 second real time
    VERY_FAST = auto() # 1 day = 0.01 second real time

class GameCalendar:
    """Represents the in-game calendar with seasons, moon phases, etc."""
    
    def __init__(self, start_year: int = 1, start_month: int = 1, start_day: int = 1):
        self.year = start_year
        self.month = start_month
        self.day = start_day
        self.total_days = 0
        self.seasons = {
            1: "Winter", 2: "Winter", 3: "Spring", 4: "Spring", 5: "Spring",
            6: "Summer", 7: "Summer", 8: "Summer", 9: "Autumn", 10: "Autumn",
            11: "Autumn", 12: "Winter"
        }
    
    def advance_day(self):
        """Advance the calendar by one day."""
        self.day += 1
        self.total_days += 1
        
        # Handle month/year rollover
        if self.day > 30:  # 30-day months for simplicity
            self.day = 1
            self.month += 1
            if self.month > 12:
                self.month = 1
                self.year += 1
    
    def get_season(self) -> str:
        """Get the current season."""
        return self.seasons.get(self.month, "Unknown")
    
    def get_date_string(self) -> str:
        """Get formatted date string."""
        return f"Year {self.year}, Month {self.month}, Day {self.day}"
    
    def to_dict(self) -> Dict[str, Any]:
        """Serialize calendar to dictionary."""
        return {
            'year': self.year,
            'month': self.month,
            'day': self.day,
            'total_days': self.total_days
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'GameCalendar':
        """Create calendar from dictionary."""
        calendar = cls(data['year'], data['month'], data['day'])
        calendar.total_days = data['total_days']
        return calendar

class TimeManager:
    """Manages game time and coordinates time-based events."""
    
    def __init__(self):
        self.calendar = GameCalendar()
        self.time_scale = TimeScale.NORMAL
        self.last_update_time = time.time()
        self.time_multipliers = {
            TimeScale.PAUSED: 0,
            TimeScale.SLOW: 1.0,
            TimeScale.NORMAL: 2.0,
            TimeScale.FAST: 10.0,
            TimeScale.VERY_FAST: 100.0
        }
        self.time_listeners = []
        self.game_speed = 1.0
        self.seasonal_effects = SeasonalEffects()  # Add this line
        self.moon_phase = 0  # 0 to 1 representing moon phase
        
    def set_time_scale(self, scale: TimeScale):
        """Set the time scale for simulation."""
        self.time_scale = scale
        
    def update(self):
        """Update the game time based on real time passed."""
        if self.time_scale == TimeScale.PAUSED:
            return 0
            
        current_time = time.time()
        real_time_delta = current_time - self.last_update_time
        self.last_update_time = current_time
        
        # Calculate game time delta based on time scale
        game_time_delta = real_time_delta * self.time_multipliers[self.time_scale] * self.game_speed
        
        # Process time-based events
        self._process_time_advancement(game_time_delta)
        
        return game_time_delta
    
    def _process_time_advancement(self, game_time_delta: float):
        """Process the advancement of game time with seasonal effects."""
        days_passed = game_time_delta
        
        # Update moon phase (approximately 29.5 day cycle)
        self.moon_phase = (self.moon_phase + days_passed / 29.5) % 1.0
        
        # Advance days
        while days_passed >= 1.0:
            self.calendar.advance_day()
            self._notify_time_listeners('day', self.calendar)
            days_passed -= 1.0
            
            # Monthly and yearly events
            if self.calendar.day == 1:
                self._notify_time_listeners('month', self.calendar)
            if self.calendar.day == 1 and self.calendar.month == 1:
                self._notify_time_listeners('year', self.calendar)
    
    def add_time_listener(self, callback: Callable[[str, GameCalendar], None]):
        """Add a listener for time events."""
        self.time_listeners.append(callback)
    
    def _notify_time_listeners(self, event_type: str, calendar: GameCalendar):
        """Notify all time listeners of an event."""
        for listener in self.time_listeners:
            try:
                listener(event_type, calendar)
            except Exception as e:
                print(f"Error in time listener: {e}")
    
    def adjust_game_speed(self, factor: float):
        """Adjust the overall game speed by a factor."""
        self.game_speed = max(0.1, min(10.0, factor))
    
    def to_dict(self) -> Dict[str, Any]:
        """Serialize time manager to dictionary."""
        return {
            'calendar': self.calendar.to_dict(),
            'time_scale': self.time_scale.name,
            'game_speed': self.game_speed
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'TimeManager':
        """Create time manager from dictionary."""
        manager = cls()
        manager.calendar = GameCalendar.from_dict(data['calendar'])
        manager.time_scale = TimeScale[data['time_scale']]
        manager.game_speed = data['game_speed']
        return manager   
    
    def get_seasonal_modifier(self, aspect: str) -> float:
        """Get current seasonal modifier for an aspect."""
        season = self.calendar.get_season()
        return self.seasonal_effects.get_seasonal_modifier(season, aspect)
    
    def get_moon_phase(self) -> str:
        """Get current moon phase as string."""
        if self.moon_phase < 0.125:
            return "New Moon"
        elif self.moon_phase < 0.25:
            return "Waxing Crescent"
        elif self.moon_phase < 0.375:
            return "First Quarter"
        elif self.moon_phase < 0.5:
            return "Waxing Gibbous"
        elif self.moon_phase < 0.625:
            return "Full Moon"
        elif self.moon_phase < 0.75:
            return "Waning Gibbous"
        elif self.moon_phase < 0.875:
            return "Last Quarter"
        else:
            return "Waning Crescent"
    


class SeasonalEffects:
    """Handles seasonal effects on various game systems."""
    
    def __init__(self):
        self.seasonal_modifiers = {
            "Winter": {
                "agriculture": 0.5,
                "construction": 0.7,
                "military_movement": 0.8,
                "energy_consumption": 1.3
            },
            "Spring": {
                "agriculture": 1.2,
                "construction": 1.1,
                "military_movement": 1.0,
                "energy_consumption": 0.9
            },
            "Summer": {
                "agriculture": 1.5,
                "construction": 1.0,
                "military_movement": 0.9,
                "energy_consumption": 0.8
            },
            "Autumn": {
                "agriculture": 1.0,
                "construction": 1.0,
                "military_movement": 1.0,
                "energy_consumption": 1.0
            }
        }
    
    def get_seasonal_modifier(self, season: str, aspect: str) -> float:
        """Get modifier for a specific aspect in a season."""
        return self.seasonal_modifiers.get(season, {}).get(aspect, 1.0)