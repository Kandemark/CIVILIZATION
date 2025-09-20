"""Efficient event handling"""

# Module implementation

# simulation_engine/event_dispatcher.py
from typing import Dict, List, Callable, Any, Set
from dataclasses import dataclass
from enum import Enum
import asyncio
import logging
from concurrent.futures import ThreadPoolExecutor

logger = logging.getLogger(__name__)

class EventPriority(Enum):
    CRITICAL = 0
    HIGH = 1
    NORMAL = 2
    LOW = 3

@dataclass
class Event:
    type: str
    data: Any
    priority: EventPriority = EventPriority.NORMAL
    timestamp: float = 0.0

class EventDispatcher:
    """
    Handles event distribution and processing across simulation systems.
    Supports synchronous and asynchronous event processing.
    """
    
    def __init__(self, max_workers: int = 4):
        self.handlers: Dict[str, List[Callable]] = {}
        self.priority_queues: Dict[EventPriority, List[Event]] = {
            priority: [] for priority in EventPriority
        }
        self.executor = ThreadPoolExecutor(max_workers=max_workers)
        self.event_history: List[Event] = []
        self.max_history_size = 10000
        
    def register_handler(self, event_type: str, handler: Callable) -> None:
        """Register a handler for a specific event type"""
        if event_type not in self.handlers:
            self.handlers[event_type] = []
        self.handlers[event_type].append(handler)
        logger.debug(f"Registered handler for event type: {event_type}")
        
    def unregister_handler(self, event_type: str, handler: Callable) -> None:
        """Unregister a handler for a specific event type"""
        if event_type in self.handlers:
            if handler in self.handlers[event_type]:
                self.handlers[event_type].remove(handler)
                
    def dispatch(self, event: Event) -> None:
        """Dispatch an event to all registered handlers"""
        self.event_history.append(event)
        if len(self.event_history) > self.max_history_size:
            self.event_history = self.event_history[-self.max_history_size:]
            
        if event.type in self.handlers:
            for handler in self.handlers[event.type]:
                try:
                    handler(event.data)
                except Exception as e:
                    logger.error(f"Error in event handler for {event.type}: {e}")
                    
    def dispatch_async(self, event: Event) -> None:
        """Dispatch an event asynchronously"""
        self.event_history.append(event)
        if event.type in self.handlers:
            for handler in self.handlers[event.type]:
                self.executor.submit(self._safe_execute_handler, handler, event.data)
                
    def _safe_execute_handler(self, handler: Callable, data: Any) -> None:
        """Safely execute a handler with error handling"""
        try:
            handler(data)
        except Exception as e:
            logger.error(f"Error in async event handler: {e}")
            
    def queue_event(self, event: Event) -> None:
        """Queue an event for processing based on priority"""
        self.priority_queues[event.priority].append(event)
        
    def process_queued_events(self) -> None:
        """Process all queued events in priority order"""
        for priority in EventPriority:
            while self.priority_queues[priority]:
                event = self.priority_queues[priority].pop(0)
                self.dispatch(event)
                
    def get_event_statistics(self) -> Dict[str, int]:
        """Get statistics about event processing"""
        stats = {
            'total_events': len(self.event_history),
            'events_by_type': {},
            'queued_events': sum(len(q) for q in self.priority_queues.values())
        }
        
        for event in self.event_history:
            stats['events_by_type'][event.type] = stats['events_by_type'].get(event.type, 0) + 1
            
        return stats