"""Performance monitoring"""

# Module implementation

# utils/perf_monitor.py
import time
import psutil
import threading
from typing import Dict, List, Optional, Callable
from dataclasses import dataclass, field
import logging
from datetime import datetime
import numpy as np
import functools

logger = logging.getLogger(__name__)

@dataclass
class PerformanceMetrics:
    """Performance metrics container"""
    timestamp: datetime
    cpu_percent: float
    memory_usage: float
    execution_time: float
    call_count: int
    custom_metrics: Dict[str, float] = field(default_factory=dict)

class PerformanceMonitor:
    """System performance monitoring"""
    
    def __init__(self, update_interval: float = 1.0):
        self.update_interval = update_interval
        self.metrics_history: List[PerformanceMetrics] = []
        self.monitoring = False
        self.monitor_thread: Optional[threading.Thread] = None
        self.custom_metrics: Dict[str, float] = {}
    
    def start_monitoring(self):
        """Start performance monitoring"""
        if self.monitoring:
            return
        
        self.monitoring = True
        self.monitor_thread = threading.Thread(target=self._monitor_loop, daemon=True)
        self.monitor_thread.start()
        logger.info("Performance monitoring started")
    
    def stop_monitoring(self):
        """Stop performance monitoring"""
        self.monitoring = False
        if self.monitor_thread:
            self.monitor_thread.join(timeout=2.0)
        logger.info("Performance monitoring stopped")
    
    def _monitor_loop(self):
        """Main monitoring loop"""
        while self.monitoring:
            try:
                metrics = self._collect_metrics()
                self.metrics_history.append(metrics)
                
                # Keep only recent history
                if len(self.metrics_history) > 1000:
                    self.metrics_history = self.metrics_history[-1000:]
                
                time.sleep(self.update_interval)
            except Exception as e:
                logger.error(f"Performance monitoring error: {e}")
                break
    
    def _collect_metrics(self) -> PerformanceMetrics:
        """Collect current performance metrics"""
        return PerformanceMetrics(
            timestamp=datetime.now(),
            cpu_percent=psutil.cpu_percent(),
            memory_usage=psutil.virtual_memory().percent,
            execution_time=0.0,  # Would be set by specific operations
            call_count=0,  # Would be set by specific operations
            custom_metrics=self.custom_metrics.copy()
        )
    
    def add_custom_metric(self, name: str, value: float):
        """Add custom performance metric"""
        self.custom_metrics[name] = value
    
    def get_performance_report(self) -> Dict[str, float]:
        """Generate performance summary report"""
        if not self.metrics_history:
            return {}
        
        recent_metrics = self.metrics_history[-100:]  # Last 100 samples
        
        return {
            'avg_cpu': np.mean([m.cpu_percent for m in recent_metrics]),
            'max_cpu': max(m.cpu_percent for m in recent_metrics),
            'avg_memory': np.mean([m.memory_usage for m in recent_metrics]),
            'max_memory': max(m.memory_usage for m in recent_metrics),
            'sample_count': len(recent_metrics)
        }

class MemoryTracker:
    """Memory usage tracking"""
    
    def __init__(self):
        self.snapshots: List[Dict] = []
    
    def take_snapshot(self, label: str):
        """Take memory usage snapshot"""
        process = psutil.Process()
        memory_info = process.memory_info()
        
        snapshot = {
            'label': label,
            'timestamp': datetime.now(),
            'rss': memory_info.rss,  # Resident Set Size
            'vms': memory_info.vms,  # Virtual Memory Size
            'percent': process.memory_percent()
        }
        
        self.snapshots.append(snapshot)
        logger.debug(f"Memory snapshot '{label}': {snapshot['rss'] / 1024 / 1024:.2f} MB")
    
    def analyze_memory_growth(self) -> Dict[str, float]:
        """Analyze memory growth between snapshots"""
        if len(self.snapshots) < 2:
            return {}
        
        first = self.snapshots[0]
        last = self.snapshots[-1]
        
        rss_growth = last['rss'] - first['rss']
        vms_growth = last['vms'] - first['vms']
        
        return {
            'rss_growth_mb': rss_growth / 1024 / 1024,
            'vms_growth_mb': vms_growth / 1024 / 1024,
            'rss_growth_percent': (rss_growth / first['rss']) * 100 if first['rss'] > 0 else 0,
            'snapshot_count': len(self.snapshots)
        }

class ExecutionProfiler:
    """Function execution profiling"""
    
    def __init__(self):
        self.function_times: Dict[str, List[float]] = {}
        self.function_calls: Dict[str, int] = {}
    
    def profile_function(self, func: Callable) -> Callable:
        """Decorator to profile function execution"""
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            start_time = time.perf_counter()
            result = func(*args, **kwargs)
            end_time = time.perf_counter()
            
            execution_time = end_time - start_time
            func_name = func.__name__
            
            # Record execution time
            if func_name not in self.function_times:
                self.function_times[func_name] = []
                self.function_calls[func_name] = 0
            
            self.function_times[func_name].append(execution_time)
            self.function_calls[func_name] += 1
            
            return result
        
        return wrapper
    
    def get_function_stats(self) -> Dict[str, Dict[str, float]]:
        """Get statistics for all profiled functions"""
        stats = {}
        
        for func_name, times in self.function_times.items():
            if times:
                stats[func_name] = {
                    'calls': self.function_calls[func_name],
                    'total_time': sum(times),
                    'avg_time': np.mean(times),
                    'min_time': min(times),
                    'max_time': max(times),
                    'std_time': np.std(times)
                }
        
        return stats
    
    def reset_stats(self):
        """Reset profiling statistics"""
        self.function_times.clear()
        self.function_calls.clear()