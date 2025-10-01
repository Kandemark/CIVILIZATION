"""Useful decorators"""

# Module implementation

# utils/decorators.py
import time
import functools
import logging
from typing import Any, Callable, Dict, List, Optional, Type
import inspect
from datetime import datetime

logger = logging.getLogger(__name__)

def timer(func: Callable) -> Callable:
    """Decorator to measure function execution time"""
    @functools.wraps(func)
    def wrapper_timer(*args, **kwargs):
        start_time = time.perf_counter()
        result = func(*args, **kwargs)
        end_time = time.perf_counter()
        run_time = end_time - start_time
        
        logger.debug(f"Function {func.__name__} executed in {run_time:.4f} seconds")
        return result
    return wrapper_timer

def cache(max_size: int = 128, ttl: Optional[int] = None):
    """Decorator to cache function results with size and time limits"""
    def decorator_cache(func: Callable) -> Callable:
        cache_dict: Dict = {}
        cache_times: Dict = {}
        
        @functools.wraps(func)
        def wrapper_cache(*args, **kwargs):
            # Create cache key from arguments
            key = str(args) + str(sorted(kwargs.items()))
            
            # Check if cached result exists and is valid
            current_time = time.time()
            if key in cache_dict:
                if ttl is None or (current_time - cache_times[key]) < ttl:
                    return cache_dict[key]
            
            # Execute function and cache result
            result = func(*args, **kwargs)
            cache_dict[key] = result
            cache_times[key] = current_time
            
            # Enforce max cache size
            if len(cache_dict) > max_size:
                # Remove oldest entry
                oldest_key = min(cache_times, key=cache_times.get)
                del cache_dict[oldest_key]
                del cache_times[oldest_key]
            
            return result
        
        return wrapper_cache
    return decorator_cache

def retry(max_attempts: int = 3, delay: float = 1.0, 
          exceptions: Type[Exception] = Exception):
    """Decorator to retry function on exception"""
    def decorator_retry(func: Callable) -> Callable:
        @functools.wraps(func)
        def wrapper_retry(*args, **kwargs):
            for attempt in range(max_attempts):
                try:
                    return func(*args, **kwargs)
                except exceptions as e:
                    if attempt == max_attempts - 1:
                        logger.error(f"Function {func.__name__} failed after {max_attempts} attempts: {e}")
                        raise
                    else:
                        logger.warning(f"Attempt {attempt + 1} failed for {func.__name__}: {e}. Retrying in {delay}s...")
                        time.sleep(delay)
            return None
        return wrapper_retry
    return decorator_retry

def validate_input(validation_rules: Dict[str, Callable] = None):
    """Decorator to validate function inputs"""
    def decorator_validate(func: Callable) -> Callable:
        @functools.wraps(func)
        def wrapper_validate(*args, **kwargs):
            if validation_rules:
                # Get function signature
                sig = inspect.signature(func)
                bound_args = sig.bind(*args, **kwargs)
                bound_args.apply_defaults()
                
                # Validate each parameter
                for param_name, validator in validation_rules.items():
                    if param_name in bound_args.arguments:
                        value = bound_args.arguments[param_name]
                        if not validator(value):
                            raise ValueError(f"Validation failed for parameter '{param_name}': {value}")
            
            return func(*args, **kwargs)
        return wrapper_validate
    return decorator_validate

def log_execution(level: int = logging.INFO, log_args: bool = True):
    """Decorator to log function execution"""
    def decorator_log(func: Callable) -> Callable:
        @functools.wraps(func)
        def wrapper_log(*args, **kwargs):
            # Log function call
            arg_str = f" with args {args}, kwargs {kwargs}" if log_args else ""
            logger.log(level, f"Executing {func.__name__}{arg_str}")
            
            try:
                result = func(*args, **kwargs)
                logger.log(level, f"Function {func.__name__} completed successfully")
                return result
            except Exception as e:
                logger.error(f"Function {func.__name__} failed with error: {e}")
                raise
        
        return wrapper_log
    return decorator_log

def rate_limit(calls_per_second: float = 1.0):
    """Decorator to rate limit function calls"""
    last_called = 0.0
    
    def decorator_rate_limit(func: Callable) -> Callable:
        @functools.wraps(func)
        def wrapper_rate_limit(*args, **kwargs):
            nonlocal last_called
            elapsed = time.time() - last_called
            min_interval = 1.0 / calls_per_second
            
            if elapsed < min_interval:
                time.sleep(min_interval - elapsed)
            
            last_called = time.time()
            return func(*args, **kwargs)
        
        return wrapper_rate_limit
    return decorator_rate_limit

def memoize(func: Callable) -> Callable:
    """Simple memoization decorator"""
    cache = {}
    
    @functools.wraps(func)
    def wrapper_memoize(*args, **kwargs):
        key = str(args) + str(kwargs)
        if key not in cache:
            cache[key] = func(*args, **kwargs)
        return cache[key]
    
    return wrapper_memoize

def deprecated(replacement: Optional[str] = None):
    """Decorator to mark functions as deprecated"""
    def decorator_deprecated(func: Callable) -> Callable:
        @functools.wraps(func)
        def wrapper_deprecated(*args, **kwargs):
            message = f"Function {func.__name__} is deprecated"
            if replacement:
                message += f". Use {replacement} instead"
            logger.warning(message)
            return func(*args, **kwargs)
        return wrapper_deprecated
    return decorator_deprecated