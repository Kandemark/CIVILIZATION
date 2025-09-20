"""Handles different time scales"""

# Module implementation

"""
Time Management System for Civilization Simulation

This module handles the complex time scaling and synchronization needs of a 
civilization simulation game. It manages multiple time scales, seasonal effects,
lunar cycles, and coordinates time-based events across different game systems.

Key Features:
- Multiple time scales from paused to ultra-fast simulation
- Game calendar with seasons and lunar phases
- Seasonal modifiers affecting various game systems
- Event scheduling system for time-based callbacks
- Synchronization between real time and simulation time
- Serialization for saving/loading game state
"""

from enum import Enum, auto
from typing import Dict, List, Callable, Any, Optional
import time
from datetime import datetime, timedelta
import logging
from dataclasses import dataclass

# Set up logging
logger = logging.getLogger(__name__)


class TimeScale(Enum):
    """
    Enumeration of available time scales for simulation speed.
    
    Each scale represents a multiplier for how fast simulation time progresses
    compared to real time.
    """
    PAUSED = auto()      # Simulation is paused (0x speed)
    SLOW = auto()        # 1 day = 1 second real time (1x speed)
    NORMAL = auto()      # 1 day = 0.5 second real time (2x speed)
    FAST = auto()        # 1 day = 0.1 second real time (10x speed)
    VERY_FAST = auto()   # 1 day = 0.01 second real time (100x speed)
    ULTRA = auto()       # Maximum speed (custom multiplier)


class GameCalendar:
    """
    Represents the in-game calendar with seasons, moon phases, and date tracking.
    
    The calendar uses a simplified 30-day month system with 12 months in a year.
    Each season spans multiple months, affecting various game mechanics.
    """
    
    def __init__(self, start_year: int = 1, start_month: int = 1, start_day: int = 1):
        """
        Initialize the game calendar with a starting date.
        
        Args:
            start_year: The starting year of the simulation (default: 1)
            start_month: The starting month of the simulation (default: 1)
            start_day: The starting day of the simulation (default: 1)
        """
        self.year = start_year
        self.month = start_month
        self.day = start_day
        self.total_days = 0
        
        # Map months to seasons (simplified model)
        self.seasons = {
            1: "Winter", 2: "Winter", 3: "Spring", 4: "Spring", 5: "Spring",
            6: "Summer", 7: "Summer", 8: "Summer", 9: "Autumn", 10: "Autumn",
            11: "Autumn", 12: "Winter"
        }
    
    def advance_day(self):
        """Advance the calendar by one day and handle month/year transitions."""
        self.day += 1
        self.total_days += 1
        
        # Handle month/year rollover (using 30-day months for simplicity)
        if self.day > 30:
            self.day = 1
            self.month += 1
            if self.month > 12:
                self.month = 1
                self.year += 1
    
    def get_season(self) -> str:
        """
        Get the current season based on the month.
        
        Returns:
            String representing the current season (Winter, Spring, Summer, Autumn)
        """
        return self.seasons.get(self.month, "Unknown")
    
    def get_date_string(self) -> str:
        """
        Get a formatted string representation of the current date.
        
        Returns:
            String in the format "Year {year}, Month {month}, Day {day}"
        """
        return f"Year {self.year}, Month {self.month}, Day {self.day}"
    
    def to_dict(self) -> Dict[str, Any]:
        """
        Serialize the calendar state to a dictionary for saving.
        
        Returns:
            Dictionary containing all calendar state information
        """
        return {
            'year': self.year,
            'month': self.month,
            'day': self.day,
            'total_days': self.total_days
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'GameCalendar':
        """
        Reconstruct a calendar from serialized data.
        
        Args:
            data: Dictionary containing calendar state information
            
        Returns:
            A GameCalendar instance with the restored state
        """
        calendar = cls(data['year'], data['month'], data['day'])
        calendar.total_days = data['total_days']
        return calendar


@dataclass
class TimeEvent:
    """
    Data class representing a scheduled event in the simulation.
    
    Attributes:
        timestamp: Simulation time when the event should trigger
        callback: Function to call when the event triggers
        repeating: Whether the event repeats at intervals
        interval: Time between repeats for repeating events
        description: Human-readable description of the event
        event_id: Unique identifier for the event
    """
    timestamp: float
    callback: Callable
    repeating: bool
    interval: float
    description: str
    event_id: int


class SeasonalEffects:
    """
    Handles seasonal effects on various game systems.
    
    Different seasons affect gameplay mechanics like agriculture, construction,
    military movement, and energy consumption through multipliers.
    """
    
    def __init__(self):
        # Define how each season affects different game aspects
        self.seasonal_modifiers = {
            "Winter": {
                "agriculture": 0.5,          # Crops grow slower
                "construction": 0.7,         # Building takes longer
                "military_movement": 0.8,    # Troops move slower
                "energy_consumption": 1.3,   # More energy needed for heating
                "trade_efficiency": 0.9,     # Trade routes less efficient
            },
            "Spring": {
                "agriculture": 1.2,          # Crops grow faster
                "construction": 1.1,         # Building slightly faster
                "military_movement": 1.0,    # Normal movement
                "energy_consumption": 0.9,   # Less energy needed
                "trade_efficiency": 1.0,     # Normal trade efficiency
            },
            "Summer": {
                "agriculture": 1.5,          # Optimal growing conditions
                "construction": 1.0,         # Normal construction
                "military_movement": 0.9,    # Heat slows movement slightly
                "energy_consumption": 0.8,   # Minimal heating needed
                "trade_efficiency": 1.1,     # Good conditions for trade
            },
            "Autumn": {
                "agriculture": 1.0,          # Normal growth
                "construction": 1.0,         # Normal construction
                "military_movement": 1.0,    # Normal movement
                "energy_consumption": 1.0,   # Normal energy needs
                "trade_efficiency": 1.0,     # Normal trade efficiency
            }
        }
    
    def get_seasonal_modifier(self, season: str, aspect: str) -> float:
        """
        Get the modifier for a specific game aspect in the current season.
        
        Args:
            season: The current season (Winter, Spring, Summer, Autumn)
            aspect: The game aspect to get the modifier for
            
        Returns:
            Multiplier value for the specified aspect (defaults to 1.0 if not found)
        """
        return self.seasonal_modifiers.get(season, {}).get(aspect, 1.0)


class TimeManager:
    """
    Core time management system for the civilization simulation.
    
    Handles time scaling, calendar management, seasonal effects, lunar cycles,
    and coordinates time-based events across different game systems.
    """
    
    def __init__(self):
        """Initialize the time manager with default settings."""
        self.calendar = GameCalendar()
        self.time_scale = TimeScale.NORMAL
        self.time_multipliers = {
            TimeScale.PAUSED: 0,
            TimeScale.SLOW: 1.0,
            TimeScale.NORMAL: 2.0,
            TimeScale.FAST: 10.0,
            TimeScale.VERY_FAST: 100.0,
            TimeScale.ULTRA: 500.0
        }
        self.paused = False
        self.last_update_time = time.time()
        self.simulation_time = 0.0
        self.real_time_start = time.time()
        self.time_listeners = []  # For day/month/year events
        self.scheduled_events = []  # For custom timed events
        self.event_counter = 0
        self.time_handlers = []  # For frame-based time updates
        self.game_speed = 1.0  # Global speed modifier
        self.seasonal_effects = SeasonalEffects()
        self.moon_phase = 0  # 0 to 1 representing moon phase (0 = new moon, 0.5 = full moon)
        
        logger.info("TimeManager initialized with default settings")
    
    def update(self, delta_time: float = None) -> float:
        """
        Update the game time based on real time passed and process events.
        
        Args:
            delta_time: Optional fixed delta time for testing
            
        Returns:
            The amount of simulation time that has passed
        """
        if self.paused or self.time_scale == TimeScale.PAUSED:
            return 0.0
            
        # Calculate real time delta if not provided
        if delta_time is None:
            current_time = time.time()
            real_time_delta = current_time - self.last_update_time
            self.last_update_time = current_time
        else:
            real_time_delta = delta_time
        
        # Calculate game time delta based on time scale
        game_time_delta = real_time_delta * self.time_multipliers[self.time_scale] * self.game_speed
        
        # Process time-based events and calendar
        self._process_time_advancement(game_time_delta)
        
        # Process scheduled events
        self._process_events()
        
        # Notify time handlers
        for handler in self.time_handlers:
            try:
                handler(game_time_delta, self.simulation_time)
            except Exception as e:
                logger.error(f"Error in time handler: {e}")
        
        return game_time_delta
    
    def _process_time_advancement(self, game_time_delta: float):
        """
        Process the advancement of game time with seasonal and lunar effects.
        
        Args:
            game_time_delta: Amount of simulation time to advance
        """
        days_passed = game_time_delta
        self.simulation_time += game_time_delta
        
        # Update moon phase (approximately 29.5 day cycle)
        self.moon_phase = (self.moon_phase + days_passed / 29.5) % 1.0
        
        # Advance days and handle calendar events
        while days_passed >= 1.0:
            self.calendar.advance_day()
            self._notify_time_listeners('day', self.calendar)
            days_passed -= 1.0
            
            # Monthly and yearly events
            if self.calendar.day == 1:
                self._notify_time_listeners('month', self.calendar)
            if self.calendar.day == 1 and self.calendar.month == 1:
                self._notify_time_listeners('year', self.calendar)
    
    def _process_events(self) -> None:
        """Process scheduled events that are due to be triggered."""
        current_time = self.simulation_time
        events_to_remove = []
        
        for i, event in enumerate(self.scheduled_events):
            if current_time >= event.timestamp:
                try:
                    event.callback(current_time)
                    logger.debug(f"Executed time event: {event.description}")
                except Exception as e:
                    logger.error(f"Error executing time event '{event.description}': {e}")
                    
                if event.repeating:
                    # Reschedule repeating event
                    event.timestamp += event.interval
                else:
                    events_to_remove.append(i)
                    
        # Remove completed events (in reverse order to avoid index issues)
        for i in sorted(events_to_remove, reverse=True):
            self.scheduled_events.pop(i)
    
    def set_time_scale(self, scale: TimeScale):
        """
        Set the time scale for simulation.
        
        Args:
            scale: The TimeScale value to set
        """
        self.time_scale = scale
        logger.info(f"Time scale set to: {scale.name}")
        
    def adjust_game_speed(self, factor: float):
        """
        Adjust the overall game speed by a multiplicative factor.
        
        Args:
            factor: Multiplier to apply to game speed (clamped between 0.1 and 10.0)
        """
        self.game_speed = max(0.1, min(10.0, factor))
        logger.info(f"Game speed adjusted to: {self.game_speed}x")
    
    def add_time_listener(self, callback: Callable[[str, GameCalendar], None]):
        """
        Add a listener for time events (day, month, year changes).
        
        Args:
            callback: Function to call when time events occur
        """
        self.time_listeners.append(callback)
        logger.debug(f"Added time listener: {callback.__name__ if hasattr(callback, '__name__') else 'anonymous'}")
    
    def _notify_time_listeners(self, event_type: str, calendar: GameCalendar):
        """
        Notify all time listeners of an event.
        
        Args:
            event_type: Type of event ('day', 'month', or 'year')
            calendar: The current game calendar
        """
        for listener in self.time_listeners:
            try:
                listener(event_type, calendar)
            except Exception as e:
                logger.error(f"Error in time listener: {e}")
    
    def schedule_event(self, delay: float, callback: Callable, 
                      description: str = "") -> int:
        """
        Schedule a one-time event to occur after a delay.
        
        Args:
            delay: Time in simulation seconds until the event triggers
            callback: Function to call when the event triggers
            description: Human-readable description of the event
            
        Returns:
            Unique event ID that can be used to cancel the event
        """
        event = TimeEvent(
            timestamp=self.simulation_time + delay,
            callback=callback,
            repeating=False,
            interval=0.0,
            description=description,
            event_id=self.event_counter
        )
        self.scheduled_events.append(event)
        self.event_counter += 1
        logger.debug(f"Scheduled event: {description} (ID: {event.event_id})")
        return event.event_id
        
    def schedule_repeating_event(self, interval: float, callback: Callable,
                               description: str = "") -> int:
        """
        Schedule a repeating event that triggers at regular intervals.
        
        Args:
            interval: Time in simulation seconds between events
            callback: Function to call when the event triggers
            description: Human-readable description of the event
            
        Returns:
            Unique event ID that can be used to cancel the event
        """
        event = TimeEvent(
            timestamp=self.simulation_time + interval,
            callback=callback,
            repeating=True,
            interval=interval,
            description=description,
            event_id=self.event_counter
        )
        self.scheduled_events.append(event)
        self.event_counter += 1
        logger.debug(f"Scheduled repeating event: {description} (ID: {event.event_id})")
        return event.event_id
        
    def cancel_event(self, event_id: int) -> bool:
        """
        Cancel a scheduled event.
        
        Args:
            event_id: The ID of the event to cancel
            
        Returns:
            True if the event was found and cancelled, False otherwise
        """
        for i, event in enumerate(self.scheduled_events):
            if event.event_id == event_id:
                self.scheduled_events.pop(i)
                logger.debug(f"Cancelled event ID: {event_id}")
                return True
        logger.warning(f"Failed to cancel event ID: {event_id} (not found)")
        return False
        
    def pause(self) -> None:
        """Pause the simulation."""
        self.paused = True
        logger.info("Simulation paused")
        
    def resume(self) -> None:
        """Resume the simulation."""
        self.paused = False
        self.last_update_time = time.time()  # Reset to avoid large delta after pause
        logger.info("Simulation resumed")
        
    def toggle_pause(self) -> bool:
        """
        Toggle the pause state of the simulation.
        
        Returns:
            The new pause state (True for paused, False for running)
        """
        self.paused = not self.paused
        if not self.paused:
            self.last_update_time = time.time()  # Reset after unpausing
        logger.info(f"Simulation {'paused' if self.paused else 'resumed'}")
        return self.paused
        
    def register_time_handler(self, handler: Callable) -> None:
        """
        Register a handler to be called on each time update.
        
        Args:
            handler: Function that takes (delta_time, simulation_time) as arguments
        """
        self.time_handlers.append(handler)
        logger.debug(f"Registered time handler: {handler.__name__ if hasattr(handler, '__name__') else 'anonymous'}")
        
    def unregister_time_handler(self, handler: Callable) -> bool:
        """
        Unregister a time handler.
        
        Args:
            handler: The handler function to remove
            
        Returns:
            True if the handler was found and removed, False otherwise
        """
        if handler in self.time_handlers:
            self.time_handlers.remove(handler)
            logger.debug(f"Unregistered time handler: {handler.__name__ if hasattr(handler, '__name__') else 'anonymous'}")
            return True
        return False
        
    def get_seasonal_modifier(self, aspect: str) -> float:
        """
        Get current seasonal modifier for a game aspect.
        
        Args:
            aspect: The game aspect to get the modifier for
            
        Returns:
            Multiplier value for the specified aspect
        """
        season = self.calendar.get_season()
        return self.seasonal_effects.get_seasonal_modifier(season, aspect)
    
    def get_moon_phase(self) -> str:
        """
        Get the current moon phase as a descriptive string.
        
        Returns:
            String describing the current moon phase
        """
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
    
    def get_formatted_time(self) -> str:
        """
        Get a formatted string representation of the simulation time.
        
        Returns:
            String in the format "Day {days}, {HH:MM:SS}"
        """
        days = int(self.simulation_time // 86400)
        hours = int((self.simulation_time % 86400) // 3600)
        minutes = int((self.simulation_time % 3600) // 60)
        seconds = int(self.simulation_time % 60)
        
        return f"Day {days}, {hours:02d}:{minutes:02d}:{seconds:02d}"
        
    def get_real_time_ratio(self) -> float:
        """
        Get the ratio of simulation time to real time.
        
        Returns:
            Ratio of simulation seconds to real seconds
        """
        real_elapsed = time.time() - self.real_time_start
        if real_elapsed > 0:
            return self.simulation_time / real_elapsed
        return 0.0
    
    def to_dict(self) -> Dict[str, Any]:
        """
        Serialize the time manager state to a dictionary for saving.
        
        Returns:
            Dictionary containing all time manager state information
        """
        return {
            'calendar': self.calendar.to_dict(),
            'time_scale': self.time_scale.name,
            'game_speed': self.game_speed,
            'simulation_time': self.simulation_time,
            'paused': self.paused,
            'moon_phase': self.moon_phase
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'TimeManager':
        """
        Reconstruct a time manager from serialized data.
        
        Args:
            data: Dictionary containing time manager state information
            
        Returns:
            A TimeManager instance with the restored state
        """
        manager = cls()
        manager.calendar = GameCalendar.from_dict(data['calendar'])
        manager.time_scale = TimeScale[data['time_scale']]
        manager.game_speed = data['game_speed']
        manager.simulation_time = data['simulation_time']
        manager.paused = data['paused']
        manager.moon_phase = data.get('moon_phase', 0)
        
        logger.info("TimeManager state loaded from saved data")
        return manager