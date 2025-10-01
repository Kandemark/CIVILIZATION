"""Logging utility"""

# Module implementation

# utils/logger.py
import logging
import sys
from typing import Optional
from datetime import datetime
import os
from pathlib import Path

class LogLevel:
    """Log level constants"""
    DEBUG = logging.DEBUG
    INFO = logging.INFO
    WARNING = logging.WARNING
    ERROR = logging.ERROR
    CRITICAL = logging.CRITICAL

class LogFormatter(logging.Formatter):
    """Custom log formatter with colors and detailed information"""
    
    # ANSI color codes
    COLORS = {
        'DEBUG': '\033[36m',      # Cyan
        'INFO': '\033[32m',       # Green
        'WARNING': '\033[33m',    # Yellow
        'ERROR': '\033[31m',      # Red
        'CRITICAL': '\033[35m',   # Magenta
        'RESET': '\033[0m'        # Reset
    }
    
    def __init__(self, use_colors: bool = True):
        fmt = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        super().__init__(fmt)
        self.use_colors = use_colors
    
    def format(self, record):
        """Format log record with optional colors"""
        if self.use_colors and record.levelname in self.COLORS:
            record.levelname = (f"{self.COLORS[record.levelname]}{record.levelname}"
                              f"{self.COLORS['RESET']}")
        return super().format(record)

class SimulationLogger:
    """Advanced logging system for simulations"""
    
    def __init__(self, name: str = "simulation", log_dir: str = "logs", 
                 level: int = LogLevel.INFO, use_file: bool = True,
                 use_console: bool = True, max_file_size: int = 10 * 1024 * 1024):
        self.name = name
        self.log_dir = Path(log_dir)
        self.log_dir.mkdir(exist_ok=True)
        
        self.logger = logging.getLogger(name)
        self.logger.setLevel(level)
        
        # Clear existing handlers
        self.logger.handlers.clear()
        
        # Create formatters
        file_formatter = LogFormatter(use_colors=False)
        console_formatter = LogFormatter(use_colors=True)
        
        # File handler
        if use_file:
            log_file = self.log_dir / f"{name}_{datetime.now().strftime('%Y%m%d')}.log"
            file_handler = logging.FileHandler(log_file, encoding='utf-8')
            file_handler.setFormatter(file_formatter)
            file_handler.setLevel(level)
            self.logger.addHandler(file_handler)
        
        # Console handler
        if use_console:
            console_handler = logging.StreamHandler(sys.stdout)
            console_handler.setFormatter(console_formatter)
            console_handler.setLevel(level)
            self.logger.addHandler(console_handler)
    
    def debug(self, message: str, **kwargs):
        """Log debug message"""
        self.logger.debug(self._format_message(message, **kwargs))
    
    def info(self, message: str, **kwargs):
        """Log info message"""
        self.logger.info(self._format_message(message, **kwargs))
    
    def warning(self, message: str, **kwargs):
        """Log warning message"""
        self.logger.warning(self._format_message(message, **kwargs))
    
    def error(self, message: str, **kwargs):
        """Log error message"""
        self.logger.error(self._format_message(message, **kwargs))
    
    def critical(self, message: str, **kwargs):
        """Log critical message"""
        self.logger.critical(self._format_message(message, **kwargs))
    
    def _format_message(self, message: str, **kwargs) -> str:
        """Format log message with additional context"""
        if kwargs:
            context = " ".join(f"{k}={v}" for k, v in kwargs.items())
            return f"{message} [{context}]"
        return message
    
    def set_level(self, level: int):
        """Set log level for all handlers"""
        self.logger.setLevel(level)
        for handler in self.logger.handlers:
            handler.setLevel(level)
    
    def add_context(self, **context):
        """Add context to all subsequent log messages"""
        # This would require a more advanced logging setup with filters
        pass
    
    @staticmethod
    def get_logger(name: str, **kwargs) -> 'SimulationLogger':
        """Get or create a logger instance"""
        return SimulationLogger(name, **kwargs)