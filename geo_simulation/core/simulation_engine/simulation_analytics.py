"""Simulation metrics and analysis"""

# Module implementation

# simulation_engine/simulation_analytics.py
from typing import Dict, List, Any, Optional
from dataclasses import dataclass
import logging
import numpy as np
import pandas as pd
from datetime import datetime

logger = logging.getLogger(__name__)

@dataclass
class AnalyticsDataPoint:
    timestamp: datetime
    metric_name: str
    value: float
    metadata: Dict[str, Any]

class SimulationAnalytics:
    """
    Collects, analyzes, and visualizes simulation data for insights.
    """
    
    def __init__(self):
        self.data_points: List[AnalyticsDataPoint] = []
        self.metrics_config: Dict[str, Dict[str, Any]] = {}
        self.analysis_cache: Dict[str, Any] = {}
        
    def track_metric(self, metric_name: str, value: float, metadata: Optional[Dict[str, Any]] = None) -> None:
        """Track a metric with optional metadata"""
        data_point = AnalyticsDataPoint(
            timestamp=datetime.now(),
            metric_name=metric_name,
            value=value,
            metadata=metadata or {}
        )
        self.data_points.append(data_point)
        logger.debug(f"Tracked metric: {metric_name} = {value}")
        
    def configure_metric(self, metric_name: str, config: Dict[str, Any]) -> None:
        """Configure how a metric should be tracked and analyzed"""
        self.metrics_config[metric_name] = config
        
    def get_metric_history(self, metric_name: str, window: Optional[int] = None) -> List[AnalyticsDataPoint]:
        """Get history of a specific metric"""
        history = [dp for dp in self.data_points if dp.metric_name == metric_name]
        if window is not None:
            history = history[-window:]
        return history
        
    def calculate_statistics(self, metric_name: str) -> Dict[str, float]:
        """Calculate statistics for a metric"""
        values = [dp.value for dp in self.get_metric_history(metric_name)]
        
        if not values:
            return {}
            
        return {
            'mean': np.mean(values),
            'median': np.median(values),
            'std_dev': np.std(values),
            'min': np.min(values),
            'max': np.max(values),
            'count': len(values)
        }
        
    def detect_anomalies(self, metric_name: str, threshold: float = 2.0) -> List[AnalyticsDataPoint]:
        """Detect anomalous values in a metric"""
        values = [dp.value for dp in self.get_metric_history(metric_name)]
        
        if len(values) < 2:
            return []
            
        mean = np.mean(values)
        std_dev = np.std(values)
        
        anomalies = []
        for dp in self.get_metric_history(metric_name):
            z_score = abs((dp.value - mean) / std_dev) if std_dev > 0 else 0
            if z_score > threshold:
                anomalies.append(dp)
                
        return anomalies
        
    def trend_analysis(self, metric_name: str, window: int = 10) -> Dict[str, float]:
        """Analyze trends in a metric"""
        history = self.get_metric_history(metric_name, window)
        if len(history) < 2:
            return {}
            
        values = [dp.value for dp in history]
        timestamps = [dp.timestamp.timestamp() for dp in history]
        
        # Simple linear regression for trend
        if len(set(timestamps)) > 1:  # Ensure we have different timestamps
            slope, intercept = np.polyfit(timestamps, values, 1)
            return {
                'slope': slope,
                'intercept': intercept,
                'trend': 'increasing' if slope > 0 else 'decreasing' if slope < 0 else 'stable'
            }
        return {}
        
    def correlation_analysis(self, metric1: str, metric2: str) -> Dict[str, float]:
        """Analyze correlation between two metrics"""
        values1 = [dp.value for dp in self.get_metric_history(metric1)]
        values2 = [dp.value for dp in self.get_metric_history(metric2)]
        
        min_length = min(len(values1), len(values2))
        if min_length < 2:
            return {}
            
        values1 = values1[-min_length:]
        values2 = values2[-min_length:]
        
        correlation = np.corrcoef(values1, values2)[0, 1]
        
        return {
            'correlation': correlation,
            'strength': 'strong' if abs(correlation) > 0.7 else 'moderate' if abs(correlation) > 0.3 else 'weak'
        }
        
    def export_to_dataframe(self) -> pd.DataFrame:
        """Export analytics data to pandas DataFrame"""
        data = []
        for dp in self.data_points:
            row = {
                'timestamp': dp.timestamp,
                'metric_name': dp.metric_name,
                'value': dp.value
            }
            row.update(dp.metadata)
            data.append(row)
            
        return pd.DataFrame(data)
        
    def generate_report(self, metrics: List[str]) -> Dict[str, Any]:
        """Generate a comprehensive analytics report"""
        report = {}
        
        for metric in metrics:
            report[metric] = {
                'statistics': self.calculate_statistics(metric),
                'trend': self.trend_analysis(metric),
                'anomalies': len(self.detect_anomalies(metric))
            }
            
        return report